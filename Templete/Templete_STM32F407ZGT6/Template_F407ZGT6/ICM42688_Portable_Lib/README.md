# ICM-42688 陀螺仪驱动与姿态解算 — 可移植库

> **版本**: v1.0 | **日期**: 2026-07-23 | **作者**: HUST-STI
> **适用芯片**: ICM-42688-P (TDK InvenSense)
> **已验证平台**: STM32F407ZGT6 @ 168MHz, Cortex-M4F

---

## 目录

1. [架构概览](#1-架构概览)
2. [文件清单](#2-文件清单)
3. [Base 驱动层](#3-base-驱动层)
4. [滤波方法 A：互补滤波](#4-滤波方法-a互补滤波)
5. [滤波方法 B：Mahony AHRS（推荐）](#5-滤波方法-bmahony-ahrs推荐)
6. [两种滤波对比与选型](#6-两种滤波对比与选型)
7. [使用示例](#7-使用示例)
8. [使用注意事项](#8-使用注意事项)
9. [跨芯片移植指南](#9-跨芯片移植指南)
10. [常见问题](#10-常见问题)

---

## 1. 架构概览

```
┌────────────────────────────────────────────────┐
│  应用层 (demo.c / Mode_2.c)                     │
│  - 初始化: Init(1) 或 Init(0)                   │
│  - 20ms Tick: Update_Tick()                     │
│  - 显示: OLED / 串口CSV                          │
├────────────────────────────────────────────────┤
│  滤波层 (Function/)                             │
│  ┌──────────────────┐ ┌──────────────────────┐ │
│  │ ICM42688_Angle   │ │ ICM42688_Mahony ★推荐 │ │
│  │ 互补滤波          │ │ 四元数+PI Mahony AHRS │ │
│  │ 简单/快速/yaw漂移 │ │ 无死锁/yaw相对准确    │ │
│  └──────────────────┘ └──────────────────────┘ │
│  共用: Imu_Types.h (ImuReal_Typedef 输出)       │
├────────────────────────────────────────────────┤
│  驱动层 (Hardware/)                             │
│  ICM_42688_base: I2C读写 + 量程配置 + 灵敏度转换 │
│  输出: ICM_Raw_Data (AX/AY/AZ/g, GX/GY/GZ °/s) │
├────────────────────────────────────────────────┤
│  硬件: ICM-42688-P (I2C 0x68, AD0=GND)         │
└────────────────────────────────────────────────┘
```

**核心原则**：滤波层代码 **100% 纯 C 数学运算**（仅依赖 `math.h`），可无修改跨芯片移植。只有 Base 驱动层需要根据目标平台适配 I2C 接口。

---

## 2. 文件清单

| 文件 | 层 | 移植难度 | 说明 |
|------|-----|---------|------|
| `ICM_42688_base.h` | 驱动 | **需适配** | 寄存器地址、量程宏、灵敏度、`ICM42688_Raw_Data` 结构体 |
| `ICM_42688_base.c` | 驱动 | **需适配** | I2C 读写、初始化配置、数据读取与灵敏度转换 |
| `Imu_Types.h` | 共用 | 零改动 | 通用数据类型: `ImuReal_Typedef`(roll/pitch/yaw) |
| `ICM42688_Angle.h` | 滤波 | 零改动 | 互补滤波系数 + API 声明 |
| `ICM42688_Angle.c` | 滤波 | 零改动 | 互补滤波实现 (0.98 gyro + 0.02 accel) |
| `ICM42688_Mahony.h` | 滤波 | 零改动 | **★推荐** Mahony AHRS 参数 + API |
| `ICM42688_Mahony.c` | 滤波 | 零改动 | **★推荐** Mahony AHRS 四元数实现 |
| `demo.c` | 示例 | 参考 | Mode_2.c 提取的完整使用示例 |

---

## 3. Base 驱动层

### 3.1 硬件连接

+ 注意：ICM42688如果使用IIC驱动，那么必须拉高CS引脚(低默认为SPI，3V3即为IIC)

| ICM-42688 引脚 | 连接 |
|---------------|------|
| SDA | I2C 数据线 (上拉 4.7kΩ) |
| SCL | I2C 时钟线 (上拉 4.7kΩ) |
| AD0/SDO | **GND** → 7-bit 地址 **0x68** |
| VDD | 3.3V (或 1.8V，取决于 VDDIO) |
| VDDIO | 3.3V (与 MCU IO 电平一致) |
| INT | 可选，未使用 |

### 3.2 当前配置

| 参数 | 值 | 寄存器 |
|------|-----|--------|
| 加速度计量程 | **±4g** | ACCEL_CONFIG0[7:5] = 010 |
| 陀螺仪量程 | **±500°/s** | GYRO_CONFIG0[7:5] = 010 |
| 加速度计 ODR | **1000 Hz** | ACCEL_CONFIG0[3:0] = 6 |
| 陀螺仪 ODR | **1000 Hz** | GYRO_CONFIG0[3:0] = 6 |
| 工作模式 | **低噪声 (Low Noise)** | PWR_MGMT0 = 0x0F |

### 3.3 ⚠️ 关键：FS_SEL 编码陷阱

ICM-42688 的量程选择位编码与 MPU6050 **完全相反**。这是本库从 MPU6050 移植时踩过的坑——如果不注意，所有读数恰好错 2 倍。

| FS_SEL 值 | MPU6050 含义 | ICM-42688 含义 |
|-----------|-------------|---------------|
| `000` | ±250°/s (最小) | **±2000°/s (最大)** |
| `001` | ±500°/s | **±1000°/s** |
| `010` | ±1000°/s | **±500°/s** |
| `011` | ±2000°/s (最大) | **±250°/s (最小)** |

加速度计同理：`000=±16g, 001=±8g, 010=±4g, 011=±2g`

本库已修正，**无需再改**。

### 3.4 驱动层 API

```c
// ===== 初始化与状态 =====
void     ICM42688_Init(void);              // 硬件复位 + 配置量程 + 低噪声模式
uint8_t  ICM42688_GetID(void);             // 返回 WHO_AM_I (应为 0x47)

// ===== 数据读取 (内部使用，滤波层自动调用) =====
void     ICM42688_Update_Data(void);       // 读 12 字节 → ICM_Raw_Data (g / °/s)

// ===== 直接 I2C 操作 (移植时重写) =====
void     ICM42688_WriteReg(uint8_t addr, uint8_t data);
uint8_t  ICM42688_ReadReg(uint8_t addr);
void     ICM42688_GetData(int16_t *ax, *ay, *az, *gx, *gy, *gz); // 原始ADC
```

### 3.5 数据结构

```c
// 原始数据 (Base 层输出)
typedef struct {
    float AX, AY, AZ;   // 加速度 (g)
    float GX, GY, GZ;   // 角速度 (°/s)
} ICM42688_Raw_Data;

extern ICM42688_Raw_Data ICM_Raw_Data;
```

### 3.6 灵敏度

| 量程 | 加速度灵敏度 | 陀螺灵敏度 |
|------|------------|-----------|
| ±2g / ±250°/s | 16384 LSB/g | 131.0 LSB/(°/s) |
| ±4g / ±500°/s | 8192 LSB/g | 65.5 LSB/(°/s) |
| ±8g / ±1000°/s | 4096 LSB/g | 32.8 LSB/(°/s) |
| ±16g / ±2000°/s | 2048 LSB/g | 16.4 LSB/(°/s) |

当前使用 ±4g / ±500°/s。

---

## 4. 滤波方法 A：互补滤波

### 4.1 算法原理

```
roll_acc  = atan2(AY,  AZ)       // 加速度计静态角度
pitch_acc = atan2(-AX, AZ)

roll_gyro  += GX * dt            // 陀螺积分
pitch_gyro += GY * dt
yaw        += GZ * dt            // Yaw: 纯积分，无绝对参考！

roll  = 0.98 * roll_gyro  + 0.02 * roll_acc    // 互补融合
pitch = 0.98 * pitch_gyro + 0.02 * pitch_acc
```

- 高通陀螺 (0.98) + 低通加速度计 (0.02)
- **优势**: 极简，运算量极小
- **劣势**: Yaw 纯积分 → **会漂移**；倾斜时轴间耦合 → **Yaw 误差大**

### 4.2 API

```c
void ICM42688_Angle_Init(void);                    // 硬件初始化 + 设置零偏
void ICM42688_Data_Error_Check(int sample_cnt);    // 手动标定零偏 (静止采样)
void ICM42688_Angle_Update_Tick(void);             // 20ms Tick 入口
```

### 4.3 数据结构

```c
extern ImuReal_Typedef  ICM_Real;    // 输出: .roll, .pitch, .yaw (°)
extern ImuOffset_Typedef ICM_Offset; // 零偏: .AccErrorX/Y/Z, .GyroErrorX/Y/Z
extern ImuCali_Typedef   ICM_Cali;   // 校准后中间量
```

---

## 5. 滤波方法 B：Mahony AHRS（推荐）

### 5.1 算法原理

基于 Robert Mahony 2008 年论文的显式互补滤波器 AHRS 算法，仅使用加速度计+陀螺仪（无磁力计）：

```
每 Tick (dt=20ms, halfT=10ms):

1. 加速度归一化
2. 从四元数计算估计重力方向: v = R(q)' · [0,0,1]
3. 叉积求误差: e = accel × v
4. PI 修正陀螺: g_corr = g + Kp·e + Ki·∫e
5. 四元数一阶RK积分: q += 0.5·dt·q⊗[0, g_corr]
6. 四元数归一化
7. 提取欧拉角 (atan2, ±180°)
```

**关键优势**:
- **四元数** → 无万向节死锁，任意姿态都正确
- **PI 控制器** → 重力矢量连续修正陀螺漂移
- **3D 轴间解耦** → 倾斜下 yaw 仍相对准确

### 5.2 参数

| 宏 | 值 | 说明 |
|----|-----|------|
| `MAHONY_KP` | 5.12 | PI 比例增益（低噪声 ICM 可加大至 8~10） |
| `MAHONY_KI` | 0.001 | PI 积分增益 |
| `MAHONY_HALF_T` | 0.010 | 半采样周期 (20ms Tick) |
| `MAHONY_CALIB_SAMPLES` | 1000 | 自动标定采样数 |

### 5.3 API

```c
// ===== 初始化 =====
void ICM42688_Mahony_Init(uint8_t doCalib);
//   doCalib=1: 自动采样 MAHONY_CALIB_SAMPLES 次标定陀螺零偏（需静止）
//   doCalib=0: 跳过标定，使用 ICM_Mahony_GyroBiasX/Y/Z 当前值

// ===== 运行时 =====
void ICM42688_Mahony_Update_Tick(void);              // 20ms Tick 入口
void ICM42688_Mahony_Calibrate(int samples);         // 运行时手动重标定

// ===== 零偏变量（extern，供 AT24C02/EEPROM 持久化） =====
extern float ICM_Mahony_GyroBiasX;   // 陀螺X零偏 (°/s)，可直接赋值
extern float ICM_Mahony_GyroBiasY;   // 陀螺Y零偏 (°/s)
extern float ICM_Mahony_GyroBiasZ;   // 陀螺Z零偏 (°/s)

// ===== 输出 =====
extern ImuReal_Typedef ICM_Mahony_Real;  // .roll, .pitch, .yaw (°)

// ===== 绝对累计偏航角（yaw_abs） =====
float ICM_Yaw_Abs_Get(void);          // 顺时针持续增大，无 ±180° 跳变，可超过 360°
void  ICM_Yaw_Abs_Reset(void);        // 归零 yaw_abs（不影响 yaw 解算）
```

### 5.4 绝对累计偏航角 (yaw_abs)

`ICM_Mahony_Real.yaw` 在 ±180° 处跳变（+180→-180），不利于统计"转了多少度"。`ICM_Yaw_Abs_Get()` 自动解绕，提供无跳变的累计角度：

| yaw | yaw_abs | 说明 |
|-----|---------|------|
| 0° | 0° | 初始 |
| -90° | 90° | 顺时针转 90° |
| -179° | 179° | 即将跨边界 |
| +179° | 181° | 自动解绕，无跳变 |
| +90° | 270° | 继续顺时针 |
| 0° | 360° | 满一圈 |

- **顺时针持续增大**，逆时针减小
- 超过 360° 继续累加（转两圈 = 720°）
- `ICM_Yaw_Abs_Reset()` 可随时归零（不影响姿态解算）

### 5.5 标定与持久化流程

**首次使用**:
```c
ICM42688_Mahony_Init(1);                          // 自动标定
// 此时 ICM_Mahony_GyroBiasX/Y/Z 已更新为实测值
// → 保存这三个变量到 EEPROM
```

**后续上电（有 EEPROM）**:
```c
ICM_Mahony_GyroBiasX = 从EEPROM读取值;             // 恢复零偏
ICM_Mahony_GyroBiasY = 从EEPROM读取值;
ICM_Mahony_GyroBiasZ = 从EEPROM读取值;
ICM42688_Mahony_Init(0);                          // 跳过标定，秒启动
```

**硬编码方式（无 EEPROM）**:
```c
// 直接在 ICM42688_Mahony.c 中填入实测值（已填入示例值）:
float ICM_Mahony_GyroBiasX = -0.00227481127f;      // ← 改这里
float ICM_Mahony_GyroBiasY =  0.208900467f;        // ← 改这里
float ICM_Mahony_GyroBiasZ =  0.0901221409f;       // ← 改这里
// 然后调用 Init(0) 即可
```

---

## 6. 两种滤波对比与选型

| 维度 | 互补滤波 | Mahony AHRS |
|------|---------|-------------|
| 算法 | 2轴互补融合 | 四元数 + PI 修正 |
| 代码量 | ~120 行 | ~175 行 |
| 运算量/次 | ~50 周期 | ~600 周期 (<100μs @168MHz) |
| Yaw 精度 | **差** (纯积分漂移) | **较好** (相对准确) |
| 倾斜 Yaw | **很差** (轴间耦合) | **好** (四元数解耦) |
| 万向节死锁 | 无 (仅做2轴) | 无 (四元数天然) |
| 需要调参 | 1 个系数 | 2 个系数 (Kp, Ki) |
| 适用场景 | 短时水平姿态 | **所有场景 ★推荐** |

**推荐**：一律使用 Mahony AHRS。互补滤波仅保留作学习参考和超低算力场景（如 8-bit MCU 无 FPU）。

---

## 7. 使用示例

以下是一个完整的最小示例（从 `demo.c` 提取），展示初始化 → 处理 → 显示的完整流程：

```c
#include "ICM42688_Mahony.h"   // 或 ICM42688_Angle.h
#include <stdio.h>             // printf (调试用)

// ==================== 初始化 ====================
void setup(void)
{
    // 方式 A: 自动标定 (首次/不确定零偏时使用)
    ICM42688_Mahony_Init(1);   // 设备必须保持静止！

    // 方式 B: 使用已知零偏 (快启动)
    // ICM_Mahony_GyroBiasX = -0.0023f;  // 从 EEPROM 恢复
    // ICM_Mahony_GyroBiasY =  0.2089f;
    // ICM_Mahony_GyroBiasZ =  0.0901f;
    // ICM42688_Mahony_Init(0);
}

// ==================== 20ms 定时中断 ====================
void timer_20ms_callback(void)
{
    ICM42688_Mahony_Update_Tick();  // 读数据 + Mahony 解算

    // 输出角度 (串口 CSV，方便绘图)
    printf("%.2f,%.2f,%.2f\r\n",
           ICM_Mahony_Real.roll,
           ICM_Mahony_Real.pitch,
           ICM_Mahony_Real.yaw);
}

// ==================== 主循环 (显示) ====================
void loop(void)
{
    printf("Roll:%.1f  Pitch:%.1f  Yaw:%.1f\n",
           ICM_Mahony_Real.roll,
           ICM_Mahony_Real.pitch,
           ICM_Mahony_Real.yaw);
    delay_ms(100);
}
```

**输出说明**:
- `roll` (±180°): 绕 X 轴旋转，正值 = 右侧下沉
- `pitch` (±90°): 绕 Y 轴旋转，正值 = 抬头
- `yaw` (±180°): 绕 Z 轴旋转，正值 = 顺时针（从上往下看）
- `AccX/Y/Z`: 归一化加速度（方向余弦，用于调试）

---

## 8. 使用注意事项

### 8.1 处理周期

- **默认 20ms (50Hz)**，由 `ICM42688_Mahony_Update_Tick()` 固定使用
- 传感器 ODR 为 1000Hz，远高于处理周期，不会丢数据
- **如需改周期**：修改 `ICM42688_Mahony_Update(0.020f)` 中的 dt 参数，并同步改 `MAHONY_HALF_T`（值为 dt/2）
- 互补滤波同理：修改 `ICM42688_Raw_Deal(20)` 的传参

### 8.2 标定

- **自动标定时设备必须绝对静止！** 否则零偏不准，yaw 会快速漂移
- 标定时长 = `MAHONY_CALIB_SAMPLES / ODR ≈ 1000/1000 = 1 秒`
- 运行时可调用 `ICM42688_Mahony_Calibrate(1000)` 重新标定（角度会归零）

### 8.3 Yaw 漂移

- **无磁力计时 yaw 没有绝对参考**，长期会缓慢漂移（典型 <0.2°/s）
- Mahony PI 积分项会自动补偿陀螺零偏漂移，保持相对 yaw 准确
- 如需绝对 yaw，需外接磁力计（HMC5883L/QMC5883L 等）并启用 Mahony 磁力计融合

### 8.4 初始化顺序

```c
// 正确顺序:
ICM42688_Mahony_Init(0);      // 1. 硬件初始化 + 四元数复位
// ... 其他初始化 ...
Initial_Timer();              // 2. 启动 20ms 定时器（最后！）
// 定时器 ISR 中调用 ICM42688_Mahony_Update_Tick()
```

**注意**: 定时器必须在 Init 之后启动，否则 ISR 可能在硬件未就绪时触发 Update，导致 I2C 错误或读取垃圾数据。

### 8.5 浮点精度

- 所有运算使用 `float` (IEEE 754 单精度)，STM32F4 有硬件 FPU
- 四元数归一化用 `sqrtf()` 而非 `sqrt()`（单精度更快）
- 若移植到无 FPU 的 MCU（如 MSPM0、AVR），考虑：
  - 软件浮点（慢但正确）
  - 或改用定点数 Mahony（需重写）

### 8.6 坐标系

- ICM-42688 参考系：芯片丝印面朝上时，**Z 轴向上 = +1g**
- 欧拉角提取：roll(atan2 ±180°), pitch(asin ±90°), yaw(atan2 ±180°)
- Pitch 超过 ±90° 会翻转，这是 asin 提取的固有特性。如需全范围 pitch，可改用 atan2 提取

---

## 9. 跨芯片移植指南

### 9.1 移植工作量

| 文件 | 需改动行数 | 改动说明 |
|------|-----------|---------|
| `ICM_42688_base.c` | **~30 行** | I2C 读写 + 延时函数 |
| `ICM_42688_base.h` | 0 行 | 数据类型通用 |
| 所有滤波层文件 | **0 行** | 纯数学，不涉及任何硬件 |

### 9.2 需要替换的函数

在 `ICM_42688_base.c` 中，只需替换以下 4 个与平台相关的部分：

#### (a) I2C 句柄

```c
// STM32 HAL:
extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef* hi2c_ICM42688 = &hi2c1;

// → 替换为你的平台 I2C 对象指针
```

#### (b) I2C 写寄存器

```c
// STM32 HAL:
void ICM42688_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    HAL_I2C_Mem_Write(hi2c_ICM42688, ICM42688_ADDRESS, RegAddress,
                      I2C_MEMADD_SIZE_8BIT, &Data, 1, 10000);
}

// TI MSPM0 (DriverLib):
void ICM42688_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    DL_I2C_fillControllerTXFIFO(I2C0, &ICM42688_ADDRESS, 1);
    DL_I2C_fillControllerTXFIFO(I2C0, &RegAddress, 1);
    DL_I2C_fillControllerTXFIFO(I2C0, &Data, 1);
    DL_I2C_startControllerTransfer(I2C0);
    while (DL_I2C_getControllerStatus(I2C0) & DL_I2C_CONTROLLER_STATUS_BUSY);
}

// ESP32 (ESP-IDF):
void ICM42688_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ICM42688_ADDRESS, true);
    i2c_master_write_byte(cmd, RegAddress, true);
    i2c_master_write_byte(cmd, Data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(cmd);
}
```

#### (c) I2C 读寄存器

```c
// STM32 HAL:
uint8_t ICM42688_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;
    HAL_I2C_Mem_Read(hi2c_ICM42688, ICM42688_ADDRESS, RegAddress,
                     I2C_MEMADD_SIZE_8BIT, &Data, 1, 10000);
    return Data;
}

// → 替换为你平台的 I2C 读函数
```

#### (d) 延时函数

```c
// STM32 HAL:
HAL_Delay(20);

// → 替换为你平台的延时 (ms 级)
// delay_ms(20)  /  vTaskDelay(pdMS_TO_TICKS(20))  /  _delay_ms(20)
```

### 9.3 《ICM42688_I2C_Recover》函数

I2C 总线恢复是本驱动的增强功能（I2C 卡死时自动恢复）。移植时有两种处理方式：

- **保留**：替换其中的 GPIO 操控 + HAL_I2C_DeInit/Init
- **删除**：直接从 `ICM42688_Update_Data()` 中移除重试和恢复逻辑，只保留单次读取。适合 I2C 可靠性高的平台

### 9.4 ESP32 完整移植清单

```
1. ICM_42688_base.c 中:
   - 删除 #include "LED_Flash.h" 和 "Timer_Counter.h"
   - 删除 ICM42688_I2C_Recover() 整个函数
   - 替换 I2C 句柄为: static i2c_port_t i2c_port = I2C_NUM_0;
   - 替换 ICM42688_WriteReg/ReadReg 用 ESP-IDF i2c_master_* API
   - 替换 HAL_Delay → vTaskDelay(pdMS_TO_TICKS(x))
   - ICM42688_Update_Data() 简化为单次读取 (无重试/恢复)

2. 其余文件: 0 行改动
```

### 9.5 TI MSPM0 完整移植清单

```
1. ICM_42688_base.c 中:
   - 删除 #include "LED_Flash.h" 和 "Timer_Counter.h"
   - 删除 ICM42688_I2C_Recover() 整个函数
   - 替换 I2C 句柄为: 全局 I2C 实例 (如 I2C0)
   - 替换 ICM42688_WriteReg/ReadReg 用 DriverLib DL_I2C_* API
   - 替换 HAL_Delay → delay_ms(x) (MSPM0 SDK自带)
   - ICM42688_Update_Data() 简化为单次读取

2. 如果使用 SysConfig:
   - 配置 I2C 外设 (100kHz, 7-bit addressing)
   - 配置 20ms 定时器中断

3. 其余文件: 0 行改动
```

### 9.6 通用移植模板（伪代码）

任何平台只需实现这三个函数，其余代码零改动：

```c
// ---- 你只需要实现这三个函数 ----

void my_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    // 你的平台 I2C 写: START → dev_addr(W) → reg_addr → data → STOP
}

uint8_t my_i2c_read(uint8_t dev_addr, uint8_t reg_addr) {
    // 你的平台 I2C 读: START → dev_addr(W) → reg_addr →
    //                   RESTART → dev_addr(R) → data → STOP
    return data;
}

void my_delay_ms(uint32_t ms) {
    // 你的平台毫秒延时
}

// ---- 然后在 ICM_42688_base.c 中替换对应调用即可 ----
```

---

## 10. 常见问题

### Q: 上电后 yaw 疯狂旋转？
**A**: 陀螺零偏标定不准。确认 Init(1) 时设备完全静止。或者手动填入正确的 bias 值后 Init(0)。

### Q: 旋转 90° 只显示 45°？
**A**: 量程寄存器配置错误。检查 `ICM_42688_base.c` 中 FS_SEL 是否是 ICM-42688 的编码格式（见 §3.3 陷阱）。

### Q: 静止时 pitch/roll 不是 0？
**A**: 加速度计有零偏。在互补滤波中调用 `ICM42688_Data_Error_Check(1000)`。Mahony 依赖 PI 收敛（Kp=5.12 下约 0.5 秒），无需额外校准。

### Q: 剧烈抖动后角度回不来？
**A**: 加速度计被外部加速度污染。Mahony PI 会逐渐修正（约 0.5~1 秒）。如果持续剧烈运动（如无人机飞行），可适当降低 Kp（如 2.0~3.0）。

### Q: 如何知道处理耗时？
**A**: `ICM42688_Mahony_Update_Tick()` 实测约 1.38ms @168MHz（整个 Tick 含 I2C 读取）。远低于 20ms 周期预算。

### Q: 可以从 20ms 改为 10ms 吗？
**A**: 可以。修改 `ICM42688_Mahony_Update(0.010f)`，同步改 `MAHONY_HALF_T` 为 `0.005f`。注意 10ms 下 Kp/Ki 效果等价于原参数的 sqrt(2) 倍缩放。

---

> **文件版本一致性**: 本 README 描述的文件版本与同目录下的 `.c/.h` 文件匹配。
> 如果从本工程复制文件到其他项目，请将整个 `ICM42688_Portable_Lib/` 目录一并复制。
