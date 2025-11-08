#include "Mymain.h"

// *******************调试模式*******************
//#define PID_Check			// 调试电机PID
//#define PI_Line_Mode	// 树莓派视觉巡线模式
#define Y8_Line_Mode	// 8度寻迹巡线模式
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

// 小车状态机编写
int Car_Flag ;

// *******************实验区域*******************
int check1 ;
int check2 ;
int check[50] ;
extern float Y8_Line_C ;
extern Pid_Typedef Y8_Line_PID ;

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
		Y8_Line_Init(0.0f , 0.0f , 0.0f , 30 , -30 , 1000 ) ;															// 巡线模块初始化
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
	#ifdef Y8_Line_Mode	// 8度寻迹巡线模式
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
		#ifdef Y8_Line_Mode		// 8度寻迹巡线模式
		Motor_VOFA_Set_Y8() ;
		Y8_LineSensor_Update() ;
		#endif
		// 树莓派数据更新+亮灯调节
		RasPi_Data_Update() ;
		// 树莓派停止和驱动指令
		RasPi_Func() ;
		// 菜单执行功能
		Menu_Func() ;
		// **********实验区域**********
		
	}
}
void Motor_Update_Entray_Y8(void)	// Mode1:Y8寻迹
{
	// 对电机B进行Kp限制
	if (fabs(Motor_B.PID_s.PreError) < 5)
	{
		Motor_B.PID_s.Kp = 0.04f * fabs(Motor_B.PID_s.PreError) ;
	}
	else if (fabs(Motor_B.PID_s.PreError) >= 5 && fabs(Motor_B.PID_s.PreError) < 20)
	{
		Motor_B.PID_s.Kp = 0.03f * fabs(Motor_B.PID_s.PreError) + 0.05f ;
	}
	else
	{
		Motor_B.PID_s.Kp = 0.6f ;
	}
	// 刹车判断
	if (isBreak)
	{
		Motor_A.GoalSpeed = 0 ;
		Motor_B.GoalSpeed = 0 ;
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
	printf("%f,%f,%f\n", Y8_Line_PID.goalPoint , Y8_Line_PID.realPoint_Now , Y8_Line_PID.setPoint ) ;
}

void Motor_Update_Entray_Pi(void)			// Mode2:树莓派
{
	// 对电机B进行Kp限制
	if (fabs(Motor_B.PID_s.PreError) < 5)
	{
		Motor_B.PID_s.Kp = 0.04f * fabs(Motor_B.PID_s.PreError) ;
	}
	else if (fabs(Motor_B.PID_s.PreError) >= 5 && fabs(Motor_B.PID_s.PreError) < 20)
	{
		Motor_B.PID_s.Kp = 0.03f * fabs(Motor_B.PID_s.PreError) + 0.05f ;
	}
	else
	{
		Motor_B.PID_s.Kp = 0.6f ;
	}
	// 树莓派Line_PID更新
	PID_Line.realPoint_Now = Pi_xLine_real ;
	PID_Line.goalPoint = Pi_xLine_goal ;
	// 树莓派Line_PID计算
	PID_Update(&PID_Line , PID_Line.realPoint_Now) ;
	// 目标速度控制
	if (isBreak)
	{
		Motor_A.GoalSpeed = 0 ;
		Motor_B.GoalSpeed = 0 ;
	}
	else
	{
		// 双环控制:外环x轴环PID,内环速度环PID
		Motor_A.GoalSpeed = goalPointTwo - PID_Line.setPoint ;
		Motor_B.GoalSpeed = goalPointTwo + PID_Line.setPoint ;
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

void Motor_Pi_Check(void)							// Mode2:树莓派
{
	if (Serial_GetNewPackageFlag_ABC() == 1)
	{
		// 文本包调试程序
		
		Serial_SetIntData("xLine_goal" , "xLine_goal=%d" , &Pi_xLine_goal) ;
		Serial_SetIntData("xLine_real" , "xLine_real=%d" , &Pi_xLine_real) ;
		
		// 测试
		Serial_SetIntData("Pi_Speed_Max" , "Pi_Speed_Max=%d" , &Pi_Speed_Max) ;
		PID_Line.OutMax = Pi_Speed_Max ;
		PID_Line.OutMin = -Pi_Speed_Max;
		
		Serial_SetFloatData("KpC" , "KpC=%f" , &PID_Line.Kp) ;
		Serial_SetFloatData("KiC" , "KiC=%f" , &PID_Line.Ki) ;
		Serial_SetFloatData("KdC" , "KdC=%f" , &PID_Line.Kd) ;
		
		// 两个轮子调试
		// 刹车
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
		// 一起跑
		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPointTwo))
		{
			goalPoint_A = goalPointTwo ;
			goalPoint_B = goalPointTwo ;
		}
		
	}
	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
	// VOFA展示电机状态
//	printf("%d,%d,%f,%d,%d,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , PID_Line.goalPoint,Motor_B.GoalSpeed , Motor_B.RealSpeed , PID_Line.realPoint_Now);
	printf("%f,%f,%f,%f,%f\n",PID_Line.goalPoint , PID_Line.realPoint_Now ,PID_Line.setPoint ,PID_Line.pout,PID_Line.dout );
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
}

void Motor_Update_Entray_Check(void)	// Mode1:电机PID检查
{
	// 对电机B进行Kp限制
	if (fabs(Motor_B.PID_s.PreError) < 5)
	{
		Motor_B.PID_s.Kp = 0.04f * fabs(Motor_B.PID_s.PreError) ;
	}
	else if (fabs(Motor_B.PID_s.PreError) >= 5 && fabs(Motor_B.PID_s.PreError) < 20)
	{
		Motor_B.PID_s.Kp = 0.03f * fabs(Motor_B.PID_s.PreError) + 0.05f ;
	}
	else
	{
		Motor_B.PID_s.Kp = 0.6f ;
	}
	Motor_Speed_Update(&Motor_A) ;			// 编码器测速
	Motor_PID_Update(&Motor_A) ;				// PID更新
	
	Motor_Speed_Update(&Motor_B) ;			// 编码器测速
	Motor_PID_Update(&Motor_B) ;				// PID更新
}

void Motor_PID_Check(void)						// Mode1:电机PID检查
{
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
	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
	// VOFA展示PID调参
	// 单独展示
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_A.PID_s.pout,Motor_A.PID_s.iout,Motor_A.PID_s.dout);
		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
	// 联调
//	printf("%d,%d,%d,%d,%d,%d\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed);

	// 电机目标速度和输出速度更新
	Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;
	
	Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;
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

