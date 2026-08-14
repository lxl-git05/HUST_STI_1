// Menu_Param.c — 调参任务菜单（参照 Control.c: 枚举→回调→任务表→Con_Task）
// 按键: KEY_1单击=下一项, KEY_1长按=入队/再次长按=Skip
//  Tick: 所有任务20ms输出调试数据到Serial1（PID:goal/real/set, GyroCal:biasX/Y/Z）
// Serial1: Kp/Ki/Kd/Goal 修改PID
// H743 裁剪说明: F4 原有 Oran/Stepper 等 Tune 项已删除，保留 IMU 两项 + 恢复 7 个电机/直行调参项
#include "AllHeader.h"
#include "Menu_Param.h"

// ==================== 菜单显示名（用于OLED）====================
typedef struct {
    const char *cat;   // 分类 "Motor_A"
    const char *name;  // 任务 "Speed"
} TuneLabel;

// ==================== 菜单状态 ====================
static int8_t s_cursor = 0;  // 当前浏览位置

// Gyro_Cal 状态机
static int      s_cal_state = 0;    // 0=等待按键, 1=等待稳定, 2=完成
static uint32_t s_cal_timer = 0;    // 计时器

// ==================== 辅助：导航 ====================
#define NEXT_CURSOR(c)  (((c) + 1) % TUNE_COUNT)

// ==================== 通用 OLED 渲染：PID 值 ====================
static void OLED_ShowPID(const char *cat, const char *name, Pid_Typedef *pid)
{
    OLED_Printf(0, 0,  OLED_6X8, "%-7s %-8s", cat, name);
    OLED_Printf(0, 10, OLED_6X8, "Kp:%.2f Ki:%.2f", pid->Kp, pid->Ki);
    OLED_Printf(0, 20, OLED_6X8, "Kd:%.2f", pid->Kd);
    OLED_Printf(0, 30, OLED_6X8, "Goal:%.1f", pid->goalPoint);
    OLED_Printf(0, 40, OLED_6X8, "Real:%.1f", pid->realPoint_Now);
    OLED_Printf(0, 50, OLED_6X8, "Set:%.1f L:Back", pid->setPoint);
}

// ==================== 通用 Serial1 路由：PID（返回 Goal 是否变化）====================
static bool Serial_RoutePID(Pid_Typedef *pid)
{
    if (!Serial_GetNewPackageFlag_ABC(&Serial1)) return false;
    float old = pid->goalPoint;
    Serial_SetFloatData(&Serial1, "Kp",   "Kp=%f",   &pid->Kp);
    Serial_SetFloatData(&Serial1, "Ki",   "Ki=%f",   &pid->Ki);
    Serial_SetFloatData(&Serial1, "Kd",   "Kd=%f",   &pid->Kd);
    Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &pid->goalPoint);
    return (pid->goalPoint != old);
}

// ==================== TUNE_GYRO_YAW ====================
// H743 说明: F4 的 PID_Angle（整车yaw角环）定义在 Con_Motor.c 中，
// H743 无此模块，故在本文件内置独立 yaw PID（初值同 F4: Kp=2.47, Kd=7.16, Out±100）
static Pid_Typedef s_YawPID ;

void Tune_Gyro_Yaw_Setup(float p[4])
{
    // 关闭双电机角度环: yaw 环需要直接控制速度,避免全局角度环覆盖差速输出
    Motor_A.Angle_Ring_Enable = 0;
    Motor_B.Angle_Ring_Enable = 0;
    PID_Init(&s_YawPID, 2.47f, 0.0f, 7.16f, 100, -100, 1000);
    PID_Param_Reset(&s_YawPID);
    // 目标=当前yaw（串口未输入Goal时保持静止），旋转目标由串口 "Goal=%f" 修改
    s_YawPID.goalPoint = IMU_Yaw_Abs_Get();
}
void Tune_Gyro_Yaw_Run(float p[4])
{
    Serial_RoutePID(&s_YawPID);  // Goal变化时只需更新goalPoint，Tick自动处理
    OLED_ShowPID("Gyro", "YawPID", &s_YawPID);
    Serial_printf(&Serial1, "Yaw:%.1f Tar:%.1f Out:%.1f\n",
        IMU_Mahony_Real.yaw, s_YawPID.goalPoint, s_YawPID.setPoint);
}
void Tune_Gyro_Yaw_Tick(float p[4])
{
    // 同 F4 PID_Angle_Tick: 获取真实yaw→PID→差速输出（A正转 B反转 = 顺时针为正）
    s_YawPID.realPoint_Now = IMU_Yaw_Abs_Get();
    PID_Update(&s_YawPID, s_YawPID.realPoint_Now);
    Motor_SetSpeed(&Motor_A,  s_YawPID.setPoint);
    Motor_SetSpeed(&Motor_B, -s_YawPID.setPoint);
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        s_YawPID.goalPoint, s_YawPID.realPoint_Now, s_YawPID.setPoint);
}

