#include "Servo.h"
// **************舵机全局参数***************

// 舵机相关宏定义
#define Servo_htim 	  htim3
#define Servo_Channel TIM_CHANNEL_3

// 舵机方向码
typedef enum
{
	Servo_DOWN 		 = 0x00U,			// 不动
	Servo_Left		 = 0x01U,			// 指示牌L
	Servo_Right 	 = 0x02U			// 指示牌R
}RGB_Color_Num ;

// **************舵机参数**************

mytask Servo_Auto_Task ;					// 舵机自动档任务
int Servo_Manu_Num = 0 ;					// 舵机手动挡方向
int Servo_Left_Position	 = 0	  ; // L舵机方向
int Servo_Right_Position = 180	; // R舵机方向

// 舵机自动档任务处理函数Entry
void Servo_Auto_Task_Entry(void);

// 舵机初始化
void Servo_Init(void)
{
	// PWM初始化
	PWM_Init(&Servo_htim , Servo_Channel ) ;
	// 舵机自动档初始化
	taskInit(&Servo_Auto_Task, 0 , 5000 , Servo_Auto_Task_Entry) ;
	Servo_Auto_Task.Enable = 0 ;	// 先关掉
}

// 舵机调节角度:0度-180度
void Servo_Set_Angle(int Angle)
{
	// 舵机角度限幅
	if (Angle < 0)
		Angle = 0 ;
	else if (Angle > 180)
		Angle = 180 ;
	// 舵机角度调整
	PWM_SetCompare1( Servo_htim , Servo_Channel , 500 + (Angle * 2000 / 180) ) ;
}

// 舵机转动任务,Mode为1代表自动挡,Mode为0代表手动挡
//2.1 自动档: 每个周期5s, R 5s - L 5s
//2.2 手动档: 可任意选择指示牌的方向: R / L
void Servo_Control(bool Servo_Mode)
{
	// 自动档
	if (Servo_Mode == 1)
	{
		Servo_Auto_Task.Enable = 1 ;	// 打开自动化函数
	}
	// 手动档
	else
	{
		Servo_Auto_Task.Enable = 0 ;	// 关闭自动化函数
		if (Servo_Manu_Num == Servo_Left)
		{
			Servo_Set_Angle(Servo_Left_Position) ;
		}
		else if (Servo_Manu_Num == Servo_Right)
		{
			Servo_Set_Angle(Servo_Right_Position) ;
		}
	}
}

// 自动档Task执行函数
void Servo_Auto_Task_Entry(void)
{
	// 舵机方向参数
	static int Servo_Auto_Position = 1;	
	// 舵机方向自动转
	if (Servo_Auto_Position == Servo_Left)
	{
		Servo_Set_Angle(Servo_Left_Position) ;
	}
	else if (Servo_Auto_Position == Servo_Right)
	{
		Servo_Set_Angle(Servo_Right_Position) ;
	}
	// 舵机方向改变
	Servo_Auto_Position ++ ;
	if (Servo_Auto_Position >= 3)
	{
		Servo_Auto_Position = 1 ;
	}
}

// 舵机自动档执行任务
void Servo_Auto_Task__Possess(void)
{
	task_possess(&Servo_Auto_Task) ;
}
