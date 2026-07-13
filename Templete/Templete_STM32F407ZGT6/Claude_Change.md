# 变更记录

## 2026-07-13 13:01 | 移植 Software 和 Tools 层并编写测试例程

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| MyPID.h | Template_F407ZGT6/Software/MyPID.h | 新增 | PID控制器头文件，从H743模板直接移植（芯片无关） |
| MyPID.c | Template_F407ZGT6/Software/MyPID.c | 新增 | PID控制器实现（P/I/D分离，积分限幅，微分先行，死区，不完全微分） |
| Task.h | Template_F407ZGT6/Software/Task.h | 新增 | 任务管理器头文件（周期任务+单次延迟任务） |
| Task.c | Template_F407ZGT6/Software/Task.c | 新增 | 任务管理器实现，支持10个并发单次任务 |
| LED_Flash.h | Template_F407ZGT6/Tools/LED_Flash.h | 新增 | LED闪烁控制头文件（5种模式：常亮/常灭/慢闪/快闪/瞬闪） |
| LED_Flash.c | Template_F407ZGT6/Tools/LED_Flash.c | 新增 | LED闪烁控制实现，绑定MyGPIO_LED0(PB5) |
| Timer_Counter.h | Template_F407ZGT6/Tools/Timer_Counter.h | 新增 | DWT代码计时器头文件 |
| Timer_Counter.c | Template_F407ZGT6/Tools/Timer_Counter.c | 新增 | DWT代码计时器实现（Cortex-M4 DWT外设） |
| AllHeader.h | Template_F407ZGT6/Top/AllHeader.h | 修改 | 启用Tools和Software层头文件引用 |
| AllHeader.c | Template_F407ZGT6/Top/AllHeader.c | 修改 | 启用Flash_Mode_Init()和Timer_Counter_Init()调用 |
| Mode_G.c | Template_F407ZGT6/Mode/Mode_G.c | 修改 | 1ms中断中启用Flash_Mode_Tick()和task_Once_Cnt_Tick()，单击启用Flash_Mode_Set |
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | 编写完整测试例程（PID/Task/LED_Flash/Timer_Counter 4个子演示） |
| CLAUDE.md | Template_F407ZGT6/CLAUDE.md | 修改 | 添加移植进度表和测试例程说明 |

## 2026-07-13 14:43 | RGB 模块移植（GPIO开关控制）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| MyGPIO.h | Template_F407ZGT6/MySystem/MyGPIO.h | 修改 | 新增 MyGPIO_RGB_R/G/B 外部声明 |
| MySystem.c | Template_F407ZGT6/MySystem/MySystem.c | 修改 | 新增3个RGB GPIO实例（PG2=R, PG4=G, PG6=B） |
| RGB.h | Template_F407ZGT6/Hardware/RGB.h | 修改 | 写入颜色枚举（5色）+ API声明 |
| RGB.c | Template_F407ZGT6/Hardware/RGB.c | 修改 | 实现：Init/Set_Color/Control（自动循环+手动选色） |
| Mode_G.c | Template_F407ZGT6/Mode/Mode_G.c | 修改 | 20ms Tick 中加入 RGB_Auto_Task__Possess() |
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | RGB 测试例程（KEY1自动/手动, KEY2切换颜色） |

## 2026-07-13 21:15 | Speed ramp: Acc_Val 单位改为 rpm/s + Speed_Tick 移到 1ms 回调

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Stepper_PWM.h | Template_F407ZGT6/Hardware/Stepper_PWM.h | 修改 | Acc_Val 注释更新为 rpm/s |
| Stepper_PWM.c | Template_F407ZGT6/Hardware/Stepper_PWM.c | 修改 | Speed_Tick 内部 Acc_Val/1000 得到每ms步长；注释更新 |
| Mode_G.c | Template_F407ZGT6/Mode/Mode_G.c | 修改 | Speed_Tick 从 Timer_20ms 移到 Timer_1ms（1ms丝滑ramp，粒度提升20倍） |
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | acc 值适配 rpm/s（2→100, 5→250） |

## 2026-07-13 20:50 | 实现步进电机T型速度曲线位置控制

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Stepper_PWM.h | Template_F407ZGT6/Hardware/Stepper_PWM.h | 修改 | 结构体新增7个位置控制字段 + Pos_Set/Pos_Tick函数声明 |
| Stepper_PWM.c | Template_F407ZGT6/Hardware/Stepper_PWM.c | 修改 | 实现Pos_Set规划器（三角形/梯形判定）+ Pos_Tick执行器（加速→匀速→减速）+ Init/Stop/Speed_Tick加guard |
| Mode_G.c | Template_F407ZGT6/Mode/Mode_G.c | 修改 | Timer_20ms_Callback中新增Pos_Tick调用（优先级高于Speed_Tick） |
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | 替换限位测试为T型位置控制测试（KEY1:+30° KEY2:-30° KEY3:0°） |
| CLAUDE.md | Template_F407ZGT6/CLAUDE.md | 修改 | TODO标记完成，更新API文档 |
| stepper-pwm-driver.md | memory/stepper-pwm-driver.md | 修改 | 新增位置控制API文档和结构体字段说明 |
