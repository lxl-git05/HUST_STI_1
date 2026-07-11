# Stepper_Driver 步进电机驱动

## 项目概述

基于 STM32H7 + Emm_V5 闭环步进电机的双串口驱动，支持命令队列调度，防止 DMA 异步发送导致的死锁和粘包问题。

---

## 系统架构

### 主循环 (Mymain.c)

```
main() → Mymain() → while(1)循环
    ├── OLED_Clear()
    ├── Mode_G_Loop()        // KEY_0检测
    ├── Mode_X_Loop()        // 当前模式主循环
    ├── Mode切换 → Exit() → Setup()
    └── OLED_Update()
```

### 定时器架构 (Mode_G.c)

| 定时器 | 硬件 | 周期 | 回调 |
|--------|------|------|------|
| TIM6 (htim6) | 5ms | 5ms | `Timer_5ms_Callback()` |
| TIM16 (htim16) | 20ms | 20ms | `Timer_20ms_Callback()` |
| TIM17 (htim17) | 1ms | 1ms | `Timer_1ms_Callback()` |

### 定时器分发

```c
// 1ms: 全局共享
void Timer_1ms_Callback(void) {
    Key_Tick();          // 按键扫描
    Flash_Mode_Tick();   // LED闪烁
}

// 5ms: Timer_5ms_Callback 由 Mode_2 覆盖定义
void Timer_5ms_Callback(void) {
    Stepper_Polling_5ms();  // 处理队列和状态查询
}

// 20ms: 按curr_mode分发
void Timer_20ms_Callback(void) {
    switch(curr_mode) {
        case 1: Mode_1_Tick(); break;
        case 2: Mode_2_Tick(); break;
        case 3: Mode_3_Tick(); break;  // 20ms周期到达判定
        case 4: Mode_4_Tick(); break;
    }
}
```

### Mode 枚举 (Mode_G.h)

```c
typedef enum {
    Mode_Null = 0,  // 起始/空模式
    Mode_PID,       // 1: PID调试(已注释)
    Mode_2,         // 2: 步进电机测试
    Mode_3,         // 3: 正方形绘制
    Mode_4,         // 4: 主程序
    Mode_End        // 哨兵，防止越界
} Mode_Typedef;

Mode_Typedef curr_mode = Mode_Null;
Mode_Typedef next_mode = Mode_2;  // 默认进入Mode_2
```

### Mode 切换流程

```
KEY_0双击 → Mode_To_Next()
    ↓
curr_mode != next_mode → 执行Exit() → 执行Setup()
    ↓
curr_mode = next_mode → 只执行Loop()
```

每个 Mode 有独立的 4 个函数：
- `Mode_X_Setup()`: 进入时调用一次
- `Mode_X_Loop()`: 每次 while 循环调用
- `Mode_X_Tick()`: 20ms 定时器调用
- `Mode_X_Exit()`: 退出时调用一次

---

## 硬件配置

| 项目 | 电机1 | 电机2 |
|------|-------|-------|
| UART | USART3 | USART6 |
| TX/RX 引脚 | PB10/PB11 | PC6/PC7 |
| DMA RX | DMA1_Stream4 | DMA1_Stream6 |
| DMA TX | DMA1_Stream5 | DMA1_Stream7 |
| 波特率 | 115200 | 115200 |
| 电机地址 | 0x01 | 0x01 |

---

## 核心概念：为什么需要命令队列？

DMA 发送是**异步**的。`Emm_V5_xxx()` 将数据放入 DMA 缓冲区后立即返回，不等待发送完成。

```
旧设计: while(tx_busy);  // 死等 → 可能卡死
新设计: Enqueue → 队列 → 5ms轮询出队 → 发送
```

**关键优势**：
1. 主循环调用 `Enqueue` 立即返回，不阻塞
2. 5ms 定时器统一调度，不会粘包
3. STOP 命令优先级最高，队列满时丢弃旧命令

---

## 步进电机驱动 (Stepper.c)

### 命令队列机制

```
主循环 → Enqueue() → 队列 → Timer_5ms_Callback → Pop() → Exec_Cmd()
                                                        ↓
                                                    DMA发送 → TX完成中断 → tx_busy=false
```

### 队列操作 (原子化)

```c
Queue_Push_ISR()   // 入队，关闭中断保护
Queue_Pop_ISR()    // 出队，关闭中断保护
Queue_Push_Force() // 强制入队(STOP用)，满时丢弃最旧命令
```

### 入队 API (主循环/中断调用)

```c
bool Stepper_Vel_Enqueue(Stepper_Typedef* p, int16_t vel, uint8_t acc);
bool Stepper_RelPos_Enqueue(Stepper_Typedef* p, int16_t vel, uint8_t acc, float angle);
bool Stepper_AbsPos_Enqueue(Stepper_Typedef* p, int16_t vel, uint8_t acc, float angle);
bool Stepper_ResetZero_Enqueue(Stepper_Typedef* p);
bool Stepper_Stop_Enqueue(Stepper_Typedef* p);  // 最高优先级
```

### 5ms 轮询调度 (Timer_5ms_Callback)

