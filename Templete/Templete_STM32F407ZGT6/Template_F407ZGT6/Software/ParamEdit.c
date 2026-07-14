#include "ParamEdit.h"
#include "Encoder_Key.h"

#if PARAM_USE_AT24C02
#include "at24c02_manager.h"
#endif

#include "Orange.h"

// ============= 参数结构体 =============
typedef struct
{
    const char *name;           // 参数名称
    void       *var;            // 变量指针
    float       step;           // 步长
    ParamType   type;           // 变量类型
    uint8_t     is_at;          // 1=此参数已注册在AT24C02表中
    uint8_t     dirty;          // 1=已修改但未保存到AT（仅is_at=1时有效）
} ParamItem;

// ============= OLED 显示相关 =============
#define OLED_MAX_SHOW  4        // OLED 最多同时显示 4 行参数

static int is_active = 0;       // 是否处于参数编辑模式
static int cursor    = 0;       // 当前选中参数索引
static int OLED_Start = 0;      // OLED 滚动起始索引

// ============= 参数表 =============
static ParamItem ParamList[MAX_PARAM];
static int ParamCount = 0;      // 已注册参数个数

// ============= 初始化 =============
void Param_Init(void)
{
    ParamCount = 0;
    cursor     = 0;
    OLED_Start = 0;
    is_active  = 0;
}

// ============= 清空参数表 =============
void Param_Clear(void)
{
    ParamCount = 0;
    cursor     = 0;
    OLED_Start = 0;
}

// ============= 查询编辑状态 =============
int Param_IsActive(void)
{
    return is_active;
}

// ============= 注册参数 =============
void Param_Register(const char *name, void *var, float step, ParamType type)
{
    if (ParamCount >= MAX_PARAM) return;

    ParamItem *item = &ParamList[ParamCount];
    item->name  = name;
    item->var   = var;
    item->step  = step;
    item->type  = type;
    item->is_at = 0;
    item->dirty = 0;

#if PARAM_USE_AT24C02
    // 用变量指针反查AT表：命中则标记is_at，并从AT加载存储值
    if (AT_Manager_GetATAddr(var) != 0xFF)
    {
        item->is_at = 1;
        AT_Manager_Read(var);   // 从EEPROM恢复值（若AT已初始化则覆盖当前值）
    }
#endif

    ParamCount++;
}

// ============= OLED 显示参数列表（含步长）=============
static void Param_Show(void)
{
    OLED_Clear();

    // 标题行
    OLED_Printf(0, 0, OLED_6X8, "=== Param Edit ===");

    if (ParamCount == 0)
    {
        OLED_Printf(0, 16, OLED_6X8, "No Params");
        // OLED_Update() 由 Mymain 末尾统一调用，此处不单独刷新
        return;
    }

    // 计算可见范围
    int end = OLED_Start + OLED_MAX_SHOW;
    if (end > ParamCount) end = ParamCount;

    // 逐行显示参数
    for (int i = OLED_Start, line = 0; i < end; i++, line++)
    {
        ParamItem *item = &ParamList[i];
        int y = (line + 1) * 11 + 1;  // 第1行=12, 第2行=23, ...

        // 光标标记
        (i == cursor)
            ? OLED_Printf(0, y, OLED_6X8, ">")
            : OLED_Printf(0, y, OLED_6X8, " ");

        // 脏标记：AT参数被修改但未保存时显示 *
        const char *dirty_mark = "";
#if PARAM_USE_AT24C02
        if (item->is_at && item->dirty)
            dirty_mark = "*";
#endif

        // 参数名 + 值 + 步长
        if (item->type == PARAM_FLOAT)
        {
            OLED_Printf(8, y, OLED_6X8, "%s:%s%.2f st:%.4g",
                        item->name, dirty_mark, *(float*)item->var, item->step);
        }
        else
        {
            int32_t val = 0;
            switch (item->type)
            {
                case PARAM_INT8:  val = *(int8_t*)item->var;  break;
                case PARAM_INT16: val = *(int16_t*)item->var; break;
                case PARAM_INT32: val = *(int32_t*)item->var; break;
                default: break;
            }
            OLED_Printf(8, y, OLED_6X8, "%s:%s%d st:%.4g",
                        item->name, dirty_mark, val, item->step);
        }
    }

    // 底部提示
#if PARAM_USE_AT24C02
    OLED_Printf(0, 55, OLED_6X8, "LONG=Exit  KEY3=Save");
#else
    OLED_Printf(0, 55, OLED_6X8, "LONG=Exit");
#endif

    // OLED_Update() 由 Mymain 末尾统一调用，此处不单独刷新
}

