# -*- coding: utf-8 -*-
# 扫描工程所有 .c/.h 文件的编码：UTF-8 / GBK / 混合 / 无中文
# ARMCC v5 在中文 Windows 下按 GBK 读源码，UTF-8 中文会编译报错
import os

ROOT = r"D:\github\HUST_STI\HUST_STI_1\Templete\Template_STM32H743VIT6"
SKIP = {"Drivers", "MDK-ARM", "Claude_Temp"}  # 不扫第三方库和临时目录

def classify(path):
    with open(path, "rb") as f:
        data = f.read()
    if b"\x00" in data:
        return "BINARY"
    def has_cjk(b):
        try:
            return any('\u4e00' <= ch <= '\u9fff' for ch in b.decode("utf-8"))
        except Exception:
            return False
    utf8_ok = False
    try:
        data.decode("utf-8")
        utf8_ok = True
    except UnicodeDecodeError:
        pass
    gbk_ok = False
    try:
        data.decode("gbk")
        gbk_ok = True
    except UnicodeDecodeError:
        pass
    if not has_cjk(data if utf8_ok else None or b""):
        # 直接按字节找 CJK 太麻烦，用下面分支决定
        pass
    if utf8_ok and gbk_ok:
        # 纯 ASCII
        return "ASCII"
    if utf8_ok:
        return "UTF-8"
    if gbk_ok:
        return "GBK"
    return "MIXED"

bad = []
for dirpath, dirnames, filenames in os.walk(ROOT):
    dirnames[:] = [d for d in dirnames if d not in SKIP]
    for fn in filenames:
        if fn.endswith((".c", ".h")):
            p = os.path.join(dirpath, fn)
            kind = classify(p)
            rel = os.path.relpath(p, ROOT)
            if kind in ("UTF-8", "MIXED", "BINARY"):
                bad.append((rel, kind))
            print("%-6s %s" % (kind, rel))
print("\n===== 非 GBK/ASCII 文件（会导致中文串编译问题）=====")
for rel, kind in bad:
    print("%-6s %s" % (kind, rel))