```c
void Stepper_Polling_5ms(void) {
    // 电机1: 队列非空则出队执行，否则查询状态
    if (!Stepper1.tx_busy) {
        if (Queue_Pop_ISR(&Stepper1.cmd_queue, &cmd)) {
            Stepper_Exec_Cmd(&Stepper1, &cmd);
        } else {
            // 查询位置/速度
        }
    }
    // 电机2: 独立处理
}
```

### 命令类型

```c
typedef enum {
    CMD_NONE,        // 空
    CMD_VEL,         // 速度模式
    CMD_REL_POS,     // 相对位置
    CMD_ABS_POS,     // 绝对位置
    CMD_RESET_ZERO,  // 位置清零
    CMD_STOP,        // 立即停车
    CMD_QUERY_POS,   // 查询位置
    CMD_QUERY_VEL,   // 查询速度
} Stepper_CmdType;
```

### 两电机独立性

- 独立 UART (USART3 / USART6)
- 独立 DMA Stream
- 独立 `tx_busy` 标志
- 独立命令队列
- 独立串口接收缓冲区
- 各自 5ms 轮询，互不阻塞

---

## 按键系统 (Key.c)

### 按键标志

```c
#define KEY_HOLD    0x01  // 按住(持续)
#define KEY_DOWN    0x02  // 按下(瞬时)
#define KEY_UP      0x04  // 抬起(瞬时)
#define KEY_SINGLE  0x08  // 单击
#define KEY_DOUBLE  0x10  // 双击
#define KEY_LONG    0x20  // 长按
#define KEY_REPEAT  0x40  // 长按重复
```

### 定时参数

```c
#define KEY_TIME_DOUBLE 200   // 双击间隔 ×20ms = 4000ms (太大!)
#define KEY_TIME_LONG   2000  // 长按判定 ×1ms = 2000ms
#define KEY_TIME_REPEAT 100   // 长按重复 ×1ms = 100ms
```

### 使用方式

```c
if (Key_Check(KEY_1, KEY_SINGLE)) { ... }  // 单击检测，自动清除标志
if (Key_Check(KEY_1, KEY_LONG))   { ... }  // 长按检测
```

---

## 文件结构

```
Stepper_Driver/
├── Core/Src/
│   └── main.c           # 入口，调用Mymain()
├── Top/
│   ├── Mymain.c         # 主循环架构
│   ├── Mymain.h
│   ├── AllHeader.h      # 统一头文件
│   └── AllHeader.c      # 初始化集合
├── Mode/
│   ├── Mode_G.c/.h      # Mode枚举、定时器分发、切换逻辑
│   ├── Mode_1.c/.h      # PID调试(已注释)
│   ├── Mode_2.c/.h      # 步进电机测试
│   ├── Mode_3.c/.h      # 正方形绘制
│   └── Mode_4.c/.h      # 主程序(空)
├── Hardware/
│   ├── Stepper.c/.h     # 步进电机队列驱动
│   ├── Emm_V5.c/.h      # 电机协议库
│   └── Key.c/.h         # 按键驱动
└── CLAUDE.md            # 本文档
```

---

## pulse_angle 参数

每脉冲对应的角度值：

```
angle(度) = clk(脉冲数) × pulse_angle(度)
```

| 电机 | pulse_angle | 说明 |
|------|-------------|------|
| 电机1 | 0.004f | 1.8°步距角 / 75:1减速比 / 162细分 |
| 电机2 | 0.01f | 1.8°步距角 / 180细分 |

---

## 常见问题

### Q: 为什么主循环调用 Enqueue 后没有立即执行？
A: 命令入队后等待 5ms 定时器轮询才出队发送，这是设计预期。

### Q: 队列满了会怎样？
A: 普通命令入队失败返回 `false`；STOP 命令会丢弃最旧命令保证执行。

### Q: 两个电机可以同时发送吗？
A: 可以，两个 UART 完全独立，各自轮询，互不干扰。

### Q: 如何调试队列状态？
A: `Stepper1.cmd_queue.count` 显示队列中的命令数量。

### Q: Timer_5ms_Callback 是谁定义的？
A: 由 Mode_2 定义，但属于硬件定时器中断，所有 Mode 共用。

### Q: Mode_X_Tick 和 Timer_5ms_Callback 的区别？
A: `Mode_X_Tick` 是 20ms 周期（由 Timer_20ms_Callback 分发），用于模式相关的处理。`Timer_5ms_Callback` 是 5ms 周期（硬件定时器），用于步进电机队列轮询。

---

## 注意事项

1. **必须调用 `Stepper_Init()`** 在使用步进电机之前（在 Mode_2_Setup 或 Mode_3_Setup 中）
2. **必须调用 `Stepper_Polling_5ms()`** 在 Timer_5ms_Callback 中，否则队列不会执行
3. **不要直接调用旧 API** 如 `Stepper_Speed_Set()`，它们存在死锁风险
4. **STOP 命令优先级最高**，队列满时会丢弃其他命令
5. **Mode_X_Tick 只做到达判定**，不要在其中调用 `Stepper_Polling_5ms()`
