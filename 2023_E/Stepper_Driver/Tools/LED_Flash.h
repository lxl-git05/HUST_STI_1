#ifndef __LED_FLASH_H
#define __LED_FLASH_H

#include "MySystem.h"

// =========== 枚举类型 ===========
// LED闪烁模式
typedef enum
{
	Flash_Mode_ON ,							// LED常亮
	Flash_Mode_OFF,							// LED常灭
	Flash_Mode_Slow , 					// 慢闪 0.5灭 + 0.5亮
	Flash_Mode_Fast ,						// 快闪 0.1亮 + 0.1灭
	Flash_Mode_Topp							// 瞬闪 0.9灭 + 0.1亮
}Flash_Mode_Typedef ;

// =========== 函数 ===========

// LED闪烁模式初始化
void Flash_Mode_Init(void);
// LED状态更新,放在1ms中断
void Flash_Mode_Tick(void);
// LED闪烁模式设置
void Flash_Mode_Set(Flash_Mode_Typedef Mode);

#endif
