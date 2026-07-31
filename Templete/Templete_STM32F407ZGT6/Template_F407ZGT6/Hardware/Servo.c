#include "Servo.h"

// 角度→比较值线性映射: compare = 500 + angle * (2000/180)
// 500=0°, 2500=180°, 1μs分辨率
void Servo_Init(void)
{
    MyPWM_Init(&MyPWM_Servo4);
    Servo_SetAngle(90.0f);  // 上电回中位
}

void Servo_SetAngle(float angle)
{
    // 限幅
    if (angle < SERVO_ANGLE_MIN) angle = SERVO_ANGLE_MIN;
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;

    // 线性映射：0°→500, 180°→2500
    float compare = 500.0f + angle * (2000.0f / 180.0f);
    MyPWM_SetCompare(&MyPWM_Servo4, compare);
}
