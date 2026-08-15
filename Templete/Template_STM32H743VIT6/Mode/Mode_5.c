// Mode_5.c — 单任务测试（只测 TASK_MOTOR_TO，全局任务表）
// K1单击=电机A 转到测试角度一次, K2单击=电机B 转到测试角度一次
// OLED 只显示参数，不显示进度；连续按键任务会排队顺序执行
#include "Mode_5.h"
#include "AllHeader.h"

#define MODE5_TEST_TOL       20      // 容差°

int S_Angle = 0 ;

void Mode_5_Setup(void)
{
    OLED_Clear();
    Con_Task_Init(Control_TaskTable, TASK_COUNT);   // 注册全局任务表
}

void Mode_5_Loop(void)
{
	OLED_Printf(0,0,OLED_6X8,"===Mode_5===") ;
	// 逻辑: 电机
	if (Key_Check(KEY_1, KEY_SINGLE)) Con_Task_Enqueue(TASK_MOTOR_TO, 0, Th_Trans_Step, MODE5_TEST_TOL, 0);
	if (Key_Check(KEY_2, KEY_SINGLE)) Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Down , MODE5_TEST_TOL, 0);
	
	if (Key_Check(KEY_1, KEY_DOUBLE)) Con_Task_Enqueue(TASK_MOTOR_TO, 0, 0 , MODE5_TEST_TOL, 0);
	if (Key_Check(KEY_2, KEY_DOUBLE)) Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Up , MODE5_TEST_TOL, 0);
	
	// 逻辑: 舵机
	// 夹爪闭合
	if (LCD_Key_Check(LCD_KEY_1)) 
	{
		Con_Task_Enqueue(TASK_CLAW_SET, Th_ClawA_Close, Th_ClawB_Close, ROBOT_SERVO_HOLD_CLAW_CLOSE_MS, 0);
	}
	// 夹爪打开
	if (LCD_Key_Check(LCD_KEY_2)) 
	{
		Con_Task_Enqueue(TASK_CLAW_SET, Th_ClawA_Open, Th_ClawB_Open, ROBOT_SERVO_HOLD_CLAW_OPEN_MS, 0);
	}
	// 衣架1打开
	if (LCD_Key_Check(LCD_KEY_3)) 
	{
		Con_Task_Enqueue(TASK_SERVO_SET, ROBOT_SERVO_HANGER_1, Th_Hanger1_Open, ROBOT_SERVO_HOLD_HANGER_MS, 0);
	}
	// 衣架1闭合
	if (LCD_Key_Check(LCD_KEY_4)) 
	{
		Con_Task_Enqueue(TASK_SERVO_SET, ROBOT_SERVO_HANGER_1, Th_Hanger1_Close, ROBOT_SERVO_HOLD_HANGER_MS, 0);
	}
	
	OLED_Printf(0,20, OLED_6X8 ,"S_Angle=%d",S_Angle) ;
	// 开始进行标定
	if (LCD_Set_Int(LCD_PARAM_1 , &S_Angle , 0 , 180))
	{
		Servo_SetAngle(&Servo_5 , S_Angle) ;
	}
	
	// 执行
  Con_Task_Loop();  
}

void Mode_5_Tick(void)
{
}

void Mode_5_Exit(void)
{
}
