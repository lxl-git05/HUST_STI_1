# Con_Task 任务队列调度器 — 使用说明书

> **版本**: v1.0 | **日期**: 2026-07-16 | **文件**: `Function/Con_Task.h` + `Function/Con_Task.c`

---

## 1. 涉及文件

| 文件 | 角色 | 说明 |
|------|------|------|
| `Function/Con_Task.h` | 调度器头文件 | 全局任务枚举 `Task_Type`、回调 typedefs、描述结构体、8 个 API 声明 |
| `Function/Con_Task.c` | 调度器实现 | 状态机、内部队列管理、边界检查、Init 自动 Clear |
| `Software/Queue.h` | 队列数据结构 | `QueueData_Typedef` = `struct {int type; float params[4];}` |
| `Software/Queue.c` | 环形队列实现 | `Queue_Init/Enqueue/Dequeue/Peek/Size/Clear/IsEmpty/IsFull` |
| `Mode/Mode_G.c` | Tick 集成点 | `Timer_20ms_Callback` 中调用 `Con_Task_Tick()` |
| `Mode/Mode_2.c` | 参考示例 | 完整的测试任务实现（速度 + 角度 + 按键入队） |

---

## 2. 设计原理

### 2.1 核心思想

```
不想每次都手写状态机？→ 把"任务"抽象为：Setup(一次) → Run(每帧) → IsExit? → 自动出队下一个
```

每个任务类型 = **4 个函数指针**，注册到一张全局描述表中。调度器只负责按表调用，完全不知道每个任务具体干什么。

### 2.2 生命周期

```
                    ┌──────────────────────────────────┐
                    │         Con_Task_Loop()           │
                    │                                  │
  Queue 不空? ──→ 出队 entry ──→ Task_Next = entry.type │
       │                         memcpy(Task_Params)    │
       │           边界检查(越界→Flash快闪+死循环)       │
       ▼                                                │
  ┌──────────┐                                          │
  │  Setup   │  desc->Setup(params)   // 只调用一次      │
  └────┬─────┘                                          │
       ▼                                                │
  ┌──────────┐                                          │
  │   Run    │  desc->Run(params)     // 每帧调用        │
  └────┬─────┘                                          │
       ▼                                                │
  ┌──────────┐                                          │
  │ IsExit?  │  desc->IsExit(params)  // 返回true?       │
  └────┬─────┘                                          │
       │ true                                            │
       ▼                                                │
  Task_Curr = -1  ──→ 下个 Loop 自动出队下一个任务       │
                    └──────────────────────────────────┘

  20ms 中断: Con_Task_Tick() → desc->Tick(params)  // 仅当前活跃任务
```

### 2.3 参数传递：`float params[4]`

| 槽位 | 约定 | 示例 |
|------|------|------|
| `p[0]` | 主要参数 | 目标速度 rpm、目标角度°、等待时间 ms |
| `p[1]` | 次要参数 | 持续时间 ms、容差° |
| `p[2]` | 内部使用 | 存储时间戳 `HAL_GetTick()` |
| `p[3]` | 扩展预留 | 电机编号等 |

> `params` 的生命周期 = 当前任务的生命周期。任务 Exit 后，下个任务出队时 `memcpy` 覆盖，不会有残留。

### 2.4 两种入队方式

| 方式 | 调用位置 | 用途 |
|------|---------|------|
| **预设入队** | `Mode_X_Setup()` | 定义固定的任务序列（编排） |
| **手动入队** | `Mode_X_Loop()` | 按键/串口等外部事件动态追加（响应） |

两种方式共用同一个队列，可以混用——预设任务在排头，手动任务追加到队尾，FIFO 顺序执行。

### 2.5 边界检查

```c
if (Task_Next < 0 || Task_Next >= Task_Table_Size) {
    Flash_Mode_Set(Flash_Mode_Fast);  // LED 快闪告警
    while (1);                        // 停止一切调度
}
```

触发条件：入队了一个未在 `Task_Table` 中注册的枚举值。调试时看到 LED 快闪 + 系统卡死，说明任务表配置有误。

### 2.6 Tick 的两种集成方式

| 方式 | 位置 | 特点 |
|------|------|------|
| **全局 Tick（当前）** | `Mode_G.c` → `Timer_20ms_Callback` → `Oran_Update()` 之后、`switch` 之前 | 所有 Mode 自动享受，无任务时零开销 |
| **按 Mode Tick** | 各 `Mode_x_Tick()` 内部调用 `Con_Task_Tick()` | 精确控制哪些 Mode 启用 Con_Task Tick |

两种方式任选，`Con_Task_Tick()` 内部有 `if (Task_Curr == -1) return` 保护，无任务时无开销。

