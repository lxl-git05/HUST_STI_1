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
| 步进电机1 PWM | TIM9 (CH1, PE5) | — |
| 步进电机2 PWM | TIM12 (CH1, PB14) | — |

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
| Hardware | Stepper_PWM (步进PWM驱动) | ✅ 完成 | 2026-07-13 |

### Stepper_PWM — 步进电机 PWM 驱动

#### 结构体 `Stepper_PWM_Typedef`

```c
typedef struct {
    // 驱动
    MyPWM_Typedef* PWM;          // PWM输出 (TIM9_CH1 / TIM12_CH1)
    MyGPIO_Typedef* GPIO_Dir;    // 方向引脚
    // 电机参数
    float pulse_angle;           // 每脉冲角度（度）
    int8_t Positive_Dir;         // 正方向（+1/-1）
    // 运行状态
    float Pos_Now;               // 当前绝对角度（度），脉冲中断累加
    float Pos_Tar;               // 目标角度（度）
    float Speed_Now;             // 当前速度（rpm），带符号
    float Speed_Tar;             // 目标速度（rpm），加速度ramp终点
    // 限位功能（纯软件，基于 Pos_Now）
    float Limit_Angle_Max;       // 正向限位角度
    float Limit_Angle_Min;       // 反向限位角度
    uint8_t Limit_Enable;        // 限位使能
    // 加速度控制
    float Acc_Val;               // 加速度步进（rpm/Tick），0=瞬时
    // PID
    Pid_Typedef PID_Angle;       // 角度PID控制器
} Stepper_PWM_Typedef;
```

#### API

| 函数 | 说明 |
|------|------|
| `Stepper_PWM_Init(pStepper, PWM, GPIO_Dir, pulse_angle, Positive_Dir)` | 初始化：绑定PWM/方向引脚，配置NVIC+更新中断，限位/加速度默认关闭 |
| `Stepper_PWM_Speed_Set(pStepper, Speed, acc)` | 速度控制：acc=0瞬时响应，acc>0存入Speed_Tar等待Tick ramp |
| `Stepper_PWM_Speed_Tick(pStepper)` | **20ms中断调用**：逐步将Speed_Now推向Speed_Tar |
| `Stepper_PWM_Stop(pStepper)` | 急停：PWM置0，Speed_Now清零 |
| `Stepper_PWM_Pulse_Count(pStepper)` | **脉冲中断调用**：更新Pos_Now + 第2层限位兜底 |
| `Stepper_PWM_Limit_Config(pStepper, max, min)` | 配置软件限位角度并启用 |
| `Stepper_PWM_Limit_Check(pStepper, speed)` | 限位询问：返回1=放行/0=拦截（反向退回始终放行） |
| `Stepper_PWM_Limit_LED_Update(void)` | 更新RGB限位指示灯 |

#### 双层限位

| 层 | 位置 | 机制 |
|----|------|------|
| 第1层 | `_Stepper_Apply_Speed` 入口 | Speed_Set/Speed_Tick 发出的速度被 Limit_Check 前置拦截 |
| 第2层 | `Pulse_Count` 脉冲中断 | 每个脉冲后检查 Pos_Now，超限立即 Stop |

#### 加速度架构

```
Speed_Set(target, acc)  →  存储 Speed_Tar, Acc_Val
                              │ acc=0 → 即时应用
                              │ acc>0 → 不做任何事
                              
Timer_20ms_Callback
  →  Speed_Tick()      →  每20ms: Speed_Now ±= Acc_Val, 趋近 Speed_Tar
  →  _Stepper_Apply_Speed   (内部: 限位→方向→频率→PWM硬件)
```

#### 中断链路

```
TIM9/TIM12 Update → IRQHandler → HAL_TIM_IRQHandler
  → HAL_TIM_PeriodElapsedCallback (MyTimer.c)
  → Timer_Stepper1/2_Pulse_Callback (Mode_G.c 弱回调重写)
  → Stepper_PWM_Pulse_Count       (更新 Pos_Now + 限位兜底)
```

