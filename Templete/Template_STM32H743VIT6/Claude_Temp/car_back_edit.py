# -*- coding: utf-8 -*-
# 小车工程 Car_Back 倒车：GBK 文件字节级插入，防编码损坏
# 锚点全部用 ASCII 字节（编码无关），插入文本统一 GBK 编码
CAR = r"D:\github\2-2-STM32\STM32\Projects\Robot2026\Sheng\Car"


def G(s):
    b = s.encode("gbk")
    return b


def insert_after_line(data, anchor, insertion):
    """在包含 anchor 的那一行末尾插入 insertion（insertion 自带头尾换行）"""
    idx = data.find(anchor)
    assert idx >= 0, "anchor not found: %r" % anchor
    eol = data.find(b"\n", idx)
    assert eol >= 0
    return data[:eol + 1] + insertion + data[eol + 1:]


def insert_before(data, anchor, insertion):
    idx = data.find(anchor)
    assert idx >= 0, "anchor not found: %r" % anchor
    return data[:idx] + insertion + data[idx:]


def edit(path, fn):
    p = CAR + "\\" + path
    with open(p, "rb") as f:
        data = f.read()
    data = fn(data)
    with open(p, "wb") as f:
        f.write(data)
    print("OK %s" % path)


# 1. Con_Wheel_Control.h — 枚举加 Car_Turn_B
edit(r"Function\Con_Wheel_Control.h", lambda d: insert_after_line(
    d,
    b"Car_Turn_H  ,",
    G("    Car_Turn_B  ,   // 直线倒车          5\n"),
))

# 2. Con_Wheel_Control.c — 三个函数（插在 // 3. Car_Turn_L 段头前，glo_time 已声明）
edit(r"Function\Con_Wheel_Control.c", lambda d: insert_before(
    d,
    b"// 3. Car_Turn_L",
    G(
        "// 2.5 Car_Turn_B 倒车（负目标直线后退，克隆 Car_Turn_F）\n"
        "void Car_Turn_B_Setup(void)\n"
        "{\n"
        "\tPID_Angle_Curr_Reset() ;\n"
        "\tPID_Goal_Angle_Set(0) ;\n"
        "\tMotor_Pos_Clear() ;\n"
        "\tPID_ALL_Pos_Reset() ;\n"
        "\tPID_ALL_Pos_Set_Goal(-50) ;\t// 倒车 50cm（与前进 50cm 对应）\n"
        "\tglo_time = HAL_GetTick() ;\t// 防 Is_Exit 用旧 glo_time 误判\n"
        "}\n"
        "void Car_Turn_B_Tick(void)\n"
        "{\n"
        "\tPID_Angle_Tick(PID_ALL_Pos_Tick()) ;\t// 负速度由负位置环输出给出\n"
        "}\n"
        "bool Car_Turn_B_Is_Exit(void)\n"
        "{\n"
        "\treturn Car_Turn_F_Is_Exit() ;\t\t// fabs 对负目标兼容\n"
        "}\n"
        "\n"
    ),
))

# 3. Con_Wheel_Control.c — Car_Control Tick switch 加 case
edit(r"Function\Con_Wheel_Control.c", lambda d: insert_after_line(
    d,
    b"Car_Turn_F_Tick(60)",
    G("            case Car_Turn_B   : Car_Turn_B_Tick() ;    	 break;    // 倒车\n"),
))

# 4. Con_Wheel_Control.c — Car_Control Setup switch 加 case
edit(r"Function\Con_Wheel_Control.c", lambda d: insert_after_line(
    d,
    b"Car_Turn_F_Setup();",
    G("            case Car_Turn_B   : Car_Turn_B_Setup(); break;  // 倒车\n"),
))

# 5. Con_Wheel_Control.c — Car_Control_Change 加退出分支（插在 Car_Turn_L 分支前）
edit(r"Function\Con_Wheel_Control.c", lambda d: insert_before(
    d,
    b"else if(curr_Status == Car_Turn_L)",
    G(
        "else if(curr_Status == Car_Turn_B)\t// 倒车完成停\n"
        "\t{\n"
        "\t\tif (Car_Turn_B_Is_Exit())\n"
        "\t\t{\n"
        "\t\t\tnext_Status = Car_Stop ;\n"
        "\t\t}\n"
        "\t}\n"
        "\t"
    ),
))

# 6. Mode_5.c — 收 Car_Back 分支
edit(r"Mode\Mode_5.c", lambda d: insert_after_line(
    d,
    b"next_Status = Car_Turn_F ;",
    G(
        "\n"
        "\tif (Serial_CheckCmd(&Serial3 , \"Car_Back\"))\n"
        "\t{\n"
        "\t\tSerial_Clear_ABC(&Serial3) ;\n"
        "\t\tnext_Status = Car_Turn_B ;\n"
        "\t}\n"
    ),
))

print("DONE")
