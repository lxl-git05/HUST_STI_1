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
// *******************ǰ��*******************
/*
	USART1�Ѿ���Serial�ӹ�,���ݽ���ֻ����һ����Э�����
	
	���Զ�Ϊ��λ��,ͨ��VOFA����ָ��(HEX���ı���������)
	STM32��Ϊ��λ��,ͨ��USART1����ָ��,ִ����Ӧ����,����STM32���͸�����(VOFA)����Ϣ����Ҫ��ѭ���Э��
	
	ʹ��USART1�����ͨ��:
	VOFA��Ϊ115200 Serial��Ϊ1 Printf�ض���Ϊ1
	ʹ�����������ͨ��:
	����USART1 VOFA��Ϊ9600 Serial��Ϊ2 Printf�ض���Ϊ2 
*/

// *******************��/��������*******************
// ϵͳ��
#include <stdlib.h>
#include "string.h"
#include <stdio.h>
// �����
#include "OLED.h"
#include "Key.h"
#include "Serial.h"

#include "Encoder_Motor.h"
#include "Encoder.h"
#include "Motor.h"

// *******************ȫ�ֱ���*******************
extern Serial_HEX_Data_Typedef   Serial_Hex_Data ;			// �����õ�HEX���ݰ�
extern Serial_ABC_Data_Typedef   Serial_ABC_Data ;			// �����õ�ABC���ݰ�

extern Motor_Typedef Motor_A ;	// ���A
extern Motor_Typedef Motor_B ;	// ���B

int goalPoint_A ;	// ���Ŀ��ת��
int goalPoint_B ;	// ���Ŀ��ת��

// *******************ʵ������*******************

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
	
	// ��ʱ�ر� SysTick �ж�,��������û�н��г�ʼ��������,����ָ��ָ���,����Խ�����
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
	// ����Systickʱ��
	HAL_SYSTICK_Config(SystemCoreClock / 1000);
	// ��ʼ��OLED
	OLED_Init() ;
	// ���ڳ�ʼ��
	Serial_Init(&Serial_huart) ;
	// �����ʼ��:PWM , Encoder , PID
	Motor_A_Init();
	Motor_B_Init();
	// ȫ����ʼ����Ϻ��ٿ����ж�
  __enable_irq();
	
	// ******************* ʵ������ *******************
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// ******************* while *******************
		// ���԰�������
		if (Key_Check(KEY_1 , KEY_SINGLE))
		{
			HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin) ;
		}
		// ******************* ʵ������ *******************
		// �߼�:����ͨ��VOFA�������ݰ�,STM32ͨ������1����ָ��,Ȼ�������Ӧ�Ĳ���,����:
		if (Serial_GetNewPackageFlag_ABC() == 1)
		{
			// �ı������Գ���
			Serial_SetIntData("goalPoint_A" , "goalPoint_A=%d" , &goalPoint_A) ;
			Serial_SetIntData("goalPoint_B" , "goalPoint_B=%d" , &goalPoint_B) ;
			
			Serial_SetFloatData("KpA" , "KpA=%f" , &Motor_A.PID_s.Kp) ;
			Serial_SetFloatData("KiA" , "KiA=%f" , &Motor_A.PID_s.Ki) ;
			Serial_SetFloatData("KdA" , "KdA=%f" , &Motor_A.PID_s.Kd) ;
			
			Serial_SetFloatData("KpB" , "KpB=%f" , &Motor_B.PID_s.Kp) ;
			Serial_SetFloatData("KiB" , "KiB=%f" , &Motor_B.PID_s.Ki) ;
			Serial_SetFloatData("KdB" , "KdB=%f" , &Motor_B.PID_s.Kd) ;
			
			// �������ӵ���
			// ɲ��
			if ( Serial_SetIntData("break" , "break=%d" , &goalPoint_A) )
			{
				goalPoint_A = 0 ;
				goalPoint_B = 0 ;
			}
			// һ����
			if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPoint_A))
			{
				goalPoint_B = goalPoint_A ;
			}
		}
		// OLEDչʾ
		OLED_Printf(0 , 0 , OLED_8X16 , "Asrg:%d %d %d" ,Motor_A.SetSpeed, Motor_A.RealSpeed, Motor_A.GoalSpeed ) ;
		OLED_Printf(0 ,15 , OLED_8X16 , "A:%.2f,%.2f,%.2f",Motor_A.PID_s.Kp,Motor_A.PID_s.Ki,Motor_A.PID_s.Kd) ;
		
		OLED_Printf(0 ,30 , OLED_8X16 , "Bsrg:%d %d %d" ,Motor_B.SetSpeed, Motor_B.RealSpeed, Motor_B.GoalSpeed ) ;
		OLED_Printf(0 ,45 , OLED_8X16 , "B:%.2f,%.2f,%.2f",Motor_B.PID_s.Kp,Motor_B.PID_s.Ki,Motor_B.PID_s.Kd) ;
		
		Set_Current_USART(USART2_IDX); /* ��Ҫָ����ͬ���ڱ�����printfǰ���ϴ˺��� */
		// ����չʾ
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_A.PID_s.pout,Motor_A.PID_s.iout,Motor_A.PID_s.dout);
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
		
		// ����
		printf("%d,%d,%d,%d,%d,%d\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed);

		// ���Ŀ���ٶȺ�����ٶȸ���
		Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;
		Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;
		
		Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;
		Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;
		
		// �������:OLED����
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
// Systick��ʱ�ж�
void HAL_SYSTICK_Callback(void)
{
	// ��ʱ
	static int count_sys = 0 ;
	count_sys ++ ;
	// ����1 : ����
	Key_Tick() ;
	// ����2 : ����������+PID����
	if (count_sys % Encoder_PID_Gap_Time == 0)
	{
		Motor_Speed_Update(&Motor_A) ;			// ����������
		Motor_PID_Update(&Motor_A) ;				// PID����
		
		Motor_Speed_Update(&Motor_B) ;			// ����������
		Motor_PID_Update(&Motor_B) ;				// PID����
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
