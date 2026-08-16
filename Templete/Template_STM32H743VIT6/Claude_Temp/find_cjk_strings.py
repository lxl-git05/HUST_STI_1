# -*- coding: utf-8 -*-
# 找出"字符串字面量里含中文"的文件 —— 这些文件在 ARMCC v5 下会编译出错
# （UTF-8 中文在注释里没事，在字符串里会被按 GBK 误读导致引号错位）
import os, re

ROOT = r"D:\github\HUST_STI\HUST_STI_1\Templete\Template_STM32H743VIT6"
SKIP = {"Drivers", "MDK-ARM", "Claude_Temp"}

cjk = re.compile(r'[\u4e00-\u9fff]')
for dirpath, dirnames, filenames in os.walk(ROOT):
    dirnames[:] = [d for d in dirnames if d not in SKIP]
    for fn in filenames:
        if not fn.endswith((".c", ".h")):
            continue
        p = os.path.join(dirpath, fn)
        try:
            text = open(p, "rb").read().decode("utf-8")
        except UnicodeDecodeError:
            continue  # 非 UTF-8 文件跳过（GBK 的没问题）
        lines = text.split("\n")
        hits = []
        for i, line in enumerate(lines, 1):
            # 去掉 // 注释后再找字符串
            code = line.split("//")[0]
            for m in re.finditer(r'"((?:[^"\\]|\\.)*)"', code):
                if cjk.search(m.group(1)):
                    hits.append((i, m.group(1)[:20]))
        if hits:
            print("=== %s (%d 处)" % (os.path.relpath(p, ROOT), len(hits)))
            for ln, s in hits[:8]:
                print("   L%-4d %s" % (ln, s))
