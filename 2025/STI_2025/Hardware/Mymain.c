#include "Mymain.h"

// *******************调试模式*******************
//#define PID_Check			// 调试电机PID
//#define PI_Line_Mode	// 树莓派视觉巡线模式
#define Y8_LineMode	// 8度寻迹巡线模式
// *******************全局变量*******************
// 电机
int goalPoint_A ;					// 电机目标转速
int goalPoint_B ;					// 电机目标转速
int goalPointTwo;					// 共同速度
bool isBreak = true;			// 刹车判断
int goalPoint_Basic_High;	// 基础高速模式
int goalPoint_Basic_Low ;	// 基础低速模式

// 树莓派视觉传感器
// 巡线
Pid_Typedef PID_Line ;							// 树莓派巡线PID

extern int Pi_xLine_goal ;					// x 的目标值
// 数据包内容
extern int Pi_xLine_real ;					// x 的真实值,数据量 x_real + 100
extern int Pi_task1	;								// 运动: 0 , 停止: 1 ,等停5秒: 2
extern int Pi_angle 	;							// angle + 100:偏转角度

int Pi_Speed_Max = 10 ;			// 速度环最大差值
int Pi_Wait_Flag = 0 ;			// 等停标示标志位,0:等待停止标志位中 , 1:识别到等停 2:注销等停模式

// 计时器
extern float time_us ;			// 计时参数,计算时间戳

// *******************实验区域*******************
int check1 ;
int check2 ;
int check[50] ;
extern float Y8_Line_C ;
extern Pid_Typedef Y8_Line_PID ;
extern Car_Position_Typedef Car_Y8_Pos ;	

extern float Y8_JQ[9] ;

int OLED_MODE = 0 ;

// *******************任务调度*******************
// 任务1:电机状态更新
mytask Motor_Status ;	
void Motor_Update_Entray_Pi(void) ;
void Motor_Pi_Check(void) ;		// 电机与树莓派联调函数(模拟)

// 实验
void Motor_Update_Entray_Check(void) ;
void Motor_PID_Check(void) ;	// 电机调节PID测试函数
// 任务2:
extern mytask Y8_Line_Status ;
void Motor_Update_Entray_Y8(void)	;// Mode1:Y8寻迹
void Motor_VOFA_Set_Y8(void) ;		 // Mode1:Y8寻迹
int Y8_Speed_MAX = 40;

