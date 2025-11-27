#include "Mymain.h"

// *******************调试模式*******************
//#define PID_Check			// 调试电机PID
#define Y8_LineMode	// 8度寻迹巡线模式
// *******************全局变量*******************
// 电机
int Car_Wait_Flag  ;							// 小车等停标志位,初始值为0,也就是不停止
int Car_Wait_cnt ;								// 小车等停计时器
bool is_Car_Turn_Left = false	 ;	// ***重要参数:小车偏转方向*** , 1左 , 0右

// 计时器
int Y8_Cnt = 0 ;

// Y8巡线
int Y8_Speed_MAX = 40;

// 菜单调控任务执行
int Car_Task_Num = 0 ;	// 初始为0,也就是没有任务

// MPU6050参数
int Turn_Num_MPU = 0;
int turning_Flag_Bef ;

// *******************实验区域*******************
int check1 ;
int check2 ;
int check[50] ;
int Y8_C = 1 ;
bool go ;
extern int Menu_Open_Mode ;

// *******************任务调度*******************
mytask Motor_Status ;	
void Motor_PID_Check(void) ;	// 电机调节PID测试函数
void Motor_VOFA_Set_Y8(void) ;// Y8寻迹+VOFA监控测试函数

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
		Y8_Line_Init(5.73f , 0.0f , 6.50f , Y8_Speed_MAX , -Y8_Speed_MAX , 1000 ) ;  // 巡线模块初始化
		Menu_Init() ;																																 // 菜单初始化
		taskInit(&Motor_Status , 0 , 10 , Motor_Speed_Update_Entray) ;	// 电机速度更新函数任务调度
		// 全部初始化完毕后再开启Systick中断
		__enable_irq();
		#ifndef USE_HARDWARE_I2C
		Software_I2C_Init();
		#endif
		// MPU6050初始化,必须放在Sys初始化之后
		MPU6050_Init();
		init_adaptive_compensation();
	}
	// ***********调参清单***********
	{
	Key_AddParam("isLeft" , &is_Car_Turn_Left , 1 , PARAM_INT ) ;	// int Car_Task_Num
	Key_AddParam("Turn_Num_MPU" , &Turn_Num_MPU , 0.5 , PARAM_INT ) ;
	}
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
		// 巡线数据更新,以后放在中断中
//		Y8_LineSensor_Update() ;
		// 核心代码:OLED控制小车实现赛题
//		Car_Task(Car_Task_Num) ;
		// **********实验区域**********	
		// 与树莓派融合
		
		//MPU6050模块功能函数
		sensor_data = MPU6050_Data_Update();
		uint32_t current_time = HAL_GetTick();
		
    float dt = (current_time - current_angle.last_time) / 1000.0f; 
    current_angle.last_time = current_time;
		
		
		calculate_angle_from_gyro(sensor_data.Gx_, sensor_data.Gy_, sensor_data.Gz_, dt);
		turning_state_judge(&sensor_data);
    
		// 动态调节Y8限幅
		if (current_angle.yaw >= 15)
		{
			Y8_Speed_MAX = 70 ;
		}
		else
		{
			Y8_Speed_MAX = 40 ;
		}
		// 动态更新Y8的限幅最大值,很重要
		Y8_Line_PID.OutMax = Y8_Speed_MAX ;
		Y8_Line_PID.OutMin = -Y8_Speed_MAX ;
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
		if (Y8_Cnt == 0)
		{
			Car_LR_Speed_Mode = false ;
		}
	}
	// 任务4:等停5s
	if (Car_Wait_cnt > 0)
	{
		Car_Wait_cnt -- ;
		if (Car_Wait_cnt == 1)
		{
			isBreak = 0 ;					// 继续跑
			Car_Wait_Flag = 2 ;		// 等停标志位变为2,表明以后都不会再次识别了
			Y8_Lose_Line_isOK = true ;
		}
	}
	// 任务5:
	Y8_Error_Update() ;
}
