#include "ICM_42688_base.h"
#include "LED_Flash.h"
#include "Timer_Counter.h"

// ==================== ICM42688 寄存器地址宏定义 ====================
// 数据输出寄存器 (Bank0)
#define	ICM42688_TEMP_DATA1			0x1D
#define	ICM42688_TEMP_DATA0			0x1E
#define	ICM42688_ACCEL_DATA_X1		0x1F
#define	ICM42688_ACCEL_DATA_X0		0x20
#define	ICM42688_ACCEL_DATA_Y1		0x21
#define	ICM42688_ACCEL_DATA_Y0		0x22
#define	ICM42688_ACCEL_DATA_Z1		0x23
#define	ICM42688_ACCEL_DATA_Z0		0x24
#define	ICM42688_GYRO_DATA_X1		0x25
#define	ICM42688_GYRO_DATA_X0		0x26
#define	ICM42688_GYRO_DATA_Y1		0x27
#define	ICM42688_GYRO_DATA_Y0		0x28
#define	ICM42688_GYRO_DATA_Z1		0x29
#define	ICM42688_GYRO_DATA_Z0		0x2A

// 配置寄存器 (Bank0)
#define	ICM42688_DEVICE_CONFIG		0x11
#define	ICM42688_INTF_CONFIG0		0x4C
#define	ICM42688_PWR_MGMT0			0x4E
#define	ICM42688_GYRO_CONFIG0		0x4F
#define	ICM42688_ACCEL_CONFIG0		0x50
#define	ICM42688_WHO_AM_I			0x75

// ==================== 量程宏定义 ====================
// 加速度计量程对应寄存器值 (ACCEL_CONFIG0[7:5])
// ★ ICM-42688: FS_SEL 编码与 MPU6050 相反 (000=±16g, 001=±8g, 010=±4g, 011=±2g)
#define ACCEL_FS_16G       	(0x00 << 5)    // ±16g
#define ACCEL_FS_8G        	(0x01 << 5)    // ±8g
#define ACCEL_FS_4G        	(0x02 << 5)    // ±4g
#define ACCEL_FS_2G        	(0x03 << 5)    // ±2g

// 陀螺仪量程对应寄存器值 (GYRO_CONFIG0[7:5])
// ★ ICM-42688: FS_SEL 编码与 MPU6050 相反 (000=±2000, 001=±1000, 010=±500, 011=±250)
#define GYRO_FS_2000       	(0x00 << 5)    // ±2000°/s
#define GYRO_FS_1000       	(0x01 << 5)    // ±1000°/s
#define GYRO_FS_500        	(0x02 << 5)    // ±500°/s
#define GYRO_FS_250        	(0x03 << 5)    // ±250°/s

// 加速度计ODR对应寄存器值 (ACCEL_CONFIG0[3:0], 值 = enum_index + 1)
#define ACCEL_ODR_1000HZ    6             // enum=5(1000HZ), reg_val=5+1=6
#define ACCEL_ODR_500HZ     15            // enum=14(500HZ), reg_val=14+1=15
#define ACCEL_ODR_200HZ     7             // enum=6(200HZ), reg_val=6+1=7

// 陀螺仪ODR对应寄存器值 (GYRO_CONFIG0[3:0], 值 = enum_index + 1)
#define GYRO_ODR_1000HZ     6             // enum=5(1000HZ), reg_val=5+1=6
#define GYRO_ODR_500HZ      15            // enum=14(500HZ), reg_val=14+1=15
#define GYRO_ODR_200HZ      7             // enum=6(200HZ), reg_val=6+1=7

// ********** 重要: 确定想要的量程 **********
#define ACCEL_RANGE_ICM     ACCEL_FS_4G
#define GYRO_RANGE_ICM      GYRO_FS_500

// ********** ODR选择 **********
#define ACCEL_ODR_ICM       ACCEL_ODR_1000HZ
#define GYRO_ODR_ICM        GYRO_ODR_1000HZ

// ==================== 灵敏度宏 ====================
#if   ACCEL_RANGE_ICM == ACCEL_FS_2G
    #define ACCEL_SENSITIVITY_ICM   16384.0f   // LSB/g
