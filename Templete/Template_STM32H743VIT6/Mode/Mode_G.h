#ifndef __MODE_G_H
#define __MODE_G_H

#include "main.h"

typedef enum
{
    Mode_Null = 0U ,
		// 加模式在下面加即可,必须顺序添加,因为库的名字已经决定了其函数位置
    Mode_PID ,      // 模式1:调试电机的PID
		Mode_Angle ,    // 模式2:控制电机的旋转角度(PID)
    Mode_Check ,    // 模式3:测试硬件连接
		Mode_Main ,     // 模式4:控制主程序

    Mode_End ,      // 注册模式不能在这个下面!!!
}Mode_Typedef;

extern Mode_Typedef curr_mode ;    // 当前模式
extern Mode_Typedef next_mode ;    // 下一个模式

void Mode_To_Next(void) ;
void Mode_ChangeTo(Mode_Typedef nextmode) ;

void Mode_G_Setup(void) ;
void Mode_G_Loop(void) ;

#endif
