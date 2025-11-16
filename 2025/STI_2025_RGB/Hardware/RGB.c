#include "RGB.h"
// **************RGB高级参数**************

// RGB相关参数
#define RGB_htim htim2
#define RGB_R_Channel TIM_CHANNEL_1
#define RGB_G_Channel TIM_CHANNEL_2
#define RGB_B_Channel TIM_CHANNEL_3

// RGB颜色码
typedef enum
{
	RGB_DOWN = 0x00U,			// 闭灯
	RGB_R		 = 0x01U,			// 红色
	RGB_Y 	 = 0x02U,			// 黄色
	RGB_G 	 = 0x03U,			// 绿色
	RGB_B 	 = 0x04U, 		// 蓝色
}RGB_Color_Num ;

// **************RGB参数**************

mytask RGB_Auto_Task ;	// RGB自动档任务
int RGB_Manu_Num = 0 ;	// RGB手动挡颜色码

// RGB自动档任务处理函数
void RGB_Auto_Task_Entry(void) ;

// RGB初始化
void RGB_Init(void)
{
	// 初始化
	PWM_Init(&RGB_htim , RGB_R_Channel ) ;
	PWM_Init(&RGB_htim , RGB_G_Channel ) ;
	PWM_Init(&RGB_htim , RGB_B_Channel ) ;
	// 初始为不发光
	PWM_SetCompare1(RGB_htim , RGB_R_Channel , 100 ) ;
	PWM_SetCompare1(RGB_htim , RGB_G_Channel , 100 ) ;
	PWM_SetCompare1(RGB_htim , RGB_B_Channel , 100 ) ;
	// RGB自动档初始化
	taskInit(&RGB_Auto_Task, 0 , 3000 , RGB_Auto_Task_Entry) ;
	RGB_Auto_Task.Enable = 0 ;	// 先关掉
}

// RGB调色,由于是共阳极,所以100-x
void RGB_Set_Color(int R_Color , int G_Color , int B_Color )
{
	// 限幅
	if (R_Color > 100)
		R_Color = 100 ;
	else if (R_Color < 0)
		R_Color = 0 ;
	if (G_Color > 100)
		G_Color = 100 ;
	else if (G_Color < 0)
		G_Color = 0 ;
	if (B_Color > 100)
		B_Color = 100 ;
	else if (B_Color < 0)
		B_Color = 0 ;
	
	// 调色
	PWM_SetCompare1(RGB_htim , RGB_R_Channel , 100-R_Color ) ;
	PWM_SetCompare1(RGB_htim , RGB_G_Channel , 100-G_Color ) ;
	PWM_SetCompare1(RGB_htim , RGB_B_Channel , 100-B_Color ) ;
}

// RGB闪烁任务,Mode为1代表自动挡,Mode为2代表手动挡
//1. RGB灯:(可开关)
//1.1 自动档: 每个周期3s, 红灯3s - 黄灯3s - 绿灯3s
//1.2 手动挡: 可选择RGB颜色
void RGB_Control(bool Mode)
{
	// 自动档
	if (Mode == 1)
	{
		RGB_Auto_Task.Enable = 1 ;	// 打开自动化函数
	}
	// 手动档
	else
	{
		RGB_Auto_Task.Enable = 0 ;	// 关闭自动化函数
		// 手动改变RGB颜色
		if (RGB_Manu_Num == RGB_DOWN)				// 关闭
		{
			RGB_Set_Color( 0 , 0 , 0 ) ;	
		}
		if (RGB_Manu_Num == RGB_R)						// 红色
		{
			RGB_Set_Color( 100 , 0 , 0 ) ;
		}
		else if (RGB_Manu_Num == RGB_G)		  	// 绿色
		{
			RGB_Set_Color( 0 , 100 , 0 ) ;
		}
		else if (RGB_Manu_Num == RGB_Y)		  	// 黄色
		{
			RGB_Set_Color( 75 , 100 , 0 ) ;
		}
	}
}

// 自动档Task执行函数
void RGB_Auto_Task_Entry(void)
{
	// 亮灯顺序
	static int RGB_Seq = 1 ;
	// 亮红色
	if (RGB_Seq == RGB_R)
	{
		RGB_Set_Color( 100 , 0 , 0 ) ;
	}
	// 亮黄色
	else if (RGB_Seq == RGB_Y)
	{
		RGB_Set_Color( 75 , 100 , 0 ) ;
	}
	// 亮绿色
	else if (RGB_Seq == RGB_G)
	{
		RGB_Set_Color( 0 , 100 , 0 ) ;
	}
	// 亮灯序列
	RGB_Seq ++ ;
	if (RGB_Seq >= 4)
	{
		RGB_Seq = 1 ;
	}
}

// RGB自动档执行任务
void RGB_Auto_Task__Possess(void)
{
	task_possess(&RGB_Auto_Task) ;
}
