#include "Mode_2.h"
#include "AllHeader.h"

// ====================================================================
// 测试变量
// ====================================================================
static uint32_t led_toggle_cnt = 0;        // LED翻转次数计数
static uint32_t key0_press_cnt = 0;        // Key0按下次数计数

void Mode_2_Setup(void)
{
    OLED_Clear();
}

void Mode_2_Loop(void)
{
    // 显示标题
    OLED_Printf(0, 0, OLED_6X8, "=====Mode_2=====");

    // 显示主频
    OLED_Printf(0, 10, OLED_6X8, "Fre:%dHz", MySystem_Fre);

    // 显示 LED 翻转次数（每 Tick 20ms 翻转一次）
    OLED_Printf(0, 20, OLED_6X8, "LED_Toggle:%d", led_toggle_cnt);

    // 显示 Key0 按下次数
    OLED_Printf(0, 30, OLED_6X8, "Key0_Press:%d", key0_press_cnt);

    // 显示 Key0 当前状态
    int key0_state = My_GPIO_ReadPin(&MyGPIO_Key0);
    OLED_Printf(0, 40, OLED_6X8, "Sta:%d", key0_state);
}

// 20ms Tick — LED 翻转 + Key 检测
void Mode_2_Tick(void)
{
    static uint8_t led_state = 0;

    // LED 翻转（每 tick 20ms 翻转一次 = 25Hz 闪烁）
    led_state = !led_state;
    My_GPIO_WritePin(&MyGPIO_LED0, led_state);
    led_toggle_cnt++;

    // Key0 下降沿检测（按下时计数）
    static uint8_t key0_last = 1;
    uint8_t key0_now = My_GPIO_ReadPin(&MyGPIO_Key0);
    if (key0_last == 1 && key0_now == 0) 
		{
        key0_press_cnt++;
    }
    key0_last = key0_now;
}

void Mode_2_Exit(void)
{
    // 退出时关闭 LED
    My_GPIO_WritePin(&MyGPIO_LED0, 0);
}
