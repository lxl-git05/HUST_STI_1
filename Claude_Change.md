
## 2026-08-14 13:37 | 加入 6 路舵机驱动（Servo + Con_Servo）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Servo.h | ./Hardware/Servo.h | 新增 | 简化舵机驱动头文件（180°、双限幅字段、Init/SetAngle/Get_Angle 三 API） |
| Servo.c | ./Hardware/Servo.c | 新增 | 驱动实现：50Hz 校验 + 双限幅 + 线性映射 + 初始角度写入 |
| Con_Servo.h | ./Function/Con_Servo.h | 新增 | 6 路舵机实例 extern + Con_Servo_Init 声明 |
| Con_Servo.c | ./Function/Con_Servo.c | 新增 | 6 路实例定义，统一初始 90°（限幅 0~180°） |
| MyPWM.h | ./MySystem/MyPWM.h | 修改 | 补 MyPWM_Servo5/6 extern 声明 |
| MySystem.c | ./MySystem/MySystem.c | 修改 | 定义 6 路舵机 PWM 实例（TIM1 CH1-4 + TIM8 CH3/4），限幅修正为 250/50 tick |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 硬件驱动库组加 Servo.h，硬件实现库组加 Con_Servo.h |
| AllHeader.c | ./Top/AllHeader.c | 修改 | Initial_ALL 调用 Con_Servo_Init（开机 6 路归中 90°） |

> Keil 工程添加 Servo.c / Con_Servo.c 由用户自行操作。

## 2026-08-14 14:00 | 基于 Mode_4 新建 Mode_5 / Mode_6 空骨架

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_5.h | ./Mode/Mode_5.h | 新增 | Mode_4 骨架克隆（Setup/Loop/Tick/Exit 声明） |
| Mode_5.c | ./Mode/Mode_5.c | 新增 | Mode_4 骨架克隆（OLED 显示 ===Mode_5===） |
| Mode_6.h | ./Mode/Mode_6.h | 新增 | Mode_4 骨架克隆（Setup/Loop/Tick/Exit 声明） |
| Mode_6.c | ./Mode/Mode_6.c | 新增 | Mode_4 骨架克隆（OLED 显示 ===Mode_6===） |
| Mode_G.h | ./Mode/Mode_G.h | 修改 | 枚举新增 Mode_5 / Mode_6 |
| Mode_G.c | ./Mode/Mode_G.c | 修改 | 20ms Tick switch 新增 Mode_5 / Mode_6 分支 |
| Mymain.c | ./Top/Mymain.c | 修改 | 三处 switch（Loop/Exit/Setup）新增 Mode_5 / Mode_6 分支 |
| AllHeader.h | ./Top/AllHeader.h | 修改 | Mode 库组新增 Mode_5.h / Mode_6.h |

> Keil 工程添加 Mode_5.c / Mode_6.c 由用户自行操作。
