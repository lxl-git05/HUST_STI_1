// Robot_Task.c — 晾衣机器人业务库实现
// 业务: 晾衣第1轮 + 复位；阈值: LCD 脱机示教 + AT24C02 持久化
// 命令: ABC 帧（Serial4=LCD / Serial1=调试 同集），完整帧格式 @cmd$#
// 任务: 全部走全局任务表（Control.c 的 TASK_MOTOR_TO / TASK_SERVO_SET）
#include "AllHeader.h"
#include "Robot_Task.h"
#include <string.h>
#include <stdio.h>

// ==================== 1. 阈值定义（默认值 = V2 实测）====================
int32_t Th_Hanger_Up        = 0;        // 丝杆顶位
int32_t Th_Hanger_Mid       = 1000;     // 丝杆中位
int32_t Th_Hanger_Down      = 6900;     // 丝杆低位
int32_t Th_Trans_Step       = 330;      // 传送带一格
int32_t Th_ClawA_Open       = 54;       // 夹爪A开
int32_t Th_ClawA_Close      = 81;       // 夹爪A闭
int32_t Th_ClawB_Open       = 97;       // 夹爪B开
int32_t Th_ClawB_Close      = 68;       // 夹爪B闭
int32_t Th_Hanger1_Open     = 134;      // 衣架1开
int32_t Th_Hanger1_Close    = 61;       // 衣架1闭

// ==================== 2. 初始化 ====================
void Robot_Task_Init(void)
{
    Con_Task_Init(Control_TaskTable, TASK_COUNT);   // 注册全局任务表
}

// ==================== 3. 序列构建（只引用 Th_* 与 ROBOT_* 常量）====================

// 晾衣第1轮入队（内部使用，外部走 Robot_Hang_Try 做空闲判定）
void Robot_Hang_Enqueue(void)
{
		Serial_printf(&Serial3 , "@Car_Start$#") ;
    Flash_Mode_Set(Flash_Mode_Slow);

    // ① 丝杆下降到底（夹爪此时已夹住衣架）
    Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Down, ROBOT_ANGLE_TOL_DEFAULT, 0);
    // ② 夹爪闭合
    Con_Task_Enqueue(TASK_CLAW_SET, Th_ClawA_Close, Th_ClawB_Close, ROBOT_SERVO_HOLD_CLAW_CLOSE_MS, 0);
    // ③ 丝杆升中位（挂上晾衣杆）
    Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Mid, ROBOT_ANGLE_TOL_DEFAULT, 0);
    // ④ 衣架1打开
    Con_Task_Enqueue(TASK_SERVO_SET, ROBOT_SERVO_HANGER_1, Th_Hanger1_Open, ROBOT_SERVO_HOLD_HANGER_MS, 0);
    // ⑤ 夹爪张开
    Con_Task_Enqueue(TASK_CLAW_SET, Th_ClawA_Open, Th_ClawB_Open, ROBOT_SERVO_HOLD_CLAW_OPEN_MS, 0);
    // ⑥ 丝杆升顶
    Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Up, ROBOT_ANGLE_TOL_DEFAULT, 0);
    // ⑦ 传送带步进+1格（相对当前角度，第2轮天然接续）
    Con_Task_Enqueue(TASK_MOTOR_TO, 0, (int)Motor_Get_Angle(&Motor_A) + Th_Trans_Step,
                     ROBOT_ANGLE_TOL_DEFAULT, 0);
    // ⑧ 队列自然清空即结束（无 DONE 停留，完成提示由 Mode_4 状态机负责）
}

