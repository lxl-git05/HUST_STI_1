#include "Mymain.h"

// *******************????*******************
//#define PID_Check			// ????PID
//#define PI_Line_Mode	// ?????????
#define Y8_LineMode	// 8???????
// *******************????*******************
// ??
int goalPoint_A ;					// ??????
int goalPoint_B ;					// ??????
int goalPointTwo;					// ????
bool isBreak = true;			// ????
int goalPoint_Basic_High;	// ??????
int goalPoint_Basic_Low ;	// ??????

// ????????
// ??
Pid_Typedef PID_Line ;							// ?????PID

extern int Pi_xLine_goal ;					// x ????
// ?????
extern int Pi_xLine_real ;					// x ????,??? x_real + 100
extern int Pi_task1	;								// ??: 0 , ??: 1 ,??5?: 2
extern int Pi_angle 	;							// angle + 100:????

int Pi_Speed_Max = 10 ;			// ???????
int Pi_Wait_Flag = 0 ;			// ???????,0:???????? , 1:????? 2:??????

// ???
extern float time_us ;			// ????,?????

// *******************????*******************
int check1 ;
int check2 ;
int check[50] ;
extern float Y8_Line_C ;
extern Pid_Typedef Y8_Line_PID ;
extern Car_Position_Typedef Car_Y8_Pos ;	

extern float Y8_JQ[9] ;

int OLED_MODE = 0 ;

// *******************????*******************
// ??1:??????
mytask Motor_Status ;	
void Motor_Update_Entray_Pi(void) ;
void Motor_Pi_Check(void) ;		// ??????????(??)

// ??
void Motor_Update_Entray_Check(void) ;
void Motor_PID_Check(void) ;	// ????PID????
// ??2:
extern mytask Y8_Line_Status ;
void Motor_Update_Entray_Y8(void)	;// Mode1:Y8??
void Motor_VOFA_Set_Y8(void) ;		 // Mode1:Y8??
int Y8_Speed_MAX = 40;

void Mymain(void)
{
	// ***********???***********
	{
		HAL_SYSTICK_Config(SystemCoreClock / 1000);	// ??Systick??
		OLED_Init() ;																// ???OLED
		Serial_Init(&Serial_huart) ;								// ??_???
		Serial3_Init(&Serial3_huart) ;							// ??_??????
		Motor_A_Init();															// ??A???
		Motor_B_Init();															// ??B???
		Timer_Counter_Init() ;											// ??????,???????
		PID_Init(&PID_Line , 0.3f , 0.0f , 0.0f , Pi_Speed_Max , -Pi_Speed_Max , 1000) ;	// ????????
		Y8_Line_Init(15.0f , 0.0f , 0.0f , Y8_Speed_MAX , -Y8_Speed_MAX , 1000 ) ;															// ???????
		Menu_Init() ;	// ?????
		// ???????????Systick??
		__enable_irq();
	}
	// ***********??????***********
	#ifdef PID_Check		// ????PID??
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Check) ;	// ????PID??
	#endif
	#ifdef PI_Line_Mode	// ?????????
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Pi) ;			// ?????????
	#endif
	#ifdef Y8_LineMode// 8???????
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Y8) ;			// 8???????
	#endif
	
