#include "Mymain.h"

// *******************全局变量*******************

// 数据包
extern Serial_ABC_Data_Typedef   Serial_ABC_Data ;			// 解析好的ABC数据包

extern Serial3_HEX_Data_Typedef   Serial3_Hex_Data ;			// 解析好的HEX数据包

// 电机
extern Motor_Typedef Motor_A ;	// 电机A
extern Motor_Typedef Motor_B ;	// 电机B

int goalPoint_A ;	// 电机目标转速
int goalPoint_B ;	// 电机目标转速
int goalPointTwo;	// 共同速度
bool isBreak = true;			// 刹车判断
// 实验
float C = 1.0f ;	// x_real倍增系数

// 树莓派视觉传感器
// 巡线
Pid_Typedef PID_Line ;			// 树莓派巡线PID

int Pi_xLine_goal = 160;		// x 的目标值
int Pi_xLine_real = 160;		// x 的真实值,数据量 x_real + 100
int Pi_task1	;							// 识别到停止即为1,否则为0,2是等5s
int Pi_angle ;							// angle + 100:偏转角度
int Pi_Speed_Max = 10 ;			// 速度环最大差值
int Pi_Wait_Flag = 0 ;			// 等停标示标志位,0:等待停止标志位中 , 1:识别到等停 2:注销等停模式

// *******************实验区域*******************
int check1 ;
int check2 ;
int check[50] ;
float time_us ;

// *******************任务调度*******************
// 任务1:电机状态更新
mytask Motor_Status ;	
void Motor_Update_Entray(void) ;
void Motor_Update_Line_Entray(void) ;
void Motor_PID_Check(void) ;	// 电机调节PID测试函数
void Motor_Pi_Check(void) ;		// 电机与树莓派联调函数(模拟)
// 任务2:

void Mymain(void)
{
	// ***********初始化***********
	HAL_SYSTICK_Config(SystemCoreClock / 1000);	// 启动Systick时钟
	OLED_Init() ;																// 初始化OLED
	Serial_Init(&Serial_huart) ;								// 串口_初始化
	Serial3_Init(&Serial3_huart) ;							// 串口_树莓派初始化
	Motor_A_Init();															// 电机A初始化
	Motor_B_Init();															// 电机B初始化
	PID_Init(&PID_Line , 0.3f , 0.0f , 0.0f , Pi_Speed_Max , -Pi_Speed_Max , 1000) ;	// 树莓派巡线初始化
	PID_Line.d_style = 1.0f ;
	// *********实验********
	// 初始化 DWT 计时器
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;     // 清零
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	
//	uint32_t start = DWT->CYCCNT;
//	// func-begin
//	
//	// func-end
//	uint32_t end = DWT->CYCCNT;
//	uint32_t cycles = end - start;
//	time_us = (float)cycles / (SystemCoreClock / 1000000.0f);
	
	// 全部初始化完毕后再开启中断
	__enable_irq();
	
	// ***********任务调度清单***********
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray) ;	// 任务1:电机状态更新
//	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Line_Entray) ;	// 任务1:电机状态更新
	
	while (1)
	{
		if(Key_Check(0 , KEY_SINGLE) == 1)
		{
			HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
			goalPointTwo = -60 ;
			goalPoint_A = goalPointTwo ;
			goalPoint_B = goalPointTwo ;
			isBreak = false ;
		}
		// **********实验区域**********
		Motor_PID_Check() ;		// 调节电机PID
//		Motor_Pi_Check() ;		// 联合巡线机制调节巡线PID
		
		// 树莓派数据更新
		if (Serial3_GetNewPackageFlag_HEX() == 1)
		{
			// Serial3_New_Package:// 3个 : x_real , task1 , angle
			Pi_xLine_real = Serial3_Hex_Data.Serial3_New_Package[1] - 100 ;	
			Pi_task1 = Serial3_Hex_Data.Serial3_New_Package[2] ;	
			Pi_angle = Serial3_Hex_Data.Serial3_New_Package[3] - 100 ;
		}
		OLED_Printf(0 , 0 ,OLED_8X16 , "x_Line_real:%d", Pi_xLine_real ) ;

		// 必须存在:OLED更新
		OLED_Update() ;
	}
}

void Motor_Update_Line_Entray(void)
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
	Motor_Speed_Update(&Motor_A) ;			// 编码器测速
	Motor_PID_Update(&Motor_A) ;				// PID更新
	
	Motor_Speed_Update(&Motor_B) ;			// 编码器测速
	Motor_PID_Update(&Motor_B) ;				// PID更新
}

void Motor_Pi_Check(void)
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
		
		Serial_SetFloatData("C" , "C=%f" , &C) ;
		
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

	// 电机目标速度和输出速度更新
	Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;
	
	Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;
}

void Motor_Update_Entray(void)
{
//	// 计时
//	static uint32_t last = 0;
//	uint32_t now = DWT->CYCCNT;
//	time_us = (now - last) / (SystemCoreClock / 1000000.0f);
//	last = now;

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

void Motor_PID_Check(void)
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
	// OLED展示
	OLED_Printf(0 , 0 , OLED_8X16 , "Asrg:%d %d %d" ,Motor_A.SetSpeed, Motor_A.RealSpeed, Motor_A.GoalSpeed ) ;
	OLED_Printf(0 ,15 , OLED_8X16 , "A:%.2f,%.2f,%.2f",Motor_A.PID_s.Kp,Motor_A.PID_s.Ki,Motor_A.PID_s.Kd) ;
	
	OLED_Printf(0 ,30 , OLED_8X16 , "Bsrg:%d %d %d" ,Motor_B.SetSpeed, Motor_B.RealSpeed, Motor_B.GoalSpeed ) ;
	OLED_Printf(0 ,45 , OLED_8X16 , "B:%.2f,%.2f,%.2f",Motor_B.PID_s.Kp,Motor_B.PID_s.Ki,Motor_B.PID_s.Kd) ;
	
	
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
}

// 任务初始化(setup)
void taskInit(mytask* task,uint32_t cnt_init,uint32_t cycle_init , void (*callback_func)(void) )  
{
	task->Flag=0;							
	task->cnt=cnt_init;				// 计数器
	task->cycle=cycle_init;		// 计数时长(周期)
	task->Enable=1;						// 任务启动标志位,初始化之后就打开
	task->callback = callback_func;  // 注册任务函数
}

// 任务周期函数(放在定时器)
void task_possess(mytask* task)
{
	// 任务一旦启动开始进行process判断
	if(task->Enable == 1)
	{
		task->cnt++;
		if(task->cnt >= task->cycle)
		{
			task->cnt = 0;
			task->Flag = 1;
			// 自动调用任务回调函数（若存在）
			if(task->callback != NULL)
			{
					task->callback();
					task->Flag = 0;  // 任务执行后自动清零
			}
		}
	}
}