#### 引脚

| 信号 | 步进电机1 (云台水平) | 步进电机2 (云台竖直) |
|------|---------------------|---------------------|
| Stp (脉冲) | PE5 (TIM9_CH1, AF3) | PB14 (TIM12_CH1, AF9) |
| Dir (方向) | PE6 | PB15 |
| En (使能) | PC0 | PC2 |

#### Con_Stepper — 步进电机业务逻辑

| 函数 | 说明 |
|------|------|
| `Stepper_Init()` | 初始化两台步进电机 + PID参数 + 限位配置（水平±120°，竖直±50°） |
| `Stepper_PID_Tick(Gap_Time_ms)` | 20ms调用：从Orange Pi读取x/y目标，PID计算，输出速度 |

| Hardware | Stepper_PWM (步进PWM驱动) | ✅ 完成 | 2026-07-13 |
| Software | MyPID | ✅ 完成 | 2026-07-13 |
| Tools | LED_Flash / Timer_Counter | ✅ 完成 | 2026-07-13 |
| Function | Serial_porting (DMA收发) | ✅ 完成 | 2026-07-13 |
| Function | Con_Stepper (步进业务逻辑) | ✅ 完成 | 2026-07-13 |
| Mode | Mode_G / Mode_1~4 | ✅ 完成 | 2026-07-13 |
| Function | Con_Stepper (步进业务逻辑) | ✅ 完成 | 2026-07-13 |
| Hardware | Encoder_Key (EC11编码器驱动) | ✅ 完成 | 2026-07-13 |
| Hardware | bsp_at24c02 (AT24C02软件I2C) | ✅ 完成 | 2026-07-13 |
| Software | at24c02_manager (AT参数管理) | ✅ 完成 | 2026-07-13 |
| Software | ParamEdit (OLED参数编辑器) | ✅ 完成 | 2026-07-13 |
| Function | Param_AT24C02 (业务参数持久化) | ✅ 完成 | 2026-07-13 |

## 代码约定

| 规则 | 说明 |
|------|------|
| Task 库 | **已弃用**，不用 `Task.h`，用静态计数器在 20ms/1ms Tick 中实现 |
| KEY0 | Mode_G 占用（单击=LED快闪，双击=换模式），测试只用 KEY1/KEY2 |
| OLED_Update | Mymain 末尾统一调用，各 Mode 不再调用 |
| 测试代码 | 放 Mode_2，简洁为主，写清测试流程和预期现象 |
| 模式记忆 | 切模式自动保存到 AT24C02，下次上电自动恢复。Mode_Null 不保存 |
| PARAM_FORCE | 手动推送默认值到 EEPROM 用，推送完必须重新注释 |

## 串口配置

| 串口 | 引脚 | DMA TX | DMA RX | 协议 |
|------|------|--------|--------|------|
| USART1 (Serial1) | PA9/PA10 | DMA2_Stream7 | DMA2_Stream2 | ABC + HEX |
| USART2 (Serial2) | PA2/PA3 | DMA1_Stream6 | DMA1_Stream5 | ABC + HEX |
| UART4 (Serial4) | PC10/PC11 | DMA1_Stream4 | DMA1_Stream2 | ABC + HEX |
| USART3 | PB10/PB11 | — | — | 未使用 |
| USART6 | PC6/PC7 | — | — | 未使用 |

## TODO

- [x] **目标角度精确控制**：✅ 2026-07-13 完成。`Stepper_PWM_Pos_Set(target_angle, max_speed, acc)` 实现T型速度曲线（等腰三角形/梯形），自动到位停止

---

## Encoder_Key — EC11 旋转编码器驱动

### 硬件引脚

| 信号 | 引脚 | EXTI | 说明 |
|------|------|------|------|
| EC11_S1 (A相) | PF3 | EXTI3, 下降沿 | 旋转编码器A相 |
| EC11_S2 (B相) | PF7 | EXTI9_5, 下降沿 | 旋转编码器B相 |
| EC11_Key (按键) | PF5 | KEY_3 | 编码器自带的轻触按键 |

