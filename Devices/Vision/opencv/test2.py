import cv2
import numpy as np
from moment import SerialPacket
from moment import count_white_pixels_at_y

def get_center_point(img, threshold_value=51):
    """
    输入：
        img: BGR图像
        threshold_value: 二值化阈值
    输出：
        cx, cy: 道路中心点坐标
        img_binary: 二值化图像
        img_output: 可视化图像（带中心点和中心线）
    """
    img_output = img.copy()

    # 1. 灰度化
    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # 2. 反二值化，黑色道路变白
    _, img_binary = cv2.threshold(img_gray, threshold_value, 255, cv2.THRESH_BINARY_INV)

    # 3. 查找轮廓
    cnts = cv2.findContours(img_binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

    cx, cy = -1, -1

    if len(cnts) > 0:
        # 找最大轮廓
        largest_cnt = max(cnts, key=cv2.contourArea)
        m = cv2.moments(largest_cnt)
        if m['m00'] > 0:
            cx = int(m['m10'] / m['m00'])
            cy = int(m['m01'] / m['m00'])
            # 绘制中心点
            cv2.circle(img_output, (cx, cy), 5, (0, 255, 0), -1)
            # 绘制纵向中心线
            cv2.line(img_output, (cx, 0), (cx, img_output.shape[0]), (255, 0, 0), 2)

    return cx, cy, img_binary, img_output

# 打开摄像头
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
# 串口传输包
# pack = SerialPacket(port="COM1", baudrate=115200, timeout=0.1)
pack = SerialPacket(port="/dev/ttyUSB0", baudrate=115200, timeout=0.1)

if not cap.isOpened():
    print("无法打开摄像头")
    exit()

# 创建窗口和滑块
cv2.namedWindow("ROI + Center")
def nothing(x):
    pass
cv2.createTrackbar("Threshold", "ROI + Center", 31, 255, nothing)

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        height, width = frame.shape[:2]
        # 截取ROI区域
        roi = frame[height//2 - 120: height//2 + 120, width//2 - 160: width//2 + 160].copy()

        # 获取滑块阈值
        threshold_value = cv2.getTrackbarPos("Threshold", "ROI + Center")

        # 计算中心点
        cx, cy, binary, output = get_center_point(roi, threshold_value)

        # 判断是否在停止标识
        white_pixels = count_white_pixels_at_y(roi, height // 2)
        is_stop = 1 if white_pixels >= 80 # 中间一行白色像素超过80后停止标志置1

        # 在原图上画ROI框
        # cv2.rectangle(frame, (width//2 - 160, height//2 - 120),
        #               (width//2 + 160, height//2 + 120), (0, 255, 0), 2)

        # 显示图像
        # cv2.imshow("Frame", frame)
        # cv2.imshow("ROI + Center", output)
        # cv2.imshow("Binary", binary)

        # 显示中心点X坐标
        # if cx != -1:
        #     print(f"中心点: x={cx}, y={cy}, 阈值={threshold_value}")
        
        # 发送数据包
        pack.insert_byte(0x06)
        pack.insert_two_bytes(pack.num_to_bytes(cx+100))
        pack.insert_two_bytes(pack.num_to_bytes(0))
        pack.insert_two_bytes(pack.num_to_bytes(0+100))
        pack.send_packet()

        if cv2.waitKey(1) & 0xFF == 27:  # ESC退出
            break

except KeyboardInterrupt:
    print("程序退出")

finally:
    cap.release()
    cv2.destroyAllWindows()
