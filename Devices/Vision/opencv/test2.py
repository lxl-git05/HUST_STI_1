import cv2
import numpy as np
from moment import SerialPacket
from moment import get_stop
from moment import count_red_green_pixels_rgb
from moment import recognize_text
from moment import get_stop_dynamic
import time

def get_center_point(img, min_area_threshold = 40, threshold_value=51):
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
    _, img_binary_0 = cv2.threshold(img_gray, threshold_value, 255, cv2.THRESH_BINARY)

    # 3. 查找轮廓
    cnts = cv2.findContours(img_binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]

    cx, cy = -1, -1

    is_junction = 0

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
        # 筛选有效轮廓
        cnts_sorted = sorted(cnts, key=cv2.contourArea, reverse=True)
        main_contours = [cnt for cnt in cnts_sorted if cv2.contourArea(cnt) > min_area_threshold]
        is_junction = 1 if len(main_contours) > 2 else 0

    return cx, cy, img_binary,img_gray, img_output, is_junction

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
cv2.createTrackbar("Threshold", "ROI + Center", 80, 255, nothing)

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            continue

        # height, width = frame.shape[:2]
        height, width = 480, 640
        # 截取ROI区域
        roi = frame[height//2 - 120: height//2 + 120, width // 2 - 160: width // 2 + 160].copy()

        # 获取滑块阈值
        threshold_value = cv2.getTrackbarPos("Threshold", "ROI + Center")
        # red_count, green_count = count_red_green_pixels_rgb(roi)
        # rgb_control = 0
        # if red_count>3000:
        #     rgb_control = 1
        # elif green_count>3000:
        #     rgb_control = 2

        # 计算中心点
        cx, cy, binary, img_gray, output, is_junction = get_center_point(roi, 40, threshold_value)
        # text = recognize_text(img_gray)
        # str_control = 0
        # if text == 'L':
        #     str_control = 1
        # elif text == 'R':
        #     str_control = 2
        # print(f"红绿灯判断：{rgb_control}")
        # print(f"字符识别:{str_control}")

        # 判断是否在停止标识
        roi_height, roi_width = roi.shape[:2]
        # white_pixels = count_white_pixels_at_y(binary, roi_height // 2)
        # 中间一行白色像素超过80后停止标志置1
        # is_stop = 1 if white_pixels >= 80 and is_junction == 0 else 0
        # 判断停止标识和等停标识
        # is_stop = get_stop(binary, roi_height)
        is_stop = get_stop_dynamic(binary, roi_height)

        # 在原图上画ROI框
        # cv2.rectangle(frame, (width//2 - 160, height//2 - 120),
        #               (width//2 + 160, height//2 + 120), (0, 255, 0), 2)

        # 显示图像+
        print(is_stop)



        # cv2.imshow("Frame", frame)
        # cv2.imshow("ROI + Center", output)
        cv2.imshow("Binary", binary)
        # cv2.imshow("Binary_0", img_gray)

        # 显示中心点X坐标
        # if cx != -1:
        #     print(f"中心点: x={cx}, y={cy}, 阈值={threshold_value}")
        
        # 发送数据包
        pack.insert_byte(0x0A)
        pack.insert_two_bytes(pack.num_to_bytes(cx+100))
        pack.insert_two_bytes(pack.num_to_bytes(is_stop))
        pack.insert_two_bytes(pack.num_to_bytes(0+100))
        # pack.insert_two_bytes(pack.num_to_bytes(rgb_control))
        # pack.insert_two_bytes(pack.num_to_bytes(str_control))
        # pack.send_packet()
        # time.sleep(1)
        if cv2.waitKey(1) & 0xFF == 27:  # ESC退出
            break

except KeyboardInterrupt:
    print("程序退出")

finally:
    cap.release()
    cv2.destroyAllWindows()


#新增串口发送数据（红绿灯与指示牌）协议说明：
#共发送10位数据（即5个实际数据），前6位数据与之前保持不变

#第7,8位数据rgb_control发送的是红绿灯识别结果：
#当值为1为红灯与停止效果相同，但是需要能在该值变为2时离开这个状态进入正常行驶状态
#第9,10位数据strcontrol发送的是指示牌R和L识别结果：
#当值为1时走内圈，由于岔路的识别x会偏向主路，为成功转弯建议给得到的x值乘上一个倍数，值为2走外圈，与此前我们的控制代码一致，无需单独修改