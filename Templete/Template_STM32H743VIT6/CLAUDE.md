# Template_STM32H743VIT6 — 项目文档

> 📋 **完整工程小结（架构 + 全部 API + 硬件总表 + 关键坑）见 [工程小结.md](./工程小结.md)** —— 新会话/新 Agent 先读它再动手。

## 项目概述

基于 STM32H743VIT6 的嵌入式模板工程，目标打造**跨芯片移植框架**。

- 芯片：STM32H743VIT6，主频 240 MHz
- 开发环境：Keil MDK-ARM
- 代码生成工具：STM32CubeMX
- 设计目标：上层业务代码（Hardware / Function / Mode）与芯片底层隔离，未来移植到 ESP32 / TI MSPM0 等芯片时，只需重写 `MySystem/` 层

## 目录结构

```
Template_STM32H743VIT6/
├── Core/Inc, Core/Src        # CubeMX 生成代码（main.c, gpio.c, tim.c, usart.c, stm32h7xx_it.c 等）
├── Drivers/                   # CMSIS + STM32 HAL 驱动库
├── MySystem/                  # ★ 芯片抽象层（移植时只需修改这里）
├── Hardware/                  # 硬件驱动（Key, OLED, Motor, Servo, Serial_base）
├── Function/                  # 功能层（Con_Motor, Con_Servo, Serial）
├── Software/                  # 算法层（MyPID, Task）
├── Mode/                      # 状态机（Mode_G, Mode_1~Mode_4）
├── Top/                       # 入口（Mymain.c, AllHeader.h）
├── MDK-ARM/                   # Keil 工程文件
└── 模板工程文件夹配置/         # AI 对话记录
```

### 各目录职责

| 目录 | 职责 | 依赖层次 |
|------|------|---------|
| `Core/` | CubeMX 生成的 HAL 初始化代码 | 底层 |
| `MySystem/` | 芯片抽象层，封装 HAL 为统一接口 | 依赖 Core/ |
| `Hardware/` | 硬件驱动（Key, OLED, Motor 等） | 依赖 MySystem/ |
| `Function/` | 功能模块（电机控制、舵机控制、串口） | 依赖 Hardware/ + Software/ |
| `Software/` | 纯算法（PID、任务调度） | 无硬件依赖 |
| `Mode/` | 应用状态机 | 依赖 Function/ + Tools/ |
| `Top/` | 应用入口，超级循环 | 依赖 Mode/ |

## MySystem 层设计规范

### 模块定义模式

每个模块（MyGPIO、MyPWM 等）遵循统一模式：

1. **结构体定义**：包含芯片句柄 + 配置参数
2. **extern 实例声明**：在 `.h` 中声明，在 `MySystem.c` 中定义
3. **操作函数**：Init / Write-Set / Read-Get 三件套
4. **回调函数**：使用 `__attribute__((weak))` 供用户覆盖

### 实例化模式

所有外设实例统一在 `MySystem.c` 中定义，`xxx.h` 中 extern 声明：

```c
// xxx.h
extern MyXXX_Typedef MyXXX_InstanceName;

// MySystem.c
MyXXX_Typedef MyXXX_InstanceName = { .handle = &htimX, ... };
```

### 回调函数模式

```c
// MyTimer.h
__attribute__((weak)) void Timer_1ms_Callback(void);
__attribute__((weak)) void Timer_20ms_Callback(void);
```

用户在任意 `.c` 文件中重定义这些函数即可覆盖，无需修改 MySystem 层。

## 当前 MySystem 层模块

### MyGPIO

- **结构体**：`{GPIO_Port, GPIO_Pin}`
- **API**：`MyGPIO_WritePin(gpio, isHigh)`、`MyGPIO_ReadPin(gpio)`

### MyPWM

- **结构体**：`{htimx, Channel, Compare_Max, Compare_Min}`
- **API**：`MyPWM_Init(pwm)`、`MyPWM_SetCompare(pwm, compare)`、`MyPWM_GetFre(pwm)`
- **特性**：SetCompare 自动限幅到 [Compare_Min, Compare_Max]

### MyEncoder

- **结构体**：`{htimx, time_Fre, total_cnt}`
- **API**：`MyEncoder_Init(encoder)`、`MyEncoder_Get_CNT(encoder)`、`MyEncoder_Get_Total_CNT(encoder)`
- **特性**：4 倍频，自动累积计数，支持清零

### MyTimer

