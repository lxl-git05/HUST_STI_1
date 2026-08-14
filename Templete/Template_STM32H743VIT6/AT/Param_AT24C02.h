#ifndef __Param_AT24C02_H
#define __Param_AT24C02_H

/* Includes ------------------------------------------------------------------*/
#include "at24c02_manager.h"

/* ==================== 业务全局变量（extern供其他模块用）==================== */
extern int32_t  g_motorSpeed;
extern float    g_pidKp;
extern float    g_pidKi;
extern float    g_pidKd;
extern int8_t   g_mode;

/* ==================== 手动推送宏 ==================== */

/**
 * @brief 强制设置变量值并写入 AT24C02
 * @note  用于固件升级后手动推送新的默认值到 EEPROM。
 *        等效于: var = val; Param_AT24C02_Write(&var);
 *        调通后应注释掉，避免每次上电覆盖用户调好的值。
 *
 * 使用示例:
 *   PARAM_FORCE(curr_mode, 1);
 *   PARAM_FORCE(angle_shift, 50);
 *   PARAM_FORCE(Stepper1.PID_Angle.Kp, 0.217f);
 */
#define PARAM_FORCE(var, val)  do { (var) = (val); Param_AT24C02_Write(&(var)); } while(0)

/* ==================== API ==================== */
void    Param_AT24C02_Init(void);                     // 注册+上电恢复
void    Param_AT24C02_EraseAll(void);                // 擦除整个AT24C02内存（填0xFF）
void    Param_AT24C02_SaveAll(void);                 // 批量保存到AT
uint8_t Param_AT24C02_Write(void *varPtr);            // 写单个变量到AT
uint8_t Param_AT24C02_Read(void *varPtr);             // 从AT读单个变量

#endif
