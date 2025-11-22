#ifndef __CON_MOTOR_H
#define __CON_MOTOR_H

#include "main.h"
#include "usart.h"
#include "Motor.h"
#include "math.h"
#include "Serial.h"
#include <stdio.h>
#include "stdbool.h"

// 外部变量声明
extern int goalPoint_A ;					// 电机目标转速
extern int goalPoint_B ;					// 电机目标转速
extern bool isBreak ;							// 刹车判断
extern int goalPointTwo;					// 共同速度

// 函数声明

// 电机速度更新函数,放在任务调度
void Motor_Speed_Update_Entray(void);	

// *电机PID调试模式*下VOFA调参函数,放在while
void Motor_PID_Check(void);						// 电机PID检查

// Y8寻迹下VOFA调参函数,放在while
void Motor_VOFA_Set_Y8(void) ;				// Y8寻迹


/*
注释:
	放弃树莓派主控巡线机制 , 所以注释掉相关功能	
	树莓派在巡线方面只进行等停和停止功能
	树莓派在RGB识别和LR识别起主控作用


//// *树莓派巡线调节模式*下驱动函数
//void Motor_Update_Entray_Pi(void);		// Mode2:树莓派
//	
//// *树莓派巡线调节模式*下VOFA调参函数
//void Motor_Pi_Check(void);						// Mode2:树莓派

*/

#endif


