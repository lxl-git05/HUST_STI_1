#ifndef __CONTROL_H
#define __CONTROL_H

#include "MySystem.h"

// 状态转换配置
#define Cmd_Wait_Start 0	// 初始化完成，等待开始
#define Cmd_Robot_1_Go 1	// 第1轮开始
#define Cmd_Robot_1_OK 2	// 第1轮结束
#define Cmd_Robot_2_Go 3	// 第2轮开始
#define Cmd_Robot_2_OK 4	// 第2轮结束

typedef enum
{
	Robot_Init ,					// 丝杆在顶端, 夹子张开,传送带位置与衣架对应,衣架闭合
	Robot_Down ,					// 电机向下够衣服
	Robot_Claw_Close , 		// 夹衣服
	Robot_Mid ,						// 电机上升到中位线,
	Robot_Hanger1_Open ,	// 衣架张开
	Robot_Claw_Open  , 		// 夹爪松开(张开)
	Robot_Up ,						// 电机重新上升到顶点
	Robot_SiGan_Next,			// 传送带移动，整个过程完成一轮
	Robot_OK_1,						// 第1轮完成
	
	Robot_Hanger2_Open,		// 衣架2张开
	Robot_OK_2 ,					// 第2轮完成
}Hanger_Status_Typedef ;

// 队列填充
void Control_Setup(void) ;

// 控制台
void Hanger_Control(void) ;

// 状态转换配置
void Hanger_Control_Change(int *Control_Cmd) ;

#endif
