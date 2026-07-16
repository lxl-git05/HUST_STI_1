#ifndef __ALLHEADER_H
#define __ALLHEADER_H

// 底层驱动库(MySystem)
#include "MySystem.h"

// 工具库 (Tools)
#include "LED_Flash.h"
#include "Timer_Counter.h"

// 硬件驱动库
#include "Key.h"
#include "OLED.h"
#include "Encoder_Key.h"
#include "bsp_at24c02.h"
#include "Serial_porting.h"
#include "RGB.h"
#include "Stepper_PWM.h"
#include "Orange.h"
#include "Buzzer.h"
#include "Elec.h"
#include "TJC_LCD.h"
#include "Motor.h"

// 软件算法库 (Software)
#include "MyPID.h"
#include "at24c02_manager.h"
#include "ParamEdit.h"
#include "Queue.h"
#include "Control.h"

//// 硬件实现库
#include "Con_Stepper.h"
#include "Con_Motor.h"
#include "Param_AT24C02.h"
#include "Con_Task.h"

// Mode库
#include "Mode_G.h"
#include "Mode_1.h"
#include "Mode_2.h"
#include "Mode_3.h"
#include "Mode_4.h"
#include "Mode_5.h"
#include "Mode_6.h"

// Con_Mode库
#include "Con_Mode_1.h"
#include "Con_Mode_2.h"
#include "Con_Mode_3.h"
#include "Con_Mode_4.h"
#include "Con_Mode_5.h"
#include "Con_Mode_6.h"

// Mymain初始化集合,使主函数更简洁
void Initial_ALL(void) ;

// 定时器初始化,必须放在最后初始化,防止开局访问空指针
void Initial_Timer(void) ;

#endif
