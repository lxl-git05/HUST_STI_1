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
│   ├── Key.h/c          # 按键检测（4键，单击/双击/长按/重复）
│   ├── OLED.h/c         # OLED 显示屏驱动（软件I2C，完整图形库）
│   ├── OLED_Data.h/c    # OLED 字库数据（ASCII + 中文）
│   ├── RGB.h/c          # RGB LED 控制（GPIO 开关，共阳极）
│   ├── Buzzer.h/c       # 蜂鸣器（存根）
│   ├── Serial_base.h/c  # 串口协议基础（ABC + HEX 帧定义 + 错误码）
│   └── Stepper_PWM.h/c  # 步进电机 PWM 驱动（🆕 待编写业务逻辑）
├── Software/            # 软件算法层（芯片无关）
│   └── MyPID.h/c        # PID 控制器（P/I/D分离，积分限幅，微分先行，死区）
├── Tools/               # 工具层（芯片无关）
│   ├── LED_Flash.h/c    # LED 闪烁控制（5种模式，绑定LED0）
│   └── Timer_Counter.h/c # DWT 代码执行时间测量（us + ms）
├── Function/            # 功能实现层（组合 Hardware + Software，芯片无关）
│   ├── Serial_porting.h/c # 串口通信（Serial1/2，DMA收发，ABC/HEX双协议）
│   └── Con_Stepper.h/c  # 步进电机业务逻辑（🆕 存根，待编写）
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
| Hardware | RGB (GPIO开关) | ✅ 完成 | 2026-07-13 |
| Hardware | Serial_base (ABC/HEX协议) | ✅ 完成 | 2026-07-13 |
| Hardware | Buzzer | ⬜ 存根 | — |
| Hardware | Stepper_PWM (步进PWM驱动) | 🆕 框架已有 | 2026-07-13 |
| Software | MyPID | ✅ 完成 | 2026-07-13 |
| Tools | LED_Flash / Timer_Counter | ✅ 完成 | 2026-07-13 |
| Function | Serial_porting (DMA收发) | ✅ 完成 | 2026-07-13 |
| Function | Con_Stepper (步进业务逻辑) | 🆕 存根 | 2026-07-13 |
| Mode | Mode_G / Mode_1~4 | ✅ 完成 | 2026-07-13 |

## 代码约定

| 规则 | 说明 |
|------|------|
| Task 库 | **已弃用**，不用 `Task.h`，用静态计数器在 20ms/1ms Tick 中实现 |
| KEY0 | Mode_G 占用（单击=LED快闪，双击=换模式），测试只用 KEY1/KEY2 |
| OLED_Update | Mymain 末尾统一调用，各 Mode 不再调用 |
| 测试代码 | 放 Mode_2，简洁为主，写清测试流程和预期现象 |

## 串口配置

| 串口 | 引脚 | DMA TX | DMA RX | 协议 |
|------|------|--------|--------|------|
| USART1 (Serial1) | PA9/PA10 | DMA2_Stream7 | DMA2_Stream2 | ABC + HEX |
| USART2 (Serial2) | PA2/PA3 | DMA1_Stream6 | DMA1_Stream5 | ABC + HEX |
| USART3 | PB10/PB11 | — | — | 未使用 |
| USART6 | PC6/PC7 | — | — | 未使用 |

## TODO

- [ ] 下一步：协助编写步进电机驱动（Stepper_PWM）与业务逻辑（Con_Stepper）
