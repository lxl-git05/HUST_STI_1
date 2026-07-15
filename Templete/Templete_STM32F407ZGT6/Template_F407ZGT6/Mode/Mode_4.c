// ========================== 电机PID调参模式 ==========================
#include "Mode_4.h"
#include "AllHeader.h"

// 当前选择的电机：0 = Motor A, 1 = Motor B
static uint8_t Motor_Select = 0;
static uint8_t Motor_Pos_Enable = 0;  // 0=速度环, 1=位置环(角度)

// 串口通用目标值（速度环=目标速度, 位置环=目标角度）
float Motor_PID_Goal_Check = 0;

// 获取当前选中电机的指针
static Motor_Typedef* Get_Selected_Motor(void)
{
    return (Motor_Select == 0) ? &Motor_A : &Motor_B;
}

// 获取当前选中电机的名称
static const char* Get_Motor_Label(void)
{
    return (Motor_Select == 0) ? "A" : "B";
}

void Mode_4_Setup(void)
{
    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "=====Mode_PID=====");
}

void Mode_4_Loop(void)
{
    Motor_Typedef *pMotor = Get_Selected_Motor();

    // ---- 标题 + 电机 + 环类型 ----
    OLED_Printf(0, 0, OLED_6X8, "Motor_%s [%s]", Get_Motor_Label(), Motor_Pos_Enable ? "Angle" : "Speed");

    // ---- KEY_1 单击：切换电机 ----
    if (Key_Check(KEY_1, KEY_SINGLE))
    {
        Motor_Select = !Motor_Select;
    }

    // ---- KEY_2 单击：切换速度环/位置环 ----
    if (Key_Check(KEY_2, KEY_SINGLE))
    {
        Motor_Pos_Enable = !Motor_Pos_Enable;
    }

    // ---- 串口参数更改 ----
    if (Serial_GetNewPackageFlag_ABC(&Serial1))
    {
        if (Motor_Pos_Enable)
        {
            // 位置环：调 PID_Angle
            Serial_SetFloatData(&Serial1, "Kp",    "Kp=%f",    &pMotor->PID_Angle.Kp);
            Serial_SetFloatData(&Serial1, "Ki",    "Ki=%f",    &pMotor->PID_Angle.Ki);
            Serial_SetFloatData(&Serial1, "Kd",    "Kd=%f",    &pMotor->PID_Angle.Kd);
            Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Motor_PID_Goal_Check);

            Motor_SetAngle(pMotor, Motor_PID_Goal_Check);
        }
        else
        {
            // 速度环：调 PID_s
            Serial_SetFloatData(&Serial1, "Kp",    "Kp=%f",    &pMotor->PID_s.Kp);
            Serial_SetFloatData(&Serial1, "Ki",    "Ki=%f",    &pMotor->PID_s.Ki);
            Serial_SetFloatData(&Serial1, "Kd",    "Kd=%f",    &pMotor->PID_s.Kd);
            Serial_SetFloatData(&Serial1, "Goal", "Goal=%f", &Motor_PID_Goal_Check);

            Motor_SetSpeed(pMotor, Motor_PID_Goal_Check);
        }

        // OLED 展示当前 PID 参数
        if (Motor_Pos_Enable)
            OLED_Printf(0, 10, OLED_6X8, "K:%.2f,%.2f,%.2f", pMotor->PID_Angle.Kp, pMotor->PID_Angle.Ki, pMotor->PID_Angle.Kd);
        else
            OLED_Printf(0, 10, OLED_6X8, "K:%.2f,%.2f,%.2f", pMotor->PID_s.Kp, pMotor->PID_s.Ki, pMotor->PID_s.Kd);
    }
}

void Mode_4_Tick(void)
{
    Motor_Typedef *pMotor = Get_Selected_Motor();

    if (Motor_Pos_Enable)
    {
        // 位置环更新（选中电机）+ 速度内环（两电机）
				if (pMotor == &Motor_A)
				{
					Motorx_Angle_Update_Tick(pMotor, 1);
				}
				else
				{
					Motorx_Angle_Update_Tick(pMotor, -1);
				}
    }
    Motor_Speed_Update_Tick(20);

    // 每20ms通过串口打印：目标, 当前值, PID输出
    if (Motor_Pos_Enable)
        Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
                      pMotor->PID_Angle.goalPoint,
                      pMotor->PID_Angle.realPoint_Now,
                      pMotor->PID_Angle.setPoint);
    else
        Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
                      pMotor->PID_s.goalPoint,
                      pMotor->PID_s.realPoint_Now,
                      pMotor->PID_s.setPoint);
}

void Mode_4_Exit(void)
{
    Motor_Stop(Get_Selected_Motor());
    OLED_Clear();
}
