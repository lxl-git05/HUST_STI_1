#include "Mymain.h"

// *******************调试模式*******************
//#define PID_Check			// 调试电机PID
#define Y8_LineMode	// 8度寻迹巡线模式
// *******************全局变量*******************
// 电机
int goalPoint_A ;					// 电机目标转速
int goalPoint_B ;					// 电机目标转速
int goalPointTwo;					// 共同速度

bool isBreak = true;			// *********重要参数:刹车判断*********
int Car_Wait_Flag  ;			// 小车等停标志位,初始值为0,也就是不停止
int Car_Wait_cnt ;				// 小车等停计时器

// 树莓派视觉传感器
extern int Pi_RGB_Status  ;	// RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
extern int Pi_LR_Status	  ;	// LR  -> 0初始化 , 1->L  , 2->R
extern int Pi_Stop_Status ;	// wait & stop -> 0无 , 1停止 , 2等停
extern int Pi_x_Line_real ;	// 巡线x的真实值,已处理

// 计时器
extern float time_us ;			// 计时参数,计算时间戳
int Y8_Cnt = 0 ;

// Y8巡线
extern mytask Y8_Line_Status ;
extern Pid_Typedef Y8_Line_PID ;
extern Car_Position_Typedef Car_Y8_Pos ;	
int Y8_Speed_MAX = 40;
bool is_Car_Turn_Left = false	 ;	// ***重要参数:小车偏转方向***

// 菜单调控任务执行
int Car_Task_Num = 0 ;	// 初始为0,也就是没有任务

// *******************实验区域*******************
int check1 ;
int check2 ;
int check[50] ;
extern bool Car_LR_Speed_Mode ;
int Turn_Flag ;	// 转向标志位
int Turn_cnt  ;	// 转向时间计时
int Turn_cnt_n  ;	// 转向时间计时
int Turn_Num	;	// 转向次数
int Turn_ALL	; // 转过弯道的判断
// *******************任务调度*******************
// 调试状态:电机PID调试,ifndef则没用
mytask Motor_Status ;	
void Motor_Update_Entray_Check(void) ;
void Motor_PID_Check(void) ;	// 电机调节PID测试函数

// 任务1:Y8寻迹控制电机速度
void Motor_Update_Entray_Y8(void)	;	// Y8寻迹控制电机速度
void Motor_VOFA_Set_Y8(void) ;		 	// Y8寻迹+VOFA监控


void Mymain(void)
{
	// ***********初始化***********
	{
		HAL_SYSTICK_Config(SystemCoreClock / 1000);																	 // 启动Systick时钟
		OLED_Init() ;																																 // 初始化OLED
		Serial_Init(&Serial_huart) ;																								 // 串口_初始化
		Serial3_Init(&Serial3_huart) ;																							 // 串口_树莓派初始化
		Motor_A_Init();																															 // 电机A初始化
		Motor_B_Init();																															 // 电机B初始化
		Timer_Counter_Init() ;																											 // 计时器初始化,计算任务时间戳
		Y8_Line_Init(15.0f , 0.0f , 0.0f , Y8_Speed_MAX , -Y8_Speed_MAX , 1000 ) ;   // 巡线模块初始化
		Menu_Init() ;																																 // 菜单初始化
		// 全部初始化完毕后再开启Systick中断
		__enable_irq();
	}
	// ***********任务调度清单***********
	#ifdef PID_Check		// 调试电机PID模式
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Check) ;	// 调试电机PID模式
	#endif
	#ifdef Y8_LineMode// 8度寻迹巡线模式
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Y8) ;			// 8度寻迹巡线模式
	#endif
	Key_AddParam("isLeft" , &is_Car_Turn_Left , 1 , PARAM_INT ) ;	// int Car_Task_Num
	Key_AddParam("Task_Num" , &Car_Task_Num   , 1 , PARAM_INT ) ; // 
	Key_AddParam("Turn_Num" , &Turn_Num       , 1 , PARAM_INT ) ;
	Key_AddParam("Turn_ALL" , &Turn_ALL       , 1 , PARAM_INT ) ;
	while (1)
	{
		#ifdef PID_Check			// 调试电机PID模式
		Motor_PID_Check() ;		
		#endif
		#ifdef Y8_LineMode		// 8度寻迹巡线模式
		Motor_VOFA_Set_Y8() ;
		#endif
		
		// 树莓派指令更新
		RasPi_Data_Update() ;
		// OLED菜单交互控制界面
		Menu_Func() ;
		// 巡线数据更新
		Y8_LineSensor_Update() ;
		// 核心代码:OLED控制小车实现赛题
		Car_Task(Car_Task_Num) ;
		// **********实验区域**********	
		// 与树莓派融合
		
		
		// 实验:巡线判断转向:
		
		if (Motor_A.PID_s.realPoint_Now * Motor_A.DIR - Motor_B.PID_s.realPoint_Now > -20 && Motor_A.PID_s.realPoint_Now * Motor_A.DIR - Motor_B.PID_s.realPoint_Now < 20)
		{
			Turn_Num = 0 ;
		}
		
		if (Motor_A.PID_s.realPoint_Now * Motor_A.DIR > Motor_B.PID_s.realPoint_Now)
		{
			Turn_Flag = 1 ;
		}
		else
		{
			Turn_Flag = 0 ;
			Turn_cnt  = 0 ;	// 重新计时
		}
		
		if (Turn_Num >= 3)
		{
			Turn_ALL ++ ;
			Turn_Num = 0 ;
		}
		
	}
}

// Systick定时中断
void HAL_SYSTICK_Callback(void)
{
	// 按键自身就有任务调度
	Key_Tick() ;
	// 任务1:电机状态更新
	task_possess(&Motor_Status) ;
	// 任务2:Y8巡线
	task_possess(&Y8_Line_Status) ;
	// 任务3:Y8岔路差速计时器清零
	if (Y8_Cnt > 0 )
	{
		Y8_Cnt -- ;
	}
	// 任务4:等停5s
	if (Car_Wait_cnt > 0)
	{
		Car_Wait_cnt -- ;
		if (Car_Wait_cnt == 0)
		{
			isBreak = 0 ;					// 继续跑
			Car_Wait_Flag = 2 ;		// 等停标志位变为2,表明以后都不会再次识别了
		}
	}
	// 实验:小车转向时间计数
	if (Turn_Flag == 1)
	{
		Turn_cnt ++ ;
		if (Turn_cnt >= 300)
		{
			Turn_Num ++ ;
			Turn_cnt = 0 ;
		}
	}
}
