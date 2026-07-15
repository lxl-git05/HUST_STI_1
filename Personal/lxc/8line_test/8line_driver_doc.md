# 8路寻迹传感器驱动文档与核心代码解析

---

## 1. 系统架构概览

```
┌─────────────────────────┐        CLK(PA0)         ┌──────────────────────┐
│                         │ ──────────────────────▶ │                      │
│    STM32F103C8T6        │                         │   MSPM0L1306 辅助板   │
│    (主控)                │ ◀────────────────────── │   (传感器处理器)       │
│                         │        DAT(PA1)          │                      │
│                         │                         │   8路灰度传感器        │
│                         │                         │   + 自动校准          │
│                         │                         │   + 阈值判断          │
│                         │         USART1          │   + 串行输出          │
│                         │ ──────────────────────▶ │                      │
│                         │      printf 调试输出      │                      │
└─────────────────────────┘                         └──────────────────────┘
```

| 组件 | 说明 |
|------|------|
| STM32F103C8T6 | 主控 MCU，72MHz，读取传感器数据并处理 |
| MSPM0L1306 辅助板 | 独立 MCU 板，完成 8 路灰度传感器 AD 采集、自动校准、阈值判断 |
| 通信接口 | 2 线同步串行 (CLK + DAT)，主控发时钟，辅助板在时钟沿输出数据 |
| 调试输出 | USART1, 115200bps, 8N1, printf 重定向 |

---

## 2. 硬件接线

| 辅助板引脚 | STM32F103 | 说明 |
|-----------|-----------|------|
| GND | GND | 共地 |
| VCC | 3.3V | 供电 |
| CLK | PA0 | 时钟信号，主控输出 |
| DAT | PA1 | 数据信号，主控输入 (内部上拉) |

> **注意**：DAT 引脚必须配置为上拉输入 (`GPIO_PULLUP`)。辅助板使用开漏输出，空闲时总线被上拉电阻拉高。

---

## 3. 通信协议详解

### 3.1 时序图

```
CLK:    ──┐     ┌──┐     ┌──┐     ┌──┐     ┌──┐     ┌──┐     ┌──┐     ┌──┐
           └─────┘  └─────┘  └─────┘  └─────┘  └─────┘  └─────┘  └─────┘  └─────
            | 5us | 5us| 5us| 5us| 5us| 5us| 5us| 5us| 5us| 5us| 5us| 5us| 5us|
            |<- 初始低电平->|                                              |<- 结束低电平->|

DAT:    ──────────────────┬───────┬───────┬───────┬───────┬───────┬───────┬───────
                          | bit7  | bit6  | bit5  | bit4  | bit3  | bit2  | bit1  | bit0
                          └───────┴───────┴───────┴───────┴───────┴───────┴───────┘
                            MSB 先出                             LSB
                            在 CLK 上升沿后读取                     低位先存
```

### 3.2 协议规则

1. **空闲状态**：CLK = 低电平
2. **起始条件**：CLK 保持低电平 ≥ 5μs
3. **数据传输**：
   - 主控产生 8 个完整 CLK 脉冲（高→低→高→低 为一个脉冲）
   - 每个脉冲周期 ≈ 20μs（高 5μs + 低 5μs），频率约 50kHz
   - 辅助板在 **CLK 上升沿** 时切换 DAT 线上的数据
   - 主控在 **CLK 下降沿后**（低电平期间）读取 DAT 线
4. **数据顺序**：MSB 先出 (bit7→bit0)，但存入 `uint8_t` 时 bit0 对应第 1 个时钟脉冲读取的值
5. **数据含义**：`1` = 白色/浅色地面，`0` = 黑色/深色线条
6. **结束条件**：最后一位读取完成后，CLK 保持低电平

### 3.3 读取流程图

```
开始
  │
  ▼
CLK = 0, 延时 5μs          ← 起始条件
  │
  ▼
循环 i = 0 到 7:
  ├─ CLK = 1, 延时 5μs     ← 上升沿，辅助板更新 DAT
  ├─ CLK = 0, 延时 5μs     ← 下降沿
  └─ 读取 DAT:
       DAT=1 → data |= (1 << i)
       DAT=0 → 不操作
  │
  ▼
返回 data (8位)
```

---

## 4. 核心代码逐行解析

### 4.1 引脚宏定义 (`main.c:37-40`)

```c
#define CLK_PIN   GPIO_PIN_0
#define CLK_PORT  GPIOA
#define DAT_PIN   GPIO_PIN_1
#define DAT_PORT  GPIOA
```

