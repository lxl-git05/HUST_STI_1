# -*- coding: utf-8 -*-
# 把当前工程的 Serial 库（Serial_base + Serial_porting）复制到 Car 工程，替换旧 Serial 库
# 注意：Car 工程源文件多为 GBK 编码，全部按字节级处理，避免编码损坏
import os

SRC = r"D:\github\HUST_STI\HUST_STI_1\Templete\Template_STM32H743VIT6"
CAR = r"D:\github\2-2-STM32\STM32\Projects\Robot2026\Sheng\Car"


def read_utf8(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8")


def to_gbk(path, text):
    try:
        return text.encode("gbk")
    except UnicodeEncodeError as e:
        print("  !! GBK 编码失败(改用UTF-8): %s -> %s" % (os.path.basename(path), e))
        return text.encode("utf-8")


# 1. Hardware/Serial_base.h + Serial_base.c 原样覆盖（协议定义，新库）
for name in ["Serial_base.h", "Serial_base.c"]:
    text = read_utf8(os.path.join(SRC, "Hardware", name))
    dst = os.path.join(CAR, "Hardware", name)
    with open(dst, "wb") as f:
        f.write(to_gbk(dst, text))
    print("覆盖 Hardware/%s" % name)

# 2. Function/Serial_porting.h —— 去掉 Serial4_Enable（Car 只有 USART1/2/3，无 UART4）
text = read_utf8(os.path.join(SRC, "Function", "Serial_porting.h"))
old = "#define Serial4_Enable 1"
new = "// #define Serial4_Enable 1   // Car 工程无 UART4（仅 USART1/2/3），禁用 Serial4"
assert text.count(old) == 1, text.count(old)
text = text.replace(old, new)
dst = os.path.join(CAR, "Function", "Serial_porting.h")
with open(dst, "wb") as f:
    f.write(to_gbk(dst, text))
print("新增 Function/Serial_porting.h (Serial4 已禁用)")

# 3. Function/Serial_porting.c 原样复制（Serial4 相关代码已被 #ifdef 保护）
text = read_utf8(os.path.join(SRC, "Function", "Serial_porting.c"))
dst = os.path.join(CAR, "Function", "Serial_porting.c")
with open(dst, "wb") as f:
    f.write(to_gbk(dst, text))
print("新增 Function/Serial_porting.c")

# 4. Top/AllHeader.h —— include 换成 Serial_porting.h（GBK 文件，二进制替换）
p = os.path.join(CAR, "Top", "AllHeader.h")
with open(p, "rb") as f:
    data = f.read()
assert data.count(b'#include "Serial.h"') == 1, data.count(b'#include "Serial.h"')
data = data.replace(b'#include "Serial.h"', b'#include "Serial_porting.h"')
with open(p, "wb") as f:
    f.write(data)
print("修改 Top/AllHeader.h: Serial.h -> Serial_porting.h")

# 5. 删除旧 Function/Serial.h + Serial.c（git 已跟踪，可恢复）
for name in ["Serial.h", "Serial.c"]:
    p = os.path.join(CAR, "Function", name)
    if os.path.exists(p):
        os.remove(p)
        print("删除 Function/%s" % name)

# 6. MDK-ARM/Template.uvprojx —— 文件条目改为 Serial_porting（二进制替换，避免编码问题）
p = os.path.join(CAR, "MDK-ARM", "Template.uvprojx")
with open(p, "rb") as f:
    data = f.read()
reps = [
    (b"<FileName>Serial.c</FileName>", b"<FileName>Serial_porting.c</FileName>"),
    (b"<FilePath>..\\Function\\Serial.c</FilePath>", b"<FilePath>..\\Function\\Serial_porting.c</FilePath>"),
    (b"<FileName>Serial.h</FileName>", b"<FileName>Serial_porting.h</FileName>"),
    (b"<FilePath>..\\Function\\Serial.h</FilePath>", b"<FilePath>..\\Function\\Serial_porting.h</FilePath>"),
]
for old_b, new_b in reps:
    cnt = data.count(old_b)
    assert cnt == 1, (old_b, cnt)
    data = data.replace(old_b, new_b)
with open(p, "wb") as f:
    f.write(data)
print("修改 MDK-ARM/Template.uvprojx: Serial.c/h -> Serial_porting.c/h")
print("DONE")
