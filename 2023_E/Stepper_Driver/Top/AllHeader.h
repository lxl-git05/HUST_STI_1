#ifndef __ALLHEADER_H
#define __ALLHEADER_H

// 底层驱动库(MySystem)
#include "MySystem.h"

// 工具库
#include "LED_Flash.h"
#include "Timer_Counter.h"

// 硬件驱动库
#include "Key.h"
#include "OLED.h"


// 软件算法库

// 硬件实现库
#include "Con_Motor.h"
#include "Con_Servo.h"
#include "Con_Stepper.h"

// Mode库
#include "Mode_G.h"
#include "Mode_1.h"
#include "Mode_2.h"
#include "Mode_3.h"
#include "Mode_4.h"

// Mymain初始化集合,使主函数更简洁
void Initial_ALL(void) ;

// 定时器初始化,必须放在最后初始化,防止开局访问空指针
void Initial_Timer(void) ;

#endif
