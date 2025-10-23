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
// 系统库
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
// 自设库
#include "OLED.h"
#include "Key.h"
#include "Serial.h"

#include "Encoder_Motor.h"
#include "Encoder.h"
#include "Motor.h"

// *******************全局变量*******************
extern Serial_HEX_Data_Typedef   Serial_Hex_Data ;			// 解析好的HEX数据包
extern Serial_ABC_Data_Typedef   Serial_ABC_Data ;			// 解析好的ABC数据包

extern Motor_Typedef Motor_A ;	// 电机A
extern Motor_Typedef Motor_B ;	// 电机B

int goalPoint_A ;	// 电机目标转速
int goalPoint_B ;	// 电机目标转速

// *******************实验区域*******************

int check1 ;
int check2 ;
int check[50] ;

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
  /* USER CODE BEGIN 2 */
	
	// ******************* setup *******************
	// 启动Systick时钟
	HAL_SYSTICK_Config(SystemCoreClock / 1000);
	// 初始化OLED
	OLED_Init() ;
	// 串口初始化
	Serial_Init(&Serial_huart) ;
	// 电机初始化:PWM , Encoder , PID
	Motor_A_Init();
	Motor_B_Init();
	// 全部初始化完毕后再开启中断
  __enable_irq();
	
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
		}
		// ******************* 实验区域 *******************
		// 逻辑:电脑通过VOFA发送数据包,STM32通过串口1接受指令,然后进行相应的操作,如下:
		if (Serial_GetNewPackageFlag_ABC() == 1)
		{
			// 文本包调试程序
			Serial_SetIntData("goalPoint_A" , "goalPoint_A=%d" , &goalPoint_A) ;
			Serial_SetIntData("goalPoint_B" , "goalPoint_B=%d" , &goalPoint_B) ;
			
			Serial_SetFloatData("KpA" , "KpA=%f" , &Motor_A.PID_s.Kp) ;
			Serial_SetFloatData("KiA" , "KiA=%f" , &Motor_A.PID_s.Ki) ;
			Serial_SetFloatData("KdA" , "KdA=%f" , &Motor_A.PID_s.Kd) ;
			
			Serial_SetFloatData("KpB" , "KpB=%f" , &Motor_B.PID_s.Kp) ;
			Serial_SetFloatData("KiB" , "KiB=%f" , &Motor_B.PID_s.Ki) ;
			Serial_SetFloatData("KdB" , "KdB=%f" , &Motor_B.PID_s.Kd) ;
			
			// 两个轮子调试
			// 刹车
			if ( Serial_SetIntData("break" , "break=%d" , &goalPoint_A) )
			{
				goalPoint_A = 0 ;
				goalPoint_B = 0 ;
			}
			// 一起跑
			if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPoint_A))
			{
				goalPoint_B = goalPoint_A ;
			}
		}
		// OLED展示
		OLED_Printf(0 , 0 , OLED_8X16 , "Asrg:%d %d %d" ,Motor_A.SetSpeed, Motor_A.RealSpeed, Motor_A.GoalSpeed ) ;
		OLED_Printf(0 ,15 , OLED_8X16 , "A:%.2f,%.2f,%.2f",Motor_A.PID_s.Kp,Motor_A.PID_s.Ki,Motor_A.PID_s.Kd) ;
		
		OLED_Printf(0 ,30 , OLED_8X16 , "Bsrg:%d %d %d" ,Motor_B.SetSpeed, Motor_B.RealSpeed, Motor_B.GoalSpeed ) ;
		OLED_Printf(0 ,45 , OLED_8X16 , "B:%.2f,%.2f,%.2f",Motor_B.PID_s.Kp,Motor_B.PID_s.Ki,Motor_B.PID_s.Kd) ;
		
		Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
		// 单独展示
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_A.PID_s.pout,Motor_A.PID_s.iout,Motor_A.PID_s.dout);
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
		
		// 联调
		printf("%d,%d,%d,%d,%d,%d\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed);

		// 电机目标速度和输出速度更新
		Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;
		Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;
		
		Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;
		Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;
		
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
	// 功能1 : 按键
	Key_Tick() ;
	// 功能2 : 编码器测速+PID调控
	if (count_sys % Encoder_PID_Gap_Time == 0)
	{
		Motor_Speed_Update(&Motor_A) ;			// 编码器测速
		Motor_PID_Update(&Motor_A) ;				// PID更新
		
		Motor_Speed_Update(&Motor_B) ;			// 编码器测速
		Motor_PID_Update(&Motor_B) ;				// PID更新
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
