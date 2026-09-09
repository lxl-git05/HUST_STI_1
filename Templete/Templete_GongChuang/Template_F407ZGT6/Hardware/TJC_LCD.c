#include "TJC_LCD.h"
#include "Serial_porting.h"
#include <string.h>
#include <stdio.h>

// ============== 按键单击检测（类似 Key_Check(KEY_X, KEY_SINGLE)）==============
bool LCD_Key_Check(uint8_t key_num)
{
    if (key_num < 1 || key_num > TJC_LCD_KEY_MAX) return false;
    if (!Serial_GetNewPackageFlag_ABC(&TJC_LCD_SERIAL)) return false;

    char expected[16];
    snprintf(expected, sizeof(expected), "LCD_KEY_%d", key_num);

    if (strcmp(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC, expected) == 0)
        return true;   // 匹配，flag 已消费

    // 不匹配，恢复 flag 给其他函数
    TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_Flag = 1;
    return false;
}

// ============== 滑块整数映射（0-100 → [min, max]）==============
bool LCD_Set_Int(uint8_t param_id, int *value, int min, int max)
{
    if (param_id < 1 || param_id > TJC_LCD_PARAM_MAX) return false;
    if (!Serial_GetNewPackageFlag_ABC(&TJC_LCD_SERIAL)) return false;

    char prefix[16];
    snprintf(prefix, sizeof(prefix), "LCD_Param_%d=", param_id);

    int id = 0, raw = 0;
    if (strstr(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC, prefix) != NULL)
    {
        sscanf(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC,
               "LCD_Param_%d=%d", &id, &raw);
        if (id == param_id)
        {
            if (raw < 0)   raw = 0;
            if (raw > 100) raw = 100;
            *value = min + (int)((raw / 100.0f) * (max - min) + 0.5f);
            return true;
        }
    }

    TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_Flag = 1;
    return false;
}

// ============== 滑块浮点映射（0-100 → [min, max]）==============
bool LCD_Set_Float(uint8_t param_id, float *value, float min, float max)
{
    if (param_id < 1 || param_id > TJC_LCD_PARAM_MAX) return false;
    if (!Serial_GetNewPackageFlag_ABC(&TJC_LCD_SERIAL)) return false;

    char prefix[16];
    snprintf(prefix, sizeof(prefix), "LCD_Param_%d=", param_id);

    int id = 0, raw = 0;
    if (strstr(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC, prefix) != NULL)
    {
        sscanf(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC,
               "LCD_Param_%d=%d", &id, &raw);
        if (id == param_id)
        {
            if (raw < 0)   raw = 0;
            if (raw > 100) raw = 100;
            *value = min + (raw / 100.0f) * (max - min);
            return true;
        }
    }

    TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_Flag = 1;
    return false;
}

// ============== 通用指令子串匹配 ==============
bool LCD_Cmd_Check(char *keyword)
{
    if (!Serial_GetNewPackageFlag_ABC(&TJC_LCD_SERIAL)) return false;

    if (strstr(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC, keyword) != NULL)
        return true;

    TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_Flag = 1;
    return false;
}

// ============== 波形发送：MCU → LCD（TJC 原生命令，\xFF\xFF\xFF 终止）==============
void TJC_LCD_Wave_Send_Float(uint8_t ch, float value)
{
    if (ch > 3) return;
    char buf[48];
    int len = snprintf(buf, sizeof(buf),
        "data%d.val=%d\xFF\xFF\xFF", ch, (int)(value*100));
    if (len > 0 && len < (int)sizeof(buf))
        Serial_SendBytes(&TJC_LCD_SERIAL, (uint8_t *)buf, (uint16_t)len);
}

// ============== ABC Float100 参数接收：LCD → MCU ==============
bool LCD_Get_ABC_Float100(char *keyword, float *value)
{
    if (keyword == NULL || value == NULL) return false;
    if (!Serial_GetNewPackageFlag_ABC(&TJC_LCD_SERIAL)) return false;

    // 构造期望前缀: "Kp=", "Ki=", "Kd=", "Goal="
    char expected[16];
    snprintf(expected, sizeof(expected), "%s=", keyword);

    if (strstr(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC, expected) != NULL)
    {
        int int_val = 0;
        // %*[^=] 跳过 '=' 之前所有字符, = 匹配 '=', %d 读取整数
        sscanf(TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_ABC,
               "%*[^=]=%d", &int_val);
        *value = int_val / 100.0f;
        return true;
    }

    // 不匹配：恢复 flag 给后续函数
    TJC_LCD_SERIAL.ABC_Data.Serial_New_Package_Flag = 1;
    return false;
}
