/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY1_Pin GPIO_PIN_3
#define KEY1_GPIO_Port GPIOE
#define KEY0_Pin GPIO_PIN_4
#define KEY0_GPIO_Port GPIOE
#define Stepper_Stp_Pin GPIO_PIN_5
#define Stepper_Stp_GPIO_Port GPIOE
#define Stepper_Dir_Pin GPIO_PIN_6
#define Stepper_Dir_GPIO_Port GPIOE
#define I2C2_SCL_Pin GPIO_PIN_0
#define I2C2_SCL_GPIO_Port GPIOF
#define I2C2_SDA_Pin GPIO_PIN_1
#define I2C2_SDA_GPIO_Port GPIOF
#define EC11_S1_Pin GPIO_PIN_3
#define EC11_S1_GPIO_Port GPIOF
#define EC11_S1_EXTI_IRQn EXTI3_IRQn
#define KEY2_Pin GPIO_PIN_4
#define KEY2_GPIO_Port GPIOF
#define EC11_Key_Pin GPIO_PIN_5
#define EC11_Key_GPIO_Port GPIOF
#define KEY3_Pin GPIO_PIN_6
#define KEY3_GPIO_Port GPIOF
#define EC11_S2_Pin GPIO_PIN_7
#define EC11_S2_GPIO_Port GPIOF
#define EC11_S2_EXTI_IRQn EXTI9_5_IRQn
#define Stepper_En_Pin GPIO_PIN_0
#define Stepper_En_GPIO_Port GPIOC
#define Stepper_En2_Pin GPIO_PIN_2
#define Stepper_En2_GPIO_Port GPIOC
#define Motor_A_Encoder_1_Pin GPIO_PIN_0
#define Motor_A_Encoder_1_GPIO_Port GPIOA
#define Motor_A_Encoder_2_Pin GPIO_PIN_1
#define Motor_A_Encoder_2_GPIO_Port GPIOA
#define AT_SCL_Pin GPIO_PIN_4
#define AT_SCL_GPIO_Port GPIOA
#define AT_SDA_Pin GPIO_PIN_5
#define AT_SDA_GPIO_Port GPIOA
#define Motor_B_Encoder_1_Pin GPIO_PIN_6
#define Motor_B_Encoder_1_GPIO_Port GPIOA
#define Motor_B_Encoder_2_Pin GPIO_PIN_7
#define Motor_B_Encoder_2_GPIO_Port GPIOA
#define Y8_Out_Pin GPIO_PIN_4
#define Y8_Out_GPIO_Port GPIOC
#define Y8_Addr_0_Pin GPIO_PIN_11
#define Y8_Addr_0_GPIO_Port GPIOF
#define Err_Pin GPIO_PIN_12
#define Err_GPIO_Port GPIOF
#define Y8_Addr_1_Pin GPIO_PIN_13
#define Y8_Addr_1_GPIO_Port GPIOF
#define Y8_Addr_2_Pin GPIO_PIN_15
#define Y8_Addr_2_GPIO_Port GPIOF
#define Buzzer_Pin GPIO_PIN_1
#define Buzzer_GPIO_Port GPIOG
#define Elec_Pin GPIO_PIN_11
#define Elec_GPIO_Port GPIOE
#define Stp1_Rst_Pin GPIO_PIN_10
#define Stp1_Rst_GPIO_Port GPIOB
#define Stp1_Err_Pin GPIO_PIN_11
#define Stp1_Err_GPIO_Port GPIOB
#define Stepper_Stp2_Pin GPIO_PIN_14
#define Stepper_Stp2_GPIO_Port GPIOB
#define Stepper_Dir2_Pin GPIO_PIN_15
#define Stepper_Dir2_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOD
#define LCD_DC_Pin GPIO_PIN_12
#define LCD_DC_GPIO_Port GPIOD
#define LCD_BLK_Pin GPIO_PIN_13
#define LCD_BLK_GPIO_Port GPIOD
#define Motor_A_IN1_Pin GPIO_PIN_14
#define Motor_A_IN1_GPIO_Port GPIOD
#define Motor_B_IN1_Pin GPIO_PIN_15
#define Motor_B_IN1_GPIO_Port GPIOD
#define RGB_R_Pin GPIO_PIN_2
#define RGB_R_GPIO_Port GPIOG
#define RGB_G_Pin GPIO_PIN_4
#define RGB_G_GPIO_Port GPIOG
#define RGB_Y_Pin GPIO_PIN_6
#define RGB_Y_GPIO_Port GPIOG
#define Motor_B_IN2_Pin GPIO_PIN_8
#define Motor_B_IN2_GPIO_Port GPIOC
#define Motor_A_IN2_Pin GPIO_PIN_11
#define Motor_A_IN2_GPIO_Port GPIOA
#define Laser_Pin GPIO_PIN_12
#define Laser_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_5
#define LED0_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_7
#define I2C1_SDA_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_9
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
