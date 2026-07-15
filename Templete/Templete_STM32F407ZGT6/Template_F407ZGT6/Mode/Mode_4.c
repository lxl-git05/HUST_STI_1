// ========================== 电机PID调参模式 ==========================
#include "Mode_4.h"
#include "AllHeader.h"

// 当前选择的电机：0 = Motor A, 1 = Motor B
static uint8_t Motor_Select = 0;

// 串口设置的目标速度
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

    // ---- 标题 ----
    OLED_Printf(0, 0, OLED_6X8, "=====Mode_PID=====");

    // ---- OLED 展示两电机真实速度 ----
    OLED_Printf(0,  10, OLED_6X8, "A:%.0f", Motor_A.PID_s.realPoint_Now);
    OLED_Printf(60, 10, OLED_6X8, "B:%.0f", Motor_B.PID_s.realPoint_Now);

    // ---- 展示当前选中电机 ----
    OLED_Printf(0, 20, OLED_6X8, ">>Motor_%s<<", Get_Motor_Label());

		// OLED 展示当前电机 PID 参数
		OLED_Printf(0, 30, OLED_6X8, "%s:%.2f,%.2f,%.2f", Get_Motor_Label(), pMotor->PID_s.Kp, pMotor->PID_s.Ki, pMotor->PID_s.Kd);

    // ---- KEY_1 单击：切换选择电机 ----
    if (Key_Check(KEY_1, KEY_SINGLE))
    {
        Motor_Select = (Motor_Select == 0) ? 1 : 0;
    }

    // ---- 串口参数更改 ----
    if (Serial_GetNewPackageFlag_ABC(&Serial1))
    {
        // 通过串口更新选中电机的 Kp/Ki/Kd 和 目标速度
        Serial_SetFloatData(&Serial1, "Kp",        "Kp=%f",        &pMotor->PID_s.Kp);
        Serial_SetFloatData(&Serial1, "Ki",        "Ki=%f",        &pMotor->PID_s.Ki);
        Serial_SetFloatData(&Serial1, "Kd",        "Kd=%f",        &pMotor->PID_s.Kd);
        Serial_SetFloatData(&Serial1, "Angle",  "Angle=%f", 			 &Motor_PID_Goal_Check);

        Motor_SetSpeed(pMotor, Motor_PID_Goal_Check);
    }
}

void Mode_4_Tick(void)
{
		Motor_Speed_Update_Tick(20);
    Motor_Typedef *pMotor = Get_Selected_Motor();

    // 每20ms通过串口打印：目标速度, 当前速度, PID输出
    Serial_printf(&Serial1, "%.2f,%.2f,%.2f\n",
                  pMotor->PID_s.goalPoint,
                  pMotor->PID_s.realPoint_Now,
                  pMotor->PID_s.setPoint);
}

void Mode_4_Exit(void)
{
    // 退出时停止选中电机
    Motor_Stop(Get_Selected_Motor());
    OLED_Clear();
}