// ==================== TUNE_GYRO_CAL ====================
void Tune_Gyro_Cal_Setup(float p[4])
{
    s_cal_state = 0;
    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "IMU Calib:");
}
void Tune_Gyro_Cal_Run(float p[4])
{
    // 展示当前陀螺+加速度零偏（6个参数，紧凑双列布局）
    OLED_Printf(0, 10, OLED_6X8, "GX:%.4f AX:%.4f",
        IMU_Mahony_GyroBiasX, IMU_Mahony_AccBiasX);
    OLED_Printf(0, 20, OLED_6X8, "GY:%.4f AY:%.4f",
        IMU_Mahony_GyroBiasY, IMU_Mahony_AccBiasY);
    OLED_Printf(0, 30, OLED_6X8, "GZ:%.4f AZ:%.4f",
        IMU_Mahony_GyroBiasZ, IMU_Mahony_AccBiasZ);

    if (s_cal_state == 0)
    {
        OLED_Printf(0, 50, OLED_6X8, "K2:Cal K1:Back");
        if (Key_Check(KEY_2, KEY_SINGLE))
        {
            s_cal_state = 1;
            s_cal_timer = HAL_GetTick();
        }
    }
    else if (s_cal_state == 1)
    {
        OLED_Printf(0, 50, OLED_6X8, "Wait 1s stable.");
        if (HAL_GetTick() - s_cal_timer >= 1000)
        {
            OLED_Printf(0, 40, OLED_6X8, "Calibrating... ");
            OLED_Update();

            Timer_DisableIRQ();
            IMU_Mahony_Calibrate(1000);
            Timer_EnableIRQ();

            // 保存陀螺零偏
            Param_AT24C02_Write(&IMU_Mahony_GyroBiasX);
            Param_AT24C02_Write(&IMU_Mahony_GyroBiasY);
            Param_AT24C02_Write(&IMU_Mahony_GyroBiasZ);
            // 保存加速度零偏
            Param_AT24C02_Write(&IMU_Mahony_AccBiasX);
            Param_AT24C02_Write(&IMU_Mahony_AccBiasY);
            Param_AT24C02_Write(&IMU_Mahony_AccBiasZ);

            OLED_Printf(0, 40, OLED_6X8, "IMU_OK!        ");
            s_cal_state = 0;
        }
    }
}
bool Tune_Gyro_Cal_IsExit(float p[4]) { return (s_cal_state == 2); }
void Tune_Gyro_Cal_Tick(float p[4])
{
    Serial_printf(&Serial1, "%.4f,%.4f,%.4f,  %.4f,%.4f,%.4f\n",
        IMU_Mahony_GyroBiasX, IMU_Mahony_GyroBiasY, IMU_Mahony_GyroBiasZ,
        IMU_Mahony_AccBiasX, IMU_Mahony_AccBiasY, IMU_Mahony_AccBiasZ);
}

