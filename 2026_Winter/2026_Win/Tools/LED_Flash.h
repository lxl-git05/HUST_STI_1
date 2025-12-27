#ifndef __LED_FLASH_H
#define __LED_FLASH_H

#include "main.h"

// =========== 枚举类型 ===========
// LED闪烁模式
typedef enum
{
	LED_Flash_ON ,							// LED常亮
	LED_Flash_OFF,							// LED常灭
	LED_Flash_Slow , 						// 慢闪 0.5灭 + 0.5亮
	LED_Flash_Fast ,						// 快闪 0.1亮 + 0.1灭
	LED_Flash_Topp							// 瞬闪 0.9灭 + 0.1亮
}LED_Flash_Mode_Typedef ;

// =========== 函数 ===========

// LED闪烁模式初始化
void LED_Flash_Mode_Init(void);
// LED状态更新,放在1ms中断
void LED_Flash_Mode_Tick(void);
// LED闪烁模式设置
void LED_Flash_Mode_Set_Mode(LED_Flash_Mode_Typedef Mode);

#endif
