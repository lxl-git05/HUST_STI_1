import cv2
import time
import serial
from moment import get_center_point
from moment import SerialPacket
from moment import ls
from moment import count_black_pixels_at_y

# 打开默认摄像头
cap = cv2.VideoCapture(0)

# 设置分辨率
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

if not cap.isOpened():
    print("错误：无法打开摄像头")
    exit()

pack = SerialPacket(port="/dev/ttyUSB0", baudrate=115200, timeout=0.1)

windowname = 'window'
cv2.namedWindow(windowname)

try:
    while True:
        ret, frame = cap.read()

        if not ret:
            print("获取帧失败")
            continue

        height, width = frame.shape[:2]
        roi = frame[height // 2 - 120:height // 2 + 120, width // 2 - 160:width // 2 + 160]
        angel = ls(roi)
        x, y, roi, isj = get_center_point(roi)
        cv2.circle(roi, (x,y), 33, (255,0,0), -1)
        #print(angel)
        #print(count_black_pixels_at_y(roi,120))
        if(count_black_pixels_at_y(roi,120)>200):
            isj = 2
        # x = int(x*255/640)
        cv2.imshow(windowname, roi)
        if cv2.waitKey(1) & 0xFF == 27:
            break
        # print(f"x: {x},y: {y}, isj: {isj}")
        angel=int(angel)+100
        num = 0x06
        pack.insert_byte(num)
        pack.insert_two_bytes(pack.num_to_bytes(x+100))
        pack.insert_two_bytes(pack.num_to_bytes(isj))
        pack.insert_two_bytes(pack.num_to_bytes(angel+100))
        pack.send_packet()

    # cv2.imshow('frame',frame)
    # time.sleep(0.01)
    # cv2.destroyAllWindows()

except KeyboardInterrupt:
    print("程序退出")

    cap.release()
    cv2.destroyAllWindows()