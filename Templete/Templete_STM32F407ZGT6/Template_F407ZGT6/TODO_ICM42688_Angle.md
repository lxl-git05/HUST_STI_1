# ICM42688_Angle 移植 TODO

## 当前状态

- [x] ICM_42688_base.c/h — 底层I2C驱动完成 ✅
- [x] I2C地址确认 — 0x68 (AD0=GND)，I2C1总线 ✅
- [x] Mode_2 I2C扫描测试 — 确认通信正常 ✅
- [x] ICM42688_Angle.h — 头文件（Function/） ✅ **2026-07-23**
- [x] ICM42688_Angle.c — Mahony AHRS 实现 ✅ **2026-07-23**
- [x] Mode_2 集成测试 — 串口+OLED输出角度 ✅ **2026-07-23**
- [ ] 实机测试验证 — 确认角度输出正确、零漂达标
- [ ] 参数调优 — 根据实测调整 Kp/Ki/alpha

## ICM42688_Angle 设计决策

### 算法选型：Mahony AHRS ✅ 已实现

基于 DAIMXA `angle.c` 的实现，适配到本工程 MPU6050_Angle 的 API 风格。

**核心优势：**
- 四元数姿态，无万向节死锁
- PI控制器用重力矢量修正陀螺漂移
- ICM42688噪声远低于MPU6050（70μg vs 400μg），收敛更快

### 数据结构（直接复用MPU6050_Angle的类型）

```c
ImuOffset_Typedef  // 零偏校准值
ImuCali_Typedef    // 校准后物理量
ImuReal_Typedef    // 最终输出角度
```

### 函数列表（模仿MPU6050_Angle风格）

| 函数 | 功能 | 状态 |
|------|------|------|
| `ICM42688_Angle_Init()` | 硬件初始化 + 四元数复位 + 零偏清零 | ✅ |
| `ICM42688_Data_Error_Check(sample_cnt)` | 手动标定（N次采样平均） | ✅ |
| `ICM42688_Angle_Update_Tick()` | 20ms定时器：读数据→去零偏→Mahony AHRS→输出角度 | ✅ |

### 已确定参数

- [x] Kp = 5.12f — PI比例增益（与DAIMXA一致，ICM42688低噪声可后续调大至8~10）
- [x] Ki = 0.001f — PI积分增益
- [x] halfT = 0.010s — 半采样周期（20ms Tick）
- [x] alpha = 0.3f — 加速度低通滤波系数
- [x] 默认标定采样次数 = 500

### 实现特点

1. **欧拉角转换采用 atan2**：Roll/Yaw 范围 ±180°，优于 DAIMXA 原版的 asin（仅 ±90°）
2. **独立低通滤波**：加速度数据经过一阶 IIR 滤波（alpha=0.3），抑制高频振动
3. **归一化保护**：加速度和四元数归一化前均检查零向量，防止除零
4. **与 MPU6050_Angle API 完全对齐**：上层 Mode 代码无需修改即可切换 IMU

## 关键代码位置

| 文件 | 说明 |
|------|------|
| `Hardware/ICM_42688_base.c` | 底层驱动，提供 ICM_Raw_Data |
| `Function/ICM42688_Angle.c` | **Mahony AHRS 角度解算（本次新增）** |
| `Function/ICM42688_Angle.h` | **Angle层头文件（本次新增）** |
| `Function/MPU6050_Angle.c` | API风格参考 |
| `E:\Download\20260722-205835\...\angle.c` | Mahony算法参考 |
| `Mode/Mode_2.c` | 角度测试入口 |

## 参考：DAIMXA Mahony核心参数

```
#define Kp    5.12f      // PI比例增益
#define Ki    0.001f     // PI积分增益
#define halfT 0.006f     // 半采样周期（约12ms采样）
#define alpha 0.3f       // 加速度低通滤波系数
```

陀螺仪零偏标定：100次采样，10ms间隔，累计平均。

## 下一步

- [ ] 烧录到 STM32F407ZGT6 实机测试
- [ ] 验证静止时 roll/pitch 是否在 ±0.1° 以内
- [ ] 验证运动后停止时角度是否快速收敛（无过冲/振荡）
- [ ] 验证连续旋转 360° 后回正误差
- [ ] 根据实测结果微调 Kp/Ki/alpha
- [ ] 考虑加入自适应 Kp：加速度模长接近 1g 时用大 Kp（快收敛），偏离时用小 Kp（抗扰动）
