#ifndef __IMU_H
#define __IMU_H

// ==================== IMU 通用数据类型 ====================
// 适用于 MPU6050 和 ICM42688，实现统一的角度解算接口

// 零偏校准值（加速度误差 + 陀螺仪零漂）
typedef struct
{
	float AccErrorX ;	// 加速度X零偏 (g)
	float AccErrorY ;	// 加速度Y零偏 (g)
	float AccErrorZ ;	// 加速度Z零偏 (g) — 静止时应为 1g 的偏移量

	float GyroErrorX ;	// 陀螺仪X零漂 (°/s)
	float GyroErrorY ;	// 陀螺仪Y零漂 (°/s)
	float GyroErrorZ ;	// 陀螺仪Z零漂 (°/s)
} ImuOffset_Typedef ;

// 零偏纠正后的物理量（直接参与姿态解算）
typedef struct
{
	float AX ;	// 加速度X (g)
	float AY ;	// 加速度Y (g)
	float AZ ;	// 加速度Z (g) — 静止时接近 1.0

	float GX ;	// 角速度X (°/s)
	float GY ;	// 角速度Y (°/s)
	float GZ ;	// 角速度Z (°/s)
} ImuCali_Typedef ;

// 最终输出：归一化加速度 + 欧拉角
typedef struct
{
	float AccX ;	// 归一化加速度X (方向余弦)
	float AccY ;	// 归一化加速度Y
	float AccZ ;	// 归一化加速度Z

	float roll ;	// 横滚角 (°)
	float pitch ;	// 俯仰角 (°)
	float yaw ;		// 偏航角 (°) — 无磁力计时会漂移
} ImuReal_Typedef ;

// ==================== 传感器选择 ====================
// 两个 IMU 均挂 I2C1，地址均为 0x68，物理上只能二选一
// 注释掉 → ICM42688，取消注释 → MPU6050
#define IMU_USE_MPU6050

// ==================== 默认参数 ====================
#define IMU_TURN_YAW_DEFAULT_DEADBAND  3.0f   // 默认偏航到位死区 (°)

// ==================== 统一 API 映射宏 ====================
#ifdef IMU_USE_MPU6050

    #include "MPU6050_Mahony.h"

    #define IMU_Mahony_Init(doCalib)     MPU6050_Mahony_Init(doCalib)
    #define IMU_Mahony_Update_Tick()     MPU6050_Mahony_Update_Tick()
    #define IMU_Mahony_Calibrate(n)      MPU6050_Mahony_Calibrate(n)
    #define IMU_Yaw_Abs_Get()            MPU_Yaw_Abs_Get()
    #define IMU_Yaw_Abs_Reset()          MPU_Yaw_Abs_Reset()

    #define IMU_Mahony_Real              MPU_Mahony_Real
    #define IMU_Mahony_GyroBiasX         MPU_Mahony_GyroBiasX
    #define IMU_Mahony_GyroBiasY         MPU_Mahony_GyroBiasY
    #define IMU_Mahony_GyroBiasZ         MPU_Mahony_GyroBiasZ

    #define IMU_Mahony_AccBiasX          MPU_Mahony_AccBiasX
    #define IMU_Mahony_AccBiasY          MPU_Mahony_AccBiasY
    #define IMU_Mahony_AccBiasZ          MPU_Mahony_AccBiasZ

#else   // 默认: ICM42688

    #include "ICM42688_Mahony.h"

    #define IMU_Mahony_Init(doCalib)     ICM42688_Mahony_Init(doCalib)
    #define IMU_Mahony_Update_Tick()     ICM42688_Mahony_Update_Tick()
    #define IMU_Mahony_Calibrate(n)      ICM42688_Mahony_Calibrate(n)
    #define IMU_Yaw_Abs_Get()            ICM_Yaw_Abs_Get()
    #define IMU_Yaw_Abs_Reset()          ICM_Yaw_Abs_Reset()

    #define IMU_Mahony_Real              ICM_Mahony_Real
    #define IMU_Mahony_GyroBiasX         ICM_Mahony_GyroBiasX
    #define IMU_Mahony_GyroBiasY         ICM_Mahony_GyroBiasY
    #define IMU_Mahony_GyroBiasZ         ICM_Mahony_GyroBiasZ

    #define IMU_Mahony_AccBiasX          ICM_Mahony_AccBiasX
    #define IMU_Mahony_AccBiasY          ICM_Mahony_AccBiasY
    #define IMU_Mahony_AccBiasZ          ICM_Mahony_AccBiasZ

#endif

// ==================== 高级功能 API（实函数，非宏） ====================

// 检查是否转到了目标偏航角（基于 yaw_abs 累计值）
// target:    目标角度 (°)，以 IMU_Yaw_Abs_Get() 当前值为参考
// deadband:  允许误差 (°)，例如 3.0 表示 ±3° 内都算到位
// 返回: 1=已到位, 0=未到位
uint8_t IMU_Turn_Yaw_Is_Ok_Ex(float target, float deadband);

// 简化版：使用默认死区 IMU_TURN_YAW_DEFAULT_DEADBAND
uint8_t IMU_Turn_Yaw_Is_Ok(float target);

// 获取当前校准后的Z轴角速度绝对值 (°/s)
// 用于旋转任务退出前的稳定确认（防止机械回弹误判）
float IMU_Yaw_Gyro_Get(void);

// 获取世界坐标系下的水平加速度 (g)
// 将机体加速度旋转到世界系，自动消除重力分量（重力仅在Z轴）
// 可用于速度估计、碰撞检测、姿态控制等
float IMU_Get_Ax(void);		// 世界系X轴水平加速度 (g)
float IMU_Get_Ay(void);		// 世界系Y轴水平加速度 (g)

#endif
