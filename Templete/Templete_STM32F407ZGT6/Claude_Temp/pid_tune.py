"""
Interactive PID tuning tool for STM32 ball balance system.
- Real-time CSV monitoring from Serial1
- Interactive command input (ABC protocol: Key=Value format)
- Quick presets for speed loop and angle loop tuning

Usage: python pid_tune.py [COM_PORT] [BAUD]
Default: COM11 115200
"""

import serial
import sys
import time
import threading
from datetime import datetime

# === Config ===
PORT = sys.argv[1] if len(sys.argv) > 1 else "COM11"
BAUD = int(sys.argv[2]) if len(sys.argv) > 2 else 115200

# === Globals ===
running = True
ser = None

# === Presets ===
ANGLE_PID = {"Kp": 108, "Ki": 0, "Kd": 20}    # 角度内环：ζ≈1.0
SPEED_PID = {"Kp": 0.05, "Ki": 0, "Kd": 2}    # 速度外环：温和起点

# === Serial reader thread ===
def reader():
    """Read serial data and print to console."""
    buf = b""
    while running:
        try:
            data = ser.read(256)
            if data:
                buf += data
                while b'\n' in buf:
                    line, buf = buf.split(b'\n', 1)
                    try:
                        text = line.decode('utf-8', errors='replace').strip()
                        if text:
                            # Colorize CSV: green for data lines
                            if ',' in text:
                                print(f"\033[32m[{datetime.now():%H:%M:%S}] {text}\033[0m")
                            else:
                                print(f"[{datetime.now():%H:%M:%S}] {text}")
                    except:
                        pass
        except serial.SerialException:
            break
        except:
            pass

# === Main ===
def main():
    global ser, running

    print(f"\n{'='*60}")
    print(f"  STM32 PID 调参工具")
    print(f"  Port: {PORT}  Baud: {BAUD}")
    print(f"{'='*60}")
    print()
    print("命令格式:  Key=Value   (如 Kp=0.1, Kd=2, Goal=0)")
    print("快捷命令:  /mode X     (切换Mode_2子模式, 0=位置PID, 1=角度测试)")
    print("          /speed      (推速度环调参预设)")
    print("          /angle      (推角度内环调参预设)")
    print("          /save       (保存当前参数到 EEPROM)")
    print("          /read       (查看当前参数)")
    print("          /q          (退出)")
    print()
    print("  ★ 速度环震荡: 先试试 /speed 预设, 再微调 Kp/Kd")
    print(f"{'='*60}\n")

    # Connect
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
        print(f"已连接 {PORT}\n")
        time.sleep(0.5)
        ser.reset_input_buffer()
    except Exception as e:
        print(f"连接失败: {e}")
        return

    # Start reader thread
    t = threading.Thread(target=reader, daemon=True)
    t.start()

    # Interactive command loop
    try:
        while running:
            cmd = input("> ").strip()
            if not cmd:
                continue

            # Quick commands
            if cmd == "/q":
                break
            elif cmd == "/speed":
                for k, v in SPEED_PID.items():
                    ser.write(f"{k}={v}\n".encode())
                    print(f"  -> {k}={v}")
                    time.sleep(0.05)
                print("  ✓ 速度环预设已发送 (温和起点)")
            elif cmd == "/angle":
                for k, v in ANGLE_PID.items():
                    ser.write(f"{k}={v}\n".encode())
                    print(f"  -> {k}={v}")
                    time.sleep(0.05)
                print("  ✓ 角度内环预设已发送 (ζ≈1.0)")
            elif cmd.startswith("/mode"):
                parts = cmd.split()
                val = parts[1] if len(parts) > 1 else "0"
                ser.write(f"Mode={val}\n".encode())
                print(f"  -> Mode={val}")
            elif cmd == "/save":
                ser.write("Save=1\n".encode())
                print("  -> Save=1 (保存到EEPROM)")
            elif cmd == "/read":
                # Send a query that triggers current display
                # Mode_2/5 already display on OLED
                print("  OLED会显示当前参数, 或看串口CSV数据")
            else:
                # Treat as Key=Value
                ser.write(f"{cmd}\n".encode())
                print(f"  -> {cmd}")
                time.sleep(0.01)

    except KeyboardInterrupt:
        pass
    finally:
        running = False
        if ser and ser.is_open:
            ser.close()
        print("\n已断开")

if __name__ == "__main__":
    main()
