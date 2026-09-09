# Stepper_PWM 步进电机驱动说明书

> **适用芯片**: STM32F407ZGT6 (168MHz)  
> **可移植**: STM32F1/F4/H7 全系列（仅需修改 `MySystem/` 层）  
> **版本**: 2026-07-14  
> **依赖**: MySystem 抽象层（MyPWM / MyGPIO / MyTimer）

---

## 目录

1. [概述](#1-概述)
2. [硬件资源](#2-硬件资源)
3. [架构设计](#3-架构设计)
4. [快速上手](#4-快速上手)
5. [API 参考](#5-api-参考)
6. [驱动原理](#6-驱动原理)
7. [双层限位保护](#7-双层限位保护)
8. [速度模式与位置模式互斥](#8-速度模式与位置模式互斥)
9. [中断链路](#9-中断链路)
10. [跨芯片移植指南](#10-跨芯片移植指南)
11. [完整例程](#11-完整例程)
12. [常见问题排查](#12-常见问题排查)

---

## 1. 概述

`Stepper_PWM` 是一个基于 STM32 定时器 PWM 的步进电机驱动库，核心能力：

| 功能 | 说明 |
|------|------|
| **速度模式** | 开环速度控制，支持加速度 ramp（1ms 粒度丝滑调速） |
| **位置模式** | T 型/三角形速度曲线精确角度定位，基于匀加速运动公式规划 |
| **快速到位** | acc=0 时直接以最大速度运行，脉冲中断级停止（零延迟） |
| **双层限位** | 软件角度限位，前置门禁 + 脉冲中断兜底，防止超程 |
| **多电机** | 同结构体支持任意数量步进电机实例 |
| **芯片无关** | 通过 MySystem 抽象层隔离 HAL，换芯片只改映射表 |

### 文件清单

```
Hardware/Stepper_PWM.h      — 结构体 + API 声明
Hardware/Stepper_PWM.c      — 全部驱动实现
Function/Con_Stepper.h/c    — 业务层（多电机初始化 + PID 控制）
Mode/Mode_G.c               — 中断回调注册（1ms Tick + 脉冲中断）
```

---

## 2. 硬件资源

### 2.1 引脚分配 (F407ZGT6)

| 信号 | 步进电机1 (水平) | 步进电机2 (竖直) |
|------|:---------------:|:---------------:|
| Stp (脉冲) | **PE5** — TIM9_CH1, AF3 | **PB14** — TIM12_CH1, AF9 |
| Dir (方向) | **PE6** — GPIO | **PB15** — GPIO |
| En (使能) | PC0 — GPIO | PC2 — GPIO |

### 2.2 定时器参数

| 参数 | 电机1 (TIM9) | 电机2 (TIM12) |
|------|:-----------:|:------------:|
| 总线 | APB2 | APB1 |
| 总线时钟 | 168 MHz | 84 MHz |
| 预分频器 (PSC) | 167 (→1 MHz) | 83 (→1 MHz) |
| 计数时钟 | **1 MHz** | **1 MHz** |
| ARR 范围 | 1 ~ 65535 | 1 ~ 65535 |
| 最小频率 | ~15.26 Hz | ~15.26 Hz |
| 最大频率 | 500 kHz | 500 kHz |

### 2.3 电机参数 (默认配置)

```
步进角:     1.8° / 脉冲
细分:       16 细分
脉冲当量:   0.1125° / 脉冲  (360° / 3200 脉冲)
编码器:     无（开环控制，通过脉冲计数推算角度）
```

---

## 3. 架构设计

### 3.1 分层定位

```
┌──────────────────────────────────────────┐
│  应用层 (Mode_2 / Mode_3 / ...)          │  ← 调用 Pos_Set / Speed_Set
├──────────────────────────────────────────┤
│  业务层 (Con_Stepper)                     │  ← 多电机初始化 + PID
├──────────────────────────────────────────┤
│  驱动层 (Stepper_PWM)  ★ 本文档范围      │  ← 速度/位置/限位
├──────────────────────────────────────────┤
│  抽象层 (MySystem: MyPWM/MyGPIO/MyTimer) │  ← 芯片隔离
├──────────────────────────────────────────┤
│  HAL 层 (CubeMX: tim/gpio/...)           │  ← 寄存器操作
└──────────────────────────────────────────┘
```

### 3.2 核心结构体

```c
typedef struct {
    // ===== 硬件绑定 =====
    MyPWM_Typedef* PWM;              // PWM 输出 (TIM9_CH1 / TIM12_CH1)
    MyGPIO_Typedef* GPIO_Dir;        // 方向引脚

    // ===== 电机参数 =====
    float pulse_angle;               // 每脉冲角度 (度), 默认 0.1125
    int8_t Positive_Dir;             // 正方向 (+1 或 -1)

    // ===== 运行状态 =====
    float Pos_Now;                   // 当前绝对角度 (脉冲中断累加)
    float Pos_Tar;                   // 目标角度 (应用层设定)
    float Speed_Now;                 // 当前速度 rpm (带符号, 即实际方向)
    float Speed_Tar;                 // 目标速度 rpm (加速度 ramp 终点)

    // ===== 限位 =====
    float   Limit_Angle_Max;         // 正向限位角度
    float   Limit_Angle_Min;         // 反向限位角度
    uint8_t Limit_Enable;            // 限位使能 (1=开)

    // ===== 速度模式 =====
    float Acc_Val;                   // 加速度 rpm/s (0=瞬时)

    // ===== 位置模式 =====
    float   Pos_MaxSpeed;            // 运动最大速度 rpm
    float   Pos_Acc;                 // 加速度 rpm/s
    uint8_t Pos_Phase;               // 阶段: 0=空闲 1=加速 2=匀速 3=减速
    int32_t Pos_TotalSteps;          // 运动总步数
    int32_t Pos_AccSteps;            // 加速段步数
    int32_t Pos_CruiseSteps;         // 匀速段步数
    int32_t Pos_StepCnt;             // 当前已走步数 (脉冲中断累加)
    int8_t  Pos_MoveDir;             // 运动方向 (+1/-1)
    float   Pos_StartAngle;          // 起始角度
    float   Pos_TargetAngle;         // 目标绝对角度

    // ===== PID (外部视觉闭环) =====
    Pid_Typedef PID_Angle;           // 位置式 PID, 输入角度→输出速度
} Stepper_PWM_Typedef;
```

### 3.3 全局实例

```c
extern Stepper_PWM_Typedef Stepper1;  // 水平云台
extern Stepper_PWM_Typedef Stepper2;  // 竖直云台
```

---

## 4. 快速上手

### 4.1 最小初始化

```c
#include "Con_Stepper.h"

void main(void)
{
    // HAL + 时钟初始化 (CubeMX 生成)
    HAL_Init();
    SystemClock_Config();
    // ...

    // 一步初始化：电机 + PID + 限位
    Stepper_Init();

    // 启动定时器中断 (TIM6 1ms + TIM7 20ms)
    Timer_Initial();

    while (1);
}
```

`Stepper_Init()` 内部做的事：

```c
void Stepper_Init(void)
{
    // 绑定硬件、配置 NVIC、使能脉冲中断
    Stepper_PWM_Init(&Stepper1, &MyPWM_Stepper1, &MyGPIO_Stepper_Dir,
                      0.1125f, STEPPER_DIR_P);
    Stepper_PWM_Init(&Stepper2, &MyPWM_Stepper2, &MyGPIO_Stepper2_Dir,
                      0.1125f, STEPPER_DIR_P);

    // 配置 PID (比例 + 微分, 积分=0)
    PID_Init(&Stepper1.PID_Angle, 0.217f, 0.0f, 0.829f, 100,-100, 1000);
    PID_Init(&Stepper2.PID_Angle, 0.081f, 0.0f, 0.224f, 100,-100, 1000);

    // 软件限位
    Stepper_PWM_Limit_Config(&Stepper1,  120.0f, -120.0f);  // 水平 ±120°
    Stepper_PWM_Limit_Config(&Stepper2,   50.0f,  -50.0f);  // 竖直  ±50°
}
```

### 4.2 第一个运动指令

```c
// 以 60 rpm 速度持续正转 (无加速度)
Stepper_PWM_Speed_Set(&Stepper1, 60.0f, 0);

// 以 120 rpm/s 加速度 ramp 到 100 rpm
Stepper_PWM_Speed_Set(&Stepper1, 100.0f, 120.0f);

// 绝对角度: 转到 90° (60rpm, 120rpm/s² 加速)
Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, 60.0f, 120.0f);

// 相对角度: 再转 +30°
Stepper_PWM_Pos_Set_Rel(&Stepper1, 30.0f, 60.0f, 120.0f);

// 急停
Stepper_PWM_Stop(&Stepper1);
```

---

## 5. API 参考

### 5.1 初始化

```c
void Stepper_PWM_Init(
    Stepper_PWM_Typedef* pStepper,   // 电机实例指针
    MyPWM_Typedef*       PWM,        // PWM 抽象实例
    MyGPIO_Typedef*      GPIO_Dir,   // 方向引脚
    float                pulse_angle,// 每脉冲角度(度)
    int8_t               Positive_Dir// 正方向 (+1 或 -1)
);
```

**行为**:
1. 绑定 PWM 通道和方向 GPIO
2. 初始化运行时变量为 0（角度、速度、限位关闭、位控空闲）
3. 配置 NVIC 中断优先级 = 1,0（低于 1ms Tick 的优先级 0）
4. 使能 TIM 更新中断（每个 PWM 周期触发一次 → 脉冲计数）
5. 初始 PWM 占空比为 0（无脉冲输出）

**使用**:
```c
// F407 电机1: TIM9_CH1 (PE5), Dir=PE6, 1.8°/16细分=0.1125°/脉冲
Stepper_PWM_Init(&Stepper1, &MyPWM_Stepper1, &MyGPIO_Stepper_Dir,
                  0.1125f, STEPPER_DIR_P);
```

**注意事项**:
- `Positive_Dir` 决定了"正转"对应的硬件方向电平。设为 `STEPPER_DIR_P`(+1) 表示正速度 → 方向引脚高电平；设为 `STEPPER_DIR_N`(-1) 则反转
- 初始化后电机处于停止状态（PWM=0），需要调用 `Speed_Set` 或 `Pos_Set` 才会运动
- 限位默认关闭（`Limit_Enable=0`），需调用 `Limit_Config` 启用

---

### 5.2 速度模式

#### 5.2.1 `Stepper_PWM_Speed_Set` — 设置目标速度

```c
void Stepper_PWM_Speed_Set(
    Stepper_PWM_Typedef* pStepper,
    float Speed,    // 目标速度 (rpm), 正=正转, 负=反转, 0=停止
    float acc       // 加速度 (rpm/s), ≤0.001 = 瞬时响应
);
```

**行为**:

| acc 值 | 行为 |
|--------|------|
| `≤ 0.001f` | **瞬时响应**: 立即调用 `_Stepper_Apply_Speed` 输出目标速度 |
| `> 0.001f` | **ramp 模式**: 仅存储 `Speed_Tar` 和 `Acc_Val`，由 1ms `Speed_Tick` 逐步逼近 |

**副作用**: 如果有正在进行的位控运动 (`Pos_Phase != IDLE`)，会被**立即取消**。

**使用**:
```c
// 瞬时起步: 立即以 80 rpm 正转
Stepper_PWM_Speed_Set(&Stepper1, 80.0f, 0);

// 平滑加速: 以 200 rpm/s² 加速到 150 rpm
Stepper_PWM_Speed_Set(&Stepper1, 150.0f, 200.0f);

// 反转: 以 100 rpm/s² 减速到 -50 rpm（先减到 0 再反向加速）
Stepper_PWM_Speed_Set(&Stepper1, -50.0f, 100.0f);

// 停止 (瞬时)
Stepper_PWM_Speed_Set(&Stepper1, 0, 0);
```

#### 5.2.2 `Stepper_PWM_Speed_Tick` — 加速度 ramp (内部)

```c
void Stepper_PWM_Speed_Tick(Stepper_PWM_Typedef* pStepper);
```

**调用位置**: `Mode_G.c → Timer_1ms_Callback()`，**自动调用，用户无需关心**。

**行为**:
1. 如果 `Pos_Phase != IDLE`（位控模式激活）→ 立即返回（避免与位控冲突）
2. 如果 `Acc_Val ≤ 0.001` → 返回（瞬时模式无需 ramp）
3. 计算步长: `step = Acc_Val / 1000.0f`（rpm/s → rpm/ms）
4. 将 `Speed_Now` 以 `step` 增量向 `Speed_Tar` 逐步逼近
5. 每次更新后调用 `_Stepper_Apply_Speed` 输出到硬件

---

### 5.3 位置模式

#### 5.3.1 `Stepper_PWM_Pos_Set_Abs` — 绝对角度旋转

```c
void Stepper_PWM_Pos_Set_Abs(
    Stepper_PWM_Typedef* pStepper,
    float target_angle,  // 目标绝对角度 (度), 相对于零点
    float max_speed,     // 最大速度 (rpm), 取绝对值
    float acc            // 加速度 (rpm/s), <0.001 = 快速模式
);
```

**预计算流程** (调用时同步完成):

```
1. delta = target_angle - Pos_Now
2. |delta| < pulse_angle/2 → 已在目标, 直接返回
3. total_steps = |delta| / pulse_angle
4. 如果 acc < 0.001: → 快速模式 (直接 max_speed, 脉冲中断到位停止)
5. 否则:
   acc_steps = (max_speed² - v_min²) × 3 / (acc × pulse_angle)
   如果 2 × acc_steps ≥ total_steps:
     → 三角形 (短行程): acc_steps = total_steps/2, cruise_steps=0
   否则:
     → 梯形 (长行程): cruise_steps = total_steps - 2 × acc_steps
6. 存储参数 → Pos_Phase = ACCEL → 以 v_min 启动
```

**副作用**: 如果有正在进行的速度 ramp (`Speed_Tar != 0`)，会被**取消**。

**使用**:
```c
// 转到绝对角度 90° (60rpm, 120rpm/s²)
Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, 60.0f, 120.0f);

// 快速到位 (直接 120rpm, 脉冲中断精确停止)
Stepper_PWM_Pos_Set_Abs(&Stepper1, -45.0f, 120.0f, 0);

// 三角形曲线 (短行程 5°, 加速度小 → 自动切三角形)
Stepper_PWM_Pos_Set_Abs(&Stepper1, 95.0f, 80.0f, 30.0f);
```

#### 5.3.2 `Stepper_PWM_Pos_Set_Rel` — 相对角度旋转

```c
void Stepper_PWM_Pos_Set_Rel(
    Stepper_PWM_Typedef* pStepper,
    float relative_angle,  // 相对角度 (度), 正=正方向
    float max_speed,       // 最大速度 (rpm)
    float acc              // 加速度 (rpm/s)
);
```

**本质**: `Pos_Set_Abs(pStepper, Pos_Now + relative_angle, max_speed, acc)` 的包装。

**使用**:
```c
// 从当前位置逆时针转 30°
Stepper_PWM_Pos_Set_Rel(&Stepper1, 30.0f, 60.0f, 120.0f);

// 顺时针转 90° (负值)
Stepper_PWM_Pos_Set_Rel(&Stepper1, -90.0f, 80.0f, 100.0f);
```

#### 5.3.3 `Stepper_PWM_Pos_Tick` — 位控状态机 (内部)

```c
void Stepper_PWM_Pos_Tick(Stepper_PWM_Typedef* pStepper);
```

**调用位置**: `Mode_G.c → Timer_1ms_Callback()`，紧接 `Speed_Tick` 之后。

**状态机**:

```
                   ┌──────────────────────────────┐
                   │         POS_PHASE_IDLE        │ ← 不动
                   └──────────┬───────────────────┘
                              │ Pos_Set_Abs / Pos_Set_Rel
                              ▼
                   ┌──────────────────────────────┐
                   │        POS_PHASE_ACCEL        │ speed += acc/1000 per ms
                   │   step_cnt < AccSteps         │ 方向×v_min → ramp 到 max_speed
                   └──────────┬───────────────────┘
                              │ step_cnt ≥ AccSteps
                              ▼
              ┌───────────────────────────────┐
              │  CruiseSteps > 0 ?            │
              └───┬───────────────────┬───────┘
                  │ YES               │ NO (三角形)
                  ▼                   ▼
    ┌──────────────────────┐  直接进入 DECEL
    │   POS_PHASE_CRUISE   │  speed = max_speed (恒定)
    │ step < Acc+Cruise    │
    └──────────┬───────────┘
               │ step ≥ AccSteps + CruiseSteps
               ▼
    ┌──────────────────────┐
    │   POS_PHASE_DECEL    │  speed -= acc/1000 per ms
    │  step < TotalSteps   │  floor 到 v_min, 脉冲中断精确停止
    └──────────┬───────────┘
               │ step ≥ TotalSteps (脉冲中断)
               ▼
    ┌──────────────────────┐
    │   POS_PHASE_IDLE     │  Stepper_PWM_Stop
    └──────────────────────┘
```

**每 1ms 执行**:
1. 读取 `Pos_StepCnt`（由脉冲中断实时更新）
2. 检查是否跨越阶段阈值 → 切换 `Pos_Phase`
3. 根据当前阶段调整 `Speed_Now`:
   - ACCEL: `speed_mag += Pos_Acc / 1000`
   - CRUISE: `speed_mag = Pos_MaxSpeed`
   - DECEL: `speed_mag -= Pos_Acc / 1000`
4. 调用 `_Stepper_Apply_Speed` 输出

**快速模式** (`Pos_Acc < 0.001f`): 直接 return，不做任何 ramp。速度已在 `Pos_Set_Abs` 中直接设置，脉冲中断负责停止。

---

### 5.4 停止

```c
void Stepper_PWM_Stop(Stepper_PWM_Typedef* pStepper);
```

**行为**:
1. PWM 比较值设为 0 → 无脉冲输出
2. `Speed_Now = 0` → 脉冲中断不再累加位置
3. **不改变** `Pos_Phase`（需应用层手动设为 `POS_PHASE_IDLE` 以彻底取消位控）

**使用**:
```c
// 紧急停止
Stepper_PWM_Stop(&Stepper1);
Stepper1.Pos_Phase = POS_PHASE_IDLE;  // 同时取消位控状态
```

---

### 5.5 脉冲中断处理 (内部)

```c
void Stepper_PWM_Pulse_Count(Stepper_PWM_Typedef* pStepper);
```

**调用位置**: TIM 更新中断回调 → `Timer_Stepper1/2_Pulse_Callback`（自动）

**行为** (每个脉冲执行一次):

```
1. Speed_Now == 0 → return (无脉冲时跳过)
2. 计算方向 → Pos_Now ±= pulse_angle
3. 如果 Pos_Phase != IDLE:
   a. Pos_StepCnt++
   b. DECEL 且 StepCnt ≥ TotalSteps → Stop + IDLE (精确到位)
   c. 快速模式 (Pos_Acc < 0.001) 且角度越过目标 → Stop + IDLE
4. Layer2 限位检查 (始终有效) → 超限则 Stop
5. 更新 RGB 限位指示灯
```

**关键**: 步骤 3b 和 3c 在**脉冲中断内**停止，比 1ms Tick 更及时，确保精确角度控制。

---

### 5.6 限位保护

#### `Stepper_PWM_Limit_Config` — 配置限位

```c
void Stepper_PWM_Limit_Config(
    Stepper_PWM_Typedef* pStepper,
    float Limit_Angle_Max,   // 正向限位 (度)
    float Limit_Angle_Min    // 反向限位 (度)
);
```

#### `Stepper_PWM_Limit_Check` — 前置检查 (内部)

```c
uint8_t Stepper_PWM_Limit_Check(
    Stepper_PWM_Typedef* pStepper,
    float target_speed       // 目标速度, 用于判断方向
);
// 返回: 1=允许运动, 0=被限位阻挡
```

#### `Stepper_PWM_Limit_LED_Update` — LED 指示 (内部)

```c
void Stepper_PWM_Limit_LED_Update(void);
// 任一电机在限位边界 → RGB 亮红灯
// 均未限位 → RGB 灭
```

---

## 6. 驱动原理

### 6.1 PWM 频率 → 转速转换

步进电机每收到一个脉冲，转动一个固定角度。转速与脉冲频率成正比：

```
freq_hz = speed_rpm × (360° / pulse_angle) / 60
        = speed_rpm × 3200 / 60          (pulse_angle=0.1125° 时)
        ≈ speed_rpm × 53.33
```

**ARR 计算** (TIM9/12 计数时钟 = 1 MHz):

```
arr = 1_000_000 / freq_hz - 1
    = 60 × 1_000_000 / (speed_rpm × 360 / pulse_angle) - 1
```

限制: `arr ∈ [1, 65535]`
- arr=1 → freq=500kHz → 最大速度 ≈ 9375 rpm
- arr=65535 → freq≈15.26Hz → 最小速度 ≈ 0.286 rpm
- `speed < 0.01 rpm` 视为停止（PWM 输出 0）

**代码位置**: `_Stepper_Apply_Speed()` 第 81-89 行。

### 6.2 速度 Ramp 原理

```
Acc_Val = 120 rpm/s  意味着每秒速度变化 120 rpm

1ms Tick:  step = 120 / 1000 = 0.12 rpm/ms
20ms Tick: step = 120 / 50  = 2.4  rpm/20ms    (如果20ms调度)

每 Tick 将 Speed_Now 以 step 增量向 Speed_Tar 逐步逼近。
```

**代码位置**: `Stepper_PWM_Speed_Tick()`。

### 6.3 位置模式算法推导

#### 6.3.1 数学基础

从连续域微分关系推导步数与速度的关系：

```
已知:
  dv/dt = a           (加速度定义, a 单位 rpm/s²)
  dN/dt = 6v / φ      (脉冲速率 = 角速度 / 脉冲当量, φ=pulse_angle)

链式法则:
  dv/dN = (dv/dt) / (dN/dt) = a / (6v/φ) = a·φ / (6v)

分离变量:
  v · dv = (a·φ / 6) · dN

积分 (从 v₀ 到 v, 从 0 到 N):
  (v² - v₀²) / 2 = (a·φ / 6) · N

最终公式:
  v² = v₀² + (a·φ / 3) · N          (1) 速度-步数关系
  N  = (v² - v₀²) × 3 / (a × φ)    (2) 步数-速度关系
```

**验证** (φ=0.1125, a=100, v₀=0, v=100):
```
N = 10000 × 3 / (100 × 0.1125) = 30000 / 11.25 ≈ 2667 步
时间: 2667 / (50rpm均速 × 3200/60) ≈ 2667 / 2667 = 1.0s ✓
```

#### 6.3.2 场景判定

在 `Pos_Set_Abs` 中预计算:

```
1. total_steps = |target - current| / φ
2. acc_steps_full = (max_speed² - v_min²) × 3 / (acc × φ)

3. if 2 × acc_steps_full ≥ total_steps:
     → 场景 B (三角形): 行程不足, 无法达到 max_speed
       acc_steps  = total_steps / 2
       cruise_steps = 0
   else:
     → 场景 A (梯形): 行程充足, 能加速到 max_speed
       acc_steps  = acc_steps_full
       cruise_steps = total_steps - 2 × acc_steps
```

#### 6.3.3 速度曲线示意

**场景 A (梯形)** — 长行程:
```
Speed
  ↑
  │        ┌─────────────────┐
  │       ╱                   ╲
  │      ╱                     ╲
  │     ╱                       ╲
  │    ╱                         ╲
  │   ╱                           ╲
  └──┴─────┴─────────────────┴─────┴──→ Steps
     0    acc              acc+    total
          steps            cruise

  加速段 (acc_steps)  →  匀速段 (cruise_steps)  →  减速段 (acc_steps)
```

**场景 B (三角形)** — 短行程:
```
Speed
  ↑
  │        ╱ ╲
  │       ╱   ╲
  │      ╱     ╲
  │     ╱       ╲
  │    ╱         ╲
  │   ╱           ╲
  └──┴─────────────┴──→ Steps
     0           total/2  total

  加速段 (total/2)  →  减速段 (total - total/2)
  无匀速段
```

---

## 7. 双层限位保护

```
第1层 (前置门禁) — _Stepper_Apply_Speed 入口
  ├─ 速度模式: Limit_Check 检查目标方向是否超限
  │   → 超限则拒绝 Speed_Set, 电机不启动
  └─ 位控模式: ★ 跳过 (Pos_Phase != IDLE), 信任位置目标

第2层 (脉冲中断兜底) — Pulse_Count 每个脉冲后
  ├─ 所有模式通用 (速度 + 位控)
  ├─ 每个脉冲后检查 Pos_Now 是否越过 Limit_Angle
  └─ 超限 → Stepper_PWM_Stop 立即停止 + RGB 亮红灯
```

**设计考量**:
- Layer1 在位控模式下跳过: 因为位控目标角度可能超出限位（如从限位内走到限位边缘是合法的），且位控有自己精确的到位停止机制
- Layer2 始终保留: 作为最后的硬件级安全网，捕获任何意外超程

---

## 8. 速度模式与位置模式互斥

```
Speed_Set 被调用
  → Pos_Phase = IDLE          (取消位控)
  → Pos_StepCnt = 0
  → 开始速度 ramp

Pos_Set_Abs / Pos_Set_Rel 被调用
  → Speed_Tar = 0             (取消速度 ramp)
  → Acc_Val = 0
  → 开始位控规划

Speed_Tick (1ms)
  → if Pos_Phase != IDLE: return   (不干预位控)

Pos_Tick (1ms)
  → if Pos_Phase == IDLE: return   (不干预速度)
```

**同时只有一个模式在控制速度输出。**

---

## 9. 中断链路

### 9.1 定时器 Tick

```
TIM6 (1ms, prio=0 最高)
  → HAL_TIM_PeriodElapsedCallback
  → Timer_1ms_Callback (Mode_G.c)
      ├─ Key_Tick()
      ├─ Flash_Mode_Tick()
      ├─ Speed_Tick(&Stepper1)  ← 速度 ramp
      ├─ Speed_Tick(&Stepper2)
      ├─ Pos_Tick(&Stepper1)    ← 位控状态机
      └─ Pos_Tick(&Stepper2)

TIM7 (20ms, prio=0)
  → HAL_TIM_PeriodElapsedCallback
  → Timer_20ms_Callback (Mode_G.c)
      ├─ Oran_Update()          ← 香橙派通信
      └─ Mode_X_Tick()          ← 模式相关 (PID/打印)
```

### 9.2 脉冲中断

```
TIM9 更新 (Stepper1, prio=1)
  → TIM1_BRK_TIM9_IRQHandler
  → HAL_TIM_PeriodElapsedCallback
  → Timer_Stepper1_Pulse_Callback (Mode_G.c)
  → Stepper_PWM_Pulse_Count(&Stepper1)
      ├─ Pos_Now ±= pulse_angle
      ├─ 位控: Pos_StepCnt++ / 到位停止
      ├─ Layer2 限位检查
      └─ Limit_LED_Update

TIM12 更新 (Stepper2, prio=1) — 同上
```

**优先级**: TIM6/TIM7 (prio=0, 最高) > TIM9/TIM12 (prio=1)。确保 Tick 不丢，脉冲计数在 Tick 之间处理。

---

## 10. 跨芯片移植指南

只需要修改 **`MySystem/` 层**，其余代码完全不变。

### 步骤 1: `MySystem.h` — 改主频

```c
// 原来 (F407, 168MHz)
#define MySystem_Fre 168000000

// 改为 (H743, 480MHz)
#define MySystem_Fre 480000000
```

### 步骤 2: `MySystem.c` — 改 PWM 和 GPIO 实例

```c
// 原来 (F407)
MyPWM_Typedef MyPWM_Stepper1 = { &htim9,  TIM_CHANNEL_1, 1000, 0 };
MyGPIO_Typedef MyGPIO_Stepper_Dir = { GPIOE, GPIO_PIN_6 };

// 改为 (H743, 使用 TIM8_CH1 和 PA5)
MyPWM_Typedef MyPWM_Stepper1 = { &htim8,  TIM_CHANNEL_1, 1000, 0 };
MyGPIO_Typedef MyGPIO_Stepper_Dir = { GPIOA, GPIO_PIN_5 };
```

### 步骤 3: `MyTimer.c` — 改定时器实例

```c
// TIM6/7 通常在所有 STM32 系列都存在，无需修改
// 如果换了不同编号的定时器，修改 HAL_TIM_PeriodElapsedCallback 中的判断即可
```

### 步骤 4: CubeMX 重新生成 `Core/`

- 用新芯片的 CubeMX 工程生成 `tim.c`, `gpio.c`, `stm32f4xx_it.c` 等
- 确保 TIM 预分频器使计数时钟 ≈ 1 MHz（驱动内部会自动读取 PSC 值）

### 步骤 5: `Stepper_PWM.c` 中的 TIM 类型判断

```c
// 第 82 行: 如果新芯片的 TIM9 不在 APB2, 需修改此映射
uint32_t tim_base = (pStepper->PWM->htimx->Instance == TIM9)
    ? MySystem_Fre        // APB2
    : MySystem_Fre / 2;   // APB1
```

更健壮的写法（直接读总线时钟，不硬编码 TIM 号）:
```c
uint32_t tim_base = HAL_RCC_GetPCLK2Freq();  // 或 HAL_RCC_GetPCLK1Freq()
```

### 步骤 6: NVIC 中断号

```c
// 第 51-57 行: 如果换了定时器, 需改 IRQn
if (pStepper->PWM->htimx->Instance == TIM9) {
    HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 1, 0);
```

---

## 11. 完整例程

### 例程 1: 基础速度控制

```c
// === 需求: 电机1 以 80 rpm 匀速正转, 按下 KEY1 停止 ===
#include "AllHeader.h"

void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Speed: %.1f rpm", Stepper1.Speed_Now);

    if (Key_Check(KEY_1, KEY_SINGLE)) {
        static int running = 0;
        running = !running;
        if (running)
            Stepper_PWM_Speed_Set(&Stepper1, 80.0f, 0);  // 瞬时起步
        else
            Stepper_PWM_Stop(&Stepper1);
    }
}
```

### 例程 2: 丝滑加速 + 减速

```c
// === 需求: KEY1 平滑加速到 120 rpm, KEY2 平滑减速到 -60 rpm ===
void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Speed: %+.1f rpm", Stepper1.Speed_Now);

    if (Key_Check(KEY_1, KEY_SINGLE))
        Stepper_PWM_Speed_Set(&Stepper1, 120.0f, 200.0f);  // 200 rpm/s ramp

    if (Key_Check(KEY_2, KEY_SINGLE))
        Stepper_PWM_Speed_Set(&Stepper1, -60.0f, 150.0f);  // 减速到反转
}
// 1ms Tick 自动处理 ramp, 无需额外代码
```

### 例程 3: 绝对定位 — 梯形曲线

```c
// === 需求: KEY1 转到 +90°, KEY2 转到 -90°, 60rpm, 120rpm/s² ===
// === 预期: 角度差=180°=1600步, 加速段≈296步, 全程梯形曲线 ===
void Mode_2_Loop(void)
{
    OLED_Printf(0, 0,  OLED_6X8, "Pos: %+.2f", Stepper1.Pos_Now);
    OLED_Printf(0, 16, OLED_6X8, "Phase: %d", Stepper1.Pos_Phase);

    if (Key_Check(KEY_1, KEY_SINGLE))
        Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, 60.0f, 120.0f);

    if (Key_Check(KEY_2, KEY_SINGLE))
        Stepper_PWM_Pos_Set_Abs(&Stepper1, -90.0f, 60.0f, 120.0f);
}
```

### 例程 4: 短行程 — 自动三角形曲线

```c
// === 需求: KEY1 相对转 5°, 80rpm, 30rpm/s² ===
// === 预期: 5°≈44步, 加速段=22步, 自动三角形 (无法达到80rpm) ===
void Mode_2_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Pos: %+.2f", Stepper1.Pos_Now);

    if (Key_Check(KEY_1, KEY_SINGLE))
        Stepper_PWM_Pos_Set_Rel(&Stepper1, 5.0f, 80.0f, 30.0f);
        // acceler=30 小, 行程=5° 短 → 2×acc_steps > total → 三角形
}
```

### 例程 5: 快速到位 (acc=0)

```c
// === 需求: KEY1 快速转到 45°, 直接 200rpm, 脉冲中断精确停止 ===
void Mode_2_Loop(void)
{
    if (Key_Check(KEY_1, KEY_SINGLE))
        Stepper_PWM_Pos_Set_Abs(&Stepper1, 45.0f, 200.0f, 0);
        // acc=0 → 快速模式: 直接 max_speed, 脉冲中断到位即停
}
```

### 例程 6: 循环往复运动

```c
// === 需求: KEY1 启动循环 — 右转180° → 左转180° → 循环 ===
// ===        KEY2 启动循环 — 去+90° → 去-180° → 循环 ===
// ===        参数: 120rpm, 60rpm/s² ===
typedef enum { NONE, A_FWD, A_REV, B_POS, B_NEG } SubMode;
static SubMode sm = NONE;

void Mode_2_Loop(void)
{
    // 启动
    if (Key_Check(KEY_1, KEY_SINGLE)) {
        sm = A_FWD;
        Stepper_PWM_Pos_Set_Rel(&Stepper1, 180.0f, 120.0f, 60.0f);
    }
    if (Key_Check(KEY_2, KEY_SINGLE)) {
        sm = B_POS;
        Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, 120.0f, 60.0f);
    }
    if (Key_Check(KEY_3, KEY_SINGLE)) {
        sm = NONE;
        Stepper_PWM_Stop(&Stepper1);
        Stepper1.Pos_Phase = POS_PHASE_IDLE;
    }

    // 自动循环: 到位后触发下一段
    if (sm != NONE && Stepper1.Pos_Phase == POS_PHASE_IDLE) {
        switch (sm) {
            case A_FWD: sm = A_REV;
                Stepper_PWM_Pos_Set_Rel(&Stepper1, -180.0f, 120.0f, 60.0f); break;
            case A_REV: sm = A_FWD;
                Stepper_PWM_Pos_Set_Rel(&Stepper1,  180.0f, 120.0f, 60.0f); break;
            case B_POS: sm = B_NEG;
                Stepper_PWM_Pos_Set_Abs(&Stepper1, -180.0f, 120.0f, 60.0f); break;
            case B_NEG: sm = B_POS;
                Stepper_PWM_Pos_Set_Abs(&Stepper1,   90.0f, 120.0f, 60.0f); break;
        }
    }

    OLED_Printf(0, 0, OLED_6X8, "Pos: %+.2f  Ph:%d", Stepper1.Pos_Now, Stepper1.Pos_Phase);
}
```

### 例程 7: PID 视觉伺服 (Con_Stepper 业务层)

```c
// === 需求: 从香橙派接收目标角度 (x_tar/y_tar), PID 闭环控制 ===
// === 已在 Con_Stepper.c 中实现, 20ms 周期运行 ===

void Stepper_PID_Tick(uint32_t Gap_Time_ms)
{
    // 电机1 (水平): 目标=x_tar, 实际=x_real (香橙派视觉测量)
    Stepper1.PID_Angle.realPoint_Now = x_real;
    Stepper1.PID_Angle.goalPoint     = x_tar;
    PID_Update(&Stepper1.PID_Angle, Stepper1.PID_Angle.realPoint_Now);
    Stepper_PWM_Speed_Set(&Stepper1, -Stepper1.PID_Angle.setPoint, Stepper1.Acc_Val);

    // 电机2 (竖直): 同理
    Stepper2.PID_Angle.realPoint_Now = y_real;
    Stepper2.PID_Angle.goalPoint     = y_tar;
    PID_Update(&Stepper2.PID_Angle, Stepper2.PID_Angle.realPoint_Now);
    Stepper_PWM_Speed_Set(&Stepper2, Stepper2.PID_Angle.setPoint, Stepper2.Acc_Val);
}

// PID 参数 (在 Stepper_Init 中配置):
// Stepper1 (水平): Kp=0.217, Ki=0.0, Kd=0.829, 输出限幅±100
// Stepper2 (竖直): Kp=0.081, Ki=0.0, Kd=0.224, 输出限幅±100
```

### 例程 8: 多电机同步

```c
// === 需求: 两电机同时执行不同位置运动, 等待全部到位 ===
void Mode_2_Loop(void)
{
    if (Key_Check(KEY_1, KEY_SINGLE)) {
        Stepper_PWM_Pos_Set_Abs(&Stepper1, 90.0f, 60.0f, 120.0f);
        Stepper_PWM_Pos_Set_Abs(&Stepper2, -30.0f, 45.0f, 100.0f);
    }

    // 检测是否全部到位
    if (Stepper1.Pos_Phase == POS_PHASE_IDLE &&
        Stepper2.Pos_Phase == POS_PHASE_IDLE) {
        // 两个电机都完成运动
        OLED_Printf(0, 16, OLED_6X8, "All Done!");
    }
}
```

---

## 12. 常见问题排查

| 现象 | 可能原因 | 解决 |
|------|----------|------|
| 电机不动 | `Stepper_Init()` 未调用; 定时器未启动 | 确认 `Initial_ALL()` → `Stepper_Init()`, `Initial_Timer()` 调用顺序 |
| 电机不动 | `Speed_Set` 设置了 acc>0 但没等 ramp | 检查 `Acc_Val` 是否过大, `Speed_Tick` 是否在 1ms 中断中被调用 |
| 电机不动 | 限位阻挡 | 检查 `Limit_Enable`=1 且当前位置已在限位边界; 用 `Limit_Check` 验证 |
| 位置不准 | `pulse_angle` 计算错误 | 确认步进角/细分: `pulse_angle = 360° / (360/步进角 × 细分)` |
| 位置过冲 | DECEL 段脉冲 ISR 未停止 | 确认 `Timer_StepperX_Pulse_Callback` 正确调用 `Pulse_Count` |
| 位控和速度互相干扰 | 两个模式同时激活 | 互斥机制: `Speed_Set` 取消位控, `Pos_Set` 取消速度 ramp |
| 加速度 ramp 不平滑 | `Speed_Tick` 调用周期不对 | 当前用 1ms TIM6, 检查 CubeMX 中 TIM6 PSC/ARR 配置 |
| 高速时丢步 | 步进电机力矩不足 | 降低 max_speed 或提高驱动电流 |
| 三角形曲线未触发 | acc 太小或 max_speed 太大 | 打印 `Pos_AccSteps×2 vs Pos_TotalSteps` 确认场景判定 |
| 梯形曲线无匀速段 | 行程太短 | 检查 `Pos_CruiseSteps` 是否 > 0 |

---

> **文档维护**: 本文档与 `Stepper_PWM.h/c` 源码同步。API 签名以头文件为准。  
> **最后更新**: 2026-07-14 — 位置模式 (T型/三角形曲线) 首次发布。
