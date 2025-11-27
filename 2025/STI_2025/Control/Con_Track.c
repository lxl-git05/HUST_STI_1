#include "Con_Track.h"
#include "MPU.h"
extern int turning_Flag_Bef ;
// 寻迹相关参数
mytask Y8_Line_Status ;						// 任务:寻迹控制任务
Pid_Typedef Y8_Line_PID ;					// 寻迹PID
float Y8_Line_Error ;							// PID巡线误差
uint8_t Y8_Line_Num ;							// 巡线检测到的点数
float Car_Length = 160.0f ;
float Y8_Black_Width_Arr[9] = {0,-40.25f,-28.75f,-17.25f,-5.75f,5.75f,17.25f,28.75f,40.25f} ;
float Y8_Error_Arr[9] = {0,-14.12f,-10.18f,-6.15f,-2.05f,2.05f,6.15f,10.18f,-14.12f};
float Y8_Error[25] = {0} ;	// 0.4ms执行一次采样,25次对应10ms,也就是PID采样计算时间
int Y8_Error_Count ;

// 巡线加权值
float Y8_JQ[9] = {-4.5f, -4.5f,-4.5f,-1.5f, -0.5f,0.5f ,1.5f,4.5f,4.5f};	

// 小车位置标志位
Y8_Position_Typedef Y8_Pos ;			// 初始为在初始位置

// 小车岔路专门处理参数
extern bool is_Car_Turn_Left ;		// 小车岔路方向判断
bool Car_LR_Speed_Mode = false ;	// 小车的岔路专门处理标志位
extern int Y8_Cnt ;								// 小车岔路倒计时参数

// *巡线特殊情况处理*
bool Y8_Lose_Line_isOK = false ;	// 巡线丢线包容度,true为允许丢线,并使4号识别到线

// ================ 外部变量 ================
// 小车转的弯数
extern int Turn_Num_MPU ;

// 寻迹模块初始化,其实就是PID初始化
void Y8_Line_Init(float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax )
{
	PID_Init(&Y8_Line_PID , kp , ki , kd , OutMax , OutMin , ioutMax , 10) ;
	Y8_Line_PID.goalPoint = 0.0f ;	// 目标是偏转为0
	taskInit(&Y8_Line_Status , 5 , 10 , Y8_Line_Control) ;	// 巡线任务初始化
}

extern int Car_Task_Num ;

// ====================== 寻迹特殊情况处理代码 ======================

// Y8巡线对照函数
bool Y8_Line_Contrast(int EX1 , int EX2 , int EX3 , int EX4 , int EX5 , int EX6 , int EX7 , int EX8 )
{
	return Y8_Line_Array[1] == EX1 && Y8_Line_Array[2] == EX2 && Y8_Line_Array[3] == EX3 && Y8_Line_Array[4] == EX4 &&
		Y8_Line_Array[5] == EX5 && Y8_Line_Array[6] == EX6 && Y8_Line_Array[7] == EX7 && Y8_Line_Array[8] == EX8 ;
}

// Y8巡线岔路口判断
bool Y8_is_LR(void)
{
	// 状态1:小车在初始化短直道,准备进入分叉路口
	if (Y8_Pos == Y8_Init_Pos)
	{
		if (Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 1 , 0) || 
				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 1 , 0 , 0) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 1 , 0 , 0 , 0) || 
		
				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 0 , 1) || 
				Y8_Line_Contrast(0 , 0 , 1 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(0 , 0 , 0 , 1 , 0 , 0 , 0 , 1) || 
		
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 1 , 0 , 0) || 
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 0 , 1 , 0 , 0 , 0 , 1 , 0) 
			 )
		{
			Y8_Pos = Y8_LR_Pos ;	// 小车进入岔道
			return true ;
		}
	}
	return false ;
}