### API

| 函数 | 说明 |
|------|------|
| `Encoder_Init()` | 初始化（空操作，EXTI 由 MX_GPIO_Init 配置） |
| `Encoder_Get()` | 读取累计增量并清零，返回 int16_t（正=CW，负=CCW） |

### 工作原理

- 下降沿中断触发后，读取另一相电平判断旋转方向
- `HAL_GPIO_EXTI_Callback` 在 `Encoder_Key.c` 中重写 HAL 弱回调
- **重要**：gpio.c 中 EC11 引脚必须配置为 `GPIO_MODE_IT_FALLING`。CubeMX 重新生成后需手动改回

---

## AT24C02 EEPROM 持久化 (三层架构)

### 引脚

| 信号 | 引脚 | 说明 |
|------|------|------|
| AT_SCL | PA4 | 软件 I2C 时钟 |
| AT_SDA | PA5 | 软件 I2C 数据 |
| 设备地址 | 0xA0(写)/0xA1(读) | 7位地址 0x50 左移1位 |

### 软件 I2C 时序

- NOP 延时: `* 19` (168MHz，校准自 F103 72MHz 的 `* 8`)
- 写周期等待: `HAL_Delay(5)` (5ms/字节)

### 三层架构

```
Param_AT24C02 (Function)    — 业务层：定义哪些变量需要持久化
at24c02_manager (Software)  — 管理层：注册、地址自动分配、读写协调
bsp_at24c02 (Hardware)      — 驱动层：软件 I2C 字节级读写
```

### API 速查

| 模块 | 关键函数 |
|------|----------|
| bsp_at24c02 | `AT24C02_Init()`, `AT24C02_WriteByte(addr, data)`, `AT24C02_ReadByte(addr)` |
| at24c02_manager | `AT_Manager_Register()`, `AT_Manager_Init()`, `AT_Manager_Write/Read()`, `AT_Manager_SaveAll()` |
| Param_AT24C02 | `Param_AT24C02_Init()`, `Param_AT24C02_SaveAll()`, `Param_AT24C02_EraseAll()` |

### 注册宏

```c
AT_PARAM_I8(&var, default)    // int8_t,  1 字节
AT_PARAM_I16(&var, default)   // int16_t, 2 字节
AT_PARAM_I32(&var, default)   // int32_t, 4 字节
AT_PARAM_F(&var, default)     // float,   4 字节
```

### EEPROM 地址分配

按注册顺序自动分配。当前 12 个参数使用约 35 字节，AT24C02 共 256 字节可用。详见 `Mode/Mode_1.c` 的 `s_AT_Params[]`。

**☆ 数组顺序一旦确定不要改。** 改顺序或增删会导致 EEPROM 旧数据和新布局错位。

**空白芯片检测**：读取值为 0xFF 时判定为空白，自动使用注册时的默认值。

---

## ParamEdit — OLED 参数编辑器

### 交互模型

| 操作 | 按键 | 说明 |
|------|------|------|
| 进入/退出编辑 | KEY_1 长按 1s | 切换编辑/正常模式 |
| 下一个参数 | KEY_2 单击 | 光标下移，到末尾回绕 |
| 上一个参数 | KEY_2 双击 | 光标上移，到开头回绕 |
| 修改参数值 | 旋转编码器 | 步长 × 旋转增量 |
| 保存到 EEPROM | KEY_3(编码器按键) 单击 | 仅保存已修改的 AT 参数 |

### API

| 函数 | 说明 |
|------|------|
| `Param_Init()` | 初始化/清空参数表 |
| `Param_Register(name, var, step, type)` | 注册一个参数（type: PARAM_INT8/INT16/INT32/FLOAT） |
| `Param_Loop()` | 主循环调用，运行状态机 |
| `Param_IsActive()` | 返回 1=编辑模式, 0=正常模式 |

