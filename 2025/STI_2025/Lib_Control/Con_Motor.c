#include "Con_Motor.h"
#include "MPU.h"

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
extern int Pi_LR_Status	  ;	// LR  -> 0初始化 , 1->L  , 2->R
// Y8巡线变量
extern int Y8_Speed_MAX ;
extern Pid_Typedef Y8_Line_PID ;

// 实验*******
extern int Turn_cnt  ;	// 转向时间计时
extern int Turn_Num	;	// 转向次数
extern int Turn_ALL	;
extern int Turn_Num_MPU ;
extern int Speed_Mode ;

// MPU6050
extern int turning_flag;
extern Angle_t current_angle;


// 内部变量
int Con_NULL ;

// *************函数*************

// *电机PID调试模式*下驱动函数
void Motor_Update_Entray_Check(void)	// 调试任务:电机PID检查
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
void Motor_PID_Check(void)						// 调试任务:电机PID检查
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


// Y8寻迹下驱动函数
void Motor_Update_Entray_Y8(void)	// Mode1:Y8寻迹
{
	// 刹车判断
	if (isBreak)
	{
		goalPoint_A = 0 ;
		goalPoint_B = 0 ;
	}
	// 测速与PID更新
	Motor_Speed_Update(&Motor_A) ;								// 编码器测速,得到真实速度
	Motor_SetGoalSpeed(&Motor_A , goalPoint_A) ;	// 配置目标速度
	Motor_PID_Update(&Motor_A) ;									// PID更新,得到设定速度
	
	Motor_Speed_Update(&Motor_B) ;								// 编码器测速,得到真实速度
	Motor_SetGoalSpeed(&Motor_B , goalPoint_B) ;	// 配置目标速度
	Motor_PID_Update(&Motor_B) ;									// PID更新,得到设定速度
	
	
	// 电机配置速度
	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;	// 配置设定速度
	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;	// 配置设定速度
}

// Y8寻迹下VOFA调参函数
void Motor_VOFA_Set_Y8(void)
{
	// *文本包调试程序*
	if (Serial_GetNewPackageFlag_ABC() == 1)
	{
		// 基础速度设置
		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPointTwo)){ ; }
		
		// Y8_Speed_MAX
		if (Serial_SetIntData("Y8_Speed_MAX" , "Y8_Speed_MAX=%d" , &Y8_Speed_MAX))
		{
			Y8_Line_PID.OutMax = Y8_Speed_MAX ;
			Y8_Line_PID.OutMin = -Y8_Speed_MAX ;
		}
		
		// 调节PID
		Serial_SetFloatData("KpC" , "KpC=%f" , &Y8_Line_PID.Kp) ;
		Serial_SetFloatData("KiC" , "KiC=%f" , &Y8_Line_PID.Ki) ;
		Serial_SetFloatData("KdC" , "KdC=%f" , &Y8_Line_PID.Kd) ;
		
		// 刹车与重启 
		if ( Serial_SetIntData("break" , "break=%d" , &Con_NULL) )						
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
		if ( Serial_SetIntData("Speed_Mode" , "Speed_Mode=%d" , &Con_NULL) )						
		{
			if (Speed_Mode == false)
			{
				Speed_Mode = true ;
			}
			else
			{
				Speed_Mode = false ;
			}
		}
	}
	// *VOFA展示电机状态*
	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
//	printf("%d,%d,%f,%f,%d\n",-Motor_A.RealSpeed , Motor_B.RealSpeed , current_angle.yaw , Y8_Line_PID.setPoint,Y8_Speed_MAX) ;
//	printf("%f,%f,%f,%d,%d\n", Y8_Line_PID.goalPoint , Y8_Line_PID.realPoint_Now , Y8_Line_PID.setPoint , -Motor_A.RealSpeed , Motor_B.RealSpeed) ;
	printf("%f,%f,%f,%f\n", Y8_Line_PID.realPoint_Now * 10 , Y8_Line_PID.setPoint , Y8_Line_PID.pout , Y8_Line_PID.dout ) ;
}


/* 
注释:
	放弃树莓派主控巡线机制 , 所以注释掉相关功能	
	树莓派在巡线方面只进行等停和停止功能
	树莓派在RGB识别和LR识别起主控作用
其他注释内容:
setup:
	#ifdef PI_Line_Mode	// 树莓派视觉巡线模式
	taskInit(&Motor_Status , 0 , Encoder_PID_Gap_Time , Motor_Update_Entray_Pi) ;			// 树莓派视觉巡线模式
	#endif

// 任务1:电机状态更新
mytask Motor_Status ;	
void Motor_Update_Entray_Pi(void) ;
void Motor_Pi_Check(void) ;		// 电机与树莓派联调函数(模拟)

while:
	#ifdef PI_Line_Mode		// 树莓派视觉巡线模式
	Motor_Pi_Check() ;		
	#endif

*/

