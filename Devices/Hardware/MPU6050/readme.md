[toc]

# 模块名:

## 1. 项目简介

- MPU6050模块的使用
- 采用硬件IICPB6，PB7进行通信，保证数据的精准传输

参考资料：[STM32 MPU6050 六轴陀螺仪教程（HAL 库零基础入门）_stm32 hal mpu6050-CSDN博客](https://blog.csdn.net/h050210/article/details/145936555?ops_request_misc=%7B%22request%5Fid%22%3A%227ab73d54770aebb1620fdbe591502072%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=7ab73d54770aebb1620fdbe591502072&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-145936555-null-null.142^v102^pc_search_result_base8&utm_term=mpu6050教程&spm=1018.2226.3001.4187)主要参考资料来自这个csdn链接，其中除了欧拉角部分我没看其他部分应该是没有问题的。

## 2. 核心功能

### 1. 测定各向加速度与角速度

- 通过返回结构体sensor_data里返回值获得，由于存在温漂，当加速度小于1m/s2，角速度小于1°/s时返回值将直接为0
- 注意Az（竖直加速度）因为存在重力加速度，当引脚排针竖直向上静态值为-1，反之为1，因此在安装MPU6050注意安装角度

### 2.测定所在角度

- 原理是用角速度和测定时间函数进行积分得到的结果,因此比较适合测量变化相对不太剧烈的角度变化，通过重载程序清零，在工程中可专门设置一清零按键

## 3. 核心函数

```c
// 主要功能函数
HAL_StatusTypeDef MPU6050_Init(void);
MPUData_t MPU6050_Data_Update(void);
void turning_state_judge(MPUData_t *data);
void calculate_angle_from_gyro(float gx, float gy, float gz, float dt);

// I2C读写函数
HAL_StatusTypeDef MPU6050_I2C_Read(uint16_t MemAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef MPU6050_I2C_Write(uint16_t MemAddress, uint8_t *pData, uint16_t Size);

#ifndef USE_HARDWARE_I2C
// 软件I2C专用函数
void Software_I2C_Init(void);
#endif

// 辅助函数
void MPU6050_Read_Accel(void);
void MPU6050_Read_Gyro(void);
void MPU6050_Read_Temp(void);
void init_adaptive_compensation(void);
MPUData_t process_MPUdata(void);
```

## 4. 基础必备代码

### 4-1 库导入

```c
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "MPU.h"
```



### 4-2 全局变量(域)

```c
typedef struct {
    float Ax_, Ay_, Az_;
    float Gx_, Gy_, Gz_;
    float Temperature_;
} MPUData_t;

typedef struct {
    float pitch;  // 俯仰角
    float roll;   // 横滚角  
    float yaw;    // 偏航角
    uint32_t last_time;
} Angle_t;

// 全局声明
extern MPUData_t sensor_data;
extern Angle_t current_angle;
extern int turning_flag;
```



### 4-3 setup

```c
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
#ifndef USE_HARDWARE_I2C
  Software_I2C_Init();
#endif
  
  MPU6050_Init();
  init_adaptive_compensation();
```



### 4-4 while

```c
sensor_data = MPU6050_Data_Update();
    
    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - current_angle.last_time) / 1000.0f; 
    current_angle.last_time = current_time;
    
    calculate_angle_from_gyro(sensor_data.Gx_, sensor_data.Gy_, sensor_data.Gz_, dt);
    turning_state_judge(&sensor_data);
    
    HAL_Delay(10);
```

## 5. Cube配置

只需要注意有个PB6,PB7的I2C配置，其余正常

## 6. 引脚定义

| 引脚号 | 标签 |
| :----: | :--: |
|  PB6   | SCL  |
|  PB7   | SDA  |

MPU6050除了电源和接地接STM32对应引脚还有I2C通信的引脚外，注意AD0接地，上述是硬件I2C的引脚，对软件I2C只需要注释下面这句就行

```
 #define USE_HARDWARE_I2C 
```

对于引脚在

```
#else
    // 使用软件I2C - 定义软件I2C引脚
    #define SCL_PIN    GPIO_PIN_3
    #define SDA_PIN    GPIO_PIN_4
    #define I2C_PORT   GPIOB
```

这里修改，记得cubemx里进行配置，两个引脚设置GPIO_OUTPUT，需要修改的是OUTPUT_OPEN_DRAIN和PULL_UP

## 7. 注意事项

- 数据需要读取处理后的数据就是结构体sensordata里的数据
- MPU6050安装角度要尽量绝对水平

## 8. 更新日志

* 2025/xx/xx
  * 完成该工程
* 2025/xx/xx
  * 增/删/补/换了......
