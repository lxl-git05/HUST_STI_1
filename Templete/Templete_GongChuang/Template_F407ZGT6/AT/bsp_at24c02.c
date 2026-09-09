#include "bsp_at24c02.h"

#if AT24C02_USE_SW_I2C
/* ========================================================================
 * 软件I2C实现（PA4=AT_SCL, PA5=AT_SDA）
 * ======================================================================== */

/* SDA方向切换：输入=浮空，输出=推挽 */
static void SW_I2C_SDA_In(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = AT24C02_SDA_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    HAL_GPIO_Init(AT24C02_SDA_PORT, &GPIO_InitStruct);
}

static void SW_I2C_SDA_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = AT24C02_SDA_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(AT24C02_SDA_PORT, &GPIO_InitStruct);
}

/**
 * @brief I2C起始信号
 */
static void SW_I2C_Start(void)
{
    SW_I2C_SDA_Out();
    AT24C02_SDA_H();
    AT24C02_SCL_H();
    AT24C02_DELAY_US(5);
    AT24C02_SDA_L();        // SCL=1时SDA由H变L，起始信号
    AT24C02_DELAY_US(5);
    AT24C02_SCL_L();
    AT24C02_DELAY_US(2);
}

/**
 * @brief I2C停止信号
 */
static void SW_I2C_Stop(void)
{
    SW_I2C_SDA_Out();
    AT24C02_SCL_L();
    AT24C02_SDA_L();
    AT24C02_DELAY_US(2);
    AT24C02_SCL_H();
    AT24C02_DELAY_US(5);
    AT24C02_SDA_H();        // SCL=1时SDA由L变H，停止信号
    AT24C02_DELAY_US(5);
}

/**
 * @brief 等待从机应答
 * @return 0=有应答, 1=无应答/超时
 */
static uint8_t SW_I2C_WaitAck(void)
{
    uint8_t timeout = 0;

    SW_I2C_SDA_In();
    AT24C02_SCL_L();
    AT24C02_DELAY_US(2);
    AT24C02_SCL_H();
    AT24C02_DELAY_US(2);

    while (AT24C02_SDA_READ() == GPIO_PIN_SET)
    {
        if (++timeout > 200)
        {
            SW_I2C_Stop();
            return 1;
        }
        AT24C02_DELAY_US(5);
    }
    AT24C02_SCL_L();
    SW_I2C_SDA_Out();
    return 0;
}

/**
 * @brief 主机发送应答位
 * @param ack 0=应答, 1=非应答
 */
static void SW_I2C_SendAck(uint8_t ack)
{
    SW_I2C_SDA_Out();
    AT24C02_SCL_L();
    AT24C02_DELAY_US(2);
    if (ack)
        AT24C02_SDA_H();
    else
        AT24C02_SDA_L();
    AT24C02_DELAY_US(2);
    AT24C02_SCL_H();
    AT24C02_DELAY_US(5);
    AT24C02_SCL_L();
}

/**
 * @brief 发送一个字节（MSB先发）
 */
static void SW_I2C_SendByte(uint8_t dat)
{
    uint8_t i;
    SW_I2C_SDA_Out();
    AT24C02_SCL_L();

    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
            AT24C02_SDA_H();
        else
            AT24C02_SDA_L();
        dat <<= 1;
        AT24C02_DELAY_US(2);
        AT24C02_SCL_H();
        AT24C02_DELAY_US(5);
        AT24C02_SCL_L();
        AT24C02_DELAY_US(2);
    }
}

/**
 * @brief 读取一个字节
 * @param ack 0=主机回应答, 1=主机回非应答
 */
static uint8_t SW_I2C_ReadByte(uint8_t ack)
{
    uint8_t i, dat = 0;
    SW_I2C_SDA_In();

    for (i = 0; i < 8; i++)
    {
        AT24C02_SCL_L();
        AT24C02_DELAY_US(5);
        AT24C02_SCL_H();
        AT24C02_DELAY_US(2);
        dat <<= 1;
        if (AT24C02_SDA_READ() == GPIO_PIN_SET)
            dat |= 0x01;
        AT24C02_DELAY_US(2);
    }
    SW_I2C_SendAck(ack);
    return dat;
}

/* ========================================================================
 * 公开API（软件I2C）
 * ======================================================================== */

/**
 * @brief AT24C02 初始化（软件I2C）
 */
void AT24C02_Init(void)
{
    AT24C02_SCL_H();
    AT24C02_SDA_H();
    AT24C02_DELAY_US(10);
}

/**
 * @brief 向AT24C02指定地址写入一个字节
 */
uint8_t AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data)
{
    SW_I2C_Start();
    SW_I2C_SendByte(AT24C02_ADDR_WRITE);
    if (SW_I2C_WaitAck()) { SW_I2C_Stop(); return 0; }

    SW_I2C_SendByte(WordAddress);
    if (SW_I2C_WaitAck()) { SW_I2C_Stop(); return 0; }

    SW_I2C_SendByte(Data);
    if (SW_I2C_WaitAck()) { SW_I2C_Stop(); return 0; }

    SW_I2C_Stop();
    HAL_Delay(5);   // 等待内部写周期完成
    return 1;
}

/**
 * @brief 从AT24C02指定地址读取一个字节
 */
uint8_t AT24C02_ReadByte(uint8_t WordAddress)
{
    uint8_t dat;

    SW_I2C_Start();
    SW_I2C_SendByte(AT24C02_ADDR_WRITE);
    if (SW_I2C_WaitAck()) { SW_I2C_Stop(); return 0; }

    SW_I2C_SendByte(WordAddress);
    if (SW_I2C_WaitAck()) { SW_I2C_Stop(); return 0; }

    SW_I2C_Start();   // 重复起始
    SW_I2C_SendByte(AT24C02_ADDR_READ);
    if (SW_I2C_WaitAck()) { SW_I2C_Stop(); return 0; }

    dat = SW_I2C_ReadByte(1);   // 主机回NACK
    SW_I2C_Stop();
    return dat;
}

#else
/* ========================================================================
 * 硬件I2C实现（复用已配置的I2C2外设）
 * ======================================================================== */

/**
 * @brief AT24C02 初始化（硬件I2C2已在MX_I2C2_Init中初始化，此处为空操作）
 */
void AT24C02_Init(void)
{
    /* I2C2硬件已在main.c的MX_I2C2_Init()中初始化 */
}

/**
 * @brief 向AT24C02指定地址写入一个字节
 */
uint8_t AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Write(&hi2c2, AT24C02_ADDR_WRITE, WordAddress,
                                I2C_MEMADD_SIZE_8BIT, &Data, 1, 100);
    if (status != HAL_OK)
        return 0;

    HAL_Delay(5);   // 等待内部写周期完成
    return 1;
}

/**
 * @brief 从AT24C02指定地址读取一个字节
 */
uint8_t AT24C02_ReadByte(uint8_t WordAddress)
{
    uint8_t Data = 0;

    HAL_I2C_Mem_Read(&hi2c2, AT24C02_ADDR_READ, WordAddress,
                     I2C_MEMADD_SIZE_8BIT, &Data, 1, 100);
    return Data;
}

#endif /* AT24C02_USE_SW_I2C */
