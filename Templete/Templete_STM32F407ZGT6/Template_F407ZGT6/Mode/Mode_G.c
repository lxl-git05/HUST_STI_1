#include "Mode_G.h"
#include "AllHeader.h"

Mode_Typedef curr_mode = Mode_Null   ;     // 当前模式
Mode_Typedef next_mode = Mode_Null      ;     // 下一个模式

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
//		PARAM_FORCE(IMU_Mahony_GyroBiasX, -9.26840305f);
//    PARAM_FORCE(IMU_Mahony_GyroBiasY, 0.428176761f);
//    PARAM_FORCE(IMU_Mahony_GyroBiasZ, -1.27142811f);

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
    }
    // 进入下一个模式
    if (Key_Check(KEY_0, KEY_DOUBLE))// 双击
    {
        Mode_To_Next() ;
    }
		// 进入比赛模式
//		if (Serial_Check_Str(&Serial4 , "Con_Mode_1")){Mode_ChangeTo(Con_Mode_1); }
//		if (Serial_Check_Str(&Serial4 , "Con_Mode_2")){Mode_ChangeTo(Con_Mode_2); }
//		if (Serial_Check_Str(&Serial4 , "Con_Mode_3")){Mode_ChangeTo(Con_Mode_3); }
//		if (Serial_Check_Str(&Serial4 , "Con_Mode_4")){Mode_ChangeTo(Con_Mode_4); }
//		if (Serial_Check_Str(&Serial4 , "Con_Mode_5")){Mode_ChangeTo(Con_Mode_5); }
//		if (Serial_Check_Str(&Serial4 , "Con_Mode_6")){Mode_ChangeTo(Con_Mode_6); }

    // OLED展示
    if (curr_mode == Mode_Null)
    {
        OLED_Printf(0,0,OLED_6X8,"===Mode_G===") ;
    }
}

// ========================== 系统定时器配置 ==========================

// 1ms定时器
void Timer_1ms_Callback(void)
{

  // 功能1: 按键
	Key_Tick() ;
	// 功能2: LED闪烁指示灯
	Flash_Mode_Tick() ;
	// 功能3: 步进电机加速度Tick（1ms丝滑ramp）
	Stepper_PWM_Speed_Tick(&Stepper1);
	Stepper_PWM_Speed_Tick(&Stepper2);
	// 功能4: 步进电机位置模式Tick（1ms速度ramp + 阶段切换）
	Stepper_PWM_Pos_Tick(&Stepper1);
	Stepper_PWM_Pos_Tick(&Stepper2);

}

// 20ms定时器
void Timer_20ms_Callback(void)
{
    // 0. IMU 陀螺仪姿态更新（20ms Mahony 解算）
    IMU_Mahony_Update_Tick();
    // 1. 香橙派更新
    Oran_Update() ;
    // 2. Con_Task 通用 Tick（无任务时自动跳过）
    Con_Task_Tick() ;
    // 3. 20ms定时器逻辑
    switch (curr_mode)
	{
			case Mode_Null : break;
			case 1 : Mode_1_Tick() ; break;
			case 2 : Mode_2_Tick() ; break;
			case 3 : Mode_3_Tick() ; break;
			case 4 : Mode_4_Tick() ; break;
			case 5 : Mode_5_Tick() ; break;
			case 6 : Mode_6_Tick() ; break;
//			case Con_Mode_1 : Con_Mode_1_Tick() ; break;
//			case Con_Mode_2 : Con_Mode_2_Tick() ; break;
//			case Con_Mode_3 : Con_Mode_3_Tick() ; break;
//			case Con_Mode_4 : Con_Mode_4_Tick() ; break;
//			case Con_Mode_5 : Con_Mode_5_Tick() ; break;
//			case Con_Mode_6 : Con_Mode_6_Tick() ; break;
			default: break;
	}
	// 4. 电机PID_Tick
	Motor_Speed_Update_Tick(20);
}

// ========================== 步进电机脉冲计数（重写MyTimer弱回调） ==========================

void Timer_Stepper1_Pulse_Callback(void)
{
    Stepper_PWM_Pulse_Count(&Stepper1);
}

void Timer_Stepper2_Pulse_Callback(void)
{
    Stepper_PWM_Pulse_Count(&Stepper2);
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
