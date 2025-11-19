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
float Y8_JQ[9];								// 巡线加权
bool Y8_Lose_Line_isOK = false ;	// 巡线丢线包容度,true为允许丢线,并使4号识别到线

// 小车位置标志位
Y8_Position_Typedef Y8_Pos ;	// 初始为在初始位置
extern bool is_Car_Turn_Left ;// 小车岔路方向判断
bool Car_LR_Speed_Mode = false ;	// 小车的岔路专门处理函数
extern int Y8_Cnt ;

// 树莓派变量
extern int Pi_RGB_Status  ;	// RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
extern int Pi_LR_Status	  ;	// LR  -> 0初始化 , 1->L  , 2->R
extern int Pi_Stop_Status ;	// wait & stop -> 0无 , 1停止 , 2等停
extern int Pi_x_Line_real ;	// 巡线x的真实值,已处理

extern int Car_Wait_Flag  ;	// 小车等停标志位
extern int Car_Wait_cnt ;

// 实验
extern int Turn_Num_MPU ;

// ***************函数***************

// ====================== 底层代码 ======================

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


// ====================== 寻迹特殊情况处理代码 ======================

// Y8巡线对照函数
bool Y8_Line_Contrast(int EX1 , int EX2 , int EX3 , int EX4 , int EX5 , int EX6 , int EX7 , int EX8 )
{
	return Y8_Line_Array[1] == EX1 && Y8_Line_Array[2] == EX2 && Y8_Line_Array[3] == EX3 && Y8_Line_Array[4] == EX4 &&
		Y8_Line_Array[5] == EX5 && Y8_Line_Array[6] == EX6 && Y8_Line_Array[7] == EX7 && Y8_Line_Array[8] == EX8 ;
}

// Y8巡线岔路口判断
bool Y8_is_LR()
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
bool Y8_is_Init()
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
	if (Y8_Line_Num >= 3)
	{
		// 其他的识别都算作错误点
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		return false;
	}
	// 安全点
	return true ;
}


// ====================== 寻迹核心代码 ======================

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
			HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
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
		
		// 岔路口cnt清零判断
		if (Y8_Cnt == 0)
		{
			Car_LR_Speed_Mode = false ;
		}

		// 2. 得到偏差量
    float offset = Y8_Get_Line_Error();
		
		// 3. 丢线,数据不能传输给PID,否则会有极大值
    if ( offset - 999.0f > -0.1f && offset - 999.0f < 0.1f  )
    {
			// 包容丢线
			if (Y8_Lose_Line_isOK == true)
			{
				offset = 0.5f ;
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




// ====================== 电工基地题目处理 ======================

// 电工基地第1题
void Y8_Task1(void)
{
	// 识别停止
	if (Y8_is_Init() && Turn_Num_MPU >= 4)
	{
		isBreak = 1 ;
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 4 )
	{
		isBreak = 1 ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
	}
}

// 电工基地第2题
void Y8_Task2(void)
{
	// 等停处理
	if (Pi_Stop_Status == 2 && Car_Wait_Flag == 0)
	{
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		Car_Wait_Flag = 1 ;
		Car_Wait_cnt  = 5000 ;
		isBreak = 1 ;
	}
	// 识别停止
	if (Y8_is_Init() && Turn_Num_MPU >= 4)
	{
		isBreak = 1 ;
	}
	if (Pi_Stop_Status == 1 && Turn_Num_MPU >= 4 )
	{
		isBreak = 1 ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
	}
}

// 电工基地第4题
void Y8_Task4(void)
{
	// 识别停止,跑12个1/4圈
	if (Y8_is_Init() && Turn_Num_MPU >= 12)
	{
		isBreak = 1 ;
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 12 )
	{
		isBreak = 1 ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
	}
	// LR转换 is_Car_Turn_Left Pi_LR_Status
	if (Pi_LR_Status != 0)
	{
		is_Car_Turn_Left = Pi_LR_Status - 1 ;
	}
}

// 电工基地第5题
void Y8_Task5(void)
{
	// 识别停止,跑16个1/4圈
	if (Y8_is_Init() && Turn_Num_MPU >= 16)
	{
		isBreak = 1 ;
	}
	if (Pi_Stop_Status == 1  && Turn_Num_MPU >= 16 )
	{
		isBreak = 1 ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
		HAL_GPIO_TogglePin(LED0_GPIO_Port , LED0_Pin ) ;
		HAL_Delay(500) ;
	}
	// RGB检测命令执行 RGB -> 0初始化 , 1红灯 , 2绿灯 , 3黄灯
	if (Pi_RGB_Status != 0)
	{
		if (Pi_RGB_Status == 1)
		{
			isBreak = 1 ;
		}
		else if (Pi_RGB_Status == 2)
		{
			isBreak = 0 ;
			Y8_Lose_Line_isOK = true ;
		}
		else if (Pi_RGB_Status == 3)
		{
			// 树莓派识别到停止位置,说明小车没有超过停止线,那么就停车,否则继续跑
			if (Pi_Stop_Status != 0)
			{
				isBreak = 1 ;
			}
		}
	}
}

// 电工基地试题
void Car_Task(int Car_Task_Seq)
{
	if (Car_Task_Seq == 1)
		Y8_Task1() ;
	else if (Car_Task_Seq == 2)
		Y8_Task2() ;
	else if (Car_Task_Seq == 4)
		Y8_Task4() ;
}


