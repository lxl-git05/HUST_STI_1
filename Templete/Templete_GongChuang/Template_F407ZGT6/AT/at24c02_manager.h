#ifndef __AT24C02_MANAGER_H__
#define __AT24C02_MANAGER_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include "bsp_at24c02.h"

/* ==================== 参数类型枚举（值唯一，避免switch重复case）==================== */
typedef enum {
    AT_TYPE_INVALID = 0,
    AT_TYPE_INT8    = 1,
    AT_TYPE_INT16   = 2,
    AT_TYPE_INT32   = 3,
    AT_TYPE_FLOAT   = 4
} AT_ParamType;

/* ==================== 参数表项结构（字段顺序需匹配宏展开顺序）==================== */
typedef struct {
    AT_ParamType type;        // 类型（注册时自动识别）
    uint8_t      size;        // 字节数（1/2/4）
    void        *varPtr;     // 指向全局变量的指针
    union {
        int8_t   def_i8;
        int16_t  def_i16;
        int32_t  def_i32;
        float    def_f;
    } defVal;                 // 默认值
    uint8_t      atAddr;      // AT24C02存储地址（自动分配，注册时填0）
} AT_ParamItem;

/* ==================== 用户注册宏（按类型分开，size自动识别）====================
 * 示例：
 *   int32_t  g_speed = 100;
 *   { AT_PARAM_I32(&g_speed, 100) },    // size=4, type=INT32
 *
 *   float  g_kp = 1.5f;
 *   { AT_PARAM_F(&g_kp, 1.5f) },        // size=4, type=FLOAT
 *
 *   int8_t  g_mode = 0;
 *   { AT_PARAM_I8(&g_mode, 0) },         // size=1, type=INT8
 */
#define AT_PARAM_I8(_ptr, _def) \
    AT_TYPE_INT8, (uint8_t)sizeof(*(_ptr)), (void*)(_ptr), { .def_i8  = (int8_t)(_def) }, 0

#define AT_PARAM_I16(_ptr, _def) \
    AT_TYPE_INT16, (uint8_t)sizeof(*(_ptr)), (void*)(_ptr), { .def_i16 = (int16_t)(_def) }, 0

#define AT_PARAM_I32(_ptr, _def) \
    AT_TYPE_INT32, (uint8_t)sizeof(*(_ptr)), (void*)(_ptr), { .def_i32 = (int32_t)(_def) }, 0

#define AT_PARAM_F(_ptr, _def) \
    AT_TYPE_FLOAT, (uint8_t)sizeof(*(_ptr)), (void*)(_ptr), { .def_f  = (float)(_def) }, 0

/* ==================== API ==================== */
void    AT_Manager_Init(void);                     // 上电：读AT恢复，失败用默认值
uint8_t AT_Manager_Write(void *varPtr);            // 将变量值写入AT
uint8_t AT_Manager_Read(void *varPtr);             // 从AT读出写入变量
void    AT_Manager_SaveAll(void);                  // 批量写入所有参数
void    AT_Manager_LoadAll(void);                   // 批量从AT读取
void    AT_Manager_Register(const AT_ParamItem *item); // 注册一个参数项（地址自动分配）
uint8_t AT_Manager_GetCount(void);                  // 已注册参数数量
uint8_t AT_Manager_GetATAddr(void *varPtr);         // 通过变量指针反查AT地址

#endif /* __AT24C02_MANAGER_H__ */
