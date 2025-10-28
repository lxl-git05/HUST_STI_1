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

pack=SerialPacket(port="COM4", baudrate=115200, timeout=0.1)




try:
    while True:
        ret, frame = cap.read()

        if not ret:
            print("获取帧失败")
            continue

        height, width = frame.shape[:2]
        roi = frame[height // 2:, :]
        x, y, roi, isj = get_center_point(roi)
        # x = int(x*255/640)
        print(f"x: {x}, isj: {isj}")
        num=0x04
        pack.insert_two_bytes(pack.num_to_bytes(x))
        pack.insert_two_bytes(pack.num_to_bytes(isj))
        pack.send_packet()


        time.sleep(0.1)

except KeyboardInterrupt:
    print("程序退出")


    cap.release()
    cv2.destroyAllWindows()