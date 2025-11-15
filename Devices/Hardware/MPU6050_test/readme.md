[toc]

# 模块名:

## 1. 项目简介

- MPU6050模块的使用
- 采用硬件IICPB6，PB7进行通信，保证数据的精准传输

## 2. 核心功能

### 1. 测定各向加速度与角速度

- 通过返回结构体sensor_data里返回值获得，由于存在温漂，当加速度小于1m/s2，角速度小于1°/s时返回值将直接为0
- 注意Az（竖直加速度）因为存在重力加速度，当引脚排针竖直向上静态值为-1，反之为1，因此在安装MPU6050注意安装角度

### 2.测定所在角度

- 原理是用角速度和测定时间函数进行积分得到的结果,因此比较适合测量变化相对不太剧烈的角度变化，通过重载程序清零，在工程中可专门设置一清零按键

## 3. 核心函数

```c
//模块各参数的固定配置函数
#define MPU6050_ADDR    (0x68 << 1)  // 若AD0接地，7位地址0x68，左移1位得到8位地址0xD0
#define WHO_AM_I_REG    0x75        // WHO_AM_I寄存器地址，默认值0x68
#define PWR_MGMT_1_REG  0x6B        // 电源管理寄存器1
#define SMPLRT_DIV_REG  0x19        // 采样率分频寄存器
#define CONFIG_REG      0x1A        // 配置寄存器（含DLPF设置）
#define GYRO_CONFIG_REG 0x1B        // 陀螺仪配置寄存器
#define ACCEL_CONFIG_REG 0x1C       // 加速度计配置寄存器
HAL_StatusTypeDef MPU6050_Init(void);

//加速度与角速度的获取函数
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);

//数据更新函数，选择什么时候更新数据，在示例工程中没用，在实际可根据需求编辑
void MPU6050_Data_Update(void);

//数据处理函数
MPUData_t process_MPUdata(void)
    
//角度计算函数，注意输入参数一定是数据处理后的参数
//调用前加上  uint32_t current_time = HAL_GetTick();
//			float dt = (current_time - current_angle.last_time) / 1000.0f; 
//  		current_angle.last_time = current_time;
void calculate_angle_from_gyro(float gx, float gy, float gz, float dt)	
    
//初始化温度补偿系数，需要微调补偿可以直接从这改
void init_adaptive_compensation(void)
//其余还有一些自适应补偿函数是ai写的，我也没全看懂，从结果上说是有效的，如果发现有可以改善或删减的地方也可以根据需要自行尝试
```

## 4. 基础必备代码

### 4-1 库导入

```c
#include "main.h"
#include "i2c.h"
#include "gpio.h"
#include "MPU.h"
#include "globals.h"
#include <math.h>
```



### 4-2 全局变量(域)

```c
extern int MPU_flag;
extern float Ax;
extern float Ay;
extern float Az;
extern float Gx;
extern float Gy;
extern float Gz;
extern float Temperature;
```



### 4-3 setup

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_I2C1_Init();
MPU6050_Init();
init_adaptive_compensation(); // 初始化自适应补偿
```



### 4-4 while

```c
MPU6050_Data_Update();
MPUData_t sensor_data = process_MPUdata();
uint32_t current_time = HAL_GetTick();
float dt = (current_time - current_angle.last_time) / 1000.0f; // 转换为秒
current_angle.last_time = current_time;
calculate_angle_from_gyro(sensor_data.Gx_, sensor_data.Gy_, sensor_data.Gz_,dt);
HAL_Delay(10); 
```

## 5. Cube配置

只需要注意有个PB6,PB7的I2C配置，其余正常

## 6. 引脚定义

| 引脚号 | 标签 |
| :----: | :--: |
|  PB6   | SCL  |
|  PB7   | SDA  |

MPU6050除了电源和接地接STM32对应引脚还有I2C通信的引脚外，注意AD0接地

## 7. 注意事项

- 数据需要读取处理后的数据就是结构体sensordata里的数据
- MPU6050安装角度要尽量绝对水平

## 8. 更新日志

* 2025/xx/xx
  * 完成该工程
* 2025/xx/xx
  * 增/删/补/换了......
