#include "at24c02_manager.h"

/* ==================== 私有常量 ==================== */
#define AT_MANAGER_MAX_PARAMS    30

/* ==================== 私有变量 ==================== */
static AT_ParamItem s_table[AT_MANAGER_MAX_PARAMS];  // 参数表
static uint8_t      s_count = 0;                     // 已注册参数数量
static uint8_t      s_nextAddr = 0;                  // 下一个待分配的AT地址

/* ==================== 内部函数 ==================== */

/**
 * @brief 通过变量指针查找参数项
 */
static AT_ParamItem* FindByPtr(void *varPtr)
{
    for (uint8_t i = 0; i < s_count; i++)
    {
        if (s_table[i].varPtr == varPtr)
            return &s_table[i];
    }
    return (AT_ParamItem*)0;
}

/**
 * @brief 分配一个AT地址（顺序递增）
 */
static uint8_t AllocAddr(uint8_t size)
{
    uint8_t addr = s_nextAddr;
    s_nextAddr += size;
    return addr;
}

/* ==================== 公共函数 ==================== */

/**
 * @brief 注册一个参数项（地址自动分配）
 * @note 用户应使用 AT_PARAM_I8/I16/I32/F 宏注册
 */
void AT_Manager_Register(const AT_ParamItem *item)
{
    if (s_count >= AT_MANAGER_MAX_PARAMS)
    {
        /* 参数表溢出，死锁提示 */
        while (1);
    }

    s_table[s_count].atAddr  = AllocAddr(item->size);
    s_table[s_count].varPtr  = item->varPtr;
    s_table[s_count].type    = item->type;
    s_table[s_count].size    = item->size;

    /* 复制默认值 */
    switch (item->type)
    {
        case AT_TYPE_INT8:   s_table[s_count].defVal.def_i8  = item->defVal.def_i8;  break;
        case AT_TYPE_INT16:  s_table[s_count].defVal.def_i16 = item->defVal.def_i16; break;
        case AT_TYPE_INT32:  s_table[s_count].defVal.def_i32 = item->defVal.def_i32; break;
        case AT_TYPE_FLOAT:  s_table[s_count].defVal.def_f   = item->defVal.def_f;   break;
        default: break;
    }

    s_count++;
}

/**
 * @brief 上电初始化：从AT恢复所有参数，失败则用默认值
 */
void AT_Manager_Init(void)
{
    AT24C02_Init();

    for (uint8_t i = 0; i < s_count; i++)
    {
        AT_ParamItem *p = &s_table[i];
        uint8_t success = 0;

        switch (p->type)
        {
            case AT_TYPE_INT8:
            {
                uint8_t v = AT24C02_ReadByte(p->atAddr);
                if (v != 0xFF)
                {
										// 解释:先取p -> varPtr (在注册时转为了void*格式)，然后再转为int8_t*，从4块Byte指向1块Byte
										// 最后再*解引用，get 同类型 (int8_t)v
										*(int8_t*)p->varPtr = (int8_t)v;	
                    success = 1;
                }
                break;
            }
            case AT_TYPE_INT16:
            {
                uint8_t b0 = AT24C02_ReadByte(p->atAddr);
                uint8_t b1 = AT24C02_ReadByte(p->atAddr + 1);
                if (b0 != 0xFF || b1 != 0xFF)
                {
                    *(int16_t*)p->varPtr = (int16_t)(b0 | (b1 << 8));
                    success = 1;
                }
                break;
            }
            case AT_TYPE_INT32:
            {
                uint8_t buf[4];
                uint8_t allFF = 1;
                for (uint8_t j = 0; j < 4; j++)
                {
                    buf[j] = AT24C02_ReadByte(p->atAddr + j);
                    if (buf[j] != 0xFF) allFF = 0;
                }
                if (!allFF)
                {
                    *(int32_t*)p->varPtr = (int32_t)(buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24));
                    success = 1;
                }
                break;
            }
            case AT_TYPE_FLOAT:
            {
                uint8_t buf[4];
                uint8_t allFF = 1;
                for (uint8_t j = 0; j < 4; j++)
                {
                    buf[j] = AT24C02_ReadByte(p->atAddr + j);
                    if (buf[j] != 0xFF) allFF = 0;
                }
                if (!allFF)
                {
                    /* float按字节存储，直接拷贝内存表示 */
                    uint8_t *dst = (uint8_t*)p->varPtr;
                    for (uint8_t j = 0; j < 4; j++)
                        dst[j] = buf[j];
                    success = 1;
                }
                break;
            }
            default: break;
        }

        /* 读取失败或AT内为0xFF，使用默认值 */
        if (!success)
        {
            switch (p->type)
            {
                case AT_TYPE_INT8:   *(int8_t*)p->varPtr  = p->defVal.def_i8;            break;
                case AT_TYPE_INT16:  *(int16_t*)p->varPtr = p->defVal.def_i16;           break;
                case AT_TYPE_INT32:  *(int32_t*)p->varPtr = p->defVal.def_i32;           break;
                case AT_TYPE_FLOAT:  *(float*)p->varPtr   = p->defVal.def_f;             break;
                default: break;
            }
        }
    }
}