// ==================== TUNE_MOTOR_A_SPEED ====================
void Tune_MotorA_Speed_Setup(float p[4])
{
    // 关闭角度环: 全局 20ms 角度环会覆盖速度环 goal,速度环无法独立调参
    Motor_A.Angle_Ring_Enable = 0;
    Motor_SetSpeed(&Motor_A, Motor_A.PID_s.goalPoint);
}
void Tune_MotorA_Speed_Run(float p[4])
{
    if (Serial_RoutePID(&Motor_A.PID_s))
        Motor_SetSpeed(&Motor_A, Motor_A.PID_s.goalPoint);
    OLED_ShowPID("Motor_A", "Speed", &Motor_A.PID_s);
}
void Tune_MotorA_Speed_Tick(float p[4])
{
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",Motor_A.PID_s.goalPoint , Motor_A.PID_s.realPoint_Now , Motor_A.PID_s.setPoint) ;
}

// ==================== TUNE_MOTOR_A_ANGLE ====================
void Tune_MotorA_Angle_Setup(float p[4])
{
    // 开启角度环: 由 Mode_G 20ms 链的 Motor_Speed_Update_Tick 统一驱动
    Motor_A.Angle_Ring_Enable = 1;
    Motor_SetAngle(&Motor_A, (int)Motor_A.PID_Angle.goalPoint);
}
void Tune_MotorA_Angle_Run(float p[4])
{
    if (Serial_RoutePID(&Motor_A.PID_Angle))
        Motor_SetAngle(&Motor_A, (int)Motor_A.PID_Angle.goalPoint);
    OLED_ShowPID("Motor_A", "Angle", &Motor_A.PID_Angle);
}
void Tune_MotorA_Angle_Tick(float p[4])
{
    // 角度环已由 Mode_G 20ms 链的 Motor_Speed_Update_Tick 统一驱动,此处只输出调试数据
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        Motor_A.PID_Angle.goalPoint, Motor_A.PID_Angle.realPoint_Now, Motor_A.PID_Angle.setPoint);
}

// ==================== TUNE_MOTOR_A_POS ====================
void Tune_MotorA_Pos_Setup(float p[4])
{
    // 关闭角度环,避免与位置环抢速度输出
    Motor_A.Angle_Ring_Enable = 0;
    Motor_SetPos(&Motor_A, Motor_A.PID_Pos.goalPoint);
}
void Tune_MotorA_Pos_Run(float p[4])
{
    if (Serial_RoutePID(&Motor_A.PID_Pos))
        Motor_SetPos(&Motor_A, Motor_A.PID_Pos.goalPoint);
    OLED_ShowPID("Motor_A", "Pos", &Motor_A.PID_Pos);
}
void Tune_MotorA_Pos_Tick(float p[4])
{
    // Dir=1: H743 的方向已在 Motor_Pos_Update 内用 Encoder_Dir 处理
    Motorx_Pos_Update_Tick(&Motor_A, 1);
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        Motor_A.PID_Pos.goalPoint, Motor_A.PID_Pos.realPoint_Now, Motor_A.PID_Pos.setPoint);
}

// ==================== TUNE_MOTOR_B_SPEED ====================
void Tune_MotorB_Speed_Setup(float p[4])
{
    // 关闭角度环: 全局 20ms 角度环会覆盖速度环 goal,速度环无法独立调参
    Motor_B.Angle_Ring_Enable = 0;
    Motor_SetSpeed(&Motor_B, Motor_B.PID_s.goalPoint);
}
void Tune_MotorB_Speed_Run(float p[4])
{
    if (Serial_RoutePID(&Motor_B.PID_s))
        Motor_SetSpeed(&Motor_B, Motor_B.PID_s.goalPoint);
    OLED_ShowPID("Motor_B", "Speed", &Motor_B.PID_s);
}
void Tune_MotorB_Speed_Tick(float p[4])
{
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        Motor_B.PID_s.goalPoint, Motor_B.PID_s.realPoint_Now, Motor_B.PID_s.setPoint);
}

