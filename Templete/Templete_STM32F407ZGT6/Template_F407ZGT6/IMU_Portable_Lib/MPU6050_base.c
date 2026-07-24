#include "MPU6050_base.h"
//#include "MyI2C.h"
#include "LED_Flash.h"
#include "Timer_Counter.h"

// 宏定义MPU6050的寄存器信息,使得更好理解
// 寄存器地址宏定义
#define	MPU6050_SMPLRT_DIV		0x19
#define	MPU6050_CONFIG				0x1A
#define	MPU6050_GYRO_CONFIG		0x1B
#define	MPU6050_ACCEL_CONFIG	0x1C

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I			0x75

// 加速度计量程对应寄存器值
#define ACCEL_2G        0x00    // ±2g
#define ACCEL_4G        0x08    // ±4g
#define ACCEL_8G        0x10    // ±8g
#define ACCEL_16G       0x18    // ±16g

// 陀螺仪量程对应寄存器值
#define GYRO_250        0x00    // ±250°/s
#define GYRO_500        0x08    // ±500°/s
#define GYRO_1000       0x10    // ±1000°/s
#define GYRO_2000       0x18    // ±2000°/s

// ********** 重要:确定想要的量程 **********
#define ACCEL_RANGE     ACCEL_2G
#define GYRO_RANGE      GYRO_250

// 宏定义确定满量程
/* --------------------- 灵敏度宏（用于数据转换） --------------------- */
#if   ACCEL_RANGE == ACCEL_2G
    #define ACCEL_SENSITIVITY   16384.0f   // LSB/g
#elif ACCEL_RANGE == ACCEL_4G
    #define ACCEL_SENSITIVITY   8192.0f
#elif ACCEL_RANGE == ACCEL_8G
    #define ACCEL_SENSITIVITY   4096.0f
#elif ACCEL_RANGE == ACCEL_16G
    #define ACCEL_SENSITIVITY   2048.0f
#endif

#if   GYRO_RANGE == GYRO_250
    #define GYRO_SENSITIVITY    131.0f     // LSB/(°/s)
#elif GYRO_RANGE == GYRO_500
    #define GYRO_SENSITIVITY    65.5f
#elif GYRO_RANGE == GYRO_1000
    #define GYRO_SENSITIVITY    32.8f
#elif GYRO_RANGE == GYRO_2000
    #define GYRO_SENSITIVITY    16.4f
#endif


#define MPU6050_ADDRESS 0xD0	// 这是含读写位和移位的地址写法, (0xDx = 0x68 << 1) | x 

//#define MPU6050_IS_Soft_Drive	// 软件模拟IIC,如果注释掉就是硬件IIC

#ifndef MPU6050_IS_Soft_Drive
extern I2C_HandleTypeDef hi2c1; // HAL库硬件IIC确定IIC的总线(I2C1)
static I2C_HandleTypeDef* hi2c_MPU6050 = &hi2c1;
#endif

// 参数
MPU6050_Raw_Data  	MPU_Raw_Data ;	// 最初的角度

// 初始化MPU6050相关配置
void MPU6050_Init(void)
{
	/*MPU6050寄存器初始化，需要对照MPU6050手册的寄存器描述配置，此处仅配置了部分重要的寄存器*/
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);		// 电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);		// 电源管理寄存器2，保持默认值0，所有轴均不待机
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);		// 采样率分频寄存器，配置采样率
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);				// 配置寄存器，配置DLPF
	
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, GYRO_RANGE);	  // 陀螺仪配置寄存器
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, ACCEL_RANGE);	// 加速度计配置寄存器
}

// 硬件IIC故障修复函数
HAL_StatusTypeDef MPU6050_I2C_Recover(void)
{
    // 1. 先禁用 I2C 外设
    __HAL_RCC_I2C2_CLK_DISABLE();  // 根据自己的 I2C 改成 I2C1 或 I2C2
    HAL_Delay(10);
    __HAL_RCC_I2C2_CLK_ENABLE();
    HAL_Delay(10);

    // 2. 把 SDA 和 SCL 引脚恢复为 GPIO，强制产生 9 个时钟脉冲（标准 I2C 总线恢复）
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 假设你的 I2C2: SCL=PB10, SDA=PB11（根据 CubeMX 修改）
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 强制拉高 SDA 和 SCL
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_SET);

    HAL_Delay(1);

    // 产生 9 个 SCL 时钟脉冲（SDA 保持高）
    for(int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // 产生一个 STOP 条件（SDA 从低到高，SCL 高）
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_Delay(1);

    // 3. 重新初始化 I2C 外设
    HAL_I2C_DeInit(hi2c_MPU6050);
    HAL_Delay(10);
    if(HAL_I2C_Init(hi2c_MPU6050) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(50);
    return HAL_OK;
}

// 写入数据
void MPU6050_WriteReg(uint8_t RegAddress , uint8_t Data)
{
	// 软件I2C
	#ifdef MPU6050_IS_Soft_Drive
	// 开始信号
	MyI2C_Start() ;	
	// 第1个字节(从机地址)
	MyI2C_SendByte(MPU6050_ADDRESS) ;	// 定位从机地址
	MyI2C_ReceiveAck() ;	// 得到应答位,后续可以处理,这里没用
	
	// 第2个字节(从机寄存器地址)
	MyI2C_SendByte(RegAddress) ;			// 定位从机寄存器地址
	MyI2C_ReceiveAck() ;	// 得到应答位
	
	// 第3个字节(写入的数据),可for_loop从数组中写入多个字节
	MyI2C_SendByte(Data) ;			// 写入的数据
	MyI2C_ReceiveAck() ;	// 得到应答位
	// 停止信号
	MyI2C_Stop() ;
	#else
	// HAL库硬件IIC
	//或者直接指定地址写：
	HAL_I2C_Mem_Write(hi2c_MPU6050, MPU6050_ADDRESS, RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, 10000);
	#endif
}

// 读取数据
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	// 读取的数据
	uint8_t Data ;
	
//	IIC软件
	#ifdef MPU6050_IS_Soft_Drive
	// 开始信号
	MyI2C_Start() ;	
	// 第1个字节(从机地址)
	MyI2C_SendByte(MPU6050_ADDRESS) ;	// 定位从机地址为写
	MyI2C_ReceiveAck() ;	// 得到应答位,后续可以处理,这里没用
	// 第2个字节(从机寄存器地址)
	MyI2C_SendByte(RegAddress) ;			// 定位从机寄存器地址
	MyI2C_ReceiveAck() ;	// 得到应答位
	
	// 重新开始信号(Sr)
	MyI2C_Start() ;	
	// 再次第1个字节(从机地址)
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01) ;	// 定位从机地址为读
	MyI2C_ReceiveAck() ;	// 得到应答位,后续可以处理,这里没用
	
	// 接收1个字节,可for_loop接收多个字节存在数组里面
	Data = MyI2C_ReceiveByte() ;
	MyI2C_SendAck(1) ; 		// 发送应答为:无应答,使得从机交出SDA控制权
	// 停止信号
	MyI2C_Stop() ;	
	// HAL库硬件IIC
	#else
	HAL_I2C_Mem_Read(hi2c_MPU6050, MPU6050_ADDRESS, RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, 10000);
	#endif
	// 返回数据
	return Data ;
}

