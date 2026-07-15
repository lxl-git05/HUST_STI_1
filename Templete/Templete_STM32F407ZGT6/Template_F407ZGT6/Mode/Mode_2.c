#include "Mode_2.h"
#include "AllHeader.h"

static int param1 = 0;

void Mode_2_Setup(void)
{
    OLED_Clear();
    param1 = 0;
}

void Mode_2_Loop(void)
{
    // ---- 标题 ----
    OLED_Printf(0, 0, OLED_6X8, "=== Serial4 Test ===");

    // ---- 检测 ABC 新包，解析 int 参数 ----
    if (Serial_GetNewPackageFlag_ABC(&Serial4))
    {
        // 解析三个 int 参数（p1 / p2 / p3）
        Serial_SetIntData(&Serial4, "Move", "Move=%d", &param1);
    }

    // ---- OLED 显示 ----
    // 第1行: 原始 ABC 字符串（截取前21字符适配 128px 宽屏）
    OLED_Printf(0, 10, OLED_6X8, "ABC: %.21s",
                Serial4.ABC_Data.Serial_New_Package_ABC);

    // 第3行: 解析出的 int 参数
    OLED_Printf(0, 28, OLED_6X8, "Move=%d", param1);
}

void Mode_2_Tick(void)
{

}

void Mode_2_Exit(void)
{
    OLED_Clear();
}
