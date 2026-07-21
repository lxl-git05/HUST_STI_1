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
