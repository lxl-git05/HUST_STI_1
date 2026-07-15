#ifndef __MODE_G_H
#define __MODE_G_H

#include "main.h"

typedef enum
{
    Mode_Null = 0U ,
		// 加模式在下面加即可,必须顺序添加,因为库的名字已经决定了其函数位置
    Mode_1 ,      // 模式1:调试电机的PID
		Mode_2 ,    		// 模式2:控制电机的旋转角度(PID)
    Mode_3 ,    		// 模式3:测试硬件连接
		Mode_4 ,     		// 模式4:电机PID调参
		Mode_5 ,        // 模式5:业务逻辑
		Mode_6 ,        // 模式6:业务逻辑
		Con_Mode_1 ,    // 电机PID调参
		Con_Mode_2 ,    // 业务逻辑
		Con_Mode_3 ,    // 业务逻辑
		Con_Mode_4 ,    // 业务逻辑
		Con_Mode_5 ,    // 业务逻辑
		Con_Mode_6 ,    // 业务逻辑

    Mode_End ,      // 注册模式不能在这个下面!!!
}Mode_Typedef;

extern Mode_Typedef curr_mode ;    // 当前模式
extern Mode_Typedef next_mode ;    // 下一个模式

void Mode_To_Next(void) ;
void Mode_ChangeTo(Mode_Typedef nextmode) ;

void Mode_G_Setup(void) ;
void Mode_G_Loop(void) ;

#endif
