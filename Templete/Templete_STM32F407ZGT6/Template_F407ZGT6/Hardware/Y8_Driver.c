#include "Y8_Driver.h"
#include <math.h>
#include <string.h>
#include "Con_Motor.h"

// Y8寻迹: 1为空 0为有
#define Y8_Get 0	// 包含寻迹点
#define Y8_Nul 1	// 没有寻迹到
#define Y8_Length 130	// Y8到中心的Y偏移(cm)
#define Y8_FILTER_WIN 5	// 中值滤波窗口大小

uint8_t Y8_Data[8]  		 = {0};
const int8_t Y8_Width[8] = {-42,-30,-18,-6,6,18,30,42} ;	// Y8各个位置到中心的距离
float Y8_Bias 					 = 0  ;														// 每次20ms的时候查看的Y8的偏移角,进入PID计算

// ==================================================== 底层驱动 ====================================================
// 1. 微秒延时 (168MHz F407) 
// F103(72MHz): us * 8 → F407(168MHz): us * 19
// 软件延时精度±30%, 协议容差大(1~100μs均可)
static void Y8_Delay_us(uint32_t us)
{
    uint32_t count = us * 19;
    for (volatile uint32_t i = 0; i < count; i++);
}

// 2. 读取8路传感器原始值 
// 协议: CLK起始低5μs → 8个CLK脉冲(高5μs+低5μs)
//       每个下降沿后读取DAT → MSB先出, 存入LSB
// 返回: 8位数据, bit0=第1个CLK, bit7=第8个CLK, 1=白0=黑
static uint8_t Y8_Read_Sensor(void)
{
    uint8_t data = 0;
    MyGPIO_WritePin(&MYGPIO_Y8_CLK, 0);
    Y8_Delay_us(5);
    for (int i = 0; i < 8; i++) {
        MyGPIO_WritePin(&MYGPIO_Y8_CLK, 1);
        Y8_Delay_us(5);
        MyGPIO_WritePin(&MYGPIO_Y8_CLK, 0);
        Y8_Delay_us(5);
        if (MyGPIO_ReadPin(&MYGPIO_Y8_DAT))
            data |= (1 << i);
    }
    return data;
}
// ==================================================== Mid层数据滤波 ====================================================
// 1. 初始化
// CLK/DAT引脚已由CubeMX在gpio.c中配置:
//   CLK=PF15: 输出推挽, 无上下拉
//   DAT=PF13: 输入, 上拉 (匹配辅助板开漏输出)
void Y8_Drive_Init(void)
{
    MyGPIO_WritePin(&MYGPIO_Y8_CLK, 0);
}

// 2. 数据更新 (中断中调用) 
// 读取传感器 → 展开各位到 Y8_Data[]
// 注意: 辅助板未校准时始终返回0x00
void Y8_Data_Update(void)
{
    uint8_t raw = Y8_Read_Sensor();
    for (int i = 0; i < 8; i++) {
        Y8_Data[i] = (raw >> i) & 0x01;
    }
}

