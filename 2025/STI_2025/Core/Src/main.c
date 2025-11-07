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
#include "i2c.h"
#include "tim.h"
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
	
	使用USART1与电脑通信:
	VOFA改为115200 Serial改为1 Printf重定向为1
	使用蓝牙与电脑通信:
	搁置USART1 VOFA改为9600 Serial改为2 Printf重定向为2 
*/

// *******************库/函数导入*******************
#include "Mymain.h"

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
	// 暂时关闭 SysTick 中断,否则电机还没有进行初始化就驱动,导致指针指向空,发生越界错误
  __disable_irq();
	
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	
	// ******************* setup *******************
	
	Mymain() ;
	
	// ******************* 实验区域 *******************
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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


// 串口空闲中断回调函数
/*串口接收中断回调*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == Serial_USART)
	{
		#ifdef Serial_Debug
		Serial_check[Serial_Count++] = Serial_Rx_Data.rx_temp ;	// 得到所有接收到的数据
		#endif 
		
		// 获得串口数据传输状态(更新)
		Serial_Rx_State = Serial_Rx_State_Check();
		
		// HEX数据包
		if (Serial_Rx_State == RX_OK_HEX)
		{
			// 开始处理原始数据包:HEX
			Serial_Data_Check_HEX() ;
		}
		// ABC数据包
		else if (Serial_Rx_State == RX_OK_ABC)
		{
			// 开始处理原始数据包:ABC
			Serial_Data_Check_ABC() ;
		}
		
		// 重新打开串口DMA接收，DMA配置为不连续模式
		HAL_UART_Receive_DMA(huart, &Serial_Rx_Data.rx_temp , 1);   
	}
	if(huart->Instance == Serial3_USART)
	{
		#ifdef Serial3_Debug
		Serial3_check[Serial3_Count++] = Serial3_Rx_Data.rx_temp ;	// 得到所有接收到的数据
		#endif 
		
		// 获得串口数据传输状态(更新)
		Serial3_Rx_State = Serial3_Rx_State_Check();
		
		// HEX数据包
		if (Serial3_Rx_State == RX_OK_HEX)
		{
			// 开始处理原始数据包:HEX
			Serial3_Data_Check_HEX() ;
		}
		// ABC数据包
		else if (Serial3_Rx_State == RX_OK_ABC)
		{
			// 开始处理原始数据包:ABC
			Serial3_Data_Check_ABC() ;
		}
		
		// 重新打开串口DMA接收，DMA配置为不连续模式
		HAL_UART_Receive_DMA(huart, &Serial3_Rx_Data.rx_temp , 1);   
	}
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
