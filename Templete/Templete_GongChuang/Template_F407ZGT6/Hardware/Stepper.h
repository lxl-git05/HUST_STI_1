#ifndef __STEPPER_H
#define __STEPPER_H

#include "MySystem.h"

typedef enum
{
    /* 未使能：PWM关闭，若配置了EN引脚则驱动器也处于失能状态 */
    STEPPER_STATE_DISABLED = 0,
    /* 已使能但没有运动 */
    STEPPER_STATE_IDLE,
    /* 立即速度模式：直接使用给定频率，不经过加减速 */
    STEPPER_STATE_SPEED_IMMEDIATE,
    /* 斜坡速度模式：由1ms规划器逐渐逼近目标速度 */
    STEPPER_STATE_SPEED_RAMP,
    /* 定速定位模式 */
    STEPPER_STATE_POS_CONSTANT,
    /* 梯形/三角形定位的加速、匀速、减速阶段 */
    STEPPER_STATE_POS_ACCEL,
    STEPPER_STATE_POS_CRUISE,
    STEPPER_STATE_POS_DECEL,
    /* 位置溢出或底层配置异常，需停机后重新处理 */
    STEPPER_STATE_ERROR
} Stepper_State_t;

typedef enum
{
    STEPPER_STATUS_OK = 0,
    STEPPER_STATUS_INVALID_ARGUMENT,
    STEPPER_STATUS_FREQUENCY_RANGE,
    STEPPER_STATUS_BUSY,
    STEPPER_STATUS_POSITION_OVERFLOW,
    STEPPER_STATUS_NOT_INITIALIZED
} Stepper_Status_t;

typedef struct
{
    /* ---------------- 硬件绑定与极性配置 ---------------- */
    MyPWM_Typedef *step_pwm;
    MyGPIO_Typedef *dir_gpio;
    MyGPIO_Typedef *en_gpio;          /* 可为NULL；当前Stepper1没有连接EN */
    uint8_t positive_dir_level;
    uint8_t enable_active_level;
    uint16_t dir_setup_us;            /* DIR改变后到第一个STEP沿的建立时间 */
    uint32_t min_speed_pps;           /* pps = STEP脉冲数/秒 */
    uint32_t max_speed_pps;

    /* ---------------- ISR与主循环共享的实时状态 ---------------- */
    volatile Stepper_State_t state;
    volatile Stepper_Status_t last_status;
    volatile bool initialized;
    volatile bool enabled;
    volatile bool pulse_running;
    volatile int8_t direction;

    /* 软件位置由“已发送STEP脉冲”推算，不等价于编码器真实位置 */
    volatile int32_t position_pulses;
    volatile int32_t target_position;
    volatile uint32_t move_total_pulses;
    volatile uint32_t move_done_pulses;
    volatile uint32_t move_remaining_pulses;

    /* 速度统一用pps；符号表示方向，绝对值表示脉冲频率 */
    volatile int32_t current_speed_pps;
    volatile int32_t target_speed_pps;
    volatile uint32_t max_motion_speed_pps;
    volatile uint32_t accel_pps2;
    volatile uint32_t decel_pps2;

    /* Q16.16速度供1ms规划器累计，防止低加速度被整数除法截断为0 */
    int32_t speed_q16;
    int32_t target_speed_q16;
    /* 定位中需要反向时，先减速到零，再换DIR并重新规划 */
    volatile bool position_replan_pending;
} Stepper_t;

extern Stepper_t Stepper1;

Stepper_Status_t Stepper_Init(Stepper_t *motor);
void Stepper_Enable(Stepper_t *motor);
void Stepper_Disable(Stepper_t *motor);

/* 无限速度运动：Immediate硬切速度；Ramp按给定加减速度平滑变化 */
Stepper_Status_t Stepper_RunImmediate(Stepper_t *motor, int32_t signed_speed_pps);
Stepper_Status_t Stepper_RunRamp(Stepper_t *motor, int32_t signed_target_pps,
                                uint32_t accel_pps2, uint32_t decel_pps2);

/* 定速定位：To使用绝对软件坐标，By使用相对脉冲数 */
Stepper_Status_t Stepper_MoveToConstant(Stepper_t *motor, int32_t target_position,
                                       uint32_t speed_pps);
Stepper_Status_t Stepper_MoveByConstant(Stepper_t *motor, int32_t delta_pulses,
                                       uint32_t speed_pps);

/* 梯形/三角形定位：距离不足以到达最高速度时会自然退化成三角形 */
Stepper_Status_t Stepper_MoveToTrapezoid(Stepper_t *motor, int32_t target_position,
                                        uint32_t max_speed_pps,
                                        uint32_t accel_pps2, uint32_t decel_pps2);
Stepper_Status_t Stepper_MoveByTrapezoid(Stepper_t *motor, int32_t delta_pulses,
                                        uint32_t max_speed_pps,
                                        uint32_t accel_pps2, uint32_t decel_pps2);

void Stepper_StopImmediate(Stepper_t *motor);
void Stepper_StopRamp(Stepper_t *motor, uint32_t decel_pps2);
/* 固定每1ms调用一次；只负责速度/制动规划，不负责脉冲计数 */
void Stepper_Tick1ms(Stepper_t *motor);
/* TIM更新中断每完成一个PWM周期调用一次，记录一枚有符号脉冲 */
void Stepper_PulseIRQHandler(Stepper_t *motor);

int32_t Stepper_GetPosition(const Stepper_t *motor);
Stepper_Status_t Stepper_SetPosition(Stepper_t *motor, int32_t position);
int32_t Stepper_GetSpeed(const Stepper_t *motor);
Stepper_State_t Stepper_GetState(const Stepper_t *motor);
bool Stepper_IsBusy(const Stepper_t *motor);

/* MyTimer中TIM9更新事件使用的无参适配回调 */
void Timer_Stepper1_Pulse_Callback(void);

#endif
