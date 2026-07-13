# STM32F407ZGT6 模板工程

## 工程概述

从 STM32H743VIT6 模板工程移植到 STM32F407ZGT6，采用分层架构设计。

## 目录结构

```
Template_F407ZGT6/
├── Core/                # CubeMX 生成的 HAL 层代码（芯片相关）
│   ├── Inc/             # main.h, gpio.h, tim.h, usart.h, i2c.h, adc.h
│   └── Src/             # main.c, gpio.c, tim.c, usart.c, i2c.c, adc.c
├── Drivers/             # HAL 库 + CMSIS（芯片相关）
├── MySystem/            # ★ 芯片抽象层（跨芯片移植唯一需改动的层）
│   ├── MySystem.h/c     # 芯片主频宏 + GPIO/PWM/Encoder 实例定义
│   ├── MyGPIO.h/c       # GPIO 读写封装
│   ├── MyPWM.h/c        # PWM 初始化和设置（自动限幅）
│   ├── MyEncoder.h/c    # 编码器初始化、读取、累计
│   └── MyTimer.h/c      # 系统定时器（1ms + 20ms）+ 弱回调 + HAL中断分发
├── Hardware/            # 硬件设备驱动层（依赖 MySystem，芯片无关）
│   ├── Key.h/c          # 按键检测（单击/双击/长按/重复）
│   ├── OLED.h/c         # OLED 显示屏驱动（软件I2C，完整图形库）
│   ├── OLED_Data.h/c    # OLED 字库数据（ASCII + 中文）
│   ├── Motor.h/c        # 直流电机抽象（PWM + 方向 + 编码器 + PID）
│   ├── Servo.h/c        # 舵机驱动（180°/360° 类型，步进控制，角度限幅）
│   ├── Buzzer.h/c       # 蜂鸣器
│   └── Serial_base.h/c  # 串口协议基础（ABC协议 + HEX协议帧定义 + 错误码）
├── Software/            # 软件算法层（芯片无关）
│   ├── MyPID.h/c        # PID 控制器（P/I/D分离，积分限幅，微分先行，死区）
│   └── Task.h/c         # 任务管理器（周期任务 + 单次延迟任务）
├── Tools/               # 工具层（芯片无关）
│   ├── LED_Flash.h/c    # LED 闪烁控制（常亮/常灭/慢闪/快闪/瞬闪）
│   └── Timer_Counter.h/c # DWT 代码执行时间测量
├── Function/            # 功能实现层（组合 Hardware + Software，芯片无关）
│   ├── Serial_porting.h/c # 串口通信移植层（Serial1/2 实例，HEX/ABC 协议解析）
│   ├── Con_Motor.h/c    # 电机控制器（Motor_A/B 实例，速度/角度双PID环）
│   └── Con_Servo.h/c    # 舵机控制器（Servo_1~4 实例，夹爪/衣架动作）
├── Mode/                # 模式状态机（芯片无关）
│   ├── Mode_G.h/c       # 全局模式管理（枚举/切换/定时器回调分发）
│   ├── Mode_1.h/c       # 脱机调参模式（参数调整并保存）
│   ├── Mode_2.h/c       # 实验模式（所有新实验代码写在这里）
│   ├── Mode_3.h/c       # 业务逻辑模式
│   └── Mode_4.h/c       # 业务逻辑模式
├── Top/                 # 顶层调度（芯片无关）
│   ├── AllHeader.h/c    # 统一头文件 + 系统初始化集合
│   └── Mymain.h/c       # 用户主函数（模式调度循环）
└── MDK-ARM/             # Keil MDK 工程文件
```

## 分层架构

