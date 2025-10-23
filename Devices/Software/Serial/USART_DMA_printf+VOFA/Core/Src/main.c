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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// *******************前言*******************
/*
	USART1已经被Serial接管,数据接收只能以一定的协议进行
	
	电脑端为上位机,通过VOFA发送指令(HEX和文本两种类型)
	STM32段为下位机,通过USART1接受指令,执行相应命令,但是STM32发送个电脑(VOFA)的信息不需要遵循相关协议
*/

// *******************库/函数导入*******************
// 系统库
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
// 自设库
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
// *******************全局变量*******************
extern Serial_HEX_Data_Typedef   Serial_Hex_Data ;			// 解析好的HEX数据包
extern Serial_ABC_Data_Typedef   Serial_ABC_Data ;			// 解析好的ABC数据包

// *******************实验区域*******************
int checka ;
int checkb ;
int check[50] ;

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	
	// ******************* setup *******************
	// 启动Systick时钟
	HAL_SYSTICK_Config(SystemCoreClock / 1000);
	// 初始化OLED
	OLED_Init() ;
	OLED_ShowString(0 , 0 , "Hello World" , OLED_8X16 ) ;
	// *串口初始化*
	Serial_Init(&Serial_huart) ;
	
	// ******************* 实验区域 *******************

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// ******************* while *******************
		// 测试按键功能
		if (Key_Check(KEY_1 , KEY_SINGLE))
		{
			HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin) ;
			
			Set_Current_USART(USART1_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
			printf("Serial=1\n");
			
		}
		// ******************* 实验区域 *******************
		// 逻辑:电脑通过VOFA发送数据包,STM32通过串口1接受指令,然后进行相应的操作,如下:
		if (Serial_GetNewPackageFlag_HEX() == 1)
		{
			// OLED展示各个数据
			OLED_ShowNum(0 , 20 , Serial_Hex_Data.Serial_New_Package[0] , 1 , OLED_8X16 ) ;
			for (int i = 1 ; i < Serial_Hex_Data.Serial_New_Package[0] + 1 ; i ++)
			{
					OLED_ShowNum(20 , 10 + 10 * i , Serial_Hex_Data.Serial_New_Package[i] , 5 , OLED_6X8 ) ;
			}
		}
		if (Serial_GetNewPackageFlag_ABC() == 1)
		{
				// 文本包调试程序
				Serial_SetIntData("checka" , "checka=%d" , &checka) ;

				// OLED展示
				OLED_ShowNum(0,  20 , checka , 3 , OLED_8X16 ) ;
		}
		// 必须存在:OLED更新
		OLED_Update() ;
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
// Systick定时中断
void HAL_SYSTICK_Callback(void)
{
	// 计时
	static int count_sys = 0 ;
	count_sys ++ ;
	// 功能1:
	if (count_sys % 1000 == 0)
	{
//		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
	}
	// 功能2: 按键
	Key_Tick() ;
}

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
  *         where the assert_param error has occurred.
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