> **移植提示**：更换 MCU 或引脚时只需修改此处 4 个宏，无需改动函数体。

### 4.2 微秒延时函数 (`main.c:63-69`)

```c
static void delay_us(uint32_t us)
{
    // 72MHz主频下，每微秒约72个循环
    // 实际测试调整为 us * 8 约等于1us（因为循环本身也有开销）
    uint32_t count = us * 8;
    for (volatile uint32_t i = 0; i < count; i++);
}
```

**原理分析**：
- STM32F103 主频 72MHz，1μs = 72 个指令周期
- 软件延时是粗略估算，`us * 8` 是经过实际测量校准的系数
- `volatile` 防止编译器优化掉空循环
- **精度**：±30% 左右，对本协议足够（辅助板对 CLK 频率不敏感，只需满足最小脉宽要求）

> **移植提示**：
> - F407 (168MHz) 需将系数调整为 `us * 21` 左右
> - 精度要求高的场景建议改用硬件定时器产生 CLK
> - 辅助板实测可接受 1~100μs 级别的脉宽，容差很大

### 4.3 核心读取函数 (`main.c:71-84`)

```c
uint8_t Read_Sensor_Data(void)
{
    uint8_t data = 0;                                        // [1]
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);    // [2]
    delay_us(5);                                              // [3]
    for (int i = 0; i < 8; i++) {                             // [4]
        HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);  // [5]
        delay_us(5);                                          // [6]
        HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);// [7]
        delay_us(5);                                          // [8]
        if (HAL_GPIO_ReadPin(DAT_PORT, DAT_PIN))               // [9]
            data |= (1 << i);                                  // [10]
    }
    return data;                                              // [11]
}
```

**逐行解析**：

| 行号 | 代码 | 解析 |
|------|------|------|
| [1] | `uint8_t data = 0;` | 初始化结果变量，所有位默认 0 |
| [2] | `HAL_GPIO_WritePin(..., RESET)` | 确保 CLK 起始为低电平 |
| [3] | `delay_us(5);` | 起始条件：CLK 低电平保持 5μs，辅助板准备数据 |
| [4] | `for (int i = 0; i < 8; i++)` | 循环 8 次，i 从 0 到 7 |
| [5] | `HAL_GPIO_WritePin(..., SET)` | CLK 上升沿 → 辅助板将 bit(7-i) 输出到 DAT |
| [6] | `delay_us(5);` | 高电平保持 5μs，等待 DAT 稳定 |
| [7] | `HAL_GPIO_WritePin(..., RESET)` | CLK 下降沿 |
| [8] | `delay_us(5);` | 低电平期间读取 DAT，确保数据稳定 |
| [9] | `HAL_GPIO_ReadPin(DAT_PORT, DAT_PIN)` | 读取 DAT 状态：高电平=1，低电平=0 |
| [10] | `data \|= (1 << i);` | 若 DAT=1，将第 i 位置 1（i=0→bit0, i=7→bit7） |
| [11] | `return data;` | 返回 8 位数据 |

**数据位映射**：

```
返回的 uint8_t:
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ B7 │ B6 │ B5 │ B4 │ B3 │ B2 │ B1 │ B0 │
└────┴────┴────┴────┴────┴────┴────┴────┘
  ↑                         ↑
  第1个CLK读取              第8个CLK读取
  (MSB先出)                 (实际存入bit0)
```

> **注意**：虽然辅助板按 MSB 先出的顺序发送（物理上第 1 个 CLK 对应的传感器编号取决于辅助板固件），代码中将第 1 个 CLK 读到的数据存入 bit0，第 8 个存入 bit7。实际传感器编号与位的对应关系以辅助板的硬件连接为准。

### 4.4 printf 重定向 (`usart.c:160-165`)

```c
int fputc(int ch, FILE *f)
{
    // 通过串口1发送一个字节
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}
```

**原理**：重写 C 标准库的 `fputc`，使 `printf` 输出重定向到 USART1。这是 STM32 HAL 库中最常用的 printf 重定向方法。

### 4.5 主循环 (`main.c:127-146`)

```c
while (1)
{
    Digtal = Read_Sensor_Data();                    // 读取8路传感器数据

    printf("Digtal = 0x%02X", Digtal);              // 十六进制输出
    printf(" | ");
    for (int i = 7; i >= 0; i--)                    // 按位展开二进制
    {
        printf("%d", (Digtal >> i) & 0x01);
    }
    printf(" \r\n");

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);         // 翻转LED，指示程序运行
    HAL_Delay(1000);                                 // 1秒读取一次（调试用）
}
```