```
┌───────────────────────────────────────────┐
│  Top    (Mymain / AllHeader)  调度+初始化 │
├───────────────────────────────────────────┤
│  Mode   (Mode_G / Mode_1~4)   状态机     │
├───────────────────────────────────────────┤
│  Function (电机/舵机/串口)    功能组合    │
├───────────────────────────────────────────┤
│  Hardware (Motor/Servo/Key/OLED) 设备驱动 │
├───────────────────────────────────────────┤
│  Software (PID/Task)          软件算法    │
├───────────────────────────────────────────┤
│  Tools   (LED_Flash/TimerCounter)        │
├───────────────────────────────────────────┤
│  MySystem ★ 芯片抽象层 ★  唯一切换点      │
├───────────────────────────────────────────┤
│  Core + Drivers  HAL库 + CubeMX生成        │
└───────────────────────────────────────────┘
```

## Mode 约定

| Mode | 用途 | 说明 |
|------|------|------|
| Mode_1 | **脱机调参** | 专门用于脱机调整参数并保存到存储 |
| Mode_2 | **实验代码** | 所有后续实验代码都写在这里 |
| Mode_3 | 业务逻辑 | 具体业务功能 |
| Mode_4 | 业务逻辑 | 具体业务功能 |
| Mode_G | 全局 | 系统初始化 + 按键切换模式 + 定时器分发 |

## 芯片关键参数

| 参数 | F407ZGT6 | H743VIT6 (原) |
|------|----------|---------------|
| 主频 | 168 MHz | 240 MHz |
| 定时器时钟(APB1) | 84 MHz | 120 MHz |
| 定时器时钟(APB2) | 168 MHz | 240 MHz |
| 1ms 定时器 | TIM6 | TIM17 |
| 20ms 定时器 | TIM7 | TIM16 |
| 舵机 PWM | TIM1 (CH1-4, 50Hz) | 同 |
| 电机 PWM | TIM4 (CH3-4) | TIM4 (CH1-2) |
| 编码器A | TIM2 | 同 |
| 编码器B | TIM3 | 同 |

## 跨芯片移植指南

只需修改 `MySystem/` 目录下的文件：
1. `MySystem.h` — 改主频宏 `MySystem_Fre` 和 HAL 头文件引用
2. `MySystem.c` — 改 GPIO/PWM/Encoder 实例化从 HAL 宏映射
3. `MyTimer.c` — 改定时器实例号（htim16→htim6 等）

其余所有层（Hardware / Software / Function / Mode / Top）代码完全不用改。

## 移植进度

| 层 | 模块 | 状态 | 日期 |
|-----|--------|--------|------|
| MySystem | MyGPIO / MyPWM / MyEncoder / MyTimer | ✅ 完成 | 2026-07-13 |
| Hardware | Key / OLED / OLED_Data | ✅ 完成 | 2026-07-13 |
| Hardware | Motor / Servo / Buzzer / Serial_base | ⬜ 待移植 | — |
| **Software** | **MyPID / Task** | **✅ 完成** | **2026-07-13** |
| **Tools** | **LED_Flash / Timer_Counter** | **✅ 完成** | **2026-07-13** |
| Function | Serial_porting / Con_Motor / Con_Servo | ⬜ 待移植 | — |
| Mode | Mode_G / Mode_1~4 (含Mode_2测试例程) | ✅ 完成 | 2026-07-13 |

### 测试例程 (Mode_2)

Mode_2 包含 4 个子演示，按 KEY0 单击循环切换：
1. **PID 阶跃响应测试** — 验证 MyPID 模块，KEY1 切换目标值(300/500/800)
2. **任务调度器测试** — 验证 Task 模块(500ms/200ms周期任务 + 3s单次任务)，KEY1暂停/KEY2恢复
3. **LED 闪烁模式测试** — 验证 LED_Flash 模块，KEY1 循环切换5种模式
4. **代码计时器测试** — 验证 Timer_Counter 模块，显示函数执行时间(us)

### 1ms 中断任务列表

`Timer_1ms_Callback()` 中依次执行：
1. `Key_Tick()` — 按键扫描
2. `Flash_Mode_Tick()` — LED闪烁状态更新
3. `task_Once_Cnt_Tick()` — 单次任务倒计时