void Mymain(void)
{
	// ***********初始化***********
	{
		HAL_SYSTICK_Config(SystemCoreClock / 1000);	// 启动Systick时钟
		OLED_Init() ;																// 初始化OLED
		Serial_Init(&Serial_huart) ;								// 串口_初始化
		Serial3_Init(&Serial3_huart) ;							// 串口_树莓派初始化
		Motor_A_Init();															// 电机A初始化
		Motor_B_Init();															// 电机B初始化
		Timer_Counter_Init() ;											// 计时器初始化,计算任务时间戳
		PID_Init(&PID_Line , 0.3f , 0.0f , 0.0f , Pi_Speed_Max , -Pi_Speed_Max , 1000) ;	// 树莓派巡线初始化
		Y8_Line_Init(15.0f , 0.0f , 0.0f , Y8_Speed_MAX , -Y8_Speed_MAX , 1000 ) ;															// 巡线模块初始化
		// 全部初始化完毕后再开启Systick中断
		__enable_irq();
	}
	// ***********任务调度清单***********
	#ifdef PID_Check		// 调试电机PID模式
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Check) ;	// 调试电机PID模式
	#endif
	#ifdef PI_Line_Mode	// 树莓派视觉巡线模式
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Pi) ;			// 树莓派视觉巡线模式
	#endif
	#ifdef Y8_LineMode// 8度寻迹巡线模式
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Y8) ;			// 8度寻迹巡线模式
	#endif
	
	while (1)
	{
		#ifdef PID_Check			// 调试电机PID模式
		Motor_PID_Check() ;		
		#endif
		#ifdef PI_Line_Mode		// 树莓派视觉巡线模式
		Motor_Pi_Check() ;		
		#endif
		#ifdef Y8_LineMode		// 8度寻迹巡线模式
		Motor_VOFA_Set_Y8() ;
		#endif
		Y8_LineSensor_Update() ;
		// 树莓派数据更新+亮灯调节
		RasPi_Data_Update() ;
		// 树莓派停止和驱动指令
		RasPi_Func() ;
		// 菜单执行功能
		Menu_Func() ;
		// **********实验区域**********	
		// OLED显示参数
		{
		if (isBreak == true)
			OLED_MODE = 1 ;	// 打开OLED
		else if (OLED_MODE == 1 && isBreak == false)
			OLED_MODE = 2 ;	// 关闭OLED预留状态
		
		if (OLED_MODE == 1)
		{
			// 使用OLED
			OLED_Clear() ;
			
			OLED_Printf( 0 , 0  , OLED_6X8 , "  %d    %d    %d    %d", Y8_Line_Array[1] , Y8_Line_Array[2] , Y8_Line_Array[3] ,Y8_Line_Array[4]) ;
			OLED_Printf( 0 , 15 , OLED_6X8 , "%.1f %.1f %.1f %.1f", Y8_JQ[1] , Y8_JQ[2] , Y8_JQ[3] ,Y8_JQ[4]) ;
			
			OLED_Printf( 0 , 30  , OLED_6X8 , "  %d    %d    %d    %d", Y8_Line_Array[5] , Y8_Line_Array[6] , Y8_Line_Array[7] ,Y8_Line_Array[8]) ;
			OLED_Printf( 0 , 45 , OLED_6X8 , " %.1f  %.1f  %.1f  %.1f", Y8_JQ[5] , Y8_JQ[6] , Y8_JQ[7] ,Y8_JQ[8]) ;
			
			OLED_Update() ;
		}
		else if (OLED_MODE == 2)
		{
			OLED_Clear() ;
			OLED_Update() ;
			OLED_MODE = 0 ;	// 正式关闭
		}
	}
		
		
	
		// 巡线调试
		if (Key_Check(KEY_1 , KEY_SINGLE))
		{
			HAL_Delay(2000) ;
			goalPointTwo = 80 ;
			isBreak = 0;
		}
		if (Key_Check(KEY_2 , KEY_SINGLE))
		{
			isBreak = 1 ;
		}
		
		// 巡线相关工程调试,起始位置识别
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
		
		// 其他
	
	}
}
void Motor_Update_Entray_Y8(void)	// Mode1:Y8寻迹
{
	// 刹车判断
	if (isBreak)
	{
		goalPoint_A = 0 ;
		goalPoint_B = 0 ;
	}
	// 测速与PID更新
	Motor_Speed_Update(&Motor_A) ;								// 编码器测速,得到真实速度
	Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;	// 配置目标速度
	Motor_PID_Update(&Motor_A) ;									// PID更新,得到设定速度
	
	Motor_Speed_Update(&Motor_B) ;								// 编码器测速,得到真实速度
	Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;	// 配置目标速度
	Motor_PID_Update(&Motor_B) ;									// PID更新,得到设定速度
	
	
	// 电机配置速度
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;	// 配置设定速度
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;	// 配置设定速度
}
void Motor_VOFA_Set_Y8(void)
{
	// *文本包调试程序*
	if (Serial_GetNewPackageFlag_ABC() == 1)
	{
		// 基础速度设置
		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPointTwo)){ ; }
		
		// 寻迹倍增系数调整
		if (Serial_SetFloatData("Line_C" , "Line_C=%f" , &Y8_Line_C)) { ; }
		
		// Y8_Speed_MAX
		if (Serial_SetIntData("Y8_Speed_MAX" , "Y8_Speed_MAX=%d" , &Y8_Speed_MAX))
		{
			Y8_Line_PID.OutMax = Y8_Speed_MAX ;
			Y8_Line_PID.OutMin = -Y8_Speed_MAX ;
		}
		
		// 加权值判定
		if (Serial_SetFloatData("JQb" , "JQb=%f" , &Y8_JQ[2])) { ; }
		if (Serial_SetFloatData("JQc" , "JQc=%f" , &Y8_JQ[3])) { ; }
		if (Serial_SetFloatData("JQd" , "JQd=%f" , &Y8_JQ[4])) { ; }
		
		if (Serial_SetFloatData("JQe" , "JQe=%f" , &Y8_JQ[5])) { ; }
		if (Serial_SetFloatData("JQf" , "JQf=%f" , &Y8_JQ[6])) { ; }
		if (Serial_SetFloatData("JQg" , "JQg=%f" , &Y8_JQ[7])) { ; }
		if (Serial_SetFloatData("JQh" , "JQh=%f" , &Y8_JQ[8])) { ; }
		
		// 调节PID
		Serial_SetFloatData("KpC" , "KpC=%f" , &Y8_Line_PID.Kp) ;
		Serial_SetFloatData("KiC" , "KiC=%f" , &Y8_Line_PID.Ki) ;
		Serial_SetFloatData("KdC" , "KdC=%f" , &Y8_Line_PID.Kd) ;
		
		// 刹车与重启
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
	// *VOFA展示电机状态*
	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
	printf("%f,%f,%f,%d,%d\n", Y8_Line_PID.goalPoint , Y8_Line_PID.realPoint_Now , Y8_Line_PID.setPoint , -Motor_A.RealSpeed , Motor_B.RealSpeed) ;
//	printf("%f,%f,%f,%f,%f\n", Y8_Line_PID.realPoint_Now , Y8_Line_PID.setPoint , Y8_Line_PID.pout , Y8_Line_PID.iout , Y8_Line_PID.dout ) ;
}

// Systick定时中断
void HAL_SYSTICK_Callback(void)
{
	// 按键自身就有任务调度
	Key_Tick() ;
	// 任务1:电机状态更新
	task_possess(&Motor_Status) ;
	// 任务2:等停5s
	if (Pi_Wait_Flag == 1)
	{
		static int Pi_Line_Wait_Count = 5000 ;
		Pi_Line_Wait_Count -- ;
		if (Pi_Line_Wait_Count == 0)
		{
			isBreak = false ;
			Pi_Wait_Flag = 2 ;	// 再也不允许运行第二次了
		}
	}
	// 任务3:Y8巡线
	task_possess(&Y8_Line_Status) ;
}

