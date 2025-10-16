/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/*** 定义USART索引枚举 ***/
typedef enum {
	USART_NONE,			/* 无USART */
	USART1_IDX,			/* USART1索引 */
	USART2_IDX,			/* USART2索引 */
	USART3_IDX,			/* USART3索引 */
}Current_USART_Indx;

extern UART_HandleTypeDef* Current_USART_Handle;		  /* 当前某个USART的句柄 */
extern Current_USART_Indx Current_USART_Printf_Indx;	/* 当前某个USART的索引 */
 
void Set_Current_USART(Current_USART_Indx indx);			/* 函数声明，用于设置当前使用的USART */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