### 初始化顺序

```
Param_AT24C02_Init()  → 注册 AT 参数 + 从 EEPROM 恢复值
Param_Init()          → 清空 ParamEdit 参数表
Param_Register(...)   → 注册 UI 参数（自动关联 AT 表）
```

### 注意事项

- `ParamEdit.c` 内部的 `OLED_Update()` 调用已注释掉，遵循本工程 "Mymain 末尾统一刷新" 的约定
- `MAX_PARAM = 30`，超出会跳过注册
- `KEY_0` 被 Mode_G 占用（模式切换），ParamEdit 不得使用

---

## 模式记忆（2026-07-14 实现）

### 机制

每次成功切换模式后，**自动将新模式保存到 AT24C02**，下次上电自动恢复。

```
Mymain 主循环:
  curr_mode = next_mode                     // 模式切换完成
  Param_AT24C02_Write(&curr_mode)           // ★ 自动记忆

Mode_G_Setup() 启动:
  Param_AT24C02_Init()                      // 从 EEPROM 恢复 curr_mode
  if (curr_mode > Null && curr_mode < End):
    next_mode = curr_mode; curr_mode = Null // 恢复启动目标
```

### 行为

| 场景 | 行为 |
|------|------|
| 首次上电（EEPROM 空白） | 进 Mode_1（AT 注册宏默认值） |
| 正常工作切模式后断电 | 下次上电恢复到上次关机时的模式 |
| 切到 Mode_Null 再断电 | Mode_Null 不保存，回到上一个有效模式 |

### 关键代码位置

- **保存**：`Top/Mymain.c` 第 48-50 行
- **恢复**：`Mode/Mode_G.c` `Mode_G_Setup()` 第 31-38 行
- **Mode_To_Next 已清理**：不再自行保存，统一由 Mymain 处理

---

## PARAM_FORCE 宏（2026-07-14 新增）

定义在 `Function/Param_AT24C02.h`：

```c
#define PARAM_FORCE(var, val)  do { (var) = (val); Param_AT24C02_Write(&(var)); } while(0)
```

**用途**：手动将代码默认值推送到 AT24C02。

**使用流程**：
1. 在 `Mode_G.c` 的 `Mode_G_Setup()` 中取消对应行注释并改值
2. 编译烧录一次（新值覆盖 EEPROM）
3. 重新注释掉（否则每次上电都覆盖用户调好的值）

**位置**：所有 `PARAM_FORCE` 集中在 `Mode/Mode_G.c` 的 `Mode_G_Setup()` 末尾。

### ★ 类型必须匹配

`AT_PARAM_xx` 字节数必须和变量 `sizeof` 一致，否则内存越界。

| 常见问题 | 原因 | 现象 |
|----------|------|------|
| OLED 显示超大数字 | I32（4字节）读 1 字节 enum | 读到相邻内存垃圾 |
| 相邻变量异常 | I32 写越界覆盖 | next_mode 被破坏 |

### 参数表布局规则

`s_AT_Params[]` 定义在 `Mode/Mode_1.c`。EEPROM 地址按数组声明顺序自动分配。**一旦确定不要改顺序/增删**，否则旧数据布局和新代码错位。必须改时先 `EraseAll()` + `PARAM_FORCE` 重推。

---

## 失能/关闭功能

| 目标 | 操作 |
|------|------|
| 关闭某个 Mode | `Mymain.c` 中对应 case 改为 `break;` |
| 关闭模式记忆（上电不恢复） | `Mymain.c` 注释掉 `Param_AT24C02_Write(&curr_mode)` |
| 关闭 AT24C02 整系统 | `AllHeader.c` 注释掉 `Param_AT24C02_Init()` |
| 关闭 ParamEdit UI | `Mode_1_Loop()` 注释掉 `Param_Loop()` |
| 关闭 PARAM_FORCE | 确保所有行已注释 |