// Y8巡线停止标识判断
bool Y8_is_Init(void)
{
	// 不在岔路口 *111 *111 * *1 *
	if (Y8_Pos == Y8_LR_Pos)	// 小车判断出发点的前提是小车判断成功过岔路(这里最好缩小判断范围,*待优化*)
	{
		if (Y8_Line_Contrast(0 , 0 , 1 , 1 , 1 , 1 , 1 , 0) || Y8_Line_Contrast(0 , 0 , 1 , 1 , 1 , 1 , 1 , 1) || 
				Y8_Line_Contrast(0 , 1 , 1 , 1 , 1 , 1 , 0 , 0) || Y8_Line_Contrast(1 , 1 , 1 , 1 , 1 , 1 , 0 , 0) || Y8_Line_Contrast(0 , 1 , 1 , 1 , 1 , 1 , 1 , 0)
			 )
		{
			Y8_Pos = Y8_Init_Pos ;	// 到达出发点
			return true ;
		}
	}
	return false ;
}

// 八路巡线的异常情况判断并处理
bool Y8_Line_is_Error(void)
{
	// 大于等于3个点视为危险点
	if (Y8_Line_Num >= 2)
	{
		// 其他的识别都算作错误点
//		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		return false;
	}
	// 安全点
	return true ;
}


// ====================== 寻迹核心代码 ======================

// 岔路口速度特殊处理
void Y8_LR_Speed_Mode(void)
{
	if (is_Car_Turn_Left == true)
	{
		goalPoint_A = 120 ;
		goalPoint_B = 40 ;
		Car_LR_Speed_Mode = true ;
		Y8_Cnt = 300 ;
	}
	else
	{
		goalPoint_A = 60  ;
		goalPoint_B = 100 ;
		Car_LR_Speed_Mode = true ;
		Y8_Cnt = 100 ;
	}
}

// Y8巡线采样,放入中断1ms计次
void Y8_Error_Update(void)
{	// 更新
	Y8_LineSensor_Update() ;
	// 采样
	if (Y8_Update_Flag == true)
	{
		Y8_Update_Flag = false ;
		// 上次值
		static float last_Error  = 0.0f;
		
		float sum = 0 ;
		int blackCount = 0;
		
		// 计算偏差
		for (int i = 1; i < 9; i++)
		{
			if (Y8_Line_Array[i] == 1) // 黑线有效
			{
				sum += Y8_Error_Arr[i];
				blackCount++;
			}
		}
		// 采样一次
		if (blackCount == 0)
		{
			Y8_Error[Y8_Error_Count] = last_Error ;
		}
		else
		{
			Y8_Error[Y8_Error_Count] = sum * 1.0f / blackCount ;
			last_Error = Y8_Error[Y8_Error_Count] ;
		}
		
		Y8_Error_Count ++ ;
		if (Y8_Error_Count == 25)
		{
			Y8_Error_Count = 0 ;
		}
		// 1.判断岔路口,硬编码过弯道
		if (Y8_is_LR() == true)
		{
			Y8_LR_Speed_Mode() ;	// 岔路口速度特殊处理
		}
		
		Car_Task(Car_Task_Num) ;
		
		// MPU配合巡线检查,得到巡线转数
		
		static int Turn_Add_Update_Flag = 0 ;
		
		if (current_angle.yaw >= 60 && current_angle.yaw <= 130 && Turn_Add_Update_Flag == 0)
		{
			Turn_Num_MPU += 1 ;
			Turn_Add_Update_Flag = 1 ;	// 更新过一次了,除非连更新
		}
		else if (current_angle.yaw >= 140 && Turn_Add_Update_Flag == 1)
		{
			Turn_Num_MPU += 1 ;
			Turn_Add_Update_Flag = 2 ;	// 更新过一次了
		}
		else if (current_angle.yaw <= 10)
		{
			Turn_Add_Update_Flag = 0 ;	// 可以再次更新
		}
		
		turning_Flag_Bef = turning_flag ;		// 更新上次转向flag
		
		// 2. 计划加入终点判断,防止过快采样丢失终点判断
		
	}
}


void Y8_Line_Control(void)
{
	// 2. 得到偏差角度
	float offset = 0.0f;
	
	for (int i = 0 ; i < 25 ; i++)
	{
		offset += Y8_Error[i] ;
	}
	
	offset = offset * 1.0f / 25 ;
	
	// 3. PID计算
	PID_Update(&Y8_Line_PID , offset) ;
	
	// 如果刹车未启用，则执行
	if (!isBreak && Car_LR_Speed_Mode == false)
	{
		goalPoint_A  = goalPointTwo + Y8_Line_PID.setPoint ;
		goalPoint_B  = goalPointTwo - Y8_Line_PID.setPoint ;
	}
}

