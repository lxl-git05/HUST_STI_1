## 2026-07-15 | 电机PID调参示例代码——用KEY_1单击切换电机替代#ifdef

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | 电机PID调参实验模式：KEY_1单击切换选择Motor A/B，串口设置Kp/Ki/Kd/目标速度，Tick打印速度数据 |

## 2026-07-16 | 移植Mode_2到Mode_4 + 新建Mode_5/6 + 集成到Mymain

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_4.c | ./Template_F407ZGT6/Mode/Mode_4.c | 修改 | 从Mode_2等效移植电机PID调参代码 |
| Mode_5.h | ./Template_F407ZGT6/Mode/Mode_5.h | 新增 | Mode_5 头文件，标准四函数声明 |
| Mode_5.c | ./Template_F407ZGT6/Mode/Mode_5.c | 新增 | Mode_5 实现，空壳模板 |
| Mode_6.h | ./Template_F407ZGT6/Mode/Mode_6.h | 新增 | Mode_6 头文件，标准四函数声明 |
| Mode_6.c | ./Template_F407ZGT6/Mode/Mode_6.c | 新增 | Mode_6 实现，空壳模板 |
| Mode_G.h | ./Template_F407ZGT6/Mode/Mode_G.h | 修改 | 枚举新增 Mode_5, Mode_6 |
| Mode_G.c | ./Template_F407ZGT6/Mode/Mode_G.c | 修改 | Timer_20ms_Callback 新增 case 5/6 |
| Mymain.c | ./Template_F407ZGT6/Top/Mymain.c | 修改 | Loop/Exit/Setup 三处 switch 新增 case 5/6 |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | 新增 #include "Mode_5.h" "Mode_6.h" |
| Template.uvprojx | ./Template_F407ZGT6/MDK-ARM/Template.uvprojx | 修改 | Mode 组新增 Mode_5.c/.h, Mode_6.c/.h |

## 2026-07-16 | Mode_2清空→电机PID代码迁移到Con_Mode_1 + Con_Mode_1~6集成到Mymain框架

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | 清空为原始框架（Setup/Loop/Tick/Exit空壳） |
| Con_Mode_1.h | ./Template_F407ZGT6/Con_Mode/Con_Mode_1.h | 修改 | 电机PID调参模块，四函数声明 |
| Con_Mode_1.c | ./Template_F407ZGT6/Con_Mode/Con_Mode_1.c | 修改 | 电机PID调参实现（从Mode_2迁移），KEY_1切换Motor A/B |
| Con_Mode_2~6.h | ./Template_F407ZGT6/Con_Mode/Con_Mode_2~6.h | 修改 | 统一加入四函数声明 |
| Con_Mode_2~6.c | ./Template_F407ZGT6/Con_Mode/Con_Mode_2~6.c | 修改 | 统一加入四函数空壳实现 |
| Mode_G.h | ./Template_F407ZGT6/Mode/Mode_G.h | 修改 | 枚举新增 Con_Mode_1~6 |
| Mode_G.c | ./Template_F407ZGT6/Mode/Mode_G.c | 修改 | Timer_20ms_Callback 新增 Con_Mode_1~6 的 Tick 分发 |
| Mymain.c | ./Template_F407ZGT6/Top/Mymain.c | 修改 | Loop/Exit/Setup 三处 switch 新增 Con_Mode_1~6 |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | 新增 Con_Mode_1~6.h 的 include |

## 2026-07-16 | 从Robot_V2移植环形队列库 + gpio.c修复电机B复位误转

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Queue.h | ./Template_F407ZGT6/Software/Queue.h | 新增 | 环形队列头文件，QueueData_Typedef待用户定义后取消注释 |
| Queue.c | ./Template_F407ZGT6/Software/Queue.c | 新增 | 环形队列实现：Init/Enqueue/Dequeue/Peek/Size/Clear/IsEmpty/IsFull |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | Software 区新增 #include "Queue.h" |
| Template.uvprojx | ./Template_F407ZGT6/MDK-ARM/Template.uvprojx | 修改 | SoftWare 组新增 Queue.c/.h |
| gpio.c | ./Template_F407ZGT6/Core/Src/gpio.c | 修改 | PD14/PD15 在 MX_GPIO_Init 开头写 LOW，修复复位期间电机B误转 |
| Con_Motor.c | ./Template_F407ZGT6/Function/Con_Motor.c | 修改 | Motorx_Angle_Update_Tick 去 static 使其可外部调用 |
| Mode_4.c | ./Template_F407ZGT6/Mode/Mode_4.c | 修改 | 位置环+速度环双模式PID调参（KEY_1切电机，KEY_2切环类型） |

## 2026-07-21 14:30 | 实现 Stepper_PWM_Is_Angle 到位检测函数

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Stepper_PWM.h | ./Template_F407ZGT6/Hardware/Stepper_PWM.h | 修改 | 新增 Stepper_PWM_Is_Angle(void) 和 Stepper_PWM_Is_Angle_Stepper(pStepper) 声明 |
| Stepper_PWM.c | ./Template_F407ZGT6/Hardware/Stepper_PWM.c | 修改 | 实现角度到达检测：速度≈0 + 角度≈Pos_TargetAngle 双条件判断，容差 1.5×pulse_angle |

