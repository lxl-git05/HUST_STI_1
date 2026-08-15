#include "Mode_G.h"
#include "AllHeader.h"

Mode_Typedef curr_mode = Mode_Null   ;     // 当前模式
Mode_Typedef next_mode = Mode_2   ;     // 下一个模式

// ========================== 系统setup loop ==========================

// 初始化
void Mode_G_Setup(void)
{
    // 全局初始化
    Initial_ALL() ;
    // 定时器必须最后初始化!!!
    Initial_Timer() ;
    // ★ PARAM_FORCE：手动推送代码默认值到 AT24C02
    // 修改 C 默认值后，取消注释、改值、烧录一次，再重新注释
//		PARAM_FORCE(curr_mode, Mode_1);
//		PARAM_FORCE(IMU_Mahony_GyroBiasX, 0.0f);
//    PARAM_FORCE(IMU_Mahony_GyroBiasY, 0.0f);
//    PARAM_FORCE(IMU_Mahony_GyroBiasZ, 0.0f);
//    PARAM_FORCE(IMU_Mahony_AccBiasX, 0.0f);
//    PARAM_FORCE(IMU_Mahony_AccBiasY, 0.0f);
//    PARAM_FORCE(IMU_Mahony_AccBiasZ, 0.0f);
//
//    PARAM_FORCE(Motor_A.PID_s.Kp,      8.0f);
//    PARAM_FORCE(Motor_A.PID_s.Ki,      0.8f);
//    PARAM_FORCE(Motor_B.PID_s.Kp,      8.0f);
//    PARAM_FORCE(Motor_B.PID_s.Ki,      0.8f);
//    PARAM_FORCE(Motor_A.PID_Angle.Kp,  0.9f);
//    PARAM_FORCE(Motor_A.PID_Angle.Kd,  1.0f);
//    PARAM_FORCE(Motor_B.PID_Angle.Kp,  0.9f);
//    PARAM_FORCE(Motor_B.PID_Angle.Kd,  1.0f);
//
//    // 晾衣机器人脱机阈值默认值（首次烧录推送用，调通后保持注释）
//   PARAM_FORCE(Th_Hanger_Up,       0);
//   PARAM_FORCE(Th_Hanger_Mid,      1000);
//   PARAM_FORCE(Th_Hanger_Down,     6900);
//   PARAM_FORCE(Th_Trans_Step,      330);
//   PARAM_FORCE(Th_ClawA_Open,      54);
//   PARAM_FORCE(Th_ClawA_Close,     81);
//   PARAM_FORCE(Th_ClawB_Open,      97);
//   PARAM_FORCE(Th_ClawB_Close,     68);
//   PARAM_FORCE(Th_Hanger1_Open,    135);
//   PARAM_FORCE(Th_Hanger1_Close,   61);

    // ★ 从 AT24C02 恢复上次关机时的模式
    //    Param_AT24C02_Init 已将 EEPROM 值恢复到 curr_mode
    //    将其复制到 next_mode 并重置 curr_mode，让 Mymain 执行完整模式切换
    if (curr_mode > Mode_Null && curr_mode < Mode_End)
    {
        next_mode = curr_mode;
        curr_mode = Mode_Null;
    }
}

// 循环loop
void Mode_G_Loop(void)
{
    // 检测程序是否可行
    if (Key_Check(KEY_0, KEY_SINGLE))// 单击
    {
        Flash_Mode_Set(Flash_Mode_Fast) ;
				Mode_To_Next() ;
    }
    // OLED展示
    if (curr_mode == Mode_Null)
    {
        OLED_Printf(0,0,OLED_6X8,"===Mode_G===") ;
    }
		// 模式进入
		if (LCD_Cmd_Check("Mode_1")) { Mode_ChangeTo(Mode_1) ;}
		if (LCD_Cmd_Check("Mode_2")) { Mode_ChangeTo(Mode_2) ;}
		if (LCD_Cmd_Check("Mode_3")) { Mode_ChangeTo(Mode_3) ;}
		if (LCD_Cmd_Check("Mode_4")) { Mode_ChangeTo(Mode_4) ;}
		if (LCD_Cmd_Check("Mode_5")) { Mode_ChangeTo(Mode_5) ;}
		if (LCD_Cmd_Check("Mode_6")) { Mode_ChangeTo(Mode_6) ;}
}

// ========================== 系统定时器配置 ==========================

// 1ms定时器
void Timer_1ms_Callback(void)
{
  // 功能1: 按键
	Key_Tick() ;
	// 功能2: LED闪烁指示灯
	Flash_Mode_Tick() ;
	
}

// 20ms定时器
void Timer_20ms_Callback(void)
{
	// 0. IMU 陀螺仪姿态更新（20ms Mahony 解算）
	IMU_Mahony_Update_Tick();
	// 1. Con_Task 通用 Tick（无任务时自动跳过）
	Con_Task_Tick() ;
	// 2. 20ms定时器逻辑
	switch (curr_mode)
	{
			case Mode_Null : break;
			case Mode_1 : Mode_1_Tick() ; break;
			case Mode_2 : Mode_2_Tick() ; break;
			case Mode_3 : Mode_3_Tick() ; break;
			case Mode_4 : Mode_4_Tick() ; break;
			case Mode_5 : Mode_5_Tick() ; break;
			case Mode_6 : Mode_6_Tick() ; break;
			case Mode_End  : break;
	}
	// 3. 电机PID_Tick
	Motor_Speed_Update_Tick(20);
}

// ========================== 系统状态配置 ==========================
// 进入下一状态
void Mode_To_Next(void)
{
    // Mode_End纯属标记模式尽头防止越界
    uint32_t next_val = (uint32_t)next_mode + 1;
    next_mode = (next_val >= (uint32_t)Mode_End) ? Mode_Null : (Mode_Typedef)next_val;
    // ★ 模式记忆由 Mymain.c 在 curr_mode = next_mode 后统一保存
}

// 将当前状态转换为:
void Mode_ChangeTo(Mode_Typedef nextmode)
{
    if (nextmode >= Mode_End) { return;}  

    next_mode = nextmode ;
}