// 复位：机械回位（任何状态可用，先清队列）
void Robot_Reset_Start(void)
{
    Con_Task_Clear();
    Flash_Mode_Set(Flash_Mode_OFF);

    // ① 丝杆升顶
    Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Up, ROBOT_ANGLE_TOL_DEFAULT, 0);
    // ② 夹爪张开
    Con_Task_Enqueue(TASK_CLAW_SET, Th_ClawA_Open, Th_ClawB_Open, ROBOT_SERVO_HOLD_CLAW_OPEN_MS, 0);
    // ③ 衣架1闭合
    Con_Task_Enqueue(TASK_SERVO_SET, ROBOT_SERVO_HANGER_1, Th_Hanger1_Close, ROBOT_SERVO_HOLD_HANGER_MS, 0);
    // ④ 传送带回 0（绝对定位）
    Con_Task_Enqueue(TASK_MOTOR_TO, 0, 0, ROBOT_ANGLE_TOL_DEFAULT, 0);
}

// 收衣服：任何状态可用（先清队列）。序列：① 传送带回原位 ② 丝杆下移 ③ 松开夹爪
void Robot_Shou_Start(void)
{
    Con_Task_Clear();
    Flash_Mode_Set(Flash_Mode_OFF);

    // ① 传送带回 0（绝对定位）
    Con_Task_Enqueue(TASK_MOTOR_TO, 0, 0, ROBOT_ANGLE_TOL_DEFAULT, 0);
    // ② 衣架1松开
    Con_Task_Enqueue(TASK_SERVO_SET, ROBOT_SERVO_HANGER_1, Th_Hanger1_Close, ROBOT_SERVO_HOLD_HANGER_MS, 0);
		// ③ 丝杆回到顶端
		Con_Task_Enqueue(TASK_MOTOR_TO, 1, Th_Hanger_Up, ROBOT_ANGLE_TOL_DEFAULT, 0);
}

// ==================== 4. ABC 命令解析（Serial4=LCD，帧内 = 分隔）====================
// 约定: 帧被处理即消费 flag；全部不匹配必须恢复 flag，供链上后续解析器（LCD_Key_Check 等）使用
static int32_t  s_last_trans_rel = 0;   // 最近一次 Trans_Rel 值（Save_Trans_Step 兼容用）

