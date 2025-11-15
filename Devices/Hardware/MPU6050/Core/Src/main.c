/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "MPU.h"
#include "globals.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 定义数据结构
typedef struct {
    float Ax_, Ay_, Az_;
    float Gx_, Gy_, Gz_;
    float Temperature_;
} MPUData_t;

MPUData_t sensor_data;

typedef struct {
    float pitch;  // 俯仰角
    float roll;   // 横滚角  
    float yaw;    // 偏航角
    uint32_t last_time;
} Angle_t;

Angle_t current_angle = {0};


// 自适应补偿参数结构体
typedef struct {
    // 温度补偿参数
    float accel_temp_coeff[3];
    float gyro_temp_coeff[3];
    float accel_offset[3];
    float gyro_offset[3];
    
    // 自适应学习参数
    float learning_rate_temp;
    float learning_rate_offset;
    uint32_t calibration_count;
    uint8_t is_calibrating;
    
    // 温度历史
    float temp_history[10];
    float accel_history[3][10];
    float gyro_history[3][10];
    uint8_t history_index;
    
    // 参考温度
    float ref_temp;
} AdaptiveCompensation_t;

AdaptiveCompensation_t adaptive_comp;

// 数据滤波变量
float Ax_filtered = 0, Ay_filtered = 0, Az_filtered = 0;
float Gx_filtered = 0, Gy_filtered = 0, Gz_filtered = 0;
const float FILTER_ALPHA = 0.2f; // 低通滤波系数

int flag = 0;         //判断转向函数过程变量
int turning_flag = 0; //转向状态指示

// 数据有效性检查
uint8_t data_valid = 0;
float prev_Ax = 0, prev_Ay = 0, prev_Az = 0;
float prev_Gx = 0, prev_Gy = 0, prev_Gz = 0;

// 静止检测
uint32_t stationary_count = 0;
const uint32_t STATIONARY_THRESHOLD = 100; // 需要连续100个采样点静止
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// 自适应补偿函数
void calculate_angle_from_gyro(float gx, float gy, float gz, float dt);
void init_adaptive_compensation(void);
void update_adaptive_compensation(float temp, float ax, float ay, float az, float gx, float gy, float gz);
void apply_adaptive_compensation(float temp, float *ax, float *ay, float *az, float *gx, float *gy, float *gz);
uint8_t is_stationary(float ax, float ay, float az, float gx, float gy, float gz);

