#include "Servo.h"

// 1. 舵机初始化
void Servo_Init
(
    Servo_Typedef *Servo, MyPWM_Typedef *Servo_PWM,
    uint16_t pwm_min, uint16_t pwm_max,
    uint16_t pos_min, uint16_t pos_max, int16_t init_pos
)
{
    if (!Servo || !Servo_PWM) return;

    // PWM 启动
    Servo->Servo_PWM = Servo_PWM;
    MyPWM_Init(Servo->Servo_PWM);

    // 校验 PWM 频率是否为 50Hz（舵机标准周期 20ms，配错会烧舵机）
    if (MyPWM_GetFre(Servo->Servo_PWM) != 50)
    {
        while (1) { }
    }

    // PWM 和角度参数初始化
    Servo->pwm_min = pwm_min;
    Servo->pwm_max = pwm_max;
    Servo->pos_min = pos_min;
    Servo->pos_max = pos_max;

    // 写入初始角度（内部完成双限幅）
    Servo_SetAngle(Servo, init_pos);
}

// 2. 配置角度（双限幅 + 线性映射到 PWM 比较值）
void Servo_SetAngle(Servo_Typedef *Servo, int16_t angle)
{
    if (!Servo) return;

    uint16_t compare;

    // 角度限幅（第一层：pos_min / pos_max）
    if (angle < Servo->pos_min) angle = Servo->pos_min;
    if (angle > Servo->pos_max) angle = Servo->pos_max;
    // 角度限幅（第二层：0~180）
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // 线性映射：angle 0~180 -> compare pwm_min~pwm_max
    compare = Servo->pwm_min + (uint32_t)angle * (Servo->pwm_max - Servo->pwm_min) / 180;

    // 记录当前角度 + 输出 PWM（MyPWM_SetCompare 还会再限幅一次，双保险）
    Servo->current_pos = angle;
    MyPWM_SetCompare(Servo->Servo_PWM, compare);
}

// 3. 获取当前角度
int Servo_Get_Angle(Servo_Typedef *Servo)
{
    if (!Servo) return 0;
    return Servo->current_pos;
}
