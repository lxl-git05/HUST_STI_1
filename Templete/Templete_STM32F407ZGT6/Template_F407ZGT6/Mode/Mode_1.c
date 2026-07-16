// ========================== 脱机调参模式 ==========================
// 用途：专门用于脱机调整参数并保存到存储（如 AT24C02）
// =================================================================
#include "AllHeader.h"

/* ==================== 参数表（地址自动分配，mode放第1个）==================== */
const AT_ParamItem s_AT_Params[] = {
		// 模式存储
    { AT_PARAM_I8(&curr_mode			, 1           )} ,   // addr=0,  int8_t,  default=0
		// 云台PID
    { AT_PARAM_F(&Stepper1.PID_Angle.Kp , 0.0f 		  )} ,
    { AT_PARAM_F(&Stepper1.PID_Angle.Ki , 0.0f 		  )} ,
    { AT_PARAM_F(&Stepper1.PID_Angle.Kd , 0.0f 		  )} ,
    { AT_PARAM_F(&Stepper2.PID_Angle.Kp , 0.0f 		  )} ,
    { AT_PARAM_F(&Stepper2.PID_Angle.Ki , 0.0f 		  )} ,
    { AT_PARAM_F(&Stepper2.PID_Angle.Kd , 0.0f 		  )} ,
		// 香橙派数据
		{ AT_PARAM_I16(&angle_shift			, 0          )} ,
		{ AT_PARAM_I16(&offset				, 0          )} ,
		{ AT_PARAM_I16(&black_h				, 0          )} ,
		{ AT_PARAM_I16(&black_s				, 0          )} ,
		{ AT_PARAM_I16(&black_v				, 0          )} ,
		// 各个任务的阈值
//		{ AT_PARAM_F(&Tar_XY_Tol_Distance , 8.0f)},
//		{ AT_PARAM_F(&Tar_XY_Tol_Speed	  , 5.0f)},
//		
//		{ AT_PARAM_F(&Down_Tar_Angle , 30)},
//		{ AT_PARAM_F(&Down_Tol_Angle , 5 )},
//		
//		{ AT_PARAM_F(&Back_Tar_Angle , 30)},
//		{ AT_PARAM_F(&Back_Speed_MAX , 60)},
//		{ AT_PARAM_F(&Back_Acc       ,  0)},
//		{ AT_PARAM_F(&Back_Tol_Angle ,  5)},
//		
//		{ AT_PARAM_F(&Elec_Wait	    ,1000)},
//		{ AT_PARAM_F(&Up_Tar_Angle  ,  30)},
//		{ AT_PARAM_F(&Up_Tol_Angle  ,   5)},
};	

int At_Size = sizeof(s_AT_Params)/sizeof(s_AT_Params[0]) ;

void Mode_1_Setup(void)
{
	OLED_Clear();
	// 初始化参数编辑器
	Param_Init();
	// Param_Register 内会自动检测 AT 关联并载入已存值
	Param_Register("curr_mode",&curr_mode,1,PARAM_INT8);
//	Param_Register("Stp1_Kp",  &Stepper1.PID_Angle.Kp,0.01f,PARAM_FLOAT);
//	Param_Register("Stp1_Kd",  &Stepper1.PID_Angle.Kd,0.01f,PARAM_FLOAT);
//	Param_Register("Stp2_Kp",  &Stepper2.PID_Angle.Kp,0.01f,PARAM_FLOAT);
//	Param_Register("Stp2_Kd",  &Stepper2.PID_Angle.Kd,0.01f,PARAM_FLOAT);
	// 香橙派数据
	Param_Register("angle_shift",  &angle_shift,1,PARAM_INT16);
	Param_Register("offset",   &offset ,1,PARAM_INT16);
	Param_Register("black_h",  &black_h,1,PARAM_INT16);
	Param_Register("black_s",  &black_s,1,PARAM_INT16);
	Param_Register("black_v",  &black_v,1,PARAM_INT16);
	// 任务阈值
//	Param_Register("Tar_XY_Tol_Distance",  &Tar_XY_Tol_Distance,1.0f,PARAM_FLOAT);
//	Param_Register("Tar_XY_Tol_Speed",  &Tar_XY_Tol_Speed,1.0f,PARAM_FLOAT);
//	Param_Register("Down_Tar_Angle",  &Down_Tar_Angle,0.5f,PARAM_FLOAT);
//	Param_Register("Down_Tol_Angle",  &Down_Tol_Angle,0.5f,PARAM_FLOAT);
//	Param_Register("Back_Tar_Angle",  &Back_Tar_Angle,1.0f,PARAM_FLOAT);
//	Param_Register("Back_Speed_MAX",  &Back_Speed_MAX,1.0f,PARAM_FLOAT);
//	Param_Register("Back_Acc",  &Back_Acc,1.0f,PARAM_FLOAT);
//	Param_Register("Back_Tol_Angle",  &Back_Tol_Angle,0.5f,PARAM_FLOAT);
//	Param_Register("Elec_Wait",  &Elec_Wait,1.0f,PARAM_FLOAT);
//	Param_Register("Up_Tar_Angle",  &Up_Tar_Angle,1.0f,PARAM_FLOAT);
//	Param_Register("Up_Tol_Angle",  &Up_Tol_Angle,1.0f,PARAM_FLOAT);


}

void Mode_1_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_1=====") ;
	// 非编辑模式就是正常工作
	if (!Param_IsActive())
	{
		// 要求香橙派发送数据
		if (Key_Check(KEY_0 , KEY_LONG))
		{
			// 要求传输信息
			Serial_printf(&Serial2 , "@start:6$#") ;
			// 蜂鸣器
			Buzzer_ON() ;
			HAL_Delay(500) ;
			Buzzer_OFF() ;
		}
	}
	// 应答
	if (Serial_Check_Str(&Serial2 , "Get") && Serial_GetNewPackageFlag_ABC(&Serial2))
	{
		// 蜂鸣器
		Buzzer_ON() ;
		HAL_Delay(500) ;
		Buzzer_OFF() ;
	}

	// Param_Loop 内部会自行处理 OLED 显示 (Param_Show)
	// 编辑模式下 Param_Show 绘制完整参数列表
	Param_Loop();
}

// 打印电机A参数
void Mode_1_Tick(void)
{

}

void Mode_1_Exit(void)
{
	
}