#elif ACCEL_RANGE_ICM == ACCEL_FS_4G
    #define ACCEL_SENSITIVITY_ICM   8192.0f
#elif ACCEL_RANGE_ICM == ACCEL_FS_8G
    #define ACCEL_SENSITIVITY_ICM   4096.0f
#elif ACCEL_RANGE_ICM == ACCEL_FS_16G
    #define ACCEL_SENSITIVITY_ICM   2048.0f
#endif

#if   GYRO_RANGE_ICM == GYRO_FS_250
    #define GYRO_SENSITIVITY_ICM    131.0f     // LSB/(°/s)
#elif GYRO_RANGE_ICM == GYRO_FS_500
    #define GYRO_SENSITIVITY_ICM    65.5f
#elif GYRO_RANGE_ICM == GYRO_FS_1000
    #define GYRO_SENSITIVITY_ICM    32.8f
#elif GYRO_RANGE_ICM == GYRO_FS_2000
    #define GYRO_SENSITIVITY_ICM    16.4f
#endif

// ==================== I2C地址 ====================
// ICM42688: 7bit地址 0x68 (AD0=GND, 扫描确认), HAL格式 = (0x68 << 1) = 0xD0
#define ICM42688_ADDRESS 0xD0

// 硬件I2C — 与MPU6050共用I2C1总线
extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef* hi2c_ICM42688 = &hi2c1;

// ==================== 全局变量 ====================
ICM42688_Raw_Data  ICM_Raw_Data ;	// 原始数据

// ==================== 初始化 ====================
void ICM42688_Init(void)
{
    uint8_t id;

    // 验证设备ID
    id = ICM42688_ReadReg(ICM42688_WHO_AM_I);
    if (id != 0x47)
    {
        // ID不匹配：芯片未连接或型号错误
        // 此处可根据需要添加错误处理（LED闪烁等）
        // 仍然继续初始化，给芯片一次机会
    }

    // 1. 软件复位
    ICM42688_WriteReg(ICM42688_PWR_MGMT0, 0x00);
    HAL_Delay(20);  // datasheet要求复位后至少等待200us，保守取20ms

    // 2. 配置加速度计：量程 + ODR
    //    ACCEL_CONFIG0 = FS[7:5] | ODR[3:0]
    ICM42688_WriteReg(ICM42688_ACCEL_CONFIG0, ACCEL_RANGE_ICM | ACCEL_ODR_ICM);

    // 3. 配置陀螺仪：量程 + ODR
    //    GYRO_CONFIG0 = FS[7:5] | ODR[3:0]
    ICM42688_WriteReg(ICM42688_GYRO_CONFIG0, GYRO_RANGE_ICM | GYRO_ODR_ICM);

    // 4. 进入低噪声模式：开启加速度计和陀螺仪
    //    PWR_MGMT0 = 0x0f → ACCEL_MODE=LowNoise(011) | GYRO_MODE=LowNoise(011)
    ICM42688_WriteReg(ICM42688_PWR_MGMT0, 0x0f);
    HAL_Delay(20);
}

// ==================== I2C总线恢复 ====================
HAL_StatusTypeDef ICM42688_I2C_Recover(void)
{
    // 1. 先禁用 I2C 外设再重新使能
    __HAL_RCC_I2C1_CLK_DISABLE();
    HAL_Delay(10);
    __HAL_RCC_I2C1_CLK_ENABLE();
    HAL_Delay(10);

    // 2. 把 SDA 和 SCL 引脚恢复为 GPIO，强制产生 9 个时钟脉冲
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // I2C1: SCL=PB6, SDA=PB7
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 强制拉高
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(1);

    // 产生 9 个 SCL 时钟脉冲（SDA 保持高）
    for (int i = 0; i < 9; i++)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // 产生 STOP 条件
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(1);

    // 3. 重新初始化 I2C
    HAL_I2C_DeInit(hi2c_ICM42688);
    HAL_Delay(10);
    if (HAL_I2C_Init(hi2c_ICM42688) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_Delay(50);
    return HAL_OK;
}

// ==================== 写入数据 ====================
void ICM42688_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    HAL_I2C_Mem_Write(hi2c_ICM42688, ICM42688_ADDRESS, RegAddress,
                      I2C_MEMADD_SIZE_8BIT, &Data, 1, 10000);
}

