#ifndef __CON_STEPPER_H
#define __CON_STEPPER_H

#include "MySystem.h"
#include "Stepper.h"
#include "math.h"

// 绘制正方形
void Stepper_Draw_Square(float Init_x, float Init_y, float delta_Num, float Square_W) ;
// 偏置计算
void Stepper_Angle_Dot_Update(void) ;
// 输入(x,y)值(以左下角角点为原点)(米)，输出(α,β)角度值(°),a为1维云台(水平旋转),b为2维云台(上下圆周旋转)
void Stepper_XY2AB_Update(float x , float y , Stepper_Typedef* pS1 , Stepper_Typedef* pS2) ;
#endif