//// *树莓派巡线调节模式*下驱动函数
//void Motor_Update_Entray_Pi(void)			// Mode2:树莓派
//{
//	// 树莓派Line_PID更新
//	PID_Line.realPoint_Now = Pi_xLine_real ;
//	PID_Line.goalPoint = Pi_xLine_goal ;
//	// 树莓派Line_PID计算
//	PID_Update(&PID_Line , PID_Line.realPoint_Now) ;
//	// 目标速度控制
//	if (isBreak)
//	{
//		Motor_A.GoalSpeed = 0 ;
//		Motor_B.GoalSpeed = 0 ;
//	}
//	else
//	{
//		// 双环控制:外环x轴环PID,内环速度环PID
//		Motor_A.GoalSpeed = goalPointTwo - PID_Line.setPoint ;
//		Motor_B.GoalSpeed = goalPointTwo + PID_Line.setPoint ;
//	}
//	// 测速与PID更新
//	Motor_Speed_Update(&Motor_A) ;								// 编码器测速,得到真实速度
//	Motor_SetGoalSpeed(&Motor_A , Motor_A.GoalSpeed) ;	// 配置目标速度
//	Motor_PID_Update(&Motor_A) ;									// PID更新,得到设定速度
//	
//	Motor_Speed_Update(&Motor_B) ;								// 编码器测速,得到真实速度
//	Motor_SetGoalSpeed(&Motor_B , Motor_B.GoalSpeed) ;	// 配置目标速度
//	Motor_PID_Update(&Motor_B) ;									// PID更新,得到设定速度
//	
//	// 电机配置速度
//	Motor_SetPWM(&Motor_A , Motor_A.SetSpeed ) ;	// 配置设定速度
//	Motor_SetPWM(&Motor_B , Motor_B.SetSpeed ) ;	// 配置设定速度
//}

//// *树莓派巡线调节模式*下VOFA调参函数
//void Motor_Pi_Check(void)							// Mode2:树莓派
//{
//	if (Serial_GetNewPackageFlag_ABC() == 1)
//	{
//		// 文本包调试程序
//		
//		Serial_SetIntData("xLine_goal" , "xLine_goal=%d" , &Pi_xLine_goal) ;
//		Serial_SetIntData("xLine_real" , "xLine_real=%d" , &Pi_xLine_real) ;
//		
//		// 测试
//		Serial_SetIntData("Pi_Speed_Max" , "Pi_Speed_Max=%d" , &Pi_Speed_Max) ;
//		PID_Line.OutMax = Pi_Speed_Max ;
//		PID_Line.OutMin = -Pi_Speed_Max;
//		
//		Serial_SetFloatData("KpC" , "KpC=%f" , &PID_Line.Kp) ;
//		Serial_SetFloatData("KiC" , "KiC=%f" , &PID_Line.Ki) ;
//		Serial_SetFloatData("KdC" , "KdC=%f" , &PID_Line.Kd) ;
//		
//		// 两个轮子调试
//		// 刹车
//		if ( Serial_SetIntData("break" , "break=%d" , &goalPoint_A) )
//		{
//			if (isBreak == false)
//			{
//				isBreak = true ;
//			}
//			else
//			{
//				isBreak = false ;
//			}
//		}
//		// 一起跑
//		if (Serial_SetIntData("goalSpeed" , "goalSpeed=%d" , &goalPointTwo))
//		{
//			goalPoint_A = goalPointTwo ;
//			goalPoint_B = goalPointTwo ;
//		}
//		
//	}
//	Set_Current_USART(USART2_IDX); /* 想要指定不同串口必须在printf前加上此函数 */
//	// VOFA展示电机状态
////	printf("%d,%d,%f,%d,%d,%f\n",Motor_A.GoalSpeed , Motor_A.RealSpeed , PID_Line.goalPoint,Motor_B.GoalSpeed , Motor_B.RealSpeed , PID_Line.realPoint_Now);
//	printf("%f,%f,%f,%f,%f\n",PID_Line.goalPoint , PID_Line.realPoint_Now ,PID_Line.setPoint ,PID_Line.pout,PID_Line.dout );
////		printf("%d,%d,%d,%f,%f,%f\n",Motor_B.GoalSpeed , Motor_B.RealSpeed , Motor_B.SetSpeed,Motor_B.PID_s.pout,Motor_B.PID_s.iout,Motor_B.PID_s.dout);
//}