// ==================== TUNE_MOTOR_B_ANGLE ====================
void Tune_MotorB_Angle_Setup(float p[4])
{
    // 开启角度环: 由 Mode_G 20ms 链的 Motor_Speed_Update_Tick 统一驱动
    Motor_B.Angle_Ring_Enable = 1;
    Motor_SetAngle(&Motor_B, (int)Motor_B.PID_Angle.goalPoint);
}
void Tune_MotorB_Angle_Run(float p[4])
{
    if (Serial_RoutePID(&Motor_B.PID_Angle))
        Motor_SetAngle(&Motor_B, (int)Motor_B.PID_Angle.goalPoint);
    OLED_ShowPID("Motor_B", "Angle", &Motor_B.PID_Angle);
}
void Tune_MotorB_Angle_Tick(float p[4])
{
    // 角度环已由 Mode_G 20ms 链的 Motor_Speed_Update_Tick 统一驱动,此处只输出调试数据
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        Motor_B.PID_Angle.goalPoint, Motor_B.PID_Angle.realPoint_Now, Motor_B.PID_Angle.setPoint);
}

// ==================== TUNE_MOTOR_B_POS ====================
void Tune_MotorB_Pos_Setup(float p[4])
{
    // 关闭角度环,避免与位置环抢速度输出
    Motor_B.Angle_Ring_Enable = 0;
    Motor_SetPos(&Motor_B, Motor_B.PID_Pos.goalPoint);
}
void Tune_MotorB_Pos_Run(float p[4])
{
    if (Serial_RoutePID(&Motor_B.PID_Pos))
        Motor_SetPos(&Motor_B, Motor_B.PID_Pos.goalPoint);
    OLED_ShowPID("Motor_B", "Pos", &Motor_B.PID_Pos);
}
void Tune_MotorB_Pos_Tick(float p[4])
{
    // Dir=1: H743 的方向已在 Motor_Pos_Update 内用 Encoder_Dir 处理
    Motorx_Pos_Update_Tick(&Motor_B, 1);
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        Motor_B.PID_Pos.goalPoint, Motor_B.PID_Pos.realPoint_Now, Motor_B.PID_Pos.setPoint);
}

// ==================== TUNE_CAR_STRAIGHT ====================
void Tune_Car_Straight_Setup(float p[4])
{
    // 关闭两电机角度环,避免覆盖直行环的差速输出
    Motor_A.Angle_Ring_Enable = 0;
    Motor_B.Angle_Ring_Enable = 0;
    PID_Car_Straight_Reset();
}
void Tune_Car_Straight_Run(float p[4])
{
    if (Serial_RoutePID(&PID_Car_Straight))
    {
        PID_Car_Straight_Reset();
        // goalPoint 已被 Serial_RoutePID 更新，Reset 后会保持
    }
    OLED_ShowPID("Car", "Straight", &PID_Car_Straight);
}
void Tune_Car_Straight_Tick(float p[4])
{
    PID_Car_Straight_Tick();
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
        PID_Car_Straight.goalPoint, PID_Car_Straight.realPoint_Now, PID_Car_Straight.setPoint);
}

// ==================== 通用：永不自动退出 ====================
bool Tune_AlwaysFalse(float p[4]) { return false; }

// ==================== 任务描述表（同 Control_TaskTable）====================
// 修改次序只需要将下面两个表各自位置交换即可
static const TuneLabel s_labels[TUNE_COUNT] = {
    { "Gyro",    "Cal"      },  // TUNE_GYRO_CAL
    { "Gyro",    "YawPID"   },  // TUNE_GYRO_YAW
    { "Motor_A", "Speed"    },  // TUNE_MOTOR_A_SPEED
    { "Motor_A", "Angle"    },  // TUNE_MOTOR_A_ANGLE
    { "Motor_A", "Pos"      },  // TUNE_MOTOR_A_POS
    { "Motor_B", "Speed"    },  // TUNE_MOTOR_B_SPEED
    { "Motor_B", "Angle"    },  // TUNE_MOTOR_B_ANGLE
    { "Motor_B", "Pos"      },  // TUNE_MOTOR_B_POS
    { "Car",     "Straight" },  // TUNE_CAR_STRAIGHT
};