---

## 3. API 手册

### 3.1 Con_Task_Init

```c
void Con_Task_Init(const Task_Descriptor_Typedef *table, int size);
```

**功能**: 注册任务描述表 + 清空队列 + 终止当前任务。
**调用时机**: 每个 `Mode_X_Setup()` 中调用一次。可重复调用——每次调用都会清空旧 Mode 残留的任务。
**参数**:
- `table`: 任务描述表数组（每个 Mode 定义自己的表）
- `size`: 表大小，通常传 `TASK_COUNT`

```c
// 典型用法
Con_Task_Init(MyTaskTable, TASK_COUNT);
```

### 3.2 Con_Task_Enqueue

```c
void Con_Task_Enqueue(int task_type, float p0, float p1, float p2, float p3);
```

**功能**: 将任务追加到队尾。Setup 和 Loop 中均可调用。
**参数**:
- `task_type`: 任务类型枚举值（如 `TASK_MOTOR_SPEED`）
- `p0~p3`: 4 个 float 参数，含义由回调函数定义

```c
// 入队一个速度任务: 30rpm, 持续5000ms
Con_Task_Enqueue(TASK_MOTOR_SPEED, 30, 5000, 0, 0);
// 入队一个角度任务: 目标360°, 容差20°
Con_Task_Enqueue(TASK_MOTOR_ANGLE, 360, 20, 0, 0);
```

### 3.3 Con_Task_Clear

```c
void Con_Task_Clear(void);
```

**功能**: 清空队列 + 终止当前任务。紧急停止用。
**注意**: 不会自动停止电机/PWM，调用方需自行处理硬件停止。

```c
Con_Task_Clear();
Motor_Stop(&Motor_A);  // 记得停硬件
```

### 3.4 Con_Task_Loop

```c
void Con_Task_Loop(void);
```

**功能**: 推进状态机。必须在 `Mode_X_Loop()` 中每次循环调用。

```c
void Mode_X_Loop(void) {
    Con_Task_Loop();   // ★ 必须调用
    // 其他循环逻辑...
}
```

### 3.5 Con_Task_Tick

```c
void Con_Task_Tick(void);
```

**功能**: 20ms 中断中分发到当前活跃任务的 `.Tick` 回调。无任务时自动跳过。

### 3.6 Con_Task_IsBusy

```c
bool Con_Task_IsBusy(void);
```

**功能**: 是否有任务正在执行。`true` = 有活跃任务。

### 3.7 Con_Task_CurrType

```c
int Con_Task_CurrType(void);
```

**功能**: 返回当前任务的枚举值。`-1` = 空闲。常用于 OLED 显示。

### 3.8 Con_Task_Remaining

```c
int Con_Task_Remaining(void);
```

**功能**: 队列中等待执行的任务数。`0` 且 `!IsBusy()` = 全部完成。

---

## 4. 如何新增一个任务类型（5 步）

假设要新增 `TASK_BUZZER`：蜂鸣器响指定毫秒。

### Step 1: 在 Con_Task.h 中添加枚举

```c
typedef enum {
    TASK_NONE = 0,
    TASK_WAIT_TIME,
    TASK_MOTOR_SPEED,
    TASK_MOTOR_ANGLE,
    TASK_BUZZER,          // ★ 新增（在 TASK_COUNT 之前）
    TASK_COUNT
} Task_Type;
```

### Step 2: 写 Setup 回调

```c
// p[0] = 蜂鸣器响的时长(ms)
static void Task_Buzzer_Setup(float p[4]) {
    Buzzer_On();              // 打开蜂鸣器
    p[2] = HAL_GetTick();     // 记录开始时间
}
```

### Step 3: 写 IsExit 回调

```c
static bool Task_Buzzer_IsExit(float p[4]) {
    return (HAL_GetTick() - p[2]) >= p[0];  // 时间到
}
```

### Step 4: 注册到任务表

```c
static const Task_Descriptor_Typedef MyTaskTable[TASK_COUNT] = {
    // ... 已有任务 ...
    [TASK_BUZZER] = { .Setup = Task_Buzzer_Setup, .IsExit = Task_Buzzer_IsExit },
};
```

### Step 5: 入队使用

```c
Con_Task_Enqueue(TASK_BUZZER, 200, 0, 0, 0);  // 蜂鸣200ms
```

> **关键**: 整个过程不需要改 `Con_Task.c` 本体，只改枚举和调用方。

---

## 5. 简洁示例

### 5.1 最简使用（Mode_X 中只有一个任务）