// ============= 参数值增加 =============
static void Param_Add(ParamItem *item, float amount)
{
    switch (item->type)
    {
        case PARAM_INT8:
            *(int8_t*)item->var += (int8_t)amount;
            break;
        case PARAM_INT16:
            *(int16_t*)item->var += (int16_t)amount;
            break;
        case PARAM_INT32:
            *(int32_t*)item->var += (int32_t)amount;
            break;
        case PARAM_FLOAT:
            *(float*)item->var += amount;
            break;
    }

#if PARAM_USE_AT24C02
    if (item->is_at)
        item->dirty = 1;    // AT参数被修改 → 标记待保存
#endif
}

// ============= 滚动对齐 =============
static void Param_AdjustScroll(void)
{
    if (cursor < OLED_Start)
        OLED_Start = cursor;
    if (cursor >= OLED_Start + OLED_MAX_SHOW)
        OLED_Start = cursor - OLED_MAX_SHOW + 1;
}

// ============= 光标上移（到底则回绕到末尾）=============
static void Param_CursorUp(void)
{
    if (cursor > 0)
        cursor--;
    else
        cursor = ParamCount - 1;    // 回绕到末尾

    Param_AdjustScroll();
}

// ============= 光标下移（到末尾则回绕到开头）=============
static void Param_CursorDown(void)
{
    if (cursor < ParamCount - 1)
        cursor++;
    else
        cursor = 0;                 // 回绕到开头

    Param_AdjustScroll();
}

// ============= 主循环（状态机）=============
void Param_Loop(void)
{
    // ---- 始终检测：长按进入/退出 ----
    if (Key_Check(PARAM_KEY_ENTER_EXIT, KEY_LONG))
    {
        is_active = !is_active;

        if (is_active)
        {
            // 进入编辑模式：丢弃编码器积攒值，避免参数跳变
            Encoder_Get();
        }
        else
        {
            // 退出编辑模式：清屏
            OLED_Clear();
            // OLED_Update() 由 Mymain 末尾统一调用
        }
        return;     // 本周期不处理其他事件
    }

    // ---- 非编辑模式：什么都不做 ----
    if (!is_active)
        return;

    // ======== 以下为编辑模式 ========

    // ---- 编码器旋转：修改当前参数值 ----
    if (ParamCount > 0)
    {
        int16_t delta = Encoder_Get();      // 读后自动清零
        if (delta != 0)
        {
            ParamItem *item = &ParamList[cursor];
            Param_Add(item, item->step * delta);
        }
    }

    // ---- 按键：切换参数 ----
    if (ParamCount > 0)
    {
        if (Key_Check(PARAM_KEY_NEXT, KEY_SINGLE))
        {
            Param_CursorDown();     // 单击 → 下一个参数
        }
        else if (Key_Check(PARAM_KEY_PREV, KEY_DOUBLE))
        {
            Param_CursorUp();       // 双击 → 上一个参数
        }
    }

    // ---- 编码器按键：保存AT参数 ----
#if PARAM_USE_AT24C02
    if (Key_Check(PARAM_KEY_SAVE, KEY_SINGLE) && ParamCount > 0)
    {
        ParamItem *item = &ParamList[cursor];
        if (item->is_at && item->dirty)
        {
            AT_Manager_Write(item->var);   // 写入EEPROM
            item->dirty = 0;               // 清除脏标记
        }
        // 香橙派处理
        Oran_Send_Data((int*)item->var) ;
    }
#endif

    // ---- 刷新 OLED ----
    Param_Show();
}
