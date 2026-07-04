#include "Servo.h"

// 1. 舵机初始化
void Servo_Init
(
    Servo_Typedef* Servo, MyPWM_Typedef* Servo_PWM, Servo_Type_t type,
    uint16_t pwm_min, uint16_t pwm_max, uint16_t pos_min, uint16_t pos_max,
    int16_t init_pos, uint16_t step_10ms
)
{
    // PWM初始化
    Servo->Servo_PWM = Servo_PWM;
    MyPWM_Init(Servo->Servo_PWM);  
    Servo->type = type;
    
    // PWM和角度参数初始化
    Servo->pwm_min = pwm_min;
    Servo->pwm_max = pwm_max;
    Servo->pos_min = pos_min;
    Servo->pos_max = pos_max;
    
    // 步进控制初始化
    Servo->current_pos = init_pos;
    Servo->target_pos = init_pos;      
    Servo->step_10ms = step_10ms;
    Servo->is_Step_Enable = true;
    Servo->tick_counter = 0;           
    
    // 检查PWM的频率是否是50Hz
    if (MyPWM_GetFre(Servo->Servo_PWM) != 50)
    {
        while(1);
    }
    
    // 设置初始角度
    uint16_t compare;
    compare = Servo->pwm_min + (uint32_t)init_pos * (Servo->pwm_max - Servo->pwm_min) / ((type == SERVO_TYPE_180) ? 180 : 360);
    MyPWM_SetCompare(Servo->Servo_PWM, compare);
}

// 2. 舵机直接设置角度（内部使用，不改变步进状态）
static void Servo_SetAngleInternal(Servo_Typedef* Servo, int16_t angle)
{
    uint16_t compare;
    uint16_t pos_max = (Servo->type == SERVO_TYPE_180) ? 180 : 360;
    
    // 自设限幅
    if(angle < Servo->pos_min) angle = Servo->pos_min;
    if(angle > Servo->pos_max) angle = Servo->pos_max;
    // 物理限幅
    if(angle < 0) angle = 0;
    if(angle > pos_max) angle = pos_max;
    
    // 计算PWM比较值
    compare = Servo->pwm_min + (uint32_t)angle * (Servo->pwm_max - Servo->pwm_min) / pos_max;
    
    // 设置角度
    Servo->current_pos = angle;
    
    // 调用PWM设置函数
    MyPWM_SetCompare(Servo->Servo_PWM, compare);
}

// 2. 舵机直接设置角度（外部调用，停止步进）
void Servo_SetDirectAngle(Servo_Typedef* Servo, int16_t angle)
{
    if(!Servo) return;
    
    Servo_SetAngleInternal(Servo, angle);
    Servo->target_pos = angle;
    Servo->is_Step_Enable = false;
}

// 3. 舵机设置目标角度
void Servo_SetGoalAngle(Servo_Typedef* Servo, int16_t target_pos)
{
    if(!Servo) return;
    
    uint16_t pos_max = (Servo->type == SERVO_TYPE_180) ? 180 : 360;
    
    // 目标角度限幅
    if(target_pos < Servo->pos_min) target_pos = Servo->pos_min;
    if(target_pos > Servo->pos_max) target_pos = Servo->pos_max;
    if(target_pos < 0) target_pos = 0;
    if(target_pos > pos_max) target_pos = pos_max;
    
    Servo->target_pos = target_pos;
    Servo->is_Step_Enable = true;
//    Servo->tick_counter = 0;
}

// 4. 舵机步进控制台，放入1ms中断
void Servox_GoalAngle_Tick(Servo_Typedef* Servo)  // 建议改名为 Servo_GoalAngle_Tick
{
    if(!Servo) return;
    
    // 如果没有使能步进模式就跳过
    if (Servo->is_Step_Enable == false) return;
    
    // 已经到达目标
    if (Servo->current_pos == Servo->target_pos)
    {
        Servo->is_Step_Enable = false;
        return;
    }
    
    // 分频计数（每个舵机独立）
    Servo->tick_counter++;
    if (Servo->tick_counter >= 10)
    {
        Servo->tick_counter = 0;
        
        // 计算差值
        int16_t delta = Servo->target_pos - Servo->current_pos;
        
        // 判断是否在步长范围内（最后一步）
        if (abs(delta) <= Servo->step_10ms)
        {
            // 直接到达目标（使用内部函数，不改变步进状态）
            Servo_SetAngleInternal(Servo, Servo->target_pos);
            Servo->is_Step_Enable = false;  // 到达目标后关闭步进
        }
        else if (delta > 0)
        {
            // 正向步进
            Servo_SetAngleInternal(Servo, Servo->current_pos + Servo->step_10ms);
        }
        else  // delta < 0
        {
            // 反向步进
            Servo_SetAngleInternal(Servo, Servo->current_pos - Servo->step_10ms);
        }
    }
}

// 5. 得到舵机当前角度
int Servo_Get_Angle(Servo_Typedef* Servo)
{
	return Servo->current_pos ;
}
