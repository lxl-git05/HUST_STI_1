// ========================== 脱机调参模式 ==========================
// 用途：专门用于脱机调整参数并保存到存储（如 AT24C02）
// =================================================================
// H743 裁剪说明: F4 原有香橙派相关参数与按键业务已删除，仅保留
//   模式记忆 + IMU 零偏 + 电机双环 PID 三组参数
#include "AllHeader.h"

/* ==================== 参数表（地址自动分配，mode放第1个）==================== */
const AT_ParamItem s_AT_Params[] = {
		// 模式存储
    { AT_PARAM_I8(&curr_mode			, 1           )} ,   // addr=0,  int8_t,  default=1(首次上电默认进入调参模式)
		// 陀螺仪的M滤波6个参数（陀螺+加速度零偏）
    { AT_PARAM_F(&IMU_Mahony_GyroBiasX	   , 0.0f           )} ,
    { AT_PARAM_F(&IMU_Mahony_GyroBiasY	   , 0.0f           )} ,
    { AT_PARAM_F(&IMU_Mahony_GyroBiasZ	   , 0.0f           )} ,
    { AT_PARAM_F(&IMU_Mahony_AccBiasX	   , 0.0f           )} ,
    { AT_PARAM_F(&IMU_Mahony_AccBiasY	   , 0.0f           )} ,
    { AT_PARAM_F(&IMU_Mahony_AccBiasZ	   , 0.0f           )} ,
		// 晾衣机器人脱机阈值（Mode_4 业务）
    { AT_PARAM_I32(&Th_Hanger_Up        , 0     )} ,   // 丝杆顶位
    { AT_PARAM_I32(&Th_Hanger_Mid       , 1000  )} ,   // 丝杆中位
    { AT_PARAM_I32(&Th_Hanger_Down      , 6900  )} ,   // 丝杆低位
    { AT_PARAM_I32(&Th_Trans_Step       , 330   )} ,   // 传送带一格
    { AT_PARAM_I32(&Th_ClawA_Open       , 54    )} ,   // 夹爪A开
    { AT_PARAM_I32(&Th_ClawA_Close      , 81    )} ,   // 夹爪A闭
    { AT_PARAM_I32(&Th_ClawB_Open       , 97    )} ,   // 夹爪B开
    { AT_PARAM_I32(&Th_ClawB_Close      , 68    )} ,   // 夹爪B闭
    { AT_PARAM_I32(&Th_Hanger1_Open     , 135   )} ,   // 衣架1开
    { AT_PARAM_I32(&Th_Hanger1_Close    , 61    )} ,   // 衣架1闭
};

int At_Size = sizeof(s_AT_Params)/sizeof(s_AT_Params[0]) ;


void Mode_1_Setup(void)
{
	OLED_Clear();
	// 初始化参数编辑器
	Param_Init();
	// Param_Register 内会自动检测 AT 关联并载入已存值
//	Param_Register("curr_mode",&curr_mode,1,PARAM_INT8);
//	Param_Register("IMU_GX",&IMU_Mahony_GyroBiasX,0.01f,PARAM_FLOAT);
//	Param_Register("IMU_GY",&IMU_Mahony_GyroBiasY,0.01f,PARAM_FLOAT);
//	Param_Register("IMU_GZ",&IMU_Mahony_GyroBiasZ,0.01f,PARAM_FLOAT);
//	Param_Register("IMU_AX",&IMU_Mahony_AccBiasX,0.001f,PARAM_FLOAT);
//	Param_Register("IMU_AY",&IMU_Mahony_AccBiasY,0.001f,PARAM_FLOAT);
//	Param_Register("IMU_AZ",&IMU_Mahony_AccBiasZ,0.001f,PARAM_FLOAT);
	Param_Register("Hanger_Up",&Th_Hanger_Up,1,PARAM_INT32);
	Param_Register("Hanger_Mid",&Th_Hanger_Mid,1,PARAM_INT32);
	Param_Register("Hanger_Down",&Th_Hanger_Down,1,PARAM_INT32);
	Param_Register("Trans_Step",&Th_Trans_Step,1,PARAM_INT32);
	Param_Register("ClawA_Open",&Th_ClawA_Open,1,PARAM_INT32);
	Param_Register("ClawA_Close",&Th_ClawA_Close,1,PARAM_INT32);
	Param_Register("ClawB_Open",&Th_ClawB_Open,1,PARAM_INT32);
	Param_Register("ClawB_Close",&Th_ClawB_Close,1,PARAM_INT32);
	Param_Register("Hanger1_Open",&Th_Hanger1_Open,1,PARAM_INT32);
	Param_Register("Hanger1_Close",&Th_Hanger1_Close,1,PARAM_INT32);
}

void Mode_1_Loop(void)
{
	OLED_Printf(0, 0, OLED_6X8, "=====Mode_1=====") ;
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