// 连续读取6个参数,更快.只需要1.5ms 5.7ms
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	// 数据接收区,连续读取
	uint8_t buf[14];
	// 软件连续读取
	#ifdef MPU6050_IS_Soft_Drive
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();

	MyI2C_SendByte(MPU6050_ACCEL_XOUT_H);
	MyI2C_ReceiveAck();

	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	// 读取
	for(uint8_t i = 0; i < 14; i++)
	{
			buf[i] = MyI2C_ReceiveByte();

			if(i == 13)
					MyI2C_SendAck(1);   // NACK
			else
					MyI2C_SendAck(0);   // ACK
	}
	// 停止
	MyI2C_Stop();
	#else
	HAL_I2C_Mem_Read(hi2c_MPU6050,MPU6050_ADDRESS,MPU6050_ACCEL_XOUT_H,I2C_MEMADD_SIZE_8BIT,buf,14,1000);
	#endif
	
	// 数据处理
	// 得到加速度
	*AccX  = (int16_t)(buf[0] << 8 | buf[1]);
	*AccY  = (int16_t)(buf[2] << 8 | buf[3]);
	*AccZ  = (int16_t)(buf[4] << 8 | buf[5]);
	// 得到角速度(跳过了温度)
	*GyroX = (int16_t)(buf[8]  << 8 | buf[9]);
	*GyroY = (int16_t)(buf[10] << 8 | buf[11]);
	*GyroZ = (int16_t)(buf[12] << 8 | buf[13]);
}


// 得到MPU6050的地址
uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I) ;
}
// 原始数据更新
void MPU6050_Update_Data(void)
{
	// 数据接收区,连续读取
	uint8_t buf[14];
	// 软件连续读取
	#ifdef MPU6050_IS_Soft_Drive
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();

	MyI2C_SendByte(MPU6050_ACCEL_XOUT_H);
	MyI2C_ReceiveAck();

	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	// 读取
	for(uint8_t i = 0; i < 14; i++)
	{
			buf[i] = MyI2C_ReceiveByte();

			if(i == 13)
					MyI2C_SendAck(1);   // NACK
			else
					MyI2C_SendAck(0);   // ACK
	}
	// 停止
	MyI2C_Stop();
	#else
	// 硬件IIC可能读取失败,此时需要进行重启,而软件IIC问题就没有那么大,这里的修复逻辑有大量的delay,*待处理*
	uint8_t retry = 5 ;
	while(retry--) 
	{
		if (HAL_I2C_Mem_Read(hi2c_MPU6050,MPU6050_ADDRESS,MPU6050_ACCEL_XOUT_H,I2C_MEMADD_SIZE_8BIT,buf,14,1000) == HAL_OK)
		{
			retry = 0 ;
		}
		else	// 一般就是错误了
		{
			Flash_Mode_Set(Flash_Mode_Fast) ;
			Timer_Counter_Begin() ;
			MPU6050_I2C_Recover();
			Timer_Counter_End() ;
		}
		// 修不成就寄了,待处理
	}
	#endif
	
	// 数据处理
	// 得到加速度
	MPU_Raw_Data.AX  = ((int16_t)(buf[0] << 8 | buf[1])) * 1.0 / ACCEL_SENSITIVITY ;
	MPU_Raw_Data.AY  = ((int16_t)(buf[2] << 8 | buf[3])) * 1.0 / ACCEL_SENSITIVITY ;
	MPU_Raw_Data.AZ  = ((int16_t)(buf[4] << 8 | buf[5])) * 1.0 / ACCEL_SENSITIVITY ;
	// 得到角速度(跳过了温度)
	MPU_Raw_Data.GX = ((int16_t)(buf[8]  << 8 | buf[9]))  * 1.0 / GYRO_SENSITIVITY ;
	MPU_Raw_Data.GY = ((int16_t)(buf[10] << 8 | buf[11])) * 1.0 / GYRO_SENSITIVITY ;
	MPU_Raw_Data.GZ = ((int16_t)(buf[12] << 8 | buf[13])) * 1.0 / GYRO_SENSITIVITY ;
}
