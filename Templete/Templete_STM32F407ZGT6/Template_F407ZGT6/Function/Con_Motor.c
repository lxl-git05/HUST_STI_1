#include "Con_Motor.h"
#include "IMU.h"

Motor_Typedef Motor_A ;
Motor_Typedef Motor_B ;

#define Wheel_C (27.56747553520f)	// 小车的轮子周长(cm)

Motor_Param_Typedef Motor_Param = {13.0f , 28.0f , 300 , Wheel_C} ;	// MG513 * 2, Wheel_Cm=27.57

// 1. 电机初始化
void Con_Motor_Init(void)
{
	// PI
	PID_Init(&Motor_A.PID_s , 8.0f,1.0f,0.0f,1000 , -1000 , 1000) ;
	PID_Init(&Motor_B.PID_s , 8.0f,1.0f,0.0f,1000 , -1000 , 1000) ;

	// PD
	PID_Init(&Motor_A.PID_Angle , 2.29f,0.0f,3.0f,200 , -200 , 350) ;
	PID_Init(&Motor_B.PID_Angle , 2.29f,0.0f,3.0f,200 , -200 , 350) ;

	// PD — 位置环
	PID_Init(&Motor_A.PID_Pos , 20.0f,0.0f,6.0f,250 , -250 , 350) ;
	PID_Init(&Motor_B.PID_Pos , 20.0f,0.0f,6.0f,250 , -250 , 350) ;

	Motor_Init
	(
		&Motor_A , &MyPWM_Motor_A_IN1 , &Motor_A_Encoder ,
		&MyGPIO_Motor_A_IN1 , &MyGPIO_Motor_A_IN2 , &Motor_Param ,
		Motor_DIR_P , Motor_DIR_N ,
		Motor_A.PID_s , Motor_A.PID_Angle , Motor_A.PID_Pos
	);

	Motor_Init
	(
		&Motor_B , &MyPWM_Motor_B_IN1 , &Motor_B_Encoder ,
		&MyGPIO_Motor_B_IN1 , &MyGPIO_Motor_B_IN2 , &Motor_Param ,
		Motor_DIR_N , Motor_DIR_N ,
		Motor_B.PID_s , Motor_B.PID_Angle , Motor_B.PID_Pos
	);

	Motor_SetSpeed(&Motor_A , 0) ;
	Motor_SetSpeed(&Motor_B , 0) ;

	// 初始化车身yaw角度环
	PID_Angle_Init() ;

	// 初始化整车直行位置环
	PID_Car_Straight_Init() ;
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
void Motorx_Angle_Update_Tick(Motor_Typedef *Motor , int Dir)	// Dir: 纠正PID控制方向
{
	// 1. 计算角度
	Motor_Angle_Update(Motor) ;
	// 2. 计算PID
	PID_Update(&Motor->PID_Angle ,Motor->PID_Angle.realPoint_Now) ;
	// 3. 输出电机速度(串行环嵌套！！！)
	Motor_SetSpeed(Motor, Motor->PID_Angle.setPoint * Dir);
}

// 6.3 电机位置环PID,并不需要知道周期,但是仍然需要放在需要周期定时器内
void Motorx_Pos_Update_Tick(Motor_Typedef *Motor , int Dir)	// Dir: 纠正PID控制方向
{
	// 1. 计算位置(cm)
	Motor_Pos_Update(Motor) ;
	// 2. 计算PID
	PID_Update(&Motor->PID_Pos ,Motor->PID_Pos.realPoint_Now) ;
	// 3. 输出电机速度(串行环嵌套！！！)
	Motor_SetSpeed(Motor, Motor->PID_Pos.setPoint * Dir);
}


// 7. 电机状态更新(外部接口)
void Motor_Speed_Update_Tick(uint32_t Gap_Time_ms)
{
	// 位置环
//	Motorx_Pos_Update_Tick(&Motor_A ,  1 , 0) ;
//	Motorx_Pos_Update_Tick(&Motor_B ,  1 , 0) ;

	// 角度环
//	Motorx_Angle_Update_Tick(&Motor_A , 1) ;
//	Motorx_Angle_Update_Tick(&Motor_B , 1) ;

	// 速度环(内环,需要放在最后)
	Motorx_Speed_Update_Tick(&Motor_A ,Gap_Time_ms) ;
	Motorx_Speed_Update_Tick(&Motor_B ,Gap_Time_ms) ;
}

// ================= 电机角度环(其实也是编码器位置环) =================

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

// 10. 检查电机位置（三重检查：状态+速度+角度容差，参考 Stepper_PWM 模式）
bool Motor_Is_Angle(Motor_Typedef *Motor , float Angle , float Tolerance , float Speed_Tol)
{
	// 第1层：速度检查 — 真实速度未归零说明还在运动
	float real_speed = (Motor->PID_s.realPoint_Now > 0) ? Motor->PID_s.realPoint_Now : -Motor->PID_s.realPoint_Now ;
	if (real_speed >= Speed_Tol)
	{
		return false ;
	}
	// 第2层：角度容差检查
	float curr = Motor_Get_Angle(Motor) ;
	float diff = curr - Angle ;
	if (diff < 0) diff = -diff ;
	if (diff < Tolerance)
	{
		return true ;
	}
	return false ;
}

// ================= 电机编码器位置环 =================

// 1. 设置电机目标位移(cm)
void Motor_SetPos(Motor_Typedef *Motor , float Pos)
{
	Motor->PID_Pos.goalPoint = Pos ;
}

// 2. 得到电机当前位移(cm)
float Motor_Get_Pos(Motor_Typedef *Motor)
{
	return Motor->PID_Pos.realPoint_Now ;
}

// 3. 检查电机位置（三重检查：状态+速度+位置容差，参考 Motor_Is_Angle 模式）
bool Motor_Is_Pos(Motor_Typedef *Motor , float Pos , float Tolerance , float Speed_Tol)
{
	// 第1层：速度检查 — 真实速度未归零说明还在运动
	float real_speed = (Motor->PID_s.realPoint_Now > 0) ? Motor->PID_s.realPoint_Now : -Motor->PID_s.realPoint_Now ;
	if (real_speed >= Speed_Tol)
	{
		return false ;
	}
	// 第2层：位置容差检查
	float curr = Motor_Get_Pos(Motor) ;
	float diff = curr - Pos ;
	if (diff < 0) diff = -diff ;
	if (diff < Tolerance)
	{
		return true ;
	}
	return false ;
}

// 4. 清除累计位移
void Motor_Pos_Clear(void)
{
	MyEncoder_Total_Cnt_Clear(Motor_A.Motor_Encoder) ;
	MyEncoder_Total_Cnt_Clear(Motor_B.Motor_Encoder) ;
	Motor_A.PID_Pos.realPoint_Now = 0.0f ;	// 同步清零缓存位置，防止残留值导致IsExit瞬间误判
	Motor_B.PID_Pos.realPoint_Now = 0.0f ;
}

// =================== ICM角度环 ===================

Pid_Typedef PID_Angle ;
static float PID_Angle_StartYaw = 0.0f;  // 任务启动时的yaw基准（相对运动）

// 初始化（PD: Kp=2.47, Kd=7.16, Out±100）
void PID_Angle_Init(void)
{
	PID_Init(&PID_Angle, 2.47f, 0.0f, 7.16f, 100, -100, 1000);
}

// 记录当前yaw为基准 + 清空PID历史（每次旋转任务Setup调用，不再清零MPU_Real.yaw）
void PID_Angle_Reset(void)
{
	PID_Angle_StartYaw = IMU_Yaw_Abs_Get();
	PID_Param_Reset(&PID_Angle);
}

// 配置目标yaw角度（delta为相对基准的增量：+顺时针/-逆时针）
void PID_Angle_Tar_Yaw(float delta)
{
	PID_Angle.goalPoint = PID_Angle_StartYaw + delta;
}

// 获取相对yaw角度（当前值 - 起始基准）
float PID_Angle_Get_Yaw(void)
{
	return IMU_Yaw_Abs_Get() - PID_Angle_StartYaw;
}

// 配置旋转速度:High
void PID_Angle_Speed_High_On(void)
{
	PID_Angle.OutMax = 100 ;
	PID_Angle.OutMin =-100 ;
}

// 配置旋转速度:Low
void PID_Angle_Speed_Low_On(void)
{
	PID_Angle.OutMax = 20 ;
	PID_Angle.OutMin =-20 ;
}

// 20ms Tick: MPU更新→PID计算→差速输出（A反转 B正转 = 顺时针为正）
void PID_Angle_Tick(void)
{
	// 1. 更新MPU数据->这个作为必做任务，所以不需要在这里再调用
	// 2. 获取真实yaw
	PID_Angle.realPoint_Now = IMU_Yaw_Abs_Get();
	// 3. PID计算
	PID_Update(&PID_Angle, PID_Angle.realPoint_Now);
	// 4. 差速控制: A反转 B正转（顺时针为正）
	Motor_SetSpeed(&Motor_A,  PID_Angle.setPoint);
	Motor_SetSpeed(&Motor_B, -PID_Angle.setPoint);
}

// =================== 整车直行位置环（A轮距离 + IMU偏航PD闭环） ===================

Pid_Typedef PID_Car_Straight ;
static float Straight_StartYaw = 0.0f;	// 起始yaw基准
#define STRAIGHT_MIN_SPEED   20.0f		// 最低速度(rpm)，克服静摩擦
#define STRAIGHT_ACCEL_DIST  15.0f		// 加速距离(cm)
#define STRAIGHT_DECEL_DIST  25.0f		// 减速距离(cm)
static float Straight_MaxSpeed = 200.0f;	// 最高巡航速度(rpm)

// 直行偏航角度PD（闭环修正小角度偏差，Kp=5.0 Kd=12.0 Out±80）
Pid_Typedef PID_Straight_Yaw;

// 初始化整车直行控制器（位置PD + 偏航PD）
void PID_Car_Straight_Init(void)
{
	PID_Init(&PID_Car_Straight, 20.0f, 0.0f, 3.0f, 100, -100, 350);
	PID_Init(&PID_Straight_Yaw,   5.0f, 0.0f, 12.0f, 80, -80, 350);
}

// 清零编码器 + 记录起始yaw + 清两个PID历史（每次直行任务Setup调用）
void PID_Car_Straight_Reset(void)
{
	Motor_Pos_Clear();						// 清零两电机编码器 total_cnt
	Straight_StartYaw = IMU_Yaw_Abs_Get();	// 记录起始yaw
	PID_Param_Reset(&PID_Car_Straight);		// 清位置PID历史
	PID_Param_Reset(&PID_Straight_Yaw);		// 清偏航PID历史
	PID_Straight_Yaw.goalPoint = 0.0f;		// 目标: 偏航偏差=0°
}

// 配置最高巡航速度(rpm)，0=使用默认200
void PID_Car_Straight_SetSpeedParams(float max_speed)
{
	if (max_speed > 0.0f)
		Straight_MaxSpeed = max_speed;
}

// 20ms Tick: A轮距离→位置PID→梯形限速 + yaw PD闭环→差速修正
void PID_Car_Straight_Tick(void)
{
	// 1. 更新双电机位置(cm)
	Motor_Pos_Update(&Motor_A) ;
	Motor_Pos_Update(&Motor_B) ;

	// 2. 以A轮距离为前进距离参考
	float dist_A = Motor_Get_Pos(&Motor_A) * ( 1);
	PID_Car_Straight.realPoint_Now = dist_A ;

	// 3. 位置PID → 基础速度(rpm)
	PID_Update(&PID_Car_Straight, PID_Car_Straight.realPoint_Now) ;
	float base_speed = PID_Car_Straight.setPoint ;

	// 3.5 梯形速度包络限幅（加速段+巡航段+减速段）
	float speed_limit = Straight_MaxSpeed;
	float traveled    = PID_Car_Straight.realPoint_Now;
	float remaining   = PID_Car_Straight.goalPoint - traveled;

	// 减速段（仅正常模式，永远模式 goal≤0 则跳过）
	if (PID_Car_Straight.goalPoint > 0.0f && remaining < STRAIGHT_DECEL_DIST && remaining > 0.0f)
	{
		speed_limit = STRAIGHT_MIN_SPEED + (Straight_MaxSpeed - STRAIGHT_MIN_SPEED) * remaining / STRAIGHT_DECEL_DIST;
		if (speed_limit < STRAIGHT_MIN_SPEED) speed_limit = STRAIGHT_MIN_SPEED;
	}
	// 加速段（若与减速段重叠则取更小值）
	if (traveled < STRAIGHT_ACCEL_DIST)
	{
		float accel_limit = STRAIGHT_MIN_SPEED + (Straight_MaxSpeed - STRAIGHT_MIN_SPEED) * traveled / STRAIGHT_ACCEL_DIST;
		if (accel_limit < STRAIGHT_MIN_SPEED) accel_limit = STRAIGHT_MIN_SPEED;
		if (accel_limit < speed_limit) speed_limit = accel_limit;
	}
	// 钳位
	if (base_speed >  speed_limit) base_speed =  speed_limit;
	if (base_speed < -speed_limit) base_speed = -speed_limit;

	// 4. IMU偏航PD闭环（目标=0°, PID_Update: Error=goal-actual, 需取反匹配差速方向）
	float yaw_error = IMU_Yaw_Abs_Get() - Straight_StartYaw ;
	PID_Update(&PID_Straight_Yaw, -yaw_error) ;      // 取反：使setPoint符号 = yaw_error符号
	float yaw_correction = PID_Straight_Yaw.setPoint ;

	// 5. 差速输出（A/B同向基础速度 + 反向偏航修正）
	Motor_SetSpeed(&Motor_A, base_speed - yaw_correction) ;
	Motor_SetSpeed(&Motor_B, base_speed + yaw_correction) ;
}