//	Key_AddParam("JQ1" , &Y8_JQ[0] , 0.1f , PARAM_FLOAT ) ;
	
	while (1)
	{
		#ifdef PID_Check			// ????PID??
		Motor_PID_Check() ;		
		#endif
		#ifdef PI_Line_Mode		// ?????????
		Motor_Pi_Check() ;		
		#endif
		#ifdef Y8_LineMode		// 8???????
		Motor_VOFA_Set_Y8() ;
		#endif
		Y8_LineSensor_Update() ;
		// ???????+????
		RasPi_Data_Update() ;
		// ??????????
		RasPi_Func() ;
		// OLED????????
		Menu_Func() ;
		// **********????**********	
		
		
		// OLED????
//		{
//		if (isBreak == true)
//			OLED_MODE = 1 ;	// ??OLED
//		else if (OLED_MODE == 1 && isBreak == false)
//			OLED_MODE = 2 ;	// ??OLED????
//		
//		if (OLED_MODE == 1)
//		{
//			// ??OLED
//			OLED_Clear() ;
//			
//			OLED_Printf( 0 , 0  , OLED_6X8 , "  %d    %d    %d    %d", Y8_Line_Array[1] , Y8_Line_Array[2] , Y8_Line_Array[3] ,Y8_Line_Array[4]) ;
//			OLED_Printf( 0 , 15 , OLED_6X8 , "%.1f %.1f %.1f %.1f", Y8_JQ[1] , Y8_JQ[2] , Y8_JQ[3] ,Y8_JQ[4]) ;
//			
//			OLED_Printf( 0 , 30  , OLED_6X8 , "  %d    %d    %d    %d", Y8_Line_Array[5] , Y8_Line_Array[6] , Y8_Line_Array[7] ,Y8_Line_Array[8]) ;
//			OLED_Printf( 0 , 45 , OLED_6X8 , " %.1f  %.1f  %.1f  %.1f", Y8_JQ[5] , Y8_JQ[6] , Y8_JQ[7] ,Y8_JQ[8]) ;
//			
//			OLED_Update() ;
//		}
//		else if (OLED_MODE == 2)
//		{
//			OLED_Clear() ;
//			OLED_Update() ;
//			OLED_MODE = 0 ;	// ????
//		}
//	}
		
		
	
		// ????
//		if (Key_Check(KEY_1 , KEY_SINGLE))
//		{
//			HAL_Delay(2000) ;
//			goalPointTwo = 80 ;
//			isBreak = 0;
//		}
//		if (Key_Check(KEY_2 , KEY_SINGLE))
//		{
//			isBreak = 1 ;
//		}
//		if (Key_Check(KEY_1 , KEY_SINGLE))
//		{
//			HAL_Delay(2000) ;
//			goalPointTwo = 80 ;
//			isBreak = 0;
//		}
//		if (Key_Check(KEY_2 , KEY_SINGLE))
//		{
//			isBreak = 1 ;
//		}
		
		// ????????,??????
//		if (Y8_is_Init(&is_Car_Init_Pos))
//		{
//			 while(1)
//			 {
//				 	OLED_Clear() ;
//			
//					OLED_Printf( 0 , 0  , OLED_6X8 , "  %d    %d    %d    %d", Y8_Line_Array[1] , Y8_Line_Array[2] , Y8_Line_Array[3] ,Y8_Line_Array[4]) ;
//					OLED_Printf( 0 , 15 , OLED_6X8 , "%.1f %.1f %.1f %.1f", Y8_JQ[1] , Y8_JQ[2] , Y8_JQ[3] ,Y8_JQ[4]) ;
//					
//					OLED_Printf( 0 , 30  , OLED_6X8 , "  %d    %d    %d    %d", Y8_Line_Array[5] , Y8_Line_Array[6] , Y8_Line_Array[7] ,Y8_Line_Array[8]) ;
//					OLED_Printf( 0 , 45 , OLED_6X8 , " %.1f  %.1f  %.1f  %.1f", Y8_JQ[5] , Y8_JQ[6] , Y8_JQ[7] ,Y8_JQ[8]) ;
//					
//					OLED_Update() ;

//					isBreak = 1 ;
//				 
//			 }
//		}
		
		// ??
	
	}
}
void Motor_Update_Entray_Y8(void)	// Mode1:Y8??
{
	// ????
	if (isBreak)
	{
		goalPoint_A = 0 ;
		goalPoint_B = 0 ;
	}
	// ???PID??
	Motor_Speed_Update(&Motor_A) ;								// ?????,??????
	Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;	// ??????
	Motor_PID_Update(&Motor_A) ;									// PID??,??????
	
	Motor_Speed_Update(&Motor_B) ;								// ?????,??????
	Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;	// ??????
	Motor_PID_Update(&Motor_B) ;									// PID??,??????
	
	
	// ??????
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;	// ??????
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;	// ??????
}
void Motor_VOFA_Set_Y8(void)
{
	// *???????*
	if (Serial_GetNewPackageFlag_ABC() == 1)
	{
		// ??????
		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPointTwo)){ ; }
		
		// ????????
		if (Serial_SetFloatData("Line_C" , "Line_C=%f" , &Y8_Line_C)) { ; }
		
		// Y8_Speed_MAX
		if (Serial_SetIntData("Y8_Speed_MAX" , "Y8_Speed_MAX=%d" , &Y8_Speed_MAX))
		{
			Y8_Line_PID.OutMax = Y8_Speed_MAX ;
			Y8_Line_PID.OutMin = -Y8_Speed_MAX ;
		}
		
		// ?????
		if (Serial_SetFloatData("JQb" , "JQb=%f" , &Y8_JQ[2])) { ; }
		if (Serial_SetFloatData("JQc" , "JQc=%f" , &Y8_JQ[3])) { ; }
		if (Serial_SetFloatData("JQd" , "JQd=%f" , &Y8_JQ[4])) { ; }
		
		if (Serial_SetFloatData("JQe" , "JQe=%f" , &Y8_JQ[5])) { ; }
		if (Serial_SetFloatData("JQf" , "JQf=%f" , &Y8_JQ[6])) { ; }
		if (Serial_SetFloatData("JQg" , "JQg=%f" , &Y8_JQ[7])) { ; }
		if (Serial_SetFloatData("JQh" , "JQh=%f" , &Y8_JQ[8])) { ; }
		
		// ??PID
		Serial_SetFloatData("KpC" , "KpC=%f" , &Y8_Line_PID.Kp) ;
		Serial_SetFloatData("KiC" , "KiC=%f" , &Y8_Line_PID.Ki) ;
		Serial_SetFloatData("KdC" , "KdC=%f" , &Y8_Line_PID.Kd) ;
		
		// ?????
		if ( Serial_SetIntData("break" , "break=%d" , &check1) )						
		{
			if (isBreak == false)
			{
				isBreak = true ;
			}
			else
			{
				isBreak = false ;
			}
		}
	}
	// *VOFA??????*
	Set_Current_USART(USART2_IDX); /* ???????????printf?????? */
	printf("%f,%f,%f,%d,%d\n", Y8_Line_PID.goalPoint , Y8_Line_PID.realPoint_Now , Y8_Line_PID.setPoint , -Motor_A.RealSpeed , Motor_B.RealSpeed) ;
//	printf("%f,%f,%f,%f,%f\n", Y8_Line_PID.realPoint_Now , Y8_Line_PID.setPoint , Y8_Line_PID.pout , Y8_Line_PID.iout , Y8_Line_PID.dout ) ;
}

// Systick????
void HAL_SYSTICK_Callback(void)
{
	// ??????????
	Key_Tick() ;
	// ??1:??????
	task_possess(&Motor_Status) ;
	// ??2:??5s
	if (Pi_Wait_Flag == 1)
	{
		static int Pi_Line_Wait_Count = 5000 ;
		Pi_Line_Wait_Count -- ;
		if (Pi_Line_Wait_Count == 0)
		{
			isBreak = false ;
			Pi_Wait_Flag = 2 ;	// ???????????
		}
	}
	// ??3:Y8??
	task_possess(&Y8_Line_Status) ;
}