- **API**：`Timer_Initial()` 启动 TIM16(20ms) 和 TIM17(1ms)
- **回调**：`Timer_1ms_Callback()`（weak）、`Timer_20ms_Callback()`（weak）

## 待补充模块

- [ ] **MyUART**：USART DMA+IDLE 收发抽象
- [ ] **MyI2C**：软件 I2C 抽象（参考 `Hardware/OLED.c`）
- [ ] **MyADC**：模数转换抽象
- [ ] **MySPI**：SPI 通信抽象（预留）

## 芯片移植步骤

当需要移植到新芯片时，只需修改：

1. `MySystem.h` 中的 `#include` 头文件（替换为新芯片的头文件）
2. `MySystem.h` 中的 `MySystem_Fre` 宏（改为新芯片主频）
3. `MySystem.c` 中的实例定义（替换 HAL 句柄为新芯片的句柄）
4. 新建 `MySystem_XXX.c/h` 实现各模块的具体函数

上层代码（Hardware / Function / Mode）**无需修改**。

## 调用层次（自底向上）

```
Core/Src/main.c（CubeMX 初始化）
    ↓
Top/Mymain.c（应用入口）
    ↓
Mode/（状态机）
    ↓
Function/（设备控制器）
    ↓
Hardware/（硬件驱动）
    ↓
MySystem/（芯片抽象层）← ★ 移植时只需改这里
    ↓
Drivers/STM32H7xx_HAL_Driver（ST 官方库）
```

## 关键文件

| 文件 | 作用 |
|------|------|
| `MySystem/MySystem.h` | 统一包含头文件 + 芯片主频定义 |
| `MySystem/MySystem.c` | 所有外设实例定义 |
| `Top/AllHeader.h` | 应用层统一包含头文件 |
| `Top/Mymain.c` | 应用入口，超级循环 |
| `Core/Src/main.c` | CubeMX 入口，初始化时钟/MPU/外设 |

## Serial 模块（已重构）

### 文件结构

```
Hardware/
└── Serial_base.h/c    ← 协议定义、宏、数据结构
Function/
└── Serial_porting.h/c ← 外设实例、初始化、Idle中断、收发
```

### 协议格式

**ABC协议（上位机调参）**
```
[@][DATA...][$][#]
```
- 帧头：`@`，帧尾：`$#`
- 用途：VOFA 发送指令调参

**HEX协议（设备间通信）**
```
[0xFF][0xAA][LEN][D1_H][D1_L][D1_CK][D2_H][D2_L][D2_CK]...[DN_H][DN_L][DN_CK][0x55][0xFE]
```
- 帧头：`0xFF 0xAA`，帧尾：`0x55 0xFE`
- **LEN** = 字个数（每个字 = 1个int16_t = 2字节原始数据）
- 每字占3字节：`D_H` + `D_L` + `D_CK`（校验码 = `D_H ^ D_L`）
- **单字校验失败**：该位置保留原值，不更新

### 重要变量说明

| 变量 | 含义 |
|------|------|
| `LEN` | 帧中第3字节，表示**字个数**（非高低位对个数，非字节数） |
| `Size` | 本次Idle中断接收到的**总字节数** |
| `needed_len` | 完整帧所需最小字节数 = `2 + 1 + LEN*3 + 2` |
| `tail_idx` | 帧尾起始位置 = `2 + LEN*3`（字节偏移量） |

### API

```c
// 初始化
void Serial_Porting_Init(Serial_Typedef *pSerial, USART_TypeDef *Instance, UART_HandleTypeDef *huart);

// ABC协议（保持原接口）
uint8_t Serial_GetNewPackageFlag_ABC(Serial_Typedef *pSerial);
bool Serial_SetFloatData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, float *Data);
bool Serial_SetIntData(Serial_Typedef *pSerial, char *KeyWord, char *cmd, int *Data);
bool Serial_Check_Str(Serial_Typedef *pSerial, char *KeyWord);

// HEX协议
int16_t Serial_GetHexData(Serial_Typedef *pSerial, uint8_t index);  // 获取第index个字
uint8_t Serial_GetHexLen(Serial_Typedef *pSerial);                  // 获取实际字数
bool Serial_IsHexFrameValid(Serial_Typedef *pSerial);               // 本帧是否有效

// 错误查询
Serial_Error_Typedef Serial_Error_Get(Serial_Typedef *pSerial);
void Serial_Error_Clear(Serial_Typedef *pSerial);

// 发送
void Serial_Porting_Printf(Serial_Typedef *pSerial, const char *fmt, ...);
```
