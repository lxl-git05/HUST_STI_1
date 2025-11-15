#include "Y8_Track.h"

// 寻迹的IIC模式
//#define Y8_IIC_Soft
// 寻迹地址
#define LINE_I2C_ADDR   (0x12 << 1)   // 注意：HAL库需要左移1位
// 寻迹模块读取的数据
uint8_t Y8_Line_Value   			;
uint8_t Y8_Line_Array[9] = {0};
uint8_t Y8_Line_Num ;

// ***************寻迹算法变量***************
// 电机变量
extern bool isBreak ;							// 刹车变量
extern int goalPoint_A ;					// 电机目标转速
extern int goalPoint_B ;					// 电机目标转速
extern int goalPointTwo;					// 共同速度(待优化,算作基础速度,不分高低)

// 寻迹变量
mytask Y8_Line_Status ;				// 任务2:寻迹
bool Y8_Update_Flag = false ;	// 寻迹更新标志位
Pid_Typedef Y8_Line_PID ;			// 寻迹PID
float Y8_Line_Error ;					// 巡线误差
float Y8_Line_C = 1.0f ;			// 倍增系数,增加PID的迟钝性or敏感性
float Y8_JQ[9];

// 小车位置标志位
bool is_Car_Init_Pos = false;

// ***************函数***************

// 寻迹模块初始化,其实就是PID初始化
void Y8_Line_Init(float kp, float ki, float kd , float OutMax , float OutMin , float ioutMax )
{
	PID_Init(&Y8_Line_PID , kp , ki , kd , OutMax , OutMin , ioutMax) ;
	Y8_Line_PID.goalPoint = 0.0f ;	// 目标是偏转为0
	taskInit(&Y8_Line_Status , 0 , 20 , Y8_Line_Control) ;	// 巡线任务初始化
	Y8_JQ[0] = -4.5f ;
	Y8_JQ[1] = -4.5f ;
	Y8_JQ[2] = -4.5f ;
	Y8_JQ[3] = -1.5f ;
	Y8_JQ[4] = -0.5f ;
	Y8_JQ[5] =  0.5f ;
	Y8_JQ[6] =  1.5f ;
	Y8_JQ[7] =  4.5f ;
	Y8_JQ[8] =  4.5f ;
}
// IIC软件模拟读取数据
uint8_t MyI2C_ReadReg(uint8_t devAddr, uint8_t regAddr)
{
    uint8_t data;

    MyI2C_Start();
    MyI2C_SendByte(devAddr | 0);       // 写模式
    if (MyI2C_ReceiveAck()) return 0xFF;

    MyI2C_SendByte(regAddr);           // 寄存器地址
    if (MyI2C_ReceiveAck()) return 0xFF;

    MyI2C_Start();                     // 重启信号
    MyI2C_SendByte(devAddr | 1);       // 读模式
    if (MyI2C_ReceiveAck()) return 0xFF;

    data = MyI2C_ReceiveByte();        // 读取数据
    MyI2C_SendAck(1);                  // 发送 NACK
    MyI2C_Stop();

    return data;
}


// 读取8路数据,并转化为数组,为了方便起见,数组有9位,1-8为有效数据
void Y8_LineSensor_Update(void)
{
	// 状态寄存器
	uint8_t reg = 0x30;  

	// 从寄存器 0x30 读取 1 字节数据,分两种方法,选其中一种:
	#ifdef Y8_IIC_Soft
	Y8_Line_Value = MyI2C_ReadReg(LINE_I2C_ADDR, reg);
	#else
	HAL_I2C_Mem_Read(&hi2c1, LINE_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &Y8_Line_Value, 1, 100);
	#endif
	// 转化数据
	for (int i = 1; i < 9; i++)
	{
			Y8_Line_Array[i] = 1 - ( (Y8_Line_Value >> (8 - i)) & 0x01 );   // 从高位到低位依次提取
	}
	Y8_Update_Flag = true ;	// 寻迹更新
}

// ***************巡线算法编写***************

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
		
    if (blackCount == 0)  // 没检测到黑线,这里后续可以优化,变成根据历史找到线路
        return 999.0f;
		
		Y8_Line_Error = sum / blackCount * Y8_Line_C;
		
    return sum / blackCount;  // 平均偏移
}