Task_Descriptor_Typedef Menu_Tune_Table[TUNE_COUNT] = {
    // TUNE_GYRO_CAL
    { Tune_Gyro_Cal_Setup,     Tune_Gyro_Cal_Run,     Tune_Gyro_Cal_IsExit, Tune_Gyro_Cal_Tick },
    // TUNE_GYRO_YAW
    { Tune_Gyro_Yaw_Setup,     Tune_Gyro_Yaw_Run,     Tune_AlwaysFalse, Tune_Gyro_Yaw_Tick },
    // TUNE_MOTOR_A_SPEED
    { Tune_MotorA_Speed_Setup, Tune_MotorA_Speed_Run, Tune_AlwaysFalse, Tune_MotorA_Speed_Tick },
    // TUNE_MOTOR_A_ANGLE
    { Tune_MotorA_Angle_Setup, Tune_MotorA_Angle_Run, Tune_AlwaysFalse, Tune_MotorA_Angle_Tick },
    // TUNE_MOTOR_A_POS
    { Tune_MotorA_Pos_Setup,   Tune_MotorA_Pos_Run,   Tune_AlwaysFalse, Tune_MotorA_Pos_Tick },
    // TUNE_MOTOR_B_SPEED
    { Tune_MotorB_Speed_Setup, Tune_MotorB_Speed_Run, Tune_AlwaysFalse, Tune_MotorB_Speed_Tick },
    // TUNE_MOTOR_B_ANGLE
    { Tune_MotorB_Angle_Setup, Tune_MotorB_Angle_Run, Tune_AlwaysFalse, Tune_MotorB_Angle_Tick },
    // TUNE_MOTOR_B_POS
    { Tune_MotorB_Pos_Setup,   Tune_MotorB_Pos_Run,   Tune_AlwaysFalse, Tune_MotorB_Pos_Tick },
    // TUNE_CAR_STRAIGHT
    { Tune_Car_Straight_Setup, Tune_Car_Straight_Run, Tune_AlwaysFalse, Tune_Car_Straight_Tick },
};

// ==================== 菜单浏览 OLED ====================
static void Menu_Render(void)
{
    OLED_Printf(0, 0, OLED_6X8, "===Tune_Menu======");

    // 显示当前光标附近的4个任务
    int8_t start = s_cursor;
    // 确保能看到前后
    if (start > 0) start--;
    if (start > TUNE_COUNT - 4) start = TUNE_COUNT - 4;
    if (start < 0) start = 0;

    for (uint8_t i = 0; i < 4; i++) {
        int8_t idx = start + i;
        if (idx >= TUNE_COUNT) break;
        char c = (idx == s_cursor) ? '>' : ' ';
        OLED_Printf(0, 10 + i * 10, OLED_6X8, "%c%-7s %-8s",
                    c, s_labels[idx].cat, s_labels[idx].name);
    }

    OLED_Printf(0, 50, OLED_6X8, "LONG:Active CLK:Next");
}

// ==================== API ====================

void Menu_Tune_Init(void)
{
    s_cursor = 0;
    Con_Task_Init(Menu_Tune_Table, TUNE_COUNT);
}

int Menu_Tune_Cursor(void) { return s_cursor; }

void LCD_Check_CMD(void)
{
    if (LCD_Cmd_Check("LCD_IMU_Check")) {s_cursor = TUNE_GYRO_CAL ; Con_Task_Enqueue(s_cursor , 0 , 0 , 0 , 0) ;}
}

void Menu_Tune_Loop(void)
{
    // Con_Task_Loop 必须每帧调用（负责出队→Setup→Run→IsExit）
    Con_Task_Loop();

    if (Con_Task_IsBusy())
    {
        // ---- 任务运行中：Run 回调已处理 OLED + Serial1 ----
        if (Key_Check(KEY_1, KEY_LONG) || LCD_Cmd_Check("LCD_Param_Skip"))
            Con_Task_Skip();
    }
    else
    {
        // ---- 浏览模式：KEY_1单击下一项, KEY_1长按入队 ----
        LCD_Check_CMD() ;
        if (Key_Check(KEY_1, KEY_SINGLE))
            s_cursor = NEXT_CURSOR(s_cursor);

        if (Key_Check(KEY_1, KEY_LONG))
            Con_Task_Enqueue(s_cursor, 0, 0, 0, 0);

        Menu_Render();
    }
}
