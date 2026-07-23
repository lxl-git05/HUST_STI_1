## 2026-07-23 20:30 | 为MPU6050配置Mahony滤波并写Mode_2例程

| 文件名 | 文件路径（相对工作区） | 操作类型 | 说明 |
|--------|----------------------|----------|------|
| MPU6050_Mahony.h | ./Function/MPU6050_Mahony.h | 新增 | MPU6050 Mahony AHRS API 头文件（参数+API声明） |
| MPU6050_Mahony.c | ./Function/MPU6050_Mahony.c | 新增 | MPU6050 Mahony AHRS 实现（四元数+PI重力修正，从ICM42688移植） |
| AllHeader.h | ./Top/AllHeader.h | 修改 | 新增 #include "MPU6050_Mahony.h" |
| Mode_2.c | ./Mode/Mode_2.c | 修改 | 加入 #ifdef MPU6050_MAHONY_TEST 开关，支持ICM/MPU二选一测试 |
| CLAUDE.md | ./CLAUDE.md | 修改 | 新增 MPU6050 Mahony AHRS 滤波文档章节 |
