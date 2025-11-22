#ifndef __CON_CAR_H
#define __CON_CAR_H

#include "main.h"
#include "stdbool.h"
#include "Timer_Counter.h"
#include "RasPi.h"
#include "Motor.h"
#include "Con_Motor.h"
#include "Con_Track.h"

// 小车方位枚举结构体
typedef enum
{
	
	Car_Cross_Init_way ,			// 小车初始点位面对的一段短直线路程
	
	Car_Other_way      ,			// 小车不在初始位置
	
	Car_Turn_L_In_way ,				// 小车在岔路口的内圈转弯中
	Car_Turn_R_In_way ,				// 小车在岔路口的外圈转弯中
	
	Car_Cross_L_In_way , 			// 小车在内圈直线
	Car_Cross_R_In_way , 			// 小车在外圈直线
	
	Car_Turn_L_Out_way	,			// 小车在内圈转弯岔路出口
	Car_Turn_R_Out_way	,			// 小车在外圈转弯岔路出口
	
	Car_Cross_Oppo_way	,			// 小车在初始点位对面的长直线
	
	Car_Turn_Oppo_way		,			// 小车在初始点斜对面的弯道
	
	Car_Cross_Short_way ,			// 小车在RL线对面的短直道
	
	Car_Turn_Last_way ,				// 小车在最后的弯道
	
	Car_Cross_Last_way  			// 小车在最后的冲刺直道
	
}Car_Position_Typedef ;

// 小车核心处理逻辑

// 电工基地试题
void Car_Task(int Car_Task_Seq) ;

#endif