// 数据处理函数
void apply_low_pass_filter(float *filtered, float new_value, float alpha);
uint8_t is_data_valid(float ax, float ay, float az, float gx, float gy, float gz);
void turning_state_judge(MPUData_t *data);
MPUData_t process_MPUdata_improved(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 初始化自适应补偿
void init_adaptive_compensation(void) {
    // 初始化温度补偿系数
    adaptive_comp.accel_offset[0] = 0.101f;
    adaptive_comp.accel_offset[1] = 0.001f;
    
    // 精确调整（你可以修改这些增量值）
    float az_increment = -0.180f;   // 负值表示减小
		float gx_increment = 0.648f;    // 正值表示减小负值（这个不变）
		float gz_increment = -0.100f;   // 负值表示减小

		adaptive_comp.accel_offset[2] = 0.100f + az_increment;  // 0.100 + (-0.180) = -0.080
		adaptive_comp.gyro_offset[0] = -5.0f;    // 根据Gx ≈ -5°/s
    adaptive_comp.gyro_offset[1] = -1.15f;   // 根据Gy ≈ -1.15°/s
    adaptive_comp.gyro_offset[2] = -1.3f;    // 根据Gz ≈ -1.3°/s
    // 温度补偿系数保持不变
    adaptive_comp.accel_temp_coeff[0] = 0.0005f;
    adaptive_comp.accel_temp_coeff[1] = 0.0005f;
    adaptive_comp.accel_temp_coeff[2] = 0.0006f;
    adaptive_comp.gyro_temp_coeff[0] = 0.008f;
    adaptive_comp.gyro_temp_coeff[1] = 0.008f;
    adaptive_comp.gyro_temp_coeff[2] = 0.009f;
    
    adaptive_comp.learning_rate_temp = 0.001f;
    adaptive_comp.learning_rate_offset = 0.005f;
    adaptive_comp.calibration_count = 500;
    adaptive_comp.is_calibrating = 1;
    adaptive_comp.history_index = 0;
    adaptive_comp.ref_temp = 25.0f;
    
    // 清空历史数据
    for(int i = 0; i < 10; i++) {
        adaptive_comp.temp_history[i] = 25.0f;
        for(int j = 0; j < 3; j++) {
            adaptive_comp.accel_history[j][i] = 0.0f;
            adaptive_comp.gyro_history[j][i] = 0.0f;
        }
    }
}

// 静止检测函数
uint8_t is_stationary(float ax, float ay, float az, float gx, float gy, float gz) {
    // 检查加速度计模长是否接近1g
    float accel_magnitude = sqrtf(ax*ax + ay*ay + az*az);
    if(fabsf(accel_magnitude - 1.0f) > 0.1f) {
        return 0;
    }
    
    // 检查陀螺仪是否接近零
    float gyro_magnitude = sqrtf(gx*gx + gy*gy + gz*gz);
    if(gyro_magnitude > 2.0f) { // 2°/s阈值
        return 0;
    }
    
    return 1;
}

// 更新自适应补偿参数
void update_adaptive_compensation(float temp, float ax, float ay, float az, float gx, float gy, float gz) {
    // 更新历史数据
    adaptive_comp.temp_history[adaptive_comp.history_index] = temp;
    adaptive_comp.accel_history[0][adaptive_comp.history_index] = ax;
    adaptive_comp.accel_history[1][adaptive_comp.history_index] = ay;
    adaptive_comp.accel_history[2][adaptive_comp.history_index] = az;
    adaptive_comp.gyro_history[0][adaptive_comp.history_index] = gx;
    adaptive_comp.gyro_history[1][adaptive_comp.history_index] = gy;
    adaptive_comp.gyro_history[2][adaptive_comp.history_index] = gz;
    
    adaptive_comp.history_index = (adaptive_comp.history_index + 1) % 10;
    
    // 静止状态下更新零偏估计
    if(is_stationary(ax, ay, az, gx, gy, gz)) {
        stationary_count++;
        
        if(stationary_count > STATIONARY_THRESHOLD) {
            // 更新加速度计零偏（假设Z轴为1g）
            adaptive_comp.accel_offset[0] += adaptive_comp.learning_rate_offset * ax;
            adaptive_comp.accel_offset[1] += adaptive_comp.learning_rate_offset * ay;
            adaptive_comp.accel_offset[2] += adaptive_comp.learning_rate_offset * (az - 1.0f);
            
            // 更新陀螺仪零偏
            adaptive_comp.gyro_offset[0] += adaptive_comp.learning_rate_offset * gx;
            adaptive_comp.gyro_offset[1] += adaptive_comp.learning_rate_offset * gy;
            adaptive_comp.gyro_offset[2] += adaptive_comp.learning_rate_offset * gz;
            
            stationary_count = STATIONARY_THRESHOLD; // 防止溢出
            
            // 温度补偿参数学习（需要足够的温度变化）
            if(adaptive_comp.calibration_count < 1000) {
                adaptive_comp.calibration_count++;
            }
        }
    } else {
        stationary_count = 0;
    }
    
    // 温度补偿参数学习（简化版本）
    if(adaptive_comp.is_calibrating && adaptive_comp.calibration_count < 1000) {
        // 这里可以添加更复杂的温度模型学习算法
        // 例如基于历史数据的线性回归等
    }
}

// 应用自适应补偿
void apply_adaptive_compensation(float temp, float *ax, float *ay, float *az, float *gx, float *gy, float *gz) {
    float temp_diff = temp - adaptive_comp.ref_temp;
    
    // 应用温度补偿和零偏补偿
    *ax = (*ax - adaptive_comp.accel_offset[0]) * (1.0f - adaptive_comp.accel_temp_coeff[0] * temp_diff);
    *ay = (*ay - adaptive_comp.accel_offset[1]) * (1.0f - adaptive_comp.accel_temp_coeff[1] * temp_diff);
    *az = (*az - adaptive_comp.accel_offset[2]) * (1.0f - adaptive_comp.accel_temp_coeff[2] * temp_diff);
    
    *gx = (*gx - adaptive_comp.gyro_offset[0]) * (1.0f - adaptive_comp.gyro_temp_coeff[0] * temp_diff);
    *gy = (*gy - adaptive_comp.gyro_offset[1]) * (1.0f - adaptive_comp.gyro_temp_coeff[1] * temp_diff);
    *gz = (*gz - adaptive_comp.gyro_offset[2]) * (1.0f - adaptive_comp.gyro_temp_coeff[2] * temp_diff);
}

// 应用低通滤波
void apply_low_pass_filter(float *filtered, float new_value, float alpha) {
    *filtered = alpha * new_value + (1.0f - alpha) * (*filtered);
}

// 数据有效性检查
uint8_t is_data_valid(float ax, float ay, float az, float gx, float gy, float gz) {
    // 检查数据是否在合理范围内
    if (fabsf(ax) > 4.0f || fabsf(ay) > 4.0f || fabsf(az) > 4.0f) return 0;
    if (fabsf(gx) > 500.0f || fabsf(gy) > 500.0f || fabsf(gz) > 500.0f) return 0;
    
    // 检查数据突变（简单的变化率检查）
    float accel_change = fabsf(ax - prev_Ax) + fabsf(ay - prev_Ay) + fabsf(az - prev_Az);
    float gyro_change = fabsf(gx - prev_Gx) + fabsf(gy - prev_Gy) + fabsf(gz - prev_Gz);
    
    if (accel_change > 2.0f || gyro_change > 100.0f) return 0;
    
    return 1;
}


// 改进的数据处理函数
MPUData_t process_MPUdata(void) {
    MPUData_t data;
    
    if(MPU_flag == 1) {
        MPU_flag = 0;
        
        // 更新自适应补偿参数
        update_adaptive_compensation(Temperature, Ax, Ay, Az, Gx, Gy, Gz);
        
        // 应用自适应补偿
        apply_adaptive_compensation(Temperature, &Ax, &Ay, &Az, &Gx, &Gy, &Gz);
        
        // 检查数据有效性
        if(is_data_valid(Ax, Ay, Az, Gx, Gy, Gz)) {
            data_valid = 1;
            
            // 应用低通滤波
            apply_low_pass_filter(&Ax_filtered, Ax, FILTER_ALPHA);
            apply_low_pass_filter(&Ay_filtered, Ay, FILTER_ALPHA);
            apply_low_pass_filter(&Az_filtered, Az, FILTER_ALPHA);
            apply_low_pass_filter(&Gx_filtered, Gx, FILTER_ALPHA);
            apply_low_pass_filter(&Gy_filtered, Gy, FILTER_ALPHA);
            apply_low_pass_filter(&Gz_filtered, Gz, FILTER_ALPHA);
            
            // 更新历史数据
            prev_Ax = Ax; prev_Ay = Ay; prev_Az = Az;
            prev_Gx = Gx; prev_Gy = Gy; prev_Gz = Gz;
        } else {
            data_valid = 0;
        }
    }
    
    // 赋值输出数据
    data.Temperature_ = Temperature;
    if(Ax_filtered >-0.1&&Ax_filtered < 0.1) Ax_filtered =0;
		if(Ay_filtered >-0.1&&Ay_filtered < 0.1) Ay_filtered =0;
		if(Az_filtered >-1.1&&Az_filtered < -0.9) Az_filtered =-1;
		if(Gx_filtered >-1&&Gx_filtered < 1) Gx_filtered =0;
		if(Gy_filtered >-1&&Gy_filtered < 1) Gy_filtered =0;
		if(Gz_filtered >-1&&Gz_filtered < 1) Gz_filtered =0;
    if(data_valid) {
        data.Ax_ = Ax_filtered;
        data.Ay_ = Ay_filtered;
        data.Az_ = Az_filtered;
        data.Gx_ = Gx_filtered;
        data.Gy_ = Gy_filtered;
        data.Gz_ = Gz_filtered;
    } else {
        // 数据无效时输出滤波后的值
        data.Ax_ = Ax_filtered;
        data.Ay_ = Ay_filtered;
        data.Az_ = Az_filtered;
        data.Gx_ = Gx_filtered;
        data.Gy_ = Gy_filtered;
        data.Gz_ = Gz_filtered;
    }
    
    return data;
}

void calculate_angle_from_gyro(float gx, float gy, float gz, float dt) {
    // 积分计算角度变化
    current_angle.pitch += gx * dt;  // 绕X轴旋转影响pitch
    current_angle.roll += gy * dt;   // 绕Y轴旋转影响roll
    current_angle.yaw += gz * dt;    // 绕Z轴旋转影响yaw
}

void turning_state_judge(MPUData_t *data){
	if(data->Gz_>1&&flag == 0){
		current_angle.yaw = 0;
		flag = 1;
	}
	else if(data->Gz_<1&&flag == 1){
		flag = 0;
		turning_flag = 0;
	}
	else if(data->Gz_>1&&flag == 1){
		if(current_angle.yaw > 20) turning_flag = 1;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)

{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  MPU6050_Init();
  init_adaptive_compensation(); // 初始化自适应补偿
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    MPU6050_Data_Update();
    MPUData_t sensor_data = process_MPUdata();
    //获取参数
		uint32_t current_time = HAL_GetTick();
		float dt = (current_time - current_angle.last_time) / 1000.0f; 
		current_angle.last_time = current_time;
		calculate_angle_from_gyro(sensor_data.Gx_, sensor_data.Gy_, sensor_data.Gz_, dt);
		//获得角度（并记录在角度结构体中）
		turning_state_judge(&sensor_data);
		//获取转向状态，turning_flag为1说明转向
    
		// 可以在这里添加数据使用逻辑
    // 例如: 发送到上位机、姿态解算等
    
    HAL_Delay(10); // 添加适当延时控制采样率
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */