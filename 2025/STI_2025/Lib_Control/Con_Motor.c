#include "Con_Motor.h"

// 电机变量
extern int goalPoint_A ;					// 电机目标转速
extern int goalPoint_B ;					// 电机目标转速
extern bool isBreak ;							// 刹车判断
extern int goalPointTwo;					// 共同速度
// 树莓派视觉传感器
extern Pid_Typedef PID_Line ;							// 树莓派巡线PID
extern int Pi_xLine_goal ;					// x 的目标值
extern int Pi_xLine_real ;					// x 的真实值,数据量 x_real + 100
extern int Pi_Speed_Max  ;
// 内部变量


// *************函数*************

// *电机PID调试模式*下驱动函数
void Motor_Update_Entray_Check(void)	// Mode1:电机PID检查
{
	Motor_Speed_Update(&Motor_A) ;			// 编码器测速
	Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;
	Motor_PID_Update(&Motor_A) ;				// PID更新
	
	Motor_Speed_Update(&Motor_B) ;			// 编码器测速
	Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;
	Motor_PID_Update(&Motor_B) ;				// PID更新
	
	// 电机目标速度和输出速度更新
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;
}

// *电机PID调试模式*下VOFA调参函数
void Motor_PID_Check(void)						// Mode1:电机PID检查
{
	// 逻辑:电脑通过VOFA发送数据包,STM32通过串口1接受指令,然后进行相应的操作,如下:
	if (Serial_GetNewPackageFlag_ABC() == 1)
	{
		// 文本包调试程序
		Serial_SetIntData("goalPoint_A" , "goalPoint_A=%d" , &goalPoint_A) ;
		Serial_SetIntData("goalPoint_B" , "goalPoint_B=%d" , &goalPoint_B) ;
		
		Serial_SetFloatData("KpA" , "KpA=%f" , &Motor_A.PID_s.Kp) ;
		Serial_SetFloatData("KiA" , "KiA=%f" , &Motor_A.PID_s.Ki) ;
		Serial_SetFloatData("KdA" , "KdA=%f" , &Motor_A.PID_s.Kd) ;
		
		Serial_SetFloatData("KpB" , "KpB=%f" , &Motor_B.PID_s.Kp) ;
		Serial_SetFloatData("KiB" , "KiB=%f" , &Motor_B.PID_s.Ki) ;
		Serial_SetFloatData("KdB" , "KdB=%f" , &Motor_B.PID_s.Kd) ;
		
		// 两个轮子调试
		// 刹车
		if ( Serial_SetIntData("break" , "break=%d" , &goalPoint_A) )
		{
			goalPoint_A = 0 ;
			goalPoint_B = 0 ;
		}
		// 一起跑
		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPoint_A))
		{
			goalPoint_B = goalPoint_A ;
		}
	}
	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
	// VOFA展示PID调参
	// 单独展示
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_A.PID_s.pout,Motor_A.PID_s.iout,Motor_A.PID_s.dout);
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
	// 联调
	printf("%d,%d,%d,%d,%d,%d\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , Motor_A.SetSpeed,Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed);
}


// *树莓派巡线调节模式*下驱动函数
void Motor_Update_Entray_Pi(void)			// Mode2:树莓派
{
	// 树莓派Line_PID更新
	PID_Line.realPoint_Now = Pi_xLine_real ;
	PID_Line.goalPoint = Pi_xLine_goal ;
	// 树莓派Line_PID计算
	PID_Update(&PID_Line , PID_Line.realPoint_Now) ;
	// 目标速度控制
	if (isBreak)
	{
		Motor_A.GoalSpeed = 0 ;
		Motor_B.GoalSpeed = 0 ;
	}
	else
	{
		// 双环控制:外环x轴环PID,内环速度环PID
		Motor_A.GoalSpeed = goalPointTwo - PID_Line.setPoint ;
		Motor_B.GoalSpeed = goalPointTwo + PID_Line.setPoint ;
	}
	// 测速与PID更新
	Motor_Speed_Update(&Motor_A) ;								// 编码器测速,得到真实速度
	Motor_SetGoalSpeed(&Motor_A , Motor_A.GoalSpeed) ;	// 配置目标速度
	Motor_PID_Update(&Motor_A) ;									// PID更新,得到设定速度
	
	Motor_Speed_Update(&Motor_B) ;								// 编码器测速,得到真实速度
	Motor_SetGoalSpeed(&Motor_B , Motor_B.GoalSpeed) ;	// 配置目标速度
	Motor_PID_Update(&Motor_B) ;									// PID更新,得到设定速度
	
	// 电机配置速度
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;	// 配置设定速度
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;	// 配置设定速度
}

// *树莓派巡线调节模式*下VOFA调参函数
void Motor_Pi_Check(void)							// Mode2:树莓派
{
	if (Serial_GetNewPackageFlag_ABC() == 1)
	{
		// 文本包调试程序
		
		Serial_SetIntData("xLine_goal" , "xLine_goal=%d" , &Pi_xLine_goal) ;
		Serial_SetIntData("xLine_real" , "xLine_real=%d" , &Pi_xLine_real) ;
		
		// 测试
		Serial_SetIntData("Pi_Speed_Max" , "Pi_Speed_Max=%d" , &Pi_Speed_Max) ;
		PID_Line.OutMax = Pi_Speed_Max ;
		PID_Line.OutMin = -Pi_Speed_Max;
		
		Serial_SetFloatData("KpC" , "KpC=%f" , &PID_Line.Kp) ;
		Serial_SetFloatData("KiC" , "KiC=%f" , &PID_Line.Ki) ;
		Serial_SetFloatData("KdC" , "KdC=%f" , &PID_Line.Kd) ;
		
		// 两个轮子调试
		// 刹车
		if ( Serial_SetIntData("break" , "break=%d" , &goalPoint_A) )
		{
			if (isBreak == false)
			{
				isBreak = true ;
			}
			else
			{
				isBreak = false ;
			}
		}
		// 一起跑
		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPointTwo))
		{
			goalPoint_A = goalPointTwo ;
			goalPoint_B = goalPointTwo ;
		}
		
	}
	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
	// VOFA展示电机状态
//	printf("%d,%d,%f,%d,%d,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , PID_Line.goalPoint,Motor_B.GoalSpeed , Motor_B.RealSpeed , PID_Line.realPoint_Now);
	printf("%f,%f,%f,%f,%f\n",PID_Line.goalPoint , PID_Line.realPoint_Now ,PID_Line.setPoint ,PID_Line.pout,PID_Line.dout );
//		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
}
