#ifndef __CON_MOTOR_H
#define __CON_MOTOR_H

#include "main.h"
#include "usart.h"
#include "Motor.h"
#include "math.h"
#include "Serial.h"
#include <stdio.h>
#include "stdbool.h"

// *电机PID调试模式*下驱动函数
void Motor_Update_Entray_Check(void);	// Mode1:电机PID检查

// *电机PID调试模式*下VOFA调参函数
void Motor_PID_Check(void);						// Mode1:电机PID检查

// *树莓派巡线调节模式*下驱动函数
void Motor_Update_Entray_Pi(void);		// Mode2:树莓派
	
// *树莓派巡线调节模式*下VOFA调参函数
void Motor_Pi_Check(void);						// Mode2:树莓派
#endif


