"""Capture serial data to file, bypassing console encoding issues."""
import serial
import sys
import time
from datetime import datetime

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM11"
BAUD = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
DURATION = float(sys.argv[3]) if len(sys.argv) > 3 else 5.0

log_file = f"D:/github/HUST_STI/HUST_STI_1/Templete/Templete_STM32F407ZGT6/Claude_Temp/serial_log_{int(time.time())}.txt"

ser = serial.Serial(PORT, BAUD, timeout=0.1)
print(f"Connected to {PORT} @ {BAUD}")
print(f"Logging to: {log_file}")
print(f"Duration: {DURATION}s")
print()

start = time.time()
line_count = 0

with open(log_file, 'w', encoding='utf-8') as f:
    while time.time() - start < DURATION:
        try:
            data = ser.read(256)
            if data:
                now = datetime.now()
                ts = f"[{now:%H:%M:%S}.{now.microsecond // 1000:03d}]"
                # Try to decode as text, fallback to hex
                try:
                    text = data.decode('utf-8')
                    f.write(f"{ts} {text}")
                    print(f"{ts} {text}", end='')
                    line_count += text.count('\n')
                except:
                    hex_str = ' '.join(f"{b:02X}" for b in data)
                    f.write(f"{ts} HEX: {hex_str}\n")
                    print(f"{ts} HEX: {hex_str}")
        except KeyboardInterrupt:
            break

ser.close()
elapsed = time.time() - start
print(f"\n\nDone. {line_count} lines in {elapsed:.1f}s ({line_count/elapsed:.1f} lines/s)")
print(f"Log saved to: {log_file}")
