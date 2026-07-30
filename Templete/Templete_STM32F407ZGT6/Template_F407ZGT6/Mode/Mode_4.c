#include "Mode_4.h"
#include "AllHeader.h"

// ========== 状态机 ==========
enum {
    RACE_IDLE = 0,      // 待命
    RACE_ACCEL,         // 缓慢加速
    RACE_CRUISE,        // 匀速巡航
    RACE_PRE_DECEL,     // 预减速（yaw>300°, ramp降到低速，等待终点）
    RACE_FINAL_DECEL,   // 终点后缓慢减速（cnt 更大，柔和停车）
    RACE_STOP           // 完全停止
};

// ========== 运行参数 ==========
static float   Mode4_Tar_Speed = 70.0f;   // 巡航目标速度(rpm)
static uint8_t Ramp_Up_Cnt     = 5;       // 加速分频(tick), 越大越慢
static uint8_t Ramp_Dn_Cnt     = 3;       // 预减速分频(tick), 越大越慢
static uint8_t Ramp_Stop_Cnt   = 12;      // 终点后减速分频(tick), 更大=更柔和停车
#define PRE_DECEL_YAW   300.0f            // 预减速触发偏航角(°)
#define PRE_SPEED        30.0f            // 预减速目标低速(rpm), 不能到0
#define FINISH_YAW      330.0f            // 终点偏航阈值(°)

// ========== 运行时状态 ==========
static uint8_t  race_state = RACE_IDLE;
static uint32_t race_start = 0;
static float    race_time  = 0.0f;

void Mode_4_Setup(void)
{
    Y8U_SetSpeed(0);
    race_state = RACE_IDLE;
	Con_Mode_3_Setup();
}

void Mode_4_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "M4 Spd:%.0f Y:%.0f", Y8U_GetSpeed(), IMU_Yaw_Abs_Get());

    // 调试PID
    if (Serial_GetNewPackageFlag_ABC(&Serial1))
    {
        Serial_SetFloatData(&Serial1, "Kp",   "Kp=%f",   &Y8U_PID.Kp);
        Serial_SetFloatData(&Serial1, "Ki",   "Ki=%f",   &Y8U_PID.Ki);
        Serial_SetFloatData(&Serial1, "Kd",   "Kd=%f",   &Y8U_PID.Kd);
        Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Y8U_PID.goalPoint);
    }

    switch (race_state)
    {
    case RACE_IDLE:
        OLED_Printf(0, 10, OLED_6X8, "KEY1:Go KEY2:Spd");
        OLED_Printf(0, 20, OLED_6X8, "Tar:%.0f Up:%d Dn:%d", Mode4_Tar_Speed, Ramp_Up_Cnt, Ramp_Dn_Cnt);
        OLED_Printf(0, 30, OLED_8X16, "Time:0");

        // KEY1: 启动
        if (Key_Check(KEY_1, KEY_SINGLE))
        {
            race_state = RACE_ACCEL;
            race_start = HAL_GetTick();
            IMU_Yaw_Abs_Reset();        // 偏航归零，从头计圈
        }
        // KEY2 单击: 巡航速度+10
        if (Key_Check(KEY_2, KEY_SINGLE))
        {
            Mode4_Tar_Speed += 10.0f;
            if (Mode4_Tar_Speed > 200.0f) Mode4_Tar_Speed = 200.0f;
        }
        // KEY2 双击: 巡航速度-10
        if (Key_Check(KEY_2, KEY_DOUBLE))
        {
            Mode4_Tar_Speed -= 10.0f;
            if (Mode4_Tar_Speed < 10.0f) Mode4_Tar_Speed = 10.0f;
        }
        break;

    case RACE_ACCEL:
        OLED_Printf(0, 10, OLED_6X8, "Accel...  Tar:%.0f", Mode4_Tar_Speed);
        OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", (HAL_GetTick() - race_start) / 1000.0f);
        break;

    case RACE_CRUISE:
        OLED_Printf(0, 10, OLED_6X8, "Cruise    Yaw:%.0f", IMU_Yaw_Abs_Get());
        OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", (HAL_GetTick() - race_start) / 1000.0f);
        break;

    case RACE_PRE_DECEL:
        OLED_Printf(0, 10, OLED_6X8, "PreDecel  Yaw:%.0f", IMU_Yaw_Abs_Get());
        OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", (HAL_GetTick() - race_start) / 1000.0f);
        break;

    case RACE_FINAL_DECEL:
        OLED_Printf(0, 10, OLED_6X8, "FinalDecel Spd:%.0f", Y8U_GetSpeed());
        OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", (HAL_GetTick() - race_start) / 1000.0f);
        break;

    case RACE_STOP:
        OLED_Printf(0, 10, OLED_6X8, "Finished!");
        OLED_Printf(0, 30, OLED_8X16, "Time:%.1f", race_time);
        break;
    }
}

