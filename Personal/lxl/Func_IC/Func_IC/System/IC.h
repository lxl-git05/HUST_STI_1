#ifndef __IC_H
#define __IC_H

#include "main.h"
#include "tim.h"

#define IC_TIM 	TIM2
#define IC_htim htim2

extern int CapturePeriod ;


void IC_Init(void) ;
void IC_Capture_Update(void) ;

#endif