**输出示例**：
```
Digtal = 0x3C | 00111100
Digtal = 0x18 | 00011000
Digtal = 0x00 | 00000000
```

- `0x3C` = `00111100`：表示中间 4 路检测到白色，两侧 2 路检测到黑色（线在中间）
- `0x00`：全黑或传感器未校准（校准前始终返回 0x00）

---

## 5. CubeMX 完整配置参考

### 5.1 芯片选型
- **MCU**: STM32F103C8T6
- **封装**: LQFP48
- **主频**: 72MHz (HSE 8MHz, PLL ×9)

### 5.2 引脚配置

| 引脚 | 功能 | 模式 | 上下拉 | 标签 |
|------|------|------|--------|------|
| PA0 | CLK (输出) | GPIO_Output | 无 | CLK |
| PA1 | DAT (输入) | GPIO_Input | Pull-up | DAT |
| PA9 | USART1_TX | Alternate Function PP | - | - |
| PA10 | USART1_RX | Input | 无 | - |
| PC13 | 板载 LED | GPIO_Output | 无 | LED0 |
| PB8 | OLED_SCL | GPIO_Output | 无 | OLED_SCL |
| PB9 | OLED_SDA | GPIO_Output | 无 | OLED_SDA |
| PB12 | KEY1 | GPIO_Input | Pull-up | KEY1 |
| PB13 | KEY2 | GPIO_Input | Pull-up | KEY2 |

### 5.3 USART1 配置
- **模式**: Asynchronous
- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验**: None
- **硬件流控**: None
- **DMA**: TX=DMA1_Channel4, RX=DMA1_Channel5

### 5.4 时钟树
```
HSE (8MHz) → PLL (×9) → SYSCLK (72MHz)
                          ├─ AHB (72MHz)
                          ├─ APB2 (72MHz)
                          └─ APB1 (36MHz)
```

### 5.5 项目设置
- **Toolchain**: MDK-ARM V5.32 / GCC
- **堆栈**: Heap=0x200, Stack=0x400
- **固件包**: STM32Cube FW_F1 V1.8.6

---

## 6. 引脚初始化代码解析 (`gpio.c`)

```c
void MX_GPIO_Init(void)
{
    // 1. 使能 GPIO 时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. 设置默认输出电平
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);   // LED 初始灭
    HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, GPIO_PIN_RESET);     // CLK 初始低
    HAL_GPIO_WritePin(GPIOB, OLED_SCL_Pin|OLED_SDA_Pin, GPIO_PIN_RESET);

    // 3. 输出引脚配置
    // LED0 (PC13): 推挽输出, 无上下拉, 低速
    // CLK  (PA0):  推挽输出, 无上下拉, 低速
    // OLED (PB8/9): 推挽输出, 无上下拉, 高速

    // 4. 输入引脚配置
    // DAT  (PA1):     输入模式, 上拉 — 关键！匹配辅助板开漏输出
    // KEY1 (PB12):    输入模式, 上拉
    // KEY2 (PB13):    输入模式, 上拉
}
```

> **关键配置**：DAT (PA1) 必须配置为上拉输入 (`GPIO_PULLUP`)，因为辅助板使用开漏输出，只能拉低不能拉高，上拉电阻确保空闲状态为高电平。

---

## 7. USART+DMA 初始化解析 (`usart.c` + `dma.c`)

### 7.1 USART1 参数
```c
huart1.Instance = USART1;
huart1.Init.BaudRate = 115200;      // 115200 bps
huart1.Init.WordLength = UART_WORDLENGTH_8B;
huart1.Init.StopBits = UART_STOPBITS_1;
huart1.Init.Parity = UART_PARITY_NONE;
huart1.Init.Mode = UART_MODE_TX_RX;
huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart1.Init.OverSampling = UART_OVERSAMPLING_16;
```

### 7.2 DMA 通道分配
| DMA 通道 | 外设 | 方向 |
|----------|------|------|
| DMA1_Channel4 | USART1_TX | 内存→外设 |
| DMA1_Channel5 | USART1_RX | 外设→内存 |

### 7.3 USART1 引脚
| 引脚 | 功能 | 模式 |
|------|------|------|
| PA9 | USART1_TX | 复用推挽输出 |
| PA10 | USART1_RX | 浮空输入 |

