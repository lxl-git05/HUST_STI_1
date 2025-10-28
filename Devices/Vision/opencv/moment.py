import serial
import time
import cv2


def get_center_point(img):
    img_output = img.copy()
    min_area_threshold = 400  # 降低最小面积阈值（根据实际调整）

    # 1. 灰度转换与二值化（优化）
    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # 使用自适应阈值（适应光照变化）
    img_binary = cv2.adaptiveThreshold(
        img_gray, 255, cv2.ADAPTIVE_THRESH_MEAN_C,
        cv2.THRESH_BINARY_INV, 11, 2
    )

    # 2. 提取所有轮廓（包括内部）
    cnts = cv2.findContours(img_binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

    cx, cy = -1, -1
    is_junction = 0  # 0: 无岔路，1: 有岔路

    if len(cnts) > 0:
        print(f"原始轮廓数量: {len(cnts)}")  # 调试信息

        # 3. 计算中心点（主路径）
        largest_cnt = max(cnts, key=cv2.contourArea)
        m = cv2.moments(largest_cnt)
        if m['m00'] > 0:
            cx = int(m['m10'] / m['m00'])
            cy = int(m['m01'] / m['m00'])
            cv2.circle(img_output, (cx, cy), 5, (0, 255, 0), -1)
            cv2.putText(img_output, f'Avg X: {cx}', (10, 70),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

        # 4. 筛选有效轮廓（面积>最小阈值）
        cnts_sorted = sorted(cnts, key=cv2.contourArea, reverse=True)
        main_contours = [cnt for cnt in cnts_sorted if cv2.contourArea(cnt) > min_area_threshold]
        print(f"有效轮廓数量: {len(main_contours)}")  # 调试信息

        # 5. 判断是否为岔路（有效轮廓≥2）
        is_junction = 1 if len(main_contours) >= 2 else 0
        print(f"is_junction: {is_junction}")  # 调试信息

        # 6. 绘制所有轮廓（可视化）
        cv2.putText(img_output, f'contour: {len(cnts)}', (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
        for cnt in cnts:
            cv2.drawContours(img_output, cnt, -1, (0, 255, 0), 3)



    return cx, cy, img_output, is_junction


class SerialPacket:
    def __init__(self, port="COM6", baudrate=115200, timeout=0.1):
        """初始化串口"""
        try:
            self.ser = serial.Serial(port, baudrate, timeout=timeout)
            if self.ser.is_open:
                print(f"串口 {port} 已成功打开！")
            else:
                print(f"串口 {port} 打开失败！")
        except serial.SerialException as e:
            print(f"串口 {port} 打开失败，错误信息：{e}")
        except Exception as e:
            print(f"发生未知错误：{e}")
        # 初始化包头和包尾
        self.header = bytearray([0xFF, 0xAA])
        self.tail = bytearray([0x55, 0xFE])
        self.data = bytearray()
        self.index = 0  # 数据插入位置

    def __clear_packet(self):
        """清空包数据，只保留包头"""
        self.data = bytearray()
        self.index = 0  # 数据插入位置清零

    def insert_byte(self, value):
        """在包头后插入单个字节"""
        self.data.insert(self.index, value)  # 插入到数据部分开头（包头之后）
        self.index += 1

    def insert_two_bytes(self, values):
        """在包头后插入两个字节"""
        self.data.insert(self.index, values[0])  # 插入到数据部分开头（包头之后）
        self.index += 1
        self.data.insert(self.index, values[1])  # 插入到数据部分开头（包头之后）
        self.index += 1

    def insert_bytes(self, index, values):
        """在指定位置插入多个字节"""
        for i, val in enumerate(values):
            self.data.insert(index + i, val)
            self.index += 1

    def num_to_bytes(self, value):
        """发送16位整数并拆分为高8位和低8位"""
        if not 0 <= value <= 0xFFFF:
            raise ValueError("输入值必须在 0~65535 之间")

        high_byte = (value >> 8) & 0xFF  # 高8位
        low_byte = value & 0xFF  # 低8位

        return [high_byte, low_byte]

    def __build_packet(self):
        """生成完整数据包"""
        return self.header + self.data + self.tail

    def send_packet(self):
        """发送完整数据包"""
        # 构建帧头 + 数据包 + 帧尾
        packet = self.__build_packet()
        # 发送数据包
        self.ser.write(packet)

        # # 十六进制美化输出
        # hex_str = ' '.join([f'{b:02X}' for b in packet])
        # print(f"发送: {hex_str}")

        # 清空数据包
        self.__clear_packet()



# if __name__ == '__main__':
#     # 读取图片
#     img = cv2.imread("test1.jpg")
#     # 取ROI
#     height, width = img.shape[:2]
#     roi_top = height // 2
#     roi = img[roi_top:, :]
#     # 计算中心点坐标
#     roi = cv2.imread("roi.jpg")
#     x, y, output = get_center_point(roi)
#     # 显示图片
#     cv2.imshow("img", output)
#     cv2.imshow("cut", img)
#     cv2.waitKey(0)
#     cv2.destroyAllWindows()
