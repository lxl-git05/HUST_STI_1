#include "Mode_G.h"
#include "AllHeader.h"

Mode_Typedef curr_mode = Mode_Null   ;     // 当前模式
Mode_Typedef next_mode = Mode_Main   ;     // 下一个模式

// ========================== 系统setup loop ==========================

// 初始化
void Mode_G_Setup(void)
{
    // 全局初始化
    Initial_ALL() ;	
    // 定时器必须最后初始化!!!
    Initial_Timer() ;
}

// 循环loop
void Mode_G_Loop(void)
{
    // 检测程序是否可行
    if (Key_Check(KEY_0, KEY_SINGLE))// 单击
    {
        Flash_Mode_Set(Flash_Mode_Fast) ;   
    }
    // 进入下一个模式
    if (Key_Check(KEY_0, KEY_DOUBLE))// 双击
    {   
        Mode_To_Next() ;
    }
    // OLED展示
}

// ========================== 系统定时器配置 ==========================

// 1ms定时器
void Timer_1ms_Callback(void)
{
  // 功能1: 按键
	Key_Tick() ;
	// 功能2: LED闪烁指示灯
	Flash_Mode_Tick() ;
	// 功能3: 舵机控制台
	Con_Servo_GoalAngle_Tick() ;
}

// 20ms定时器
void Timer_20ms_Callback(void)
{
	// 1. 电机速度更新与PID控制
	Motor_Speed_Update_Tick(20) ;
	// 2. 展示电机参数
	if (curr_mode == Mode_PID  ) {Mode_1_Tick() ;}
	if (curr_mode == Mode_Angle) {Mode_2_Tick() ;}
	if (curr_mode == Mode_Main)  {Mode_4_Tick() ;}
}

// ========================== 系统状态配置 ==========================
// 进入下一状态
void Mode_To_Next(void)
{
    // Mode_End纯属标记模式尽头防止越界
    uint32_t next_val = (uint32_t)next_mode + 1;
    next_mode = (next_val == (uint32_t)Mode_End) ? Mode_Null : (Mode_Typedef)next_val;
}

// 将当前状态转换为:
void Mode_ChangeTo(Mode_Typedef nextmode)
{
    if (nextmode >= Mode_End) { return;}  

    next_mode = nextmode ;
}
