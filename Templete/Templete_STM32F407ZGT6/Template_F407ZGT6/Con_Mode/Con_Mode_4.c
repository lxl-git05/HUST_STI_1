// ========================== Con_Mode_4 ==========================
// 小车置于A点，钢球置于中心点O，按键启动后沿黑线顺时针行驶并通过B位置，
// AB间行驶时间≤8s，行驶过程中钢球须稳定在摆杆中心点附近，误差绝对值≤1cm。
#include "Con_Mode_4.h"

enum { S_IDLE, S_ACCEL, S_CRUISE, S_DECEL, S_STOP };

#define RACE_SPEED_LO  80.0f    // 前2秒低速(rpm)
#define RACE_SPEED_HI  80.0f    // 2秒后中速(rpm), 变化率靠RAMP_CNT限制
#define SPEED_UP_MS    2000     // 提速时刻(ms)
#define RAMP_CNT       2        // 加减速分频, 固定不分段(越大越平滑)
#define DECEL_YAW      20.0f    // 减速触发偏航角(°)

static uint8_t  state     = S_IDLE;
static uint32_t t0        = 0;
static float    stop_time = 0.0f;

void Con_Mode_4_Setup(void)
{
    Oran_PID_Init();
    PID_Param_Reset(&PID_Oran);
    PID_Oran.goalPoint = 0.0f;
    Oran_Real_Offset = 0.0f;
    Oran_FF_Enable   = 1.0f;        // 行驶中开前馈
    Stepper_PWM_Angle_Gains_Set(&Stepper1, 4.0f, 0.0f, 0.829f, 50.0f, -50.0f);
    Y8U_SetSpeed(0);
    state = S_IDLE;
}

// 寻迹
void Con_Mode_4_Loop(void)
{
    OLED_Printf(0, 0, OLED_6X8, "Con_Mode_4  Y:%.0f", IMU_Yaw_Abs_Get());
		
    if (state == S_IDLE)
    {
        OLED_Printf(0, 20, OLED_8X16, "KEY1:Go");
        if (Key_Check(KEY_1, KEY_SINGLE))
        {
					Serial_printf(&Serial2 , "@rec:666$#") ;
            IMU_Yaw_Abs_Reset();
            t0    = HAL_GetTick();
            state = S_ACCEL;
        }
    }
    else if (state == S_STOP)
    {
        OLED_Printf(0, 20, OLED_8X16, "Done %.1fs", stop_time);
    }
    else
    {
        OLED_Printf(0, 10, OLED_6X8, "Spd:%.0f", Y8U_GetSpeed());
        OLED_Printf(0, 20, OLED_8X16, "%.1fs", (HAL_GetTick() - t0) / 1000.0f);
    }
		
}

void Con_Mode_4_Tick(void)
{
    // 球平衡
    Oran_Update();
    Oran_PID_Update();
	
		// 打印
		Serial_printf(&Serial1 , "%.1f,%.1f,%.1f,%.1f,%.1f\n",PID_Oran.goalPoint, PID_Oran.realPoint_Now
								,PID_Oran.setPoint, IMU_Yaw_Abs_Get(),Y8U_GetSpeed());


    if (state == S_IDLE || state == S_STOP)
        return;
		
		// 寻迹
{
    Y8U_PID_Update();

    // 状态切换
    switch (state)
    {
    case S_ACCEL:
        {
            float tar = (HAL_GetTick() - t0 > SPEED_UP_MS) ? RACE_SPEED_HI : RACE_SPEED_LO;
            if (Y8U_GetSpeed() > tar - 2.0f)
                state = S_CRUISE;
        }
        break;
    case S_CRUISE:
        if (IMU_Yaw_Abs_Get() > DECEL_YAW)
            state = S_DECEL;
        else if (HAL_GetTick() - t0 > 5000 && IMU_Yaw_Abs_Get() > 10.0f)
            state = S_DECEL;   // 5s超时且已转过10°, 强制减速
        break;
    case S_DECEL:
        if (Y8U_GetSpeed() < 2.0f)
        {
            state     = S_STOP;
            stop_time = (HAL_GetTick() - t0) / 1000.0f;
            Y8U_SetSpeed(0);
            Motor_SetSpeed(&Motor_A, 0);
            Motor_SetSpeed(&Motor_B, 0);
        }
        break;
    }

    // 速度 ramp: 前2秒低速, 之后补速度
    {
        float target;
        if (state == S_DECEL)
            target = 0.0f;
        else if (HAL_GetTick() - t0 > SPEED_UP_MS)
            target = RACE_SPEED_HI;
        else
            target = RACE_SPEED_LO;
        Y8U_RampTick(target, RAMP_CNT);
    }
}
}

void Con_Mode_4_Exit(void)
{
    state = S_IDLE;
    Y8U_SetSpeed(0);
    Stepper_PWM_Stop(&Stepper1);
}