/*
// 计算偏移量函数
float Y8_Get_Line_Error(void)
{
    float sum = 0 ;
    int blackCount = 0;

    for (int i = 1; i < 9; i++)
    {
        if (Y8_Line_Array[i] == 1) // 黑线有效
        {
            sum += Y8_JQ[i];
            blackCount++;
        }
    }
		
		Y8_Line_Num = blackCount ;	// Y8本次巡到的黑线数
		// 没检测到黑线,这里后续可以优化,变成根据历史找到线路
		// (其实已经是根据历史寻迹了,因为没有返回值,所以PID没有更新,所以goal值始终是上次寻迹的值)
    if (blackCount == 0)  
		{
				return 999.0f;
		}
        
		
		Y8_Line_Error = sum / blackCount;
		
    return sum / blackCount;  // 平均偏移
}

// *巡线核心控制函数*,控制速度
void Y8_Line_Control(void)
{
	// 寻迹更新才进行控制
	if (Y8_Update_Flag == true)
	{
		// 1. 岔路口硬编码判断
		if (Y8_is_LR() == true)
		{
			Y8_LR_Speed_Mode() ;	// 岔路口速度特殊处理
		}
		
		// 2. 得到偏差角度
    float offset = Y8_Get_Line_Error();
		
		// 3. 丢线,数据不能传输给PID,否则会有极大值
    if ( offset - 999.0f > -0.1f && offset - 999.0f < 0.1f  )
    {
			// 包容丢线
			if (Y8_Lose_Line_isOK == true)
			{
				offset = 0 ;
				Y8_Lose_Line_isOK = false ;
			}
			else
			{
				return;
			}
    }
		
    // 4. 超过3个点视为错误点
		if (Y8_Line_is_Error() == false)
		{
			return ;
		}
		
		// 5. PID计算
    PID_Update(&Y8_Line_PID , offset) ;
		
    // 如果刹车未启用，则执行
    if (!isBreak && Car_LR_Speed_Mode == false)
    {
      goalPoint_A  = goalPointTwo + Y8_Line_PID.setPoint ;
			goalPoint_B  = goalPointTwo - Y8_Line_PID.setPoint ;
    }
		// 寻迹更新标志位置0,等待下次更新
		Y8_Update_Flag = false ;
	}
}
*/

/*
// 得到本次巡线点数,并且返回第一个黑点(如果没有就返回-1)
int Y8_Get_Black_Num(int *First_Black_Dot)
{
    uint8_t Y8_Black_Num = 0;
    *First_Black_Dot = -1;
    for (int i = 1; i <= 9; i++)
    {
        if (Y8_Line_Array[i] != 0)
        {
            if (*First_Black_Dot == -1) *First_Black_Dot = i; // 只记录第一个
            Y8_Black_Num++;
        }
    }
    return (int)Y8_Black_Num;
}
*/
/*
// 根据巡线点得到角度
float Y8_Get_Line_Error_Angle(float Car_Length , float Y8_Black_Width[])
{
	static float Y8_Last_Angle = 0.0f ;
	static int Last_Black_Dot = 4 ;
	int First_Black_Dot ;
	// 得到巡线点数
	Y8_Line_Num = Y8_Get_Black_Num(&First_Black_Dot) ;
	// 如果巡线数不是一个点
	if ( Y8_Line_Num != 1 )
	{
		return Y8_Last_Angle ;	// 待优化,可以试试使用滤波处理
	}
	// 巡到一个点
	else
	{
		// 如果黑点顺序变化是顺位变化则视为正常角度变化
		if (1)
		{
			// 更新上次角度值(弧度转角度)
			Last_Black_Dot = First_Black_Dot ;	// 更新
			Y8_Last_Angle = atanf(Y8_Black_Width[First_Black_Dot] / Car_Length) * (180.0f / M_PI);
			return Y8_Last_Angle;
		}
		// 如果黑点跳变则视为丢线的同时寻到了错误的线
		else
		{
			return Y8_Last_Angle ;
		}
	}
}
*/