```c
// ---- 回调 ----
static void MyTask_Setup(float p[4]) { /* 初始化 */ }
static bool MyTask_IsExit(float p[4]) { return /* 完成条件 */; }

// ---- 表 ----
static const Task_Descriptor_Typedef Table[TASK_COUNT] = {
    [TASK_MOTOR_SPEED] = { .Setup = MyTask_Setup, .IsExit = MyTask_IsExit },
};

// ---- Mode ----
void Mode_X_Setup(void) {
    Con_Task_Init(Table, TASK_COUNT);
    Con_Task_Enqueue(TASK_MOTOR_SPEED, 50, 3000, 0, 0);  // 速度50, 3秒
}

void Mode_X_Loop(void) {
    Con_Task_Loop();
}

void Mode_X_Exit(void) {
    Con_Task_Clear();  // 可选，切出时自动 Init 也会清
}
```

### 5.2 带 Tick 的任务（如角度 PID）

```c
// 角度任务需要 PID 在 20ms 中断中持续计算
static void Angle_Tick(float p[4]) {
    Motorx_Angle_Update_Tick(&Motor_A, 1);  // 角度环 PID
}

static const Task_Descriptor_Typedef Table[TASK_COUNT] = {
    [TASK_MOTOR_ANGLE] = {
        .Setup  = Angle_Setup,    // 设置目标角度
        .Tick   = Angle_Tick,     // ★ 20ms PID 更新
        .IsExit = Angle_IsExit,   // 到位检测
    },
};
```

### 5.3 按键动态追加任务

```c
void Mode_X_Loop(void) {
    Con_Task_Loop();

    if (Key_Check(KEY_1, KEY_SINGLE))
        Con_Task_Enqueue(TASK_MOTOR_SPEED, 30, 5000, 0, 0);

    if (Key_Check(KEY_2, KEY_SINGLE))
        Con_Task_Enqueue(TASK_MOTOR_ANGLE, 180, 20, 0, 0);

    if (Key_Check(KEY_3, KEY_LONG))
        Con_Task_Clear();  // 长按紧急清空
}
```

### 5.4 串口远程入队

```c
if (Serial_GetNewPackageFlag_ABC(&Serial1)) {
    float val = 0;
    Serial_SetFloatData(&Serial1, "add", "add=%f", &val);
    if (val != 0)
        Con_Task_Enqueue(TASK_MOTOR_ANGLE, val, 20, 0, 0);
}
```

### 5.5 OLED 显示队列状态

```c
OLED_Printf(0, 0, OLED_6X8, "Q:%d Task:%d",
    Con_Task_Remaining(), Con_Task_CurrType());

if (!Con_Task_IsBusy() && Con_Task_Remaining() == 0)
    OLED_Printf(0, 8, OLED_6X8, "All Done!");
```

---

## 6. 回调函数速查

| 回调 | 类型签名 | 调用时机 | NULL 允许? | 用途 |
|------|---------|---------|-----------|------|
| `Setup` | `void (*)(float p[4])` | 任务启动时**一次** | ❌ 不建议 | 配置硬件目标值、记录时间戳 |
| `Run` | `void (*)(float p[4])` | 每帧 Loop | ✅ | 持续执行的逻辑（一般用不上） |
| `IsExit` | `bool (*)(float p[4])` | 每帧 Loop | ❌ 必须有 | 判断完成条件，返回 `true` 退出 |
| `Tick` | `void (*)(float p[4])` | 每 20ms 中断 | ✅ | PID 更新、传感器读取等实时操作 |

> **经验**: 90% 的任务只需要 `Setup` + `IsExit`。需要 `Tick` 的典型场景是角度/位置 PID 控制。

---

## 7. 常见问题

### Q: 切 Mode 时正在执行的任务怎么办？
A: `Con_Task_Init()` 自动 `Queue_Clear` + `Task_Curr = -1`，旧任务直接终止。记得在 `Mode_X_Exit()` 中停掉硬件（电机等）。

### Q: 一个任务完成后队列空了怎么办？
A: `Task_Curr = -1`，状态机回到空闲。`Con_Task_Loop()` 什么都不做，等待新任务入队。

### Q: 能不能跳过队列直接立即执行某个任务？
A: 不能。设计上所有任务都经过队列。如果急需，先 `Con_Task_Clear()` 再 `Con_Task_Enqueue()`。

### Q: params[4] 够用吗？
A: 对绝大多数场景够用。需要用 int 时 `*(int*)&p[0]` 强转。4 个不够说明任务粒度太粗，应该拆成多个小任务串联。

### Q: 为什么不用动态内存？
A: 嵌入式没有 `malloc`。队列和 params 全部静态分配，确定性高、无碎片。