## 2026-07-23 16:30 | ICM42688 互补滤波增强——倾斜下 yaw 漂移削弱

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| ICM42688_Angle.h | ./Template_F407ZGT6/Function/ICM42688_Angle.h | 修改 | 新增加速度幅值门控宏、静止检测与零偏学习宏、DEG2RAD/RAD2DEG 宏、调试查询函数声明 |
| ICM42688_Angle.c | ./Template_F407ZGT6/Function/ICM42688_Angle.c | 修改 | 三项改进：(1)世界坐标系yaw投影(欧拉运动学方程) (2)加速度幅值门控(二次曲线动态权重) (3)静止检测+零偏自动学习 |
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | ICM42688 改进互补滤波测试代码：OLED显示Roll/Pitch/Yaw + 串口CSV输出 |

## 2026-07-23 17:00 | ICM42688 寄存器修复 + Mahony AHRS 新库

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| ICM_42688_base.c | ./Template_F407ZGT6/Hardware/ICM_42688_base.c | 修改 | ★ 紧急修复：量程寄存器位值反转，ICM42688的FS_SEL编码与MPU6050相反(000=最大量程)，原来照搬MPU导致所有读数减半 |
| ICM42688_Mahony.h | ./Template_F407ZGT6/Function/ICM42688_Mahony.h | 新增 | Mahony AHRS 头文件：Kp=5.12/Ki=0.001/halfT=0.010s 参数宏 + API声明 |
| ICM42688_Mahony.c | ./Template_F407ZGT6/Function/ICM42688_Mahony.c | 新增 | Mahony AHRS 实现：四元数+PI重力修正，Auto-cal 500样本标定，atan2欧拉角(roll/yaw ±180°,pitch ±90°)，<100μs/次 |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | Function 区新增 #include "ICM42688_Mahony.h" |
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | 切换到 Mahony AHRS 测试：ICM42688_Mahony_Init() + ICM42688_Mahony_Update_Tick() |

## 2026-07-25 00:13 | 实现Y8 8路寻迹传感器驱动与Mode_2测试例程

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Y8_Driver.h | ./Template_F407ZGT6/Hardware/Y8_Driver.h | 修改 | Y8驱动头文件：API声明(Y8_Drive_Init/Y8_Data_Update)、Y8_Data[8]数组、协议文档注释 |
| Y8_Driver.c | ./Template_F407ZGT6/Hardware/Y8_Driver.c | 修改 | ★ Y8驱动完整实现：delay_us(168MHz系数19)、Y8_Read_Sensor(CLK+DAT同步串行协议)、Y8_Data_Update(展开bit到数组)。从8line_test(F103)移植，仅改引脚和延时系数 |
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | Y8寻迹传感器测试例程：OLED显示8路二进制+十六进制+黑线位置(LEFT/HIT/RIGHT)，Y8_Get_Position加权平均算法(-7~+7)，串口CSV输出。★ Y8_Data_Update在Mode_2_Tick(20ms ISR)中调用保证实时性 |
| AllHeader.h | ./Template_F407ZGT6/Top/AllHeader.h | 修改 | Hardware区新增 #include "Y8_Driver.h" |

## 2026-07-26 | 实现Y8_Angle_Bias_Get完整5阶段角度计算

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Y8_Driver.c | ./Template_F407ZGT6/Hardware/Y8_Driver.c | 修改 | ★ 完整实现Y8_Angle_Bias_Get：阶段1多采样投票(cnt次Y8_Read_Sensor)、阶段2多数确认(>cnt/2)、阶段3加权位置→atan2角度、阶段4时序中值滤波(窗口5)、阶段5丢线保持。添加#include <math.h>/<string.h>和Y8_FILTER_WIN宏。删除无用变量Y8_Bias_Arr[8]和lost_cnt。PID空壳中移除废弃的"偏移角数组初始化"注释 |
| Y8_Driver.h | ./Template_F407ZGT6/Hardware/Y8_Driver.h | 修改 | 新增extern声明：Y8_Width[8]、Y8_Bias、Y8_Angle_Bias_Get函数原型及文档注释 |

## 2026-07-26 | Mode_2 8路寻迹展示/巡线双模式切换

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_2.c | ./Template_F407ZGT6/Mode/Mode_2.c | 修改 | KEY_1单击切换展示/巡线模式。展示模式：OLED显示8路原始数据+滤波角度，Tick仅Y8_Data_Update。巡线模式：OLED显示角度+PID输出+电机速度，Tick调用Y8_PID_Update全链路(传感器→角度→PID→差速转向)。Exit停双电机 |

## 2026-07-26 | 修复电机速度PID积分饱和导致严重超调振荡

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Con_Motor.c | ./Template_F407ZGT6/Function/Con_Motor.c | 修改 | Motor_A/B速度PID参数：Ki 1.0→0.1(10x降低积分累积速度)、ioutMax 1000→300(积分贡献上限30%)、Kp 8.0→5.0(降低P过冲)、Kd 0→2.0(加入微分抑制超调)。修复手拨阻塞+Goal阶跃超调219%→振荡的问题 |