void Mode_4_Tick(void)
{
	Con_Mode_3_Tick();
    Motor_Pos_Update(&Motor_A);
    Motor_Pos_Update(&Motor_B);

    // ===== IDLE/STOP: 什么也不做，直接返回 =====
    if (race_state == RACE_IDLE || race_state == RACE_STOP)
    {
//        Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
//            IMU_Yaw_Abs_Get(),
//            Motor_A.PID_s.realPoint_Now,
//            Motor_B.PID_s.realPoint_Now,
//            IMU_Get_Ax(), IMU_Get_Ay());
        return;
    }

    // ===== 以下仅 ACCEL/CRUISE/PRE_DECEL/FINAL_DECEL 状态执行 =====

    Y8U_PID_Update();

    switch (race_state)
    {
    case RACE_ACCEL:
        {
            float spd = Y8U_GetSpeed();
            float gap = (spd > Mode4_Tar_Speed) ? (spd - Mode4_Tar_Speed) : (Mode4_Tar_Speed - spd);
            if (gap < 2.0f)
                race_state = RACE_CRUISE;
        }
        break;

    case RACE_CRUISE:
        if (IMU_Yaw_Abs_Get() > PRE_DECEL_YAW)
        {
            race_state = RACE_PRE_DECEL;
            Mode4_Tar_Speed = PRE_SPEED;
        }
        break;

    case RACE_PRE_DECEL:
        // 终点命中 → 进入缓慢停车（cnt 更大，柔和减速）
        if (IMU_Yaw_Abs_Get() > FINISH_YAW && Y8U_CheckFinishLine())
        {
            race_state = RACE_FINAL_DECEL;
            Mode4_Tar_Speed = 0.0f;
        }
        break;

    case RACE_FINAL_DECEL:
        // 速度降到接近 0 → 最终制动
        if (Y8U_GetSpeed() < 2.0f)
        {
            race_state = RACE_STOP;
            race_time  = (HAL_GetTick() - race_start) / 1000.0f;
            Y8U_SetSpeed(0);
            Motor_SetSpeed(&Motor_A, 0);
            Motor_SetSpeed(&Motor_B, 0);
        }
        break;

    case RACE_STOP:
        break;

    default:
        break;
    }

    {
        uint8_t cnt;
        if      (race_state == RACE_PRE_DECEL)   cnt = Ramp_Dn_Cnt;
        else if (race_state == RACE_FINAL_DECEL)  cnt = Ramp_Stop_Cnt;
        else                                      cnt = Ramp_Up_Cnt;
        Y8U_RampTick(Mode4_Tar_Speed, cnt);
    }

//    Serial_printf(&Serial1, "%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
//        IMU_Yaw_Abs_Get(),
//        Motor_A.PID_s.realPoint_Now,
//        Motor_B.PID_s.realPoint_Now,
//        IMU_Get_Ax(), IMU_Get_Ay());
}
void Mode_4_Exit(void)
{
    race_state = RACE_IDLE;
    Y8U_SetSpeed(0);
}
