#ifndef __BSP_AT24C02_H__
#define __BSP_AT24C02_H__
/*
    声明：这是基于STM32F407ZGT6建立的工程,后续业务修改只需要修改底层驱动即可
    1. 软件GPIO重定义
    2. 硬件IIC重定义
    3. 微秒延时重定义
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>

/* ==================== 驱动模式切换 ==================== */
// 1 = 软件I2C(GPIO模拟)，0 = 硬件I2C
#define AT24C02_USE_SW_I2C    1

#if AT24C02_USE_SW_I2C
/* ---- 软件I2C GPIO定义（PA4=AT_SCL, PA5=AT_SDA，已在.ioc声明label）---- */
#define AT24C02_SCL_PORT      AT_SCL_GPIO_Port      // GPIOA
#define AT24C02_SCL_PIN       AT_SCL_Pin             // PA4
#define AT24C02_SDA_PORT      AT_SDA_GPIO_Port      // GPIOA
#define AT24C02_SDA_PIN       AT_SDA_Pin             // PA5

/* SCL/SDA操作 */
#define AT24C02_SCL_H()       HAL_GPIO_WritePin(AT24C02_SCL_PORT, AT24C02_SCL_PIN, GPIO_PIN_SET)
#define AT24C02_SCL_L()       HAL_GPIO_WritePin(AT24C02_SCL_PORT, AT24C02_SCL_PIN, GPIO_PIN_RESET)
#define AT24C02_SDA_H()       HAL_GPIO_WritePin(AT24C02_SDA_PORT, AT24C02_SDA_PIN, GPIO_PIN_SET)
#define AT24C02_SDA_L()       HAL_GPIO_WritePin(AT24C02_SDA_PORT, AT24C02_SDA_PIN, GPIO_PIN_RESET)
#define AT24C02_SDA_READ()    HAL_GPIO_ReadPin(AT24C02_SDA_PORT, AT24C02_SDA_PIN)

/* 微秒延时（168MHz空循环，约1us/19次，由72MHz的*8校准而来: 168/72≈2.33, 8*2.33≈19）*/
#define AT24C02_DELAY_US(n)    do { volatile uint32_t _d = (n) * 19; while(_d--) __NOP(); } while(0)
#endif

/* ==================== 设备地址（软硬件共用）==================== */
/* AT24C02 7-bit设备地址 = 0x50，左移1位 + 读写位 = 0xA0(写) / 0xA1(读) */
#define AT24C02_ADDR_WRITE    0xA0
#define AT24C02_ADDR_READ     0xA1

/* ==================== 函数声明（统一接口）==================== */
void     AT24C02_Init(void);
uint8_t  AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data);
uint8_t  AT24C02_ReadByte(uint8_t WordAddress);

#endif /* __BSP_AT24C02_H__ */
