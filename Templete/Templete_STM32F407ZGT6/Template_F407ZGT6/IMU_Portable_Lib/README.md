# IMU 陀螺仪驱动与姿态解算 — 可移植库

> **版本**: v2.0 | **日期**: 2026-07-24 | **作者**: HUST-STI
> **适用芯片**: ICM-42688-P (TDK) / MPU6050 (TDK InvenSense)
> **已验证平台**: STM32F407ZGT6 @ 168MHz, Cortex-M4F

---

## 目录

1. [架构概览](#1-架构概览)
2. [文件清单](#2-文件清单)
3. [★ 统一 API 层：IMU.h/c](#3--统一-api-层imuhc)
4. [IMU 类型定义](#4-imu-类型定义)
5. [ICM-42688 驱动与滤波](#5-icm-42688-驱动与滤波)
6. [MPU6050 驱动与滤波](#6-mpu6050-驱动与滤波)
7. [ICM42688 vs MPU6050 差异](#7-icm42688-vs-mpu6050-差异)
8. [两种滤波对比 (互补 vs Mahony)](#8-两种滤波对比-互补-vs-mahony)
9. [使用示例](#9-使用示例)
10. [使用注意事项](#10-使用注意事项)
11. [跨芯片移植指南](#11-跨芯片移植指南)
12. [常见问题](#12-常见问题)

---

## 1. 架构概览

```
┌──────────────────────────────────────────────────────────────┐
│  应用层 (Mode_2.c / 你的业务代码)                               │
│  - 只调 IMU_* API，不关心底层传感器型号                          │
├──────────────────────────────────────────────────────────────┤
│  ★ 统一 API 层: IMU.h / IMU.c                                 │
│  - #define IMU_USE_MPU6050 切换传感器（一行改）                  │
│  - IMU_Mahony_Init/Update_Tick/Calibrate                      │
│  - IMU_Yaw_Abs_Get/Reset                                      │
│  - IMU_Turn_Yaw_Is_Ok / Is_Ok_Ex    ← 偏航到位检测             │
│  - IMU_Mahony_Real  (roll/pitch/yaw 统一出口)                   │
├──────────────────────────────────────────────────────────────┤
│  滤波层 (Mahony ★推荐 / 互补)                                   │
│  ┌─────────────────────┐ ┌───────────────────────┐           │
│  │ ICM42688_Mahony     │ │ MPU6050_Mahony        │           │
│  │ ICM42688_Angle      │ │ MPU6050_Angle         │           │
│  └─────────────────────┘ └───────────────────────┘           │
│  共用: IMU.h (ImuReal_Typedef 等 3 种结构体)                    │
├──────────────────────────────────────────────────────────────┤
│  驱动层                                                        │
│  ┌─────────────────────┐ ┌───────────────────────┐           │
│  │ ICM_42688_base      │ │ MPU6050_base           │           │
│  │ → ICM_Raw_Data      │ │ → MPU_Raw_Data         │           │
│  └─────────────────────┘ └───────────────────────┘           │
├──────────────────────────────────────────────────────────────┤
│  硬件: ICM-42688-P / MPU6050 (I2C 0x68)                       │
└──────────────────────────────────────────────────────────────┘
```

**核心原则**:
- 统一 API 层 (`IMU.h/c`) → 上层零改动换传感器
- 滤波层 → 100% 纯 C 数学，跨芯片零改动
- 驱动层 → 仅 I2C 读写需适配目标平台 (~30 行)

---

## 2. 文件清单

| 文件 | 传感器 | 层 | 移植难度 | 说明 |
|------|--------|-----|---------|------|
| **`IMU.h`** | **共用** | **★统一API** | 零改动 | 宏切换传感器 + Turn_Yaw 函数声明 |
| **`IMU.c`** | **共用** | **★统一API** | 零改动 | `IMU_Turn_Yaw_Is_Ok` 实函数 |
| `IMU.h` | 共用 | 类型+宏 | 零改动 | 结构体定义 + 传感器切换宏 + Turn_Yaw 声明 |
| `ICM_42688_base.h` | ICM | 驱动 | **需适配** | 寄存器地址、量程宏 |
| `ICM_42688_base.c` | ICM | 驱动 | **需适配** (~30行) | I2C 读写、灵敏度转换 |
| `ICM42688_Angle.h/c` | ICM | 互补滤波 | 零改动 | 0.98 gyro + 0.02 accel |
| `ICM42688_Mahony.h/c` | ICM | **Mahony** | 零改动 | 四元数 + PI 修正 |
| `MPU6050_base.h` | MPU | 驱动 | **需适配** | 寄存器地址、量程宏 |
| `MPU6050_base.c` | MPU | 驱动 | **需适配** (~30行) | I2C 读写、灵敏度转换 |
| `MPU6050_Angle.h/c` | MPU | 互补滤波 | 零改动 | 0.98 gyro + 0.02 accel |
| `MPU6050_Mahony.h/c` | MPU | **Mahony** | 零改动 | 四元数 + PI 修正 |
| `demo.c` | 共用 | 示例 | 参考 | Mode_2 完整示例 |

---

## 3. ★ 统一 API 层：IMU.h/c

### 3.1 设计目的

上层业务代码只调 `IMU_*` 函数，编译期通过一行宏切换传感器，无需改任何业务代码。

### 3.2 传感器切换

```c
// IMU.h 顶部，一行决定:
//#define IMU_USE_MPU6050    // 取消注释 → MPU6050，注释掉 → ICM42688
```

> 两个 IMU 默认 I2C 地址均为 0x68，不可同时挂在同一总线。如需双 IMU，需用 SPI 或不同 I2C 总线。

### 3.3 完整 API

```c
// ===== 初始化 (实为宏，自动映射到对应传感器的 Init) =====
IMU_Mahony_Init(1);        // 1=自动标定零偏(需静止), 0=跳过标定
IMU_Mahony_Init(0);        // 快速启动，使用预设零偏

// ===== 20ms Tick =====
IMU_Mahony_Update_Tick();  // 读传感器→Mahony解算→更新 IMU_Mahony_Real

// ===== 校准 =====
IMU_Mahony_Calibrate(1000); // 运行时重标定（角度归零）

// ===== 输出 =====
IMU_Mahony_Real.roll        // 横滚角 (±180°)
IMU_Mahony_Real.pitch       // 俯仰角 (±90°)
IMU_Mahony_Real.yaw         // 偏航角 (±180°)
IMU_Mahony_Real.AccX/Y/Z    // 归一化加速度

// ===== 绝对累计偏航角 =====
float abs_yaw = IMU_Yaw_Abs_Get();    // 顺时针增大，无跳变，可超 360°
IMU_Yaw_Abs_Reset();                   // 归零（不影响姿态）

// ===== 偏航到位检测 ★ =====
if (IMU_Turn_Yaw_Is_Ok(90.0f))        // 转到 90° ± 3° 了？
    LED_On();
if (IMU_Turn_Yaw_Is_Ok_Ex(180.0f, 5.0f))  // 转到 180° ± 5° 了？
    Motor_Stop();

// ===== 零偏变量 (可直接读写，配合 EEPROM 持久化) =====
IMU_Mahony_GyroBiasX       // 陀螺X零偏 (°/s)
IMU_Mahony_GyroBiasY       // 陀螺Y零偏 (°/s)
IMU_Mahony_GyroBiasZ       // 陀螺Z零偏 (°/s)
```

### 3.4 Turn_Yaw 到位检测

两个实函数（非宏），定义在 `IMU.c`：

| 函数 | 参数 | 说明 |
|------|------|------|
| `IMU_Turn_Yaw_Is_Ok(target)` | target: 目标角度 | 默认死区 ±3° |
| `IMU_Turn_Yaw_Is_Ok_Ex(target, deadband)` | target + deadband | 自定义死区 |

**工作原理**: `|IMU_Yaw_Abs_Get() - target| <= deadband` → 返回 1

**典型用法**（机器人转弯控制）:
```c
void turn_to_angle(float target_yaw)
{
    IMU_Yaw_Abs_Reset();           // 从当前姿态开始计量
    Motor_Turn_Start();            // 启动旋转电机

    while (!IMU_Turn_Yaw_Is_Ok(target_yaw))
    {
        IMU_Mahony_Update_Tick();  // 持续更新
        delay_ms(20);
    }

    Motor_Stop();                  // 到位！
}
```

### 3.5 宏映射表

| 统一 API | IMU_USE_MPU6050 未定义 | IMU_USE_MPU6050 已定义 |
|----------|----------------------|----------------------|
| `IMU_Mahony_Init(n)` | `ICM42688_Mahony_Init(n)` | `MPU6050_Mahony_Init(n)` |
| `IMU_Mahony_Update_Tick()` | `ICM42688_Mahony_Update_Tick()` | `MPU6050_Mahony_Update_Tick()` |
| `IMU_Mahony_Calibrate(n)` | `ICM42688_Mahony_Calibrate(n)` | `MPU6050_Mahony_Calibrate(n)` |
| `IMU_Yaw_Abs_Get()` | `ICM_Yaw_Abs_Get()` | `MPU_Yaw_Abs_Get()` |
| `IMU_Yaw_Abs_Reset()` | `ICM_Yaw_Abs_Reset()` | `MPU_Yaw_Abs_Reset()` |
| `IMU_Mahony_Real` | `ICM_Mahony_Real` | `MPU_Mahony_Real` |
| `IMU_Mahony_GyroBiasX/Y/Z` | `ICM_Mahony_GyroBiasX/Y/Z` | `MPU_Mahony_GyroBiasX/Y/Z` |

---

## 4. IMU 类型定义

```c
// 最终输出
typedef struct {
    float AccX, AccY, AccZ;  // 归一化加速度 (方向余弦)
    float roll;              // 横滚角 (°)  ±180°
    float pitch;             // 俯仰角 (°)  ±90°
    float yaw;               // 偏航角 (°)  ±180°
} ImuReal_Typedef;

// 零偏值 (互补滤波用)
typedef struct {
    float AccErrorX, AccErrorY, AccErrorZ;   // 加速度零偏 (g)
    float GyroErrorX, GyroErrorY, GyroErrorZ; // 陀螺零漂 (°/s)
} ImuOffset_Typedef;
```

以上三种结构体定义在 `IMU.h` 中，被 Mahony 和互补滤波共用，形成统一输出格式。
不再有独立的 `Imu_Types.h` 文件。

---

## 5. ICM-42688 驱动与滤波

### 5.1 硬件连接

> ⚠️ ICM42688 使用 I2C 时 CS 必须拉高 (3.3V)，低电平默认为 SPI。

| 引脚 | 连接 |
|------|------|
| SDA / SCL | I2C (上拉 4.7kΩ) |
| AD0 | GND → 地址 0x68 |
| CS | **3.3V** → I2C 模式 |

### 5.2 当前配置

| 参数 | 值 | 噪声 |
|------|-----|------|
| 加速度计 | ±4g, 1000Hz, 低噪声 | 70 μg/√Hz ✨ |
| 陀螺仪 | ±500°/s, 1000Hz | — |

### 5.3 ⚠️ FS_SEL 编码陷阱

ICM-42688 量程位与 MPU6050 **完全相反**：

| FS_SEL | MPU6050 | ICM-42688 |
|--------|---------|-----------|
| 000 | ±250°/s (最小) | **±2000°/s (最大)** |
| 010 | ±1000°/s | **±500°/s** |
| 011 | ±2000°/s (最大) | **±250°/s (最小)** |

本库已修正。

### 5.4 驱动 API

```c
void ICM42688_Init(void);                              // 硬件初始化
uint8_t ICM42688_GetID(void);                          // 返回 0x47
void ICM42688_Update_Data(void);                       // → ICM_Raw_Data (g, °/s)

extern ICM42688_Raw_Data ICM_Raw_Data;                 // .AX/AY/AZ/GX/GY/GZ
```

### 5.5 Mahony 参数 (ICM42688_Mahony.h)

| 宏 | 值 | 说明 |
|----|-----|------|
| `MAHONY_KP` | 5.12 | PI 比例增益 |
| `MAHONY_KI` | 0.001 | PI 积分增益 |
| `MAHONY_HALF_T` | 0.010 | 半采样周期 (20ms/2) |

### 5.6 API

```c
void ICM42688_Mahony_Init(uint8_t doCalib);
void ICM42688_Mahony_Update_Tick(void);
void ICM42688_Mahony_Calibrate(int samples);
float ICM_Yaw_Abs_Get(void);
void ICM_Yaw_Abs_Reset(void);
extern ImuReal_Typedef ICM_Mahony_Real;
extern float ICM_Mahony_GyroBiasX, ICM_Mahony_GyroBiasY, ICM_Mahony_GyroBiasZ;
```

### 5.7 互补滤波 (Angle)

```c
void ICM42688_Angle_Init(void);
void ICM42688_Data_Error_Check(int sample_cnt);
void ICM42688_Angle_Update_Tick(void);
extern ImuReal_Typedef ICM_Real;
```

---

## 6. MPU6050 驱动与滤波

### 6.1 硬件连接

| 引脚 | 连接 |
|------|------|
| SDA / SCL | I2C (上拉 4.7kΩ) |
| AD0 | GND → 地址 0x68 |

### 6.2 当前配置

| 参数 | 值 | 噪声 |
|------|-----|------|
| 加速度计 | ±2g, DLPF=5 (10Hz) | 400 μg/√Hz |
| 陀螺仪 | ±250°/s | — |

> 量程可通过 `MPU6050_base.c` 顶部 `ACCEL_RANGE` / `GYRO_RANGE` 宏修改。

### 6.3 驱动 API

```c
void MPU6050_Init(void);                               // 硬件初始化
uint8_t MPU6050_GetID(void);                           // 返回 0x68
void MPU6050_Update_Data(void);                        // → MPU_Raw_Data (g, °/s)

extern MPU6050_Raw_Data MPU_Raw_Data;                  // .AX/AY/AZ/GX/GY/GZ
```

### 6.4 Mahony 参数 (MPU6050_Mahony.h)

| 宏 | 值 | 说明 |
|----|-----|------|
| `MPU_MAHONY_KP` | 5.12 | PI 比例增益 |
| `MPU_MAHONY_KI` | 0.001 | PI 积分增益 |
| `MPU_MAHONY_HALF_T` | 0.010 | 半采样周期 |
| `MPU_MAHONY_CALIB_SAMPLES` | 1000 | 标定采样数 |

> 参数与 ICM42688 完全相同，MPU6050 噪声较大 (~6倍)，高动态下收敛稍慢。

### 6.5 API

```c
void MPU6050_Mahony_Init(uint8_t doCalib);
void MPU6050_Mahony_Update_Tick(void);
void MPU6050_Mahony_Calibrate(int samples);
float MPU_Yaw_Abs_Get(void);
void MPU_Yaw_Abs_Reset(void);
extern ImuReal_Typedef MPU_Mahony_Real;
extern float MPU_Mahony_GyroBiasX, MPU_Mahony_GyroBiasY, MPU_Mahony_GyroBiasZ;
```

### 6.6 互补滤波 (Angle)

```c
void MPU6050_Angle_Init(void);
void MPU6050_Data_Error_Check(int sample_cnt);
void MPU6050_Angle_Update_Tick(void);
extern ImuReal_Typedef MPU_Real;
```

---

## 7. ICM42688 vs MPU6050 差异

| 维度 | ICM-42688 | MPU6050 |
|------|-----------|---------|
| 加速度噪声 | **70 μg/√Hz** ✨ | 400 μg/√Hz |
| 默认量程 | ±4g / ±500°/s | ±2g / ±250°/s |
| 数据长度 | 12B (无温度) | 14B (含温度) |
| WHO_AM_I | 0x47 | 0x68 |
| PWR_MGMT | 0x4E | 0x6B |
| FS_SEL 编码 | **0=最大** ⚠️ | 0=最小 |
| I2C 要求 | CS 必须拉高 | 无特殊要求 |
| 价格 | 较高 | 较低、普及 |

---

## 8. 两种滤波对比 (互补 vs Mahony)

| 维度 | 互补滤波 (Angle) | Mahony AHRS ★ |
|------|-----------------|---------------|
| 算法 | 2轴互补融合 | 四元数 + PI 修正 |
| 代码量 | ~120 行 | ~175 行 |
| 运算量 | ~50 周期 | ~600 周期 |
| Yaw 精度 | **差** (纯积分漂移) | **较好** (相对准确) |
| 倾斜 Yaw | **很差** | **好** (四元数解耦) |
| 万向节死锁 | 无 (仅2轴) | 无 (四元数天然) |
| 适用场景 | 短时水平 | **所有场景** |

**推荐**: 一律使用 Mahony。互补滤波仅保留作学习参考。

---

## 9. 使用示例

### 9.1 最简例程（通过统一 API）

```c
#include "IMU.h"   // 一行 include，自动包含对应传感器的 Mahony

void setup(void)
{
    IMU_Mahony_Init(1);          // 自动标定零偏（保持设备静止！）
    IMU_Yaw_Abs_Reset();
}

void timer_20ms_callback(void)
{
    IMU_Mahony_Update_Tick();    // 读数据 + Mahony 解算
    printf("%.1f,%.1f,%.1f,%.1f\r\n",
           IMU_Mahony_Real.roll, IMU_Mahony_Real.pitch,
           IMU_Mahony_Real.yaw, IMU_Yaw_Abs_Get());
}

void loop(void)
{
    printf("R:%.1f P:%.1f Y:%.1f\n",
           IMU_Mahony_Real.roll, IMU_Mahony_Real.pitch, IMU_Mahony_Real.yaw);

    // 转到 90° 时 LED 亮
    if (IMU_Turn_Yaw_Is_Ok(90.0f))
        LED_On();
    else
        LED_Off();

    delay_ms(100);
}
```

### 9.2 机器人转弯控制

```c
// 转 180° 掉头
void robot_turn_180(void)
{
    IMU_Yaw_Abs_Reset();                    // 从当前位置开始计量
    Motor_Left_Set(50);                     // 左轮前进
    Motor_Right_Set(-50);                   // 右轮后退 → 顺时针旋转

    while (!IMU_Turn_Yaw_Is_Ok_Ex(180.0f, 5.0f))   // 180° ± 5°
    {
        IMU_Mahony_Update_Tick();
        delay_ms(20);
    }

    Motor_Stop();                           // 到位，刹车
}
```

### 9.3 配合 EEPROM 持久化零偏

```c
// 首次使用
void first_use(void)
{
    IMU_Mahony_Init(1);    // 自动标定
    // 保存到 EEPROM:
    eeprom_write(IMU_Mahony_GyroBiasX);
    eeprom_write(IMU_Mahony_GyroBiasY);
    eeprom_write(IMU_Mahony_GyroBiasZ);
}

// 后续上电
void normal_start(void)
{
    IMU_Mahony_GyroBiasX = eeprom_read();
    IMU_Mahony_GyroBiasY = eeprom_read();
    IMU_Mahony_GyroBiasZ = eeprom_read();
    IMU_Mahony_Init(0);    // 秒启动，无标定等待
}
```

### 9.4 直接调用底层（不用统一 API）

如果不想用统一 API 层，可以直接包含底层 Mahony 头文件：

```c
#include "MPU6050_Mahony.h"    // 或 ICM42688_Mahony.h

MPU6050_Mahony_Init(1);
MPU6050_Mahony_Update_Tick();
// ...
```

### 9.5 Mode_2 完整例程

见同目录 `demo.c`，展示：OLED 显示 + 串口 CSV + Turn_Yaw 到位检测 + LED 指示。

---

## 10. 使用注意事项

### 10.1 处理周期
- 默认 **20ms (50Hz)**，`Update_Tick()` 固定
- ODR 远高于处理周期，不丢数据
- 改周期需同时改 `dt` 参数 + `MAHONY_HALF_T`

### 10.2 标定
- **自动标定时必须绝对静止！** 否则零偏不准
- 时长 ≈ 1 秒 (1000 采样)
- 运行中可调 `Calibrate(1000)` 重标定（角度归零）

### 10.3 Yaw 漂移
- 无磁力计时 yaw 无绝对参考，长期缓慢漂移
- Mahony PI 积分自动补偿
- 需绝对 yaw → 外接磁力计

### 10.4 初始化顺序

```c
IMU_Mahony_Init(0);       // 1. 硬件初始化（先）
// ... 其他初始化 ...
Initial_Timer();           // 2. 定时器启动（最后！）
```

### 10.5 坐标系
- 芯片丝印朝上 → Z 轴向上 = +1g
- roll(+右倾) / pitch(+抬头) / yaw(+顺时针)

---

## 11. 跨芯片移植指南

### 11.1 移植工作量

| 文件 | 需改动 | 说明 |
|------|--------|------|
| `ICM_42688_base.c` | ~30 行 | I2C 读写 + 延时 |
| `MPU6050_base.c` | ~30 行 | I2C 读写 + 延时 |
| `IMU.h` | 0 行 | 类型定义 + 宏 + 函数声明 |
| `IMU.c` | 0 行 | `fabsf` + 宏，平台无关 |
| 6 个滤波文件 | **0 行** | 纯数学 |

### 11.2 通用移植模板

任何平台只需实现三个函数：

```c
void     my_i2c_write(dev_addr, reg_addr, data);   // I2C 写
uint8_t  my_i2c_read (dev_addr, reg_addr);         // I2C 读
void     my_delay_ms (uint32_t ms);                 // 毫秒延时
```

然后在对应的 Base 驱动中替换 `WriteReg`/`ReadReg`/`HAL_Delay`。

### 11.3 平台示例

**ESP32 (ESP-IDF)**:
```c
void MPU6050_WriteReg(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0xD0, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(cmd);
}
```

**TI MSPM0 (DriverLib)**:
```c
void MPU6050_WriteReg(uint8_t reg, uint8_t data) {
    DL_I2C_fillControllerTXFIFO(I2C0, &dev_addr, 1);
    DL_I2C_fillControllerTXFIFO(I2C0, &reg, 1);
    DL_I2C_fillControllerTXFIFO(I2C0, &data, 1);
    DL_I2C_startControllerTransfer(I2C0);
    while (DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_BUSY);
}
```

### 11.4 I2C 总线恢复

两个 Base 驱动均包含 `I2C_Recover()` 函数。移植时可保留（替换 GPIO+HAL 调用）或删除（I2C 可靠时）。

### 11.5 ESP32 完整清单

```
1. 复制整个 IMU_Portable_Lib/ 到目标项目
2. *_base.c 中:
   - 删除 LED_Flash.h、Timer_Counter.h
   - 删除 I2C_Recover() 或替换
   - 替换 I2C 句柄为 I2C_NUM_0
   - 替换 WriteReg/ReadReg 用 ESP-IDF API
   - 替换 HAL_Delay → vTaskDelay
3. 其余文件: 0 行改动
```

---

## 12. 常见问题

### Q: 上电后 yaw 疯狂旋转？
**A**: 陀螺零偏不准。确认 Init(1) 时设备静止。或手动填入 bias 后 Init(0)。

### Q: 旋转 90° 只显示 45°？
**A**: 量程寄存器配置错误。ICM42688 的 FS_SEL 编码与 MPU6050 相反（见 §5.3）。

### Q: 静止时 pitch/roll 不是 0？
**A**: 加速度计有零偏。Mahony 依赖 PI 收敛（约 0.5s），无需额外校准。

### Q: 剧烈运动后角度回不来？
**A**: 外部加速度污染。Mahony PI 约 0.5~1s 修正。持续高动态可降低 Kp (2.0~3.0)。

### Q: ICM 和 MPU 能同时用吗？
**A**: 不能。默认 I2C 地址均为 0x68，会冲突。需不同 I2C 总线或改地址。

### Q: 怎么切换传感器？
**A**: `IMU.h` 中取消/注释 `#define IMU_USE_MPU6050`。一行改，全局生效。

### Q: 从 20ms 改 10ms Tick？
**A**: 修改 `Update(0.010f)` + `MAHONY_HALF_T = 0.005f`。Kp/Ki 等价缩放 √2 倍。

### Q: Turn_Yaw 用的是标准 yaw 还是累计 yaw？
**A**: 用的是 `yaw_abs`（累计值），顺时针持续增大无跳变。标准 `yaw` 在 ±180° 处跳变，不做到位判断。

### Q: 两个传感器用哪个好？
**A**: ICM-42688 噪声低 6 倍，精度更高。MPU6050 便宜、货源多。预算够选 ICM，量产选 MPU。

---

> **文件版本一致性**: 本 README 描述的文件版本与同目录 `.c/.h` 匹配。
> 复制到其他项目时请将整个 `IMU_Portable_Lib/` 目录一并复制。