/**
 * @brief 将变量的当前值写入AT（通过变量指针）
 * @return 1=成功, 0=失败/未注册
 */
uint8_t AT_Manager_Write(void *varPtr)
{
    AT_ParamItem *p = FindByPtr(varPtr);
    if (!p) return 0;

    switch (p->type)
    {
        case AT_TYPE_INT8:
            return AT24C02_WriteByte(p->atAddr, *(uint8_t*)p->varPtr);

        case AT_TYPE_INT16:
        {
            uint8_t b0 = ((uint8_t*)p->varPtr)[0];
            uint8_t b1 = ((uint8_t*)p->varPtr)[1];
            return AT24C02_WriteByte(p->atAddr,     b0)
                && AT24C02_WriteByte(p->atAddr + 1, b1);
        }
        case AT_TYPE_INT32:
        {
            uint8_t *bytes = (uint8_t*)p->varPtr;
            return AT24C02_WriteByte(p->atAddr,     bytes[0])
                && AT24C02_WriteByte(p->atAddr + 1, bytes[1])
                && AT24C02_WriteByte(p->atAddr + 2, bytes[2])
                && AT24C02_WriteByte(p->atAddr + 3, bytes[3]);
        }
        case AT_TYPE_FLOAT:
        {
            uint8_t *bytes = (uint8_t*)p->varPtr;
            return AT24C02_WriteByte(p->atAddr,     bytes[0])
                && AT24C02_WriteByte(p->atAddr + 1, bytes[1])
                && AT24C02_WriteByte(p->atAddr + 2, bytes[2])
                && AT24C02_WriteByte(p->atAddr + 3, bytes[3]);
        }
        default: return 0;
    }
}

/**
 * @brief 从AT读取值写入变量（通过变量指针）
 * @return 1=成功, 0=失败/未注册/数据为0xFF
 */
uint8_t AT_Manager_Read(void *varPtr)
{
    AT_ParamItem *p = FindByPtr(varPtr);
    if (!p) return 0;

    switch (p->type)
    {
        case AT_TYPE_INT8:
        {
            uint8_t v = AT24C02_ReadByte(p->atAddr);
            if (v == 0xFF) return 0;
            *(int8_t*)p->varPtr = (int8_t)v;
            break;
        }
        case AT_TYPE_INT16:
        {
            uint8_t b0 = AT24C02_ReadByte(p->atAddr);
            uint8_t b1 = AT24C02_ReadByte(p->atAddr + 1);
            if (b0 == 0xFF && b1 == 0xFF) return 0;
            *(int16_t*)p->varPtr = b0 | (b1 << 8);
            break;
        }
        case AT_TYPE_INT32:
        {
            uint8_t buf[4];
            for (uint8_t i = 0; i < 4; i++)
                buf[i] = AT24C02_ReadByte(p->atAddr + i);
            if (buf[0]==0xFF && buf[1]==0xFF && buf[2]==0xFF && buf[3]==0xFF) return 0;
            *(int32_t*)p->varPtr = (int32_t)(buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24));
            break;
        }
        case AT_TYPE_FLOAT:
        {
            uint8_t buf[4];
            for (uint8_t i = 0; i < 4; i++)
                buf[i] = AT24C02_ReadByte(p->atAddr + i);
            if (buf[0]==0xFF && buf[1]==0xFF && buf[2]==0xFF && buf[3]==0xFF) return 0;
            uint8_t *dst = (uint8_t*)p->varPtr;
            for (uint8_t i = 0; i < 4; i++)
                dst[i] = buf[i];
            break;
        }
        default: return 0;
    }
    return 1;
}

/**
 * @brief 批量保存所有参数到AT
 */
void AT_Manager_SaveAll(void)
{
    for (uint8_t i = 0; i < s_count; i++)
        AT_Manager_Write(s_table[i].varPtr);
}

/**
 * @brief 批量从AT加载所有参数
 */
void AT_Manager_LoadAll(void)
{
    for (uint8_t i = 0; i < s_count; i++)
        AT_Manager_Read(s_table[i].varPtr);
}

/**
 * @brief 获取已注册参数数量
 */
uint8_t AT_Manager_GetCount(void)
{
    return s_count;
}

/**
 * @brief 通过变量指针反查AT地址（调试用）
 * @return AT地址，未注册返回0xFF
 */
uint8_t AT_Manager_GetATAddr(void *varPtr)
{
    AT_ParamItem *p = FindByPtr(varPtr);
    return p ? p->atAddr : 0xFF;
}