// ==================== 读取数据 ====================
uint8_t ICM42688_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;
    HAL_I2C_Mem_Read(hi2c_ICM42688, ICM42688_ADDRESS, RegAddress,
                     I2C_MEMADD_SIZE_8BIT, &Data, 1, 10000);
    return Data;
}

// ==================== 连续读取6轴原始数据 ====================
// 从 ACCEL_DATA_X1(0x1F) 连续读12字节：
//   buf[0:1]=AccelX, buf[2:3]=AccelY, buf[4:5]=AccelZ
//   buf[6:7]=GyroX,  buf[8:9]=GyroY,  buf[10:11]=GyroZ
void ICM42688_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                      int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t buf[12];
    HAL_I2C_Mem_Read(hi2c_ICM42688, ICM42688_ADDRESS, ICM42688_ACCEL_DATA_X1,
                     I2C_MEMADD_SIZE_8BIT, buf, 12, 1000);

    // 加速度（大端序拼接）
    *AccX  = (int16_t)(buf[0]  << 8 | buf[1]);
    *AccY  = (int16_t)(buf[2]  << 8 | buf[3]);
    *AccZ  = (int16_t)(buf[4]  << 8 | buf[5]);
    // 角速度（跳过温度，直接从 buf[6] 开始）
    *GyroX = (int16_t)(buf[6]  << 8 | buf[7]);
    *GyroY = (int16_t)(buf[8]  << 8 | buf[9]);
    *GyroZ = (int16_t)(buf[10] << 8 | buf[11]);
}

// ==================== WHO_AM_I ====================
uint8_t ICM42688_GetID(void)
{
    return ICM42688_ReadReg(ICM42688_WHO_AM_I);
}

// ==================== 原始数据更新（含灵敏度转换，带I2C恢复） ====================
void ICM42688_Update_Data(void)
{
    uint8_t buf[12] = {0};
    // 硬件I2C读取带重试（超时100ms，总共最多3次）
    uint8_t retry = 3;
    HAL_StatusTypeDef status = HAL_ERROR;
    while (retry--)
    {
        status = HAL_I2C_Mem_Read(hi2c_ICM42688, ICM42688_ADDRESS,
                                  ICM42688_ACCEL_DATA_X1,
                                  I2C_MEMADD_SIZE_8BIT, buf, 12, 100);
        if (status == HAL_OK)
        {
            break;
        }
        Flash_Mode_Set(Flash_Mode_Fast);
        Timer_Counter_Begin();
        ICM42688_I2C_Recover();
        Timer_Counter_End();
    }

    if (status != HAL_OK)
        return;  // I2C通信失败，跳过本次更新

    // 数据处理 — 灵敏度转换
    // 加速度（大端序 → 物理量）
    ICM_Raw_Data.AX = ((int16_t)(buf[0]  << 8 | buf[1]))  * 1.0f / ACCEL_SENSITIVITY_ICM;
    ICM_Raw_Data.AY = ((int16_t)(buf[2]  << 8 | buf[3]))  * 1.0f / ACCEL_SENSITIVITY_ICM;
    ICM_Raw_Data.AZ = ((int16_t)(buf[4]  << 8 | buf[5]))  * 1.0f / ACCEL_SENSITIVITY_ICM;
    // 角速度（大端序 → 物理量）
    ICM_Raw_Data.GX = ((int16_t)(buf[6]  << 8 | buf[7]))  * 1.0f / GYRO_SENSITIVITY_ICM;
    ICM_Raw_Data.GY = ((int16_t)(buf[8]  << 8 | buf[9]))  * 1.0f / GYRO_SENSITIVITY_ICM;
    ICM_Raw_Data.GZ = ((int16_t)(buf[10] << 8 | buf[11])) * 1.0f / GYRO_SENSITIVITY_ICM;
}
