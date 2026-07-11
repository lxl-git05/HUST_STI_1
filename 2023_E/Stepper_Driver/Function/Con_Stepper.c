#include "Con_Stepper.h"
#include "OLED.h"
#include "Timer_Counter.h"

// 云台运动:电机1水平旋转(顺时针为正)，电机2竖直旋转(顺时针为正)

// 云台的物理参数
#define Zhou_Height 0.08f	// 激光轴半径为8cm
#define S_Distance 1.0f // 云台整体到平面的距离
// 左下角角点的α和β值,现场实测,脱机调整
float Dot0_Angle_A = -10.0f ;	
float Dot0_Angle_B = -2.0f ;	

// xy坐标变换到以轴心为原点后的偏置值: 也就是真实值在本坐标系需要加入的偏置值
float X_Bias = 0.0f ;	float Y_Bias = 0.0f ;	

// 偏置计算
void Stepper_Angle_Dot_Update(void)
{
	X_Bias = S_Distance * tan(Dot0_Angle_A);
	Y_Bias = (sqrt(X_Bias * X_Bias + S_Distance * S_Distance) * sin(Dot0_Angle_B) - Zhou_Height) / cos(Dot0_Angle_B);
}

// 输入(x,y)值(以左下角角点为原点)(米)，输出(α,β)角度值(°),a为1维云台(水平旋转),b为2维云台(上下圆周旋转)
void Stepper_XY2AB_Update(float x , float y , Stepper_Typedef* pS1 , Stepper_Typedef* pS2)
{
	// x , y -> a , b计算
	x += X_Bias ;
	y += Y_Bias ;
	pS1->Pos_Tar = atan(x / S_Distance);
	pS2->Pos_Tar = atan(y / sqrt(x * x + S_Distance * S_Distance)) + acos(Zhou_Height / sqrt(y * y + x * x + S_Distance * S_Distance)) + 90;
	// 云台角度更新
	Stepper_AbsPos_Enqueue(pS1,200,0,pS1->Pos_Tar) ;
	Stepper_AbsPos_Enqueue(pS2,200,0,pS2->Pos_Tar) ;
//	Stepper_Absolute_Pos_Set(pS1,200,0,x) ;
//	Stepper_Absolute_Pos_Set(pS2,200,0,y) ;
}

// 绘制正方形
void Stepper_Draw_Square(float Init_x, float Init_y, float delta_Num, float Square_W)
{
    float x_target, y_target;
    int steps = Square_W / delta_Num;  
    
    for (int i = 0; i < steps; i++) {
        x_target = Init_x + i * delta_Num ;
        y_target = Init_y ;
        Stepper_XY2AB_Update(x_target, y_target, &Stepper1, &Stepper2); HAL_Delay(20) ;
    }

    for (int i = 0; i < steps; i++) {
        x_target = Init_x + Square_W;
        y_target = Init_y + i * delta_Num;
        Stepper_XY2AB_Update(x_target, y_target, &Stepper1, &Stepper2);  HAL_Delay(20) ;
    }

    for (int i = 0; i < steps; i++) {
        x_target = Init_x + Square_W - i * delta_Num;
        y_target = Init_y + Square_W;
        Stepper_XY2AB_Update(x_target, y_target, &Stepper1, &Stepper2);  HAL_Delay(20) ;
    }

    for (int i = 0; i < steps; i++) {
        x_target = Init_x;
        y_target = Init_y + Square_W - i * delta_Num;
        Stepper_XY2AB_Update(x_target, y_target, &Stepper1, &Stepper2);  HAL_Delay(20) ;
    }
}
	


