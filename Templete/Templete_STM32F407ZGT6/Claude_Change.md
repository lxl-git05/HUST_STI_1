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

## 2026-07-13 22:50 | 移植旋转编码器+AT24C02+ParamEdit到F407工程

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Encoder_Key.h | Template_F407ZGT6/Hardware/Encoder_Key.h | 新增 | EC11旋转编码器驱动头文件（Encoder_Init/Encoder_Get） |
| Encoder_Key.c | Template_F407ZGT6/Hardware/Encoder_Key.c | 新增 | EC11 EXTI解码驱动，PF3/PF7下降沿中断+HAL_GPIO_EXTI_Callback |
| bsp_at24c02.h | Template_F407ZGT6/Hardware/bsp_at24c02.h | 新增 | AT24C02软件I2C头文件，PA4/PA5引脚宏，NOP延时*19(168MHz) |
| bsp_at24c02.c | Template_F407ZGT6/Hardware/bsp_at24c02.c | 新增 | AT24C02软件I2C实现（Start/Stop/Ack/SendByte/ReadByte） |
| at24c02_manager.h | Template_F407ZGT6/Software/at24c02_manager.h | 新增 | AT24C02参数管理层头文件（AT_ParamItem结构体+注册宏+API） |
| at24c02_manager.c | Template_F407ZGT6/Software/at24c02_manager.c | 新增 | AT24C02参数管理实现（地址自动分配+空白芯片检测+读写协调） |
| ParamEdit.h | Template_F407ZGT6/Software/ParamEdit.h | 新增 | OLED参数编辑器头文件，KEY_1=进入/退出, KEY_2=前后翻, KEY_3=保存 |
| ParamEdit.c | Template_F407ZGT6/Software/ParamEdit.c | 新增 | 参数编辑器状态机（进入/退出编辑+编码器值修改+AT脏标记+OLED显示），移除OLED_Update遵循工程约定 |
| Param_AT24C02.h | Template_F407ZGT6/Function/Param_AT24C02.h | 新增 | AT24C02业务层头文件，声明5个测试全局变量+持久化API |
| Param_AT24C02.c | Template_F407ZGT6/Function/Param_AT24C02.c | 新增 | AT24C02业务层实现，定义变量+注册AT参数表+EraseAll/读写 |
| AllHeader.h | Template_F407ZGT6/Top/AllHeader.h | 修改 | 新增6个include（Encoder_Key/bsp_at24c02/at24c02_manager/ParamEdit/Param_AT24C02） |
| AllHeader.c | Template_F407ZGT6/Top/AllHeader.c | 修改 | Initial_ALL中新增Encoder_Init()和Param_AT24C02_Init()调用 |
| gpio.c | Template_F407ZGT6/Core/Src/gpio.c | 修改 | EC11 EXTI触发边沿 IT_RISING→IT_FALLING（适配编码器解码逻辑） |
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | 完全重写为ParamEdit测试代码（5个演示参数+编辑/保存/串口日志） |
| CLAUDE.md | Template_F407ZGT6/CLAUDE.md | 修改 | 移植进度新增5个模块，追加Encoder_Key/AT24C02/ParamEdit完整文档 |

## 2026-07-14 20:30 | 实现步进电机位置模式（T型/三角形速度曲线）

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Stepper_PWM.h | Template_F407ZGT6/Hardware/Stepper_PWM.h | 修改 | 新增POS_PHASE枚举宏 + 结构体新增11个位控字段 + 3个位控API声明 |
| Stepper_PWM.c | Template_F407ZGT6/Hardware/Stepper_PWM.c | 修改 | 实现位控核心：Pos_Set_Abs(匀加速公式预计算+场景判定) / Pos_Set_Rel / Pos_Tick(1ms阶段机) / 修改Pulse_Count(步数累加+脉冲中断到位停止) / 修改_Apply_Speed(位控跳过L1限位) / 修改Speed_Set/Tick(互斥) |
| Mode_G.c | Template_F407ZGT6/Mode/Mode_G.c | 修改 | Timer_1ms_Callback中新增Stepper_PWM_Pos_Tick调用(2台步进) |
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | 位置模式测试例程：KEY1绝对±90°/KEY2相对±30°/KEY3急停/OLED显示角度+阶段+步数 |

## 2026-07-14 21:00 | 重写Mode_2循环往复测试 + 编写步进电机完整驱动说明书

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Mode_2.c | Template_F407ZGT6/Mode/Mode_2.c | 修改 | 重写为循环往复测试：KEY1=相对±180°往复(120rpm/60acc) / KEY2=绝对+90°↔-180°往复 / KEY3=停止 / 到位自动翻转方向 |
| Stepper_PWM_Manual.md | Template_F407ZGT6/Hardware/Stepper_PWM_Manual.md | 新增 | 步进电机驱动完整说明书（12章）：API参考/驱动原理/算法推导/移植指南/8个完整例程/故障排查 |
