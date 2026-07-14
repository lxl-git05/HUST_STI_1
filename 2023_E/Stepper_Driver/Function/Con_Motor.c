#include "Con_Motor.h"

Motor_Typedef Motor_A ;	// 水平传送带结构,正方向逆时针，也就是所需要的正方向
Motor_Typedef Motor_B ;	// 丝杆升降结构,正方向向下

Motor_Param_Typedef Motor_Param = {13.0f , 34.0f , 300} ;	// MG370 * 2
// 1. 电机初始化
void Con_Motor_Init(void)
{
	// PI
	PID_Init(&Motor_A.PID_s , 8.0f,0.8f,0.0f,1000 , -1000 , 1000) ;
	PID_Init(&Motor_B.PID_s , 8.0f,0.8f,0.0f,1000 , -1000 , 1000) ;
	
	// PD
	PID_Init(&Motor_A.PID_Angle , 0.9f,0.0f,1.0f,30  ,  -30 , 350) ;
	PID_Init(&Motor_B.PID_Angle , 0.9f,0.0f,1.0f,300 , -300 , 350) ;
	
	Motor_Init
	(
		&Motor_A , &MyPWM_Motor_A_IN1 , &Motor_A_Encoder ,
		&MyGPIO_Motor_A_IN1 , &MyGPIO_Motor_A_IN2 , &Motor_Param , 
		Motor_DIR_N , Motor_DIR_P , 
		Motor_A.PID_s , Motor_A.PID_Angle
	);
	
	Motor_Init
	(
		&Motor_B , &MyPWM_Motor_B_IN1 , &Motor_B_Encoder ,
		&MyGPIO_Motor_B_IN1 , &MyGPIO_Motor_B_IN2 , &Motor_Param , 
		Motor_DIR_P , Motor_DIR_N , 
		Motor_B.PID_s , Motor_B.PID_Angle
	);
	
	Motor_SetSpeed(&Motor_A , 0) ;
	Motor_SetSpeed(&Motor_B , 0) ;
}

// 2. 设置电机goal速度
void Motor_SetSpeed(Motor_Typedef *Motor, float speed)
{
    if (speed >= Motor->Motor_Param->Motor_Max_Speed)
    {
        speed = Motor->Motor_Param->Motor_Max_Speed ;
    }
    else if (speed < -Motor->Motor_Param->Motor_Max_Speed)
    {
        speed = -Motor->Motor_Param->Motor_Max_Speed ;
    }
    Motor->PID_s.goalPoint = speed ;
    Motor->State = MOTOR_RUN;
}

// 3. 得到电机goal速度
int Motor_Get_GoalSpeed(Motor_Typedef *Motor)
{
    return Motor->PID_s.goalPoint ;
}

// 4. 电机停止
void Motor_Stop(Motor_Typedef *Motor)
{
    Motor->State = MOTOR_STOP;
}

// 5. 电机急刹
void Motor_Brake(Motor_Typedef *Motor)
{
    Motor->PID_s.goalPoint = 0;
    Motor->PID_s.setPoint = 0;
    Motor_SetPWM(Motor, 0);
    
    Motor->State = MOTOR_BRAKE;
}
// 6.1 电机速度更新(内部使用)
static void Motorx_Speed_Update_Tick(Motor_Typedef *Motor , uint32_t Gap_Time_ms)
{
    // 1. 计算真实速度（编码器）
    Motor_Speed_Update(Motor , Gap_Time_ms) ;

    // 2. 状态机控制
    switch (Motor->State)
    {
        case MOTOR_STOP:    // 停车
            Motor->PID_s.goalPoint = 0;
            break;

        case MOTOR_RUN:     // 行进
            break;

        case MOTOR_BRAKE:   // 刹车
            Motor_SetPWM(Motor, 0);
            return;
    }

    // 3. PID计算
    PID_Update(&Motor->PID_s , Motor->PID_s.realPoint_Now) ;

    // 4. 输出PWM
    Motor_SetPWM(Motor, Motor->PID_s.setPoint);
}

// 6.2 电机角度环PID,并不需要知道周期,但是仍然需要放在需要周期定时器内
static void Motorx_Angle_Update_Tick(Motor_Typedef *Motor , int Dir)	// Dir: 纠正PID控制方向
{
	// 1. 计算角度
	Motor_Angle_Update(Motor) ;
	// 2. 计算PID
	PID_Update(&Motor->PID_Angle ,Motor->PID_Angle.realPoint_Now) ;
	// 3. 输出电机速度(串行环嵌套！！！)
	Motor_SetSpeed(Motor, Motor->PID_Angle.setPoint * Dir);
}

// 7. 电机状态更新(外部接口)
void Motor_Speed_Update_Tick(uint32_t Gap_Time_ms)
{
	// 速度环(内环)
	Motorx_Speed_Update_Tick(&Motor_A ,Gap_Time_ms) ;
	Motorx_Speed_Update_Tick(&Motor_B ,Gap_Time_ms) ;
	// 角度环
	Motorx_Angle_Update_Tick(&Motor_A ,  1) ;	// 使能A的角度环,那么A就不再被允许倍主动设置速度
	Motorx_Angle_Update_Tick(&Motor_B ,  1) ; 
}	

// 8. 设置电机旋转角度
void Motor_SetAngle(Motor_Typedef *Motor , int Angle)
{
	Motor->PID_Angle.goalPoint = Angle ;
}

// 9. 得到电机当前位置
float Motor_Get_Angle(Motor_Typedef *Motor)
{
	return Motor->PID_Angle.realPoint_Now ;
}

// 10. 检查电机位置
bool Motor_Is_Angle(Motor_Typedef *Motor , int Angle , int Tolerance)
{
	float curr = Motor_Get_Angle(Motor) ;
	if (curr - Angle > -Tolerance && curr - Angle < Tolerance)
	{
		return true ;
	}
	return false ;
}

// ==================== 电机驱动 ====================

void Motor_Hang_Up(void)
{
	Motor_SetAngle(&Motor_B , Motor_Hang_Up_Cnt) ;
}

void Motor_Hang_Mid(void)
{
	Motor_SetAngle(&Motor_B , Motor_Hang_Mid_Cnt) ;
}


void Motor_Hang_Down(void)
{
	Motor_SetAngle(&Motor_B , Motor_Hang_Down_Cnt) ;
}

bool IS_Motor_Hang_Up(void)
{
	return Motor_Is_Angle(&Motor_B , Motor_Hang_Up_Cnt , 20) ;
}

bool Is_Motor_Hang_Mid(void)
{
	return Motor_Is_Angle(&Motor_B , Motor_Hang_Mid_Cnt , 20) ;
}

bool Is_Motor_Hanger_Down(void)
{
	return Motor_Is_Angle(&Motor_B , Motor_Hang_Down_Cnt , 20) ;
}


int Motor_Hua_Pos = 0 ;

void Motor_Hua_Next(void)
{
	Motor_Hua_Pos += 330 ;
	Motor_SetAngle(&Motor_A , Motor_Hua_Pos) ;
}

void Motor_Hua_Back(void)
{
	Motor_Hua_Pos -= 330 ;
	Motor_SetAngle(&Motor_A , Motor_Hua_Pos) ;
}

bool Motor_Hua_is_Ok(void)
{
	return Motor_Is_Angle(&Motor_A, Motor_Hua_Pos, 20) ;
}

