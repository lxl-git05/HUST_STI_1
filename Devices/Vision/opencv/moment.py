import serial
import time
import cv2
import numpy as np


def ls(img):

    # 预处理:灰度化,二值化,取ROI
    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    ret, img_binary = cv2.threshold(img_gray, 60, 255, cv2.THRESH_BINARY_INV)
    height, width = img.shape[:2]
    roi_top = height // 2  # 从图像一半高度开始
    roi_bottom = height  # 到图像底部

    # 储存中心点
    center_points = []

    # 从下往上，每隔10个像素行进行一次扫描
    for y in range(roi_bottom - 1, roi_top, -10):
        # 求每一行中值为255的x坐标
        row = img_binary[y, :]
        white_pixels_x = np.where(row == 255)[0]
        # 如果找到了白色像素点
        if len(white_pixels_x) > 0:
            # 计算白色部分中心x坐标并写入center_points
            x_center = int(np.mean(white_pixels_x))
            center_points.append((x_center, y))

    # 至少要有两个点才能拟合直线
    if len(center_points) < 2:
        return 0.0  # 如果没有找到足够点，返回默认值

    # 使用np.polyfit对直线进行拟合,次数为1
    # 获得斜率slope和截距intercept
    x_coords, y_coords = zip(*center_points)
    params = np.polyfit(y_coords, x_coords, 1)
    slope = params[0]
    intercept = params[1]

    # 计算路径中心点的平均x坐标与中间位置的偏移
    avg_x = int(np.mean(x_coords)) - width // 2

    # 计算直线的倾斜角度
    angle_rad = np.arctan(slope)  # 得到弧度
    angle_deg = np.degrees(angle_rad)  # 转换为度

    # 绘制找到的中心点（绿色）与拟合得到的直线
    vis_image = cv2.cvtColor(img_binary, cv2.COLOR_GRAY2BGR)
    for point in center_points:
        cv2.circle(vis_image, point, 5, (0, 255, 0), -1)
    y1 = roi_bottom
    x1 = int(slope * y1 + intercept)
    y2 = roi_top
    x2 = int(slope * y2 + intercept)
    cv2.line(vis_image, (x1, y1), (x2, y2), (255, 0, 0), 3)

    # 在图像上显示计算出的角度和平均x坐标
    cv2.putText(vis_image, f'Angle: {angle_deg:.2f} degrees', (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
    cv2.putText(vis_image, f'Avg X: {avg_x}', (10, 70),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

    return angle_deg

def count_white_pixels_at_y(binary_img, y):
        try:
                # 获取指定行的所有像素
            row = binary_img[y, :]
                # 统计白色像素（值为255）
            white_count = np.sum(row == 255)
            return white_count
        except Exception as e:
            print(f"错误: {e}")
            return 0

def get_stop(binary_img, roi_height):
    state = 0
    # last_state = 0
    count = 0
    line_height = 0
    for i in range(roi_height):
        white_pixels = count_white_pixels_at_y(binary_img, i)
        state = 1 if white_pixels >= 80 else 0
        if state == 1:
            line_height += 1
        else:
            if line_height >= 5:
                count += 1
                line_height = 0
            else:
                line_height = 0
        #     line_height = 0
        # if (line_height >= 5 and state == 0):
        #     count += 1
        # count += 1 if last_state == 0 and state ==1 else 0
        # last_state = state
    if line_height >= 5:
        count += 1
    if count == 0:
        return 0
    elif count == 1:
        return 1
    else:
        return 2


def get_center_point(img):
    img_output = img.copy()
    min_area_threshold = 100  # 降低最小面积阈值（根据实际调整）

    # 1. 灰度转换与二值化（优化）
    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # 使用自适应阈值（适应光照变化）
    # threshold_value = 80
    # img_binary = cv2.adaptiveThreshold(
    #     img_gray, 255,cv2.ADAPTIVE_THRESH_MEAN_C,
    #     cv2.THRESH_BINARY_INV, 11, 2
    # )

    # 使用固定阈值（需要手动调整阈值）
    threshold_value = 80  # 阈值，范围0-255
    _, img_binary = cv2.threshold(img_gray, threshold_value, 255, cv2.THRESH_BINARY)

    # 2. 提取所有轮廓（包括内部）
    cnts = cv2.findContours(img_binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

    cx, cy = -1, -1
    is_junction = 0  # 0: 无岔路，1: 有岔路

    if len(cnts) > 0:
        #print(f"原始轮廓数量: {len(cnts)}")  # 调试信息

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
        #print(f"有效轮廓数量: {len(main_contours)}")  # 调试信息

        # 5. 判断是否为岔路（有效轮廓≥2）
        is_junction = 1 if len(main_contours) > 2 else 0
        #print(f"is_junction: {is_junction}")  # 调试信息

        # 6. 绘制所有轮廓（可视化）
        cv2.putText(img_output, f'contour: {len(cnts)}', (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
        for cnt in cnts:
            cv2.drawContours(img_output, cnt, -1, (0, 255, 0), 3)



    return cx, cy, img_binary, is_junction


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