// 巡线核心控制函数,控制速度
void Y8_Line_Control(void)
{
	// 寻迹更新才进行控制
	if (Y8_Update_Flag == true)
	{
		// 得到偏差量
    float offset = Y8_Get_Line_Error();
		
		// 丢线,数据不能传输给PID,否则会有极大值
    if (offset - 999.0f > -0.1f && offset - 999.0f < 0.1f)
    {
        return;
    }

    // 特殊情况检测,暂时没写,*待优化*
		
		
		
		// PID计算
    PID_Update(&Y8_Line_PID , offset) ;
    // 如果刹车未启用，则执行
    if (!isBreak)
    {
      goalPoint_A  = goalPointTwo + Y8_Line_PID.setPoint ;
			goalPoint_B  = goalPointTwo - Y8_Line_PID.setPoint ;
    }
		// 寻迹更新标志位置0,等待下次更新
		Y8_Update_Flag = false ;
	}
}


// **********************Y8算法逻辑**********************
// Y8巡线对照函数
bool Y8_Line_Contrast(int EX1 , int EX2 , int EX3 , int EX4 , int EX5 , int EX6 , int EX7 , int EX8 )
{
	return Y8_Line_Array[1] == EX1 && Y8_Line_Array[2] == EX2 && Y8_Line_Array[3] == EX3 && Y8_Line_Array[4] == EX4 &&
		Y8_Line_Array[5] == EX5 && Y8_Line_Array[6] == EX6 && Y8_Line_Array[7] == EX7 && Y8_Line_Array[8] == EX8 ;
}

// Y8巡线岔路口判断
bool Y8_is_LR(bool *is_Car_Init_Position)
{
	// 状态1:小车在初始化短直道,准备进入分叉路口
	if (*is_Car_Init_Position == true)
	{
		if (Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 1 , 0) || 
				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 1 , 0 , 0) || Y8_Line_Contrast(1 , 0 , 0 , 0 , 1 , 0 , 0 , 0) || 
		
				Y8_Line_Contrast(1 , 0 , 0 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 0 , 1) || 
				Y8_Line_Contrast(0 , 0 , 1 , 0 , 0 , 0 , 0 , 1) || Y8_Line_Contrast(0 , 0 , 0 , 1 , 0 , 0 , 0 , 1) || 
		
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 1 , 0 , 0) || 
				Y8_Line_Contrast(0 , 1 , 0 , 0 , 0 , 0 , 1 , 0) || Y8_Line_Contrast(0 , 0 , 1 , 0 , 0 , 0 , 1 , 0) 
			 )
		{
			*is_Car_Init_Position = false ;
			return true ;
		}
	}
	return false ;
}

// Y8巡线停止标识判断
bool Y8_is_Init(bool *is_Car_Init_Position)
{
	// 不在岔路口 *111 *111 * *1 *
	if (*is_Car_Init_Position == false)
	{
		if (Y8_Line_Contrast(0 , 0 , 1 , 1 , 1 , 1 , 1 , 0) || Y8_Line_Contrast(0 , 0 , 1 , 1 , 1 , 1 , 1 , 1) || 
				Y8_Line_Contrast(0 , 1 , 1 , 1 , 1 , 1 , 0 , 0) || Y8_Line_Contrast(1 , 1 , 1 , 1 , 1 , 1 , 0 , 0) || Y8_Line_Contrast(0 , 1 , 1 , 1 , 1 , 1 , 1 , 0)
			 )
		{
			*is_Car_Init_Position = true ;
			return true ;
		}
	}
	return false ;
}



//// Y8弯道进入直道逻辑判断
//bool Turn_to_Cross(void)
//{
//	// 走弯道时,理论上不可能出现内轮大于外轮的情况,所以内<外,而进入直道,如果还是这个状态必然偏向,所以内轮与外轮必然产生交叉,所以逻辑如下
//	// 当然,仅仅一个函数肯定不够,但是在状态机判断方位的逻辑下足够有把握
//	if (goalPoint_A - goalPoint_B > -5 && goalPoint_A - goalPoint_B < 5)
//	{
//		return true ;
//	}
//	return false ;
//}