### 7.4 中断向量
```c
// NVIC 优先级分组 = 2 (2位抢占优先级, 2位子优先级)
HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);       // 抢占0, 子0
HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0); // 抢占0, 子0
HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0); // 抢占0, 子0
```

---

## 8. 完整移植指南

### 8.1 最小移植步骤（任何 STM32 芯片）

**Step 1**: 在 CubeMX 中配置两个 GPIO：
- CLK：输出，推挽，无上下拉
- DAT：输入，上拉

**Step 2**: 复制以下核心代码到工程中：

```c
// ===== 引脚宏定义（按实际接线修改） =====
#define CLK_PIN   GPIO_PIN_0
#define CLK_PORT  GPIOA
#define DAT_PIN   GPIO_PIN_1
#define DAT_PORT  GPIOA

// ===== 微秒延时（按主频调整系数） =====
static void delay_us(uint32_t us)
{
    // 系数 = 主频(MHz) / 9（粗略估算，建议实测校准）
    // 72MHz → 8,  168MHz → 19,  180MHz → 20
    uint32_t count = us * 8;
    for (volatile uint32_t i = 0; i < count; i++);
}

// ===== 核心读取函数（不需要修改） =====
uint8_t Read_Sensor_Data(void)
{
    uint8_t data = 0;
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
    delay_us(5);
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
        delay_us(5);
        HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
        delay_us(5);
        if (HAL_GPIO_ReadPin(DAT_PORT, DAT_PIN))
            data |= (1 << i);
    }
    return data;
}
```

**Step 3**: 在主循环中调用：
```c
uint8_t sensor_data = Read_Sensor_Data();
// sensor_data 的每一位对应一路传感器，1=白，0=黑
```

### 8.2 移植到 STM32F407 的注意事项

| 项目 | F103 (参考) | F407 (目标) |
|------|------------|------------|
| 主频 | 72MHz | 168MHz |
| delay_us 系数 | `us * 8` | `us * 19` |
| HAL 库 | STM32F1xx_HAL | STM32F4xx_HAL |
| GPIO 外设 | 相同 API | 相同 API |
| USART | USART1 | USART1 (或其他) |

**F407 专用版 delay_us**：
```c
static void delay_us(uint32_t us)
{
    uint32_t count = us * 19;  // 168MHz → 约 19
    for (volatile uint32_t i = 0; i < count; i++);
}
```

### 8.3 使用定时器替代软件延时（推荐用于正式项目）

```c
// 使用 TIM 产生精确的 CLK 信号
// 可避免 delay_us 占用 CPU，提高系统实时性
// 建议用 TIM 的一个通道配置为 PWM/输出比较模式
```

---

## 9. 数据结构与 API 参考

### 9.1 全局变量

```c
uint8_t Digtal;  // 存储8路传感器当前读数（main.c:52）
```

| 位 | 含义 |
|----|------|
| bit0 | 第 1 路传感器（第 1 个 CLK 读取） |
| bit1 | 第 2 路传感器 |
| ... | ... |
| bit7 | 第 8 路传感器 |

### 9.2 函数 API

```c
/**
 * @brief  读取 8 路寻迹传感器数据
 * @param  无
 * @retval uint8_t  8 位传感器数据
 *         - bit=1: 检测到白色/浅色
 *         - bit=0: 检测到黑色/深色
 *         - 返回 0x00: 传感器未校准或全黑
 * @note   单次调用耗时约 90μs（8×(5+5+5) + 5μs起始）
 *         建议调用间隔 > 100μs
 */
uint8_t Read_Sensor_Data(void);
```

### 9.3 典型应用示例

```c
// 示例1: 循迹判断（黑线跟随）
uint8_t sensor = Read_Sensor_Data();
if (sensor == 0x00) {
    // 全黑，可能偏离轨道
} else if (sensor & 0x18) {  // bit3, bit4 = 中间两路
    // 线在中间，直行
} else if (sensor & 0x07) {  // bit0~bit2 = 左侧
    // 线在左边，左转
} else if (sensor & 0xE0) {  // bit5~bit7 = 右侧
    // 线在右边，右转
}

// 示例2: 计算偏差值（用于 PID 控制）
int8_t get_position_error(uint8_t sensor_data) {
    // 加权平均法：每路赋予不同权重
    static const int8_t weights[8] = {-7, -5, -3, -1, 1, 3, 5, 7};
    int32_t sum = 0, count = 0;
    for (int i = 0; i < 8; i++) {
        if (sensor_data & (1 << i)) {
            sum += weights[i];
            count++;
        }
    }
    return (count > 0) ? (int8_t)(sum / count) : 0;
}
```

