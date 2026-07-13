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