// 3. 得到20ms情况下的偏移角(先进行滤波)(用来计算PID)
float Y8_Angle_Bias_Get(uint16_t cnt)
{
	// 阶段0: 安全钳
	if (cnt == 0) {cnt = 1 ;}

	// ==================== 阶段1: 多采样投票 ====================
	uint8_t vote[8] = {0};
	for (uint16_t s = 0; s < cnt; s++)
	{
		uint8_t raw = Y8_Read_Sensor();
		for (int i = 0; i < 8; i++)
		{
			if (((raw >> i) & 0x01) == Y8_Get)		// 使用宏判断黑线
				vote[i]++;
		}
	}

	// ==================== 阶段2: 多数确认 ====================
	uint8_t confirm[8] = {0};
	uint8_t cnt_line = 0;
	for (int i = 0; i < 8; i++)
	{
		if (vote[i] > cnt / 2)			// 严格多数: > cnt/2 确认
		{
			confirm[i] = 1;
			cnt_line++;
		}
	}

	// 更新全局 Y8_Data 为滤波后结果（之后不再需要单独调 Y8_Data_Update）
	for (int i = 0; i < 8; i++)
		Y8_Data[i] = confirm[i];

	// ==================== 阶段3: 投票加权位置 → 角度 ====================
	static float last_valid_angle = 0.0f;

	float raw_angle;

	if (cnt_line == 0)			// 丢线: 保持上次有效角，EWMA 自然维持，丝滑回线
	{
		raw_angle = last_valid_angle;
	}
	else											// 正常: 投票权重加权平均（连续过渡，无阶梯）
	{
		// 投票权重加权: vote[i] 越大 → 该路越可信 → 权重越大
		int pos_sum = 0;
		int vote_sum = 0;
		for (int i = 0; i < 8; i++)
		{
			pos_sum  += vote[i] * Y8_Width[i];
			vote_sum += vote[i];
		}
		float pos = (float)pos_sum / (float)vote_sum;

		// 位置 → 角度: atan2(pos, Y8_Length)
		raw_angle = atan2f(pos, (float)Y8_Length) * 180.0f / 3.14159265359f;
		last_valid_angle = raw_angle;
	}

	// ==================== 阶段4: 时序中值滤波 ====================
	static float buf[Y8_FILTER_WIN] = {0.0f};
	static uint8_t buf_idx = 0;
	static uint8_t buf_full = 0;

	// 丢线帧不写入窗口（避免填充旧值，EWMA 自然维持输出）
	if (cnt_line > 0)
	{
		buf[buf_idx] = raw_angle;
		buf_idx = (buf_idx + 1) % Y8_FILTER_WIN;
		if (buf_full < Y8_FILTER_WIN) buf_full++;
	}

	float filtered_angle;
	if (buf_full >= Y8_FILTER_WIN)
	{
		// 拷贝 → 排序 → 取中位数
		float temp[Y8_FILTER_WIN];
		memcpy(temp, buf, sizeof(buf));
		for (int i = 0; i < Y8_FILTER_WIN - 1; i++)
		{
			for (int j = 0; j < Y8_FILTER_WIN - 1 - i; j++)
			{
				if (temp[j] > temp[j + 1])
				{
					float t = temp[j];
					temp[j] = temp[j + 1];
					temp[j + 1] = t;
				}
			}
		}
		filtered_angle = temp[Y8_FILTER_WIN / 2];	// 中位数
	}
	else
	{
		filtered_angle = raw_angle;		// 窗口未满, 直接用原始值
	}

	// ==================== 阶段5: EWMA 平滑 + 死区 → 更新全局并返回 ====================
	static float ewma_bias = 0.0f;
	#define Y8_EWMA_ALPHA  0.35f    // S弯跟不上就往上加，抖就往下减
	#define Y8_DEADBAND    2.7f     // 死区(°): 略高于最内圈传感器2.64°，刚好消振不浪费
	ewma_bias += (filtered_angle - ewma_bias) * Y8_EWMA_ALPHA;
	if (ewma_bias > -Y8_DEADBAND && ewma_bias < Y8_DEADBAND)
		ewma_bias = 0.0f;
	Y8_Bias = ewma_bias;
	return Y8_Bias;
}

// ==================================================== 应用层PID计算 ====================================================
// Y8巡线初始化
Pid_Typedef PID_Track ;
#define PID_Track_Dir (-1)
float Track_Base_Speed = 0 ;

void Y8_Init(void)
{
	// 硬件初始化
	Y8_Drive_Init() ;
	// PID初始化
	PID_Init(&PID_Track , 0.0f , 0.0f , 0.0f , 40 , -40 , 1000) ;
}

// Y8巡线更新 + 巡线
void Y8_PID_Update(void)
{
	// 更新数据
	// PID计算:更新真实值(目标值是0)
	PID_Track.realPoint_Now = Y8_Angle_Bias_Get(10) ;
	PID_Update(&PID_Track , PID_Track.realPoint_Now) ;
	// 配置速度
	Motor_SetSpeed(&Motor_A , Track_Base_Speed + PID_Track.setPoint * (PID_Track_Dir)) ;
	Motor_SetSpeed(&Motor_B , Track_Base_Speed + PID_Track.setPoint *(-PID_Track_Dir)) ;
}