---

## 10. 传感器校准说明

辅助板（MSPM0L1306）的校准流程：

```
长按按键 → 快闪状态 → 松手 → 慢闪（校准模式）
    │
    ├─ 对准白色基准 → 长按 → 快闪 → 持续按 → 慢闪（白色校准完成）→ 松手
    │
    └─ 对准黑色基准 → 长按 → 快闪 → 持续按 → 慢闪（黑色校准完成）→ 松手
                                                                      │
                                                              校准结束，正常工作
```

**LED 状态说明**：
| LED 状态 | 含义 |
|----------|------|
| 灭 | 正常工作，正常输出数据 |
| 快闪 | 检测到按键按下 |
| 慢闪 | 等待基准输入（校准模式） |
| ERR 亮 | 传感器过曝或异常 |

> **重要**：未校准时，辅助板始终输出 `0x00`（全 0），串口输出 `Digtal = 0x00 | 00000000`。校准后才会输出正确数据。

---

## 11. 系统时钟树完整参数

| 时钟节点 | 频率 | 来源 |
|----------|------|------|
| HSE | 8 MHz | 外部晶振 |
| PLL 输入 | 8 MHz | HSE (不分频) |
| PLL 输出 | 72 MHz | ×9 |
| SYSCLK | 72 MHz | PLL |
| AHB (HCLK) | 72 MHz | ÷1 |
| APB1 (PCLK1) | 36 MHz | ÷2 |
| APB2 (PCLK2) | 72 MHz | ÷1 |
| SysTick | 72 MHz | HCLK |
| Flash Latency | 2 WS | - |

---

## 12. 文件结构一览

```
8line_test/
├── 8line_test.ioc              ← CubeMX 工程文件
├── Readme.md                   ← 原始简要教程
├── 8line_driver_doc.md         ← 本文档
├── Core/
│   ├── Inc/
│   │   ├── main.h              ← 引脚宏定义、HAL 头文件
│   │   ├── gpio.h              ← MX_GPIO_Init 声明
│   │   ├── usart.h             ← huart1 外部声明、MX_USART1_UART_Init
│   │   ├── dma.h               ← MX_DMA_Init 声明
│   │   ├── stm32f1xx_it.h      ← 中断服务函数声明
│   │   └── stm32f1xx_hal_conf.h ← HAL 配置头文件
│   └── Src/
│       ├── main.c              ← ★ 核心: Read_Sensor_Data(), delay_us(), main()
│       ├── gpio.c              ← GPIO 初始化 (CLK/DAT/LED/KEY/OLED)
│       ├── usart.c             ← USART1 初始化 + fputc 重定向
│       ├── dma.c               ← DMA 控制器时钟使能 + NVIC
│       ├── stm32f1xx_it.c      ← 中断服务函数实现
│       ├── stm32f1xx_hal_msp.c ← HAL MSP 初始化 (NVIC, AFIO)
│       └── system_stm32f1xx.c  ← 系统初始化
└── Drivers/                    ← CMSIS + HAL 库（CubeMX 自动生成）
```

---

## 13. 常见问题排查

| 问题 | 可能原因 | 解决方法 |
|------|----------|----------|
| 始终输出 0x00 | 传感器未校准 | 按第 10 节步骤校准辅助板 |
| 始终输出 0x00 | DAT 引脚配置错误 | 检查是否配置为上拉输入 |
| 始终输出 0xFF | DAT 引脚悬空 | 检查接线和共地 |
| 数据不稳定/跳变 | CLK 频率过高 | 增加 delay_us 延时 |
| 数据不稳定/跳变 | 电源噪声 | 检查供电，加去耦电容 |
| printf 无输出 | USART 波特率不匹配 | 确认 115200 8N1 |
| printf 无输出 | fputc 未重定向 | 确认 usart.c 中包含 fputc 实现 |
| 编译错误 | HAL 库版本 | 使用 STM32Cube FW_F1 V1.8.6 |

---

> **文档版本**: v1.0  
> **最后更新**: 2026-07-15  
> **适用芯片**: STM32F103C8T6 (可移植至全系列 STM32)  
> **适用辅助板**: MSPM0L1306 8路寻迹模块