void Robot_Cmd_Handle(Serial_Typedef *ps)
{
    if (!Serial_GetNewPackageFlag_ABC(ps)) return;

    char *p = ps->ABC_Data.Serial_New_Package_ABC;
    int v = 0;

    // ---- 业务触发（Start 忙时忽略，Back/Shou 任何状态可用：内部先清队列）----
    if (strcmp(p, "Hanger_Start") == 0) { if (!Con_Task_IsBusy()) Robot_Hang_Enqueue(); return; }
    if (strcmp(p, "Hanger_Back")  == 0) { Robot_Reset_Start(); return; }
    if (strcmp(p, "Hanger_Shou")  == 0) { Robot_Shou_Start(); return; }

    // ---- 保存示教（任何状态生效，立即写 EEPROM）----
    // 舵机 6 条（LCD 在发）
    if (strcmp(p, "Save_ClawA_Close")   == 0) { Th_ClawA_Close   = (int32_t)Servo_Get_Angle(SERVO_CLAW_A);   Param_AT24C02_Write(&Th_ClawA_Close);   return; }
    if (strcmp(p, "Save_ClawA_Open")    == 0) { Th_ClawA_Open    = (int32_t)Servo_Get_Angle(SERVO_CLAW_A);   Param_AT24C02_Write(&Th_ClawA_Open);    return; }
    if (strcmp(p, "Save_ClawB_Close")   == 0) { Th_ClawB_Close   = (int32_t)Servo_Get_Angle(SERVO_CLAW_B);   Param_AT24C02_Write(&Th_ClawB_Close);   return; }
    if (strcmp(p, "Save_ClawB_Open")    == 0) { Th_ClawB_Open    = (int32_t)Servo_Get_Angle(SERVO_CLAW_B);   Param_AT24C02_Write(&Th_ClawB_Open);    return; }
    if (strcmp(p, "Save_Hanger1_Close") == 0) { Th_Hanger1_Close = (int32_t)Servo_Get_Angle(SERVO_HANGER_1); Param_AT24C02_Write(&Th_Hanger1_Close); return; }
    if (strcmp(p, "Save_Hanger1_Open")  == 0) { Th_Hanger1_Open  = (int32_t)Servo_Get_Angle(SERVO_HANGER_1); Param_AT24C02_Write(&Th_Hanger1_Open);  return; }
    // 电机 5 条（保留兼容，LCD 暂未发）
    if (strcmp(p, "Save_Hanger_Up")   == 0) { Th_Hanger_Up   = (int32_t)Motor_Get_Angle(&Motor_B); Param_AT24C02_Write(&Th_Hanger_Up);   return; }
    if (strcmp(p, "Save_Hanger_Mid")  == 0) { Th_Hanger_Mid  = (int32_t)Motor_Get_Angle(&Motor_B); Param_AT24C02_Write(&Th_Hanger_Mid);  return; }
    if (strcmp(p, "Save_Hanger_Down") == 0) { Th_Hanger_Down = (int32_t)Motor_Get_Angle(&Motor_B); Param_AT24C02_Write(&Th_Hanger_Down); return; }
    if (strcmp(p, "Save_Trans_Step")  == 0) { Th_Trans_Step  = s_last_trans_rel;                   Param_AT24C02_Write(&Th_Trans_Step);  return; }

    // ---- 运动命令（直接执行，不入队：滑条拖动实时重定目标，无需等待）----
    if (sscanf(p, "Trans_Rel=%d",  &v) == 1) { s_last_trans_rel = v; Motor_A.Angle_Ring_Enable = 1; Motor_SetAngle(&Motor_A, (int)Motor_Get_Angle(&Motor_A) + v); return; }
    if (sscanf(p, "Hanger_Rel=%d", &v) == 1) { Motor_B.Angle_Ring_Enable = 1; Motor_SetAngle(&Motor_B, (int)Motor_Get_Angle(&Motor_B) + v); return; }
    if (sscanf(p, "Trans_Abs=%d",  &v) == 1) { Motor_A.Angle_Ring_Enable = 1; Motor_SetAngle(&Motor_A, v); return; }
    if (sscanf(p, "Hanger_Abs=%d", &v) == 1) { Motor_B.Angle_Ring_Enable = 1; Motor_SetAngle(&Motor_B, v); return; }
    if (sscanf(p, "ClawA=%d",   &v) == 1) { Servo_SetAngle(SERVO_CLAW_A,   (int16_t)v); return; }
    if (sscanf(p, "ClawB=%d",   &v) == 1) { Servo_SetAngle(SERVO_CLAW_B,   (int16_t)v); return; }
    if (sscanf(p, "Hanger1=%d", &v) == 1) { Servo_SetAngle(SERVO_HANGER_1, (int16_t)v); return; }

    // ---- 舵机到位命令（直接执行，目标=已存阈值，不入队）----
    if (strcmp(p, "ClawA_Open")    == 0) { Servo_SetAngle(SERVO_CLAW_A,   (int16_t)Th_ClawA_Open);   return; }
    if (strcmp(p, "ClawA_Close")   == 0) { Servo_SetAngle(SERVO_CLAW_A,   (int16_t)Th_ClawA_Close);  return; }
    if (strcmp(p, "ClawB_Open")    == 0) { Servo_SetAngle(SERVO_CLAW_B,   (int16_t)Th_ClawB_Open);   return; }
    if (strcmp(p, "ClawB_Close")   == 0) { Servo_SetAngle(SERVO_CLAW_B,   (int16_t)Th_ClawB_Close);  return; }
    if (strcmp(p, "Hanger1_Open")  == 0) { Servo_SetAngle(SERVO_HANGER_1, (int16_t)Th_Hanger1_Open); return; }
    if (strcmp(p, "Hanger1_Close") == 0) { Servo_SetAngle(SERVO_HANGER_1, (int16_t)Th_Hanger1_Close); return; }

    // ---- 未匹配：恢复 flag 给链上后续解析器 ----
    ps->ABC_Data.Serial_New_Package_Flag = 1;
}
