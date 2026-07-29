## 2026-07-23 20:30 | 为MPU6050配置Mahony滤波并写Mode_2例程

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| MPU6050_Mahony.h | ./Function/MPU6050_Mahony.h | 新增 | MPU6050 Mahony AHRS API 头文件（参数+API声明） |
| MPU6050_Mahony.c | ./Function/MPU6050_Mahony.c | 新增 | MPU6050 Mahony AHRS 实现（四元数+PI重力修正，从ICM42688移植） |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 新增 #include "MPU6050_Mahony.h" |
| Mode_2.c | ./Mode/Mode_2.c | 修改 | 加入 #ifdef MPU6050_MAHONY_TEST 开关，支持ICM/MPU二选一测试 |
| CLAUDE.md | ./CLAUDE.md | 修改 | 新增 MPU6050 Mahony AHRS 滤波文档章节 |

## 2026-07-24 10:30 | 创建IMU统一库，将MPU6050驱动+滤波并入IMU_Portable_Lib

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| ICM42688_Portable_Lib/ | ./ICM42688_Portable_Lib/ | 删除 | 目录重命名为 IMU_Portable_Lib |
| IMU_Portable_Lib/ | ./IMU_Portable_Lib/ | 新增 | 合并 ICM+MPU 双传感器可移植库（15文件+README） |
| MPU6050_base.h | ./IMU_Portable_Lib/MPU6050_base.h | 新增 | MPU6050 驱动头文件（从 Hardware/ 复制） |
| MPU6050_base.c | ./IMU_Portable_Lib/MPU6050_base.c | 新增 | MPU6050 驱动实现（从 Hardware/ 复制） |
| MPU6050_Angle.h | ./IMU_Portable_Lib/MPU6050_Angle.h | 新增 | MPU6050 互补滤波头文件（从 Function/ 复制） |
| MPU6050_Angle.c | ./IMU_Portable_Lib/MPU6050_Angle.c | 新增 | MPU6050 互补滤波实现（从 Function/ 复制） |
| MPU6050_Mahony.h | ./IMU_Portable_Lib/MPU6050_Mahony.h | 新增 | MPU6050 Mahony 头文件（从 Function/ 复制） |
| MPU6050_Mahony.c | ./IMU_Portable_Lib/MPU6050_Mahony.c | 新增 | MPU6050 Mahony 实现（从 Function/ 复制） |
| IMU.h | ./Function/IMU.h | 新增 | ★ 统一 API 层：宏切换传感器 + Turn_Yaw 检测声明 |
| IMU.c | ./Function/IMU.c | 新增 | ★ 统一 API 层：IMU_Turn_Yaw_Is_Ok 实函数 |
| IMU.h | ./IMU_Portable_Lib/IMU.h | 新增 | 统一 API 层副本（可移植库内） |
| IMU.c | ./IMU_Portable_Lib/IMU.c | 新增 | 统一 API 层副本（可移植库内） |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 新增 #include "IMU.h" |
| Mode_2.c | ./Mode/Mode_2.c | 修改 | 用 IMU_* 统一 API 重写，移除 #ifdef 开关 |
| demo.c | ./IMU_Portable_Lib/demo.c | 修改 | 同步更新为统一 API 示例 |
| README.md | ./IMU_Portable_Lib/README.md | 修改 | 完全重写：覆盖 ICM+MPU+统一库，完整陀螺仪体系文档 |
| CLAUDE.md | ./CLAUDE.md | 修改 | 合并 ICM/MPU 文档为统一 IMU 章节 |

## 2026-07-24 11:00 | 将Imu_Types.h合并进IMU.h并删除

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Imu_Types.h | ./Function/Imu_Types.h | 删除 | 类型定义合并进 IMU.h，不再独立文件 |
| Imu_Types.h | ./IMU_Portable_Lib/Imu_Types.h | 删除 | 同上 |
| IMU.h | ./Function/IMU.h | 修改 | 将 ImuOffset/ImuCali/ImuReal 三种结构体并入，移除 #include "Imu_Types.h" |
| IMU.h | ./IMU_Portable_Lib/IMU.h | 修改 | 同步更新 |
| ICM42688_Mahony.h | ./Function/ 和 ./IMU_Portable_Lib/ | 修改 | #include "Imu_Types.h" → #include "IMU.h" |
| MPU6050_Mahony.h | ./Function/ 和 ./IMU_Portable_Lib/ | 修改 | 同上 |
| ICM42688_Angle.h | ./Function/ 和 ./IMU_Portable_Lib/ | 修改 | 同上 |
| MPU6050_Angle.h | ./Function/ 和 ./IMU_Portable_Lib/ | 修改 | 同上 + 注释更正 |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 移除 #include "Imu_Types.h" 行 |
| README.md | ./IMU_Portable_Lib/README.md | 修改 | 移除 Imu_Types.h 引用，§4 改为 "IMU 类型定义" |
| CLAUDE.md | ./CLAUDE.md | 修改 | 移除架构图/文件清单中的 Imu_Types.h |

## 2026-07-29 15:30 | 新寻迹模块 USART3 DMA+Idle 协议解析 + Mode_4 OLED 展示 8 路 ADC

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| Y8_USART.h | ./Hardware/Y8_USART.h | 修改 | 新增 Y8U_ API 声明：Y8U_ADC[8]/Y8U_Digital[8]/Init/SendCmd/DMA_RxCallback |
| Y8_USART.c | ./Hardware/Y8_USART.c | 修改 | 完整实现：USART3 DMA+Idle 接收、$A/D,xn:val# 状态机解析、命令发送 |
| Mode_4.c | ./Mode/Mode_4.c | 修改 | OLED 8 路 ADC 展示（4 行 × 2 通道），KEY1 重发 $0,1,0# |
| Serial_porting.c | ./Function/Serial_porting.c | 修改 | HAL_UARTEx_RxEventCallback 新增 USART3 → Y8U_DMA_RxCallback 分发分支 |
