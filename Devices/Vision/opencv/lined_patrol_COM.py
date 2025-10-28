import cv2
import time
import serial
from moment import get_center_point
from moment import SerialPacket
# 打开默认摄像头
cap = cv2.VideoCapture(0)

# 设置分辨率
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

if not cap.isOpened():
    print("错误：无法打开摄像头")
    exit()

pack=SerialPacket(port="COM1", baudrate=115200, timeout=0.1)


windowname = 'window'
cv2.namedWindow(windowname)

try:
    while True:
        ret, frame = cap.read()
        cv2.imshow(windowname,frame)
        if cv2.waitKey(1) & 0xFF == 27:
            break
        if not ret:
            print("获取帧失败")
            continue

        height, width = frame.shape[:2]
        roi = frame[height // 2:, :]
        x, y, roi, isj = get_center_point(roi)
        # x = int(x*255/640)
        print(f"x: {x}, isj: {isj}")
        num=0x04
        pack.insert_two_bytes(pack.num_to_bytes(x+100))
        pack.insert_two_bytes(pack.num_to_bytes(isj))
       #pack.send_packet()

        #cv2.imshow('frame',frame)
        time.sleep(0.01)
        #cv2.destroyAllWindows()

except KeyboardInterrupt:
    print("程序退出")


    cap.release()
    cv2.destroyAllWindows()