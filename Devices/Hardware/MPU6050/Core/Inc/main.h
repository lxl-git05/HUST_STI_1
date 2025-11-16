/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

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
#define SCL_Pin GPIO_PIN_3
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_4
#define SDA_GPIO_Port GPIOB
#define I2C_PORT GPIOB

// Òý½Å²Ù×÷ºê
#define SCL_HIGH()  HAL_GPIO_WritePin(I2C_PORT, SCL_PIN, GPIO_PIN_SET)
#define SCL_LOW()   HAL_GPIO_WritePin(I2C_PORT, SCL_PIN, GPIO_PIN_RESET)
#define SDA_HIGH()  HAL_GPIO_WritePin(I2C_PORT, SDA_PIN, GPIO_PIN_SET)
#define SDA_LOW()   HAL_GPIO_WritePin(I2C_PORT, SDA_PIN, GPIO_PIN_RESET)
#define SDA_READ()  HAL_GPIO_ReadPin(I2C_PORT, SDA_PIN)
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
