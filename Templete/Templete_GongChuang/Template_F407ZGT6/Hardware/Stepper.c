#include "Stepper.h"
#include <limits.h>

#define STEPPER_Q16_SHIFT          16
#define STEPPER_Q16_ONE            (1L << STEPPER_Q16_SHIFT)
#define STEPPER_TICK_HZ            1000U

Stepper_t Stepper1 = {
    .step_pwm = &MyPWM_Stepper1,
    .dir_gpio = &MyGPIO_Stepper_Dir,
    .en_gpio = 0,
    .positive_dir_level = 1U,
    .enable_active_level = 0U,
    .dir_setup_us = 2U,
    .min_speed_pps = 16U,
    .max_speed_pps = 10000U,
    .state = STEPPER_STATE_DISABLED,
    .last_status = STEPPER_STATUS_OK,
    .direction = 1
};

static uint32_t Stepper_EnterCritical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static void Stepper_ExitCritical(uint32_t primask)
{
    if (primask == 0U) {
        __enable_irq();
    }
}

static uint32_t Stepper_AbsI32(int32_t value)
{
    return (value < 0) ? (uint32_t)(-(int64_t)value) : (uint32_t)value;
}

static int8_t Stepper_SignI32(int32_t value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

static uint32_t Stepper_RampDeltaQ16(uint32_t rate_pps2)
{
    uint64_t delta = ((uint64_t)rate_pps2 << STEPPER_Q16_SHIFT) / STEPPER_TICK_HZ;
    if (delta == 0U && rate_pps2 > 0U) {
        delta = 1U;
    }
    if (delta > (uint64_t)INT32_MAX) {
        delta = (uint64_t)INT32_MAX;
    }
    return (uint32_t)delta;
}

static void Stepper_DelayUs(uint16_t delay_us)
{
    uint32_t start;
    uint32_t cycles_per_us;
    uint32_t wait_cycles;

    if (delay_us == 0U) return;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U) {
        DWT->CYCCNT = 0U;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }

    cycles_per_us = SystemCoreClock / 1000000U;
    wait_cycles = cycles_per_us * (uint32_t)delay_us;
    start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < wait_cycles) { }
}

static bool Stepper_IsPositionState(Stepper_State_t state)
{
    return state == STEPPER_STATE_POS_CONSTANT ||
           state == STEPPER_STATE_POS_ACCEL ||
           state == STEPPER_STATE_POS_CRUISE ||
           state == STEPPER_STATE_POS_DECEL;
}

static Stepper_Status_t Stepper_ValidateSpeed(const Stepper_t *motor, uint32_t speed_pps)
{
    if (motor == 0 || !motor->initialized) {
        return STEPPER_STATUS_NOT_INITIALIZED;
    }
    if (speed_pps < motor->min_speed_pps || speed_pps > motor->max_speed_pps) {
        return STEPPER_STATUS_FREQUENCY_RANGE;
    }
    return STEPPER_STATUS_OK;
}

static void Stepper_SetDirection(Stepper_t *motor, int8_t direction)
{
    uint8_t level;

    if (direction == 0 || motor->direction == direction) return;

    level = (direction > 0) ? motor->positive_dir_level : (uint8_t)!motor->positive_dir_level;
    MyGPIO_WritePin(motor->dir_gpio, level);
    motor->direction = direction;
    Stepper_DelayUs(motor->dir_setup_us);
}

static void Stepper_StopHardware(Stepper_t *motor)
{
    if (motor == 0 || motor->step_pwm == 0 || motor->step_pwm->htimx == 0) return;

    __HAL_TIM_DISABLE_IT(motor->step_pwm->htimx, TIM_IT_UPDATE);
    HAL_TIM_PWM_Stop(motor->step_pwm->htimx, motor->step_pwm->Channel);
    __HAL_TIM_CLEAR_FLAG(motor->step_pwm->htimx, TIM_FLAG_UPDATE);
    motor->pulse_running = false;
    motor->current_speed_pps = 0;
}

static Stepper_Status_t Stepper_SetPulseSpeed(Stepper_t *motor, int32_t signed_speed_pps,
                                              bool force_restart)
{
    TIM_HandleTypeDef *htim;
    uint32_t timer_clock;
    uint32_t counter_clock;
    uint32_t speed_pps;
    uint32_t arr;
    uint32_t compare;
    uint32_t actual_speed;
    int8_t direction;

    speed_pps = Stepper_AbsI32(signed_speed_pps);
    if (speed_pps == 0U) {
        Stepper_StopHardware(motor);
        return STEPPER_STATUS_OK;
    }
    if (Stepper_ValidateSpeed(motor, speed_pps) != STEPPER_STATUS_OK) {
        return STEPPER_STATUS_FREQUENCY_RANGE;
    }

    htim = motor->step_pwm->htimx;
    timer_clock = MyPWM_GetTimClock(motor->step_pwm);
    if (timer_clock == 0U || htim->Instance->PSC == UINT32_MAX) {
        return STEPPER_STATUS_INVALID_ARGUMENT;
    }
    counter_clock = timer_clock / (htim->Instance->PSC + 1U);
    if (counter_clock == 0U || speed_pps > counter_clock) {
        return STEPPER_STATUS_FREQUENCY_RANGE;
    }

    arr = counter_clock / speed_pps;
    if (arr == 0U) {
        return STEPPER_STATUS_FREQUENCY_RANGE;
    }
    arr -= 1U;
    if (arr < 1U || arr > 65535U) {
        return STEPPER_STATUS_FREQUENCY_RANGE;
    }
    compare = (arr + 1U) / 2U;
    if (compare == 0U) compare = 1U;
    actual_speed = counter_clock / (arr + 1U);
    direction = Stepper_SignI32(signed_speed_pps);

    if (force_restart || !motor->pulse_running || motor->direction != direction) {
        Stepper_StopHardware(motor);
        Stepper_SetDirection(motor, direction);
        __HAL_TIM_SET_AUTORELOAD(htim, arr);
        __HAL_TIM_SET_COMPARE(htim, motor->step_pwm->Channel, compare);
        htim->Instance->EGR = TIM_EGR_UG;
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
        __HAL_TIM_SET_COUNTER(htim, 0U);
        if (HAL_TIM_PWM_Start(htim, motor->step_pwm->Channel) != HAL_OK) {
            return STEPPER_STATUS_INVALID_ARGUMENT;
        }
        __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
        __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
        motor->pulse_running = true;
    } else {
        __HAL_TIM_SET_AUTORELOAD(htim, arr);
        __HAL_TIM_SET_COMPARE(htim, motor->step_pwm->Channel, compare);
    }

    motor->current_speed_pps = (direction > 0) ? (int32_t)actual_speed : -(int32_t)actual_speed;
    return STEPPER_STATUS_OK;
}

static Stepper_Status_t Stepper_SetMoveDistance(Stepper_t *motor, int32_t target_position)
{
    int64_t distance = (int64_t)target_position - (int64_t)motor->position_pulses;
    uint64_t magnitude = (distance < 0) ? (uint64_t)(-distance) : (uint64_t)distance;

    if (magnitude > UINT32_MAX) {
        return STEPPER_STATUS_POSITION_OVERFLOW;
    }

    motor->target_position = target_position;
    motor->move_total_pulses = (uint32_t)magnitude;
    motor->move_done_pulses = 0U;
    motor->move_remaining_pulses = (uint32_t)magnitude;
    return STEPPER_STATUS_OK;
}

static uint32_t Stepper_GetBrakeDistance(const Stepper_t *motor)
{
    uint32_t speed = Stepper_AbsI32(motor->current_speed_pps);
    uint64_t speed_sq;

    if (motor->decel_pps2 == 0U) return UINT32_MAX;
    speed_sq = (uint64_t)speed * (uint64_t)speed;
    return (uint32_t)(speed_sq / (2ULL * motor->decel_pps2));
}

static void Stepper_FinishMotion(Stepper_t *motor)
{
    Stepper_StopHardware(motor);
    motor->target_speed_pps = 0;
    motor->speed_q16 = 0;
    motor->target_speed_q16 = 0;
    motor->position_replan_pending = false;
    if (motor->state != STEPPER_STATE_DISABLED && motor->state != STEPPER_STATE_ERROR) {
        motor->state = STEPPER_STATE_IDLE;
    }
}

static Stepper_Status_t Stepper_StartTrapezoidFromCurrent(Stepper_t *motor)
{
    int64_t distance = (int64_t)motor->target_position - (int64_t)motor->position_pulses;
    int8_t direction = (distance >= 0) ? 1 : -1;
    int32_t start_speed;
    Stepper_Status_t status;

    status = Stepper_SetMoveDistance(motor, motor->target_position);
    if (status != STEPPER_STATUS_OK) return status;
    if (motor->move_remaining_pulses == 0U) {
        Stepper_FinishMotion(motor);
        return STEPPER_STATUS_OK;
    }

    start_speed = direction * (int32_t)motor->min_speed_pps;
    motor->speed_q16 = start_speed * STEPPER_Q16_ONE;
    motor->target_speed_q16 = direction * (int32_t)motor->max_motion_speed_pps * STEPPER_Q16_ONE;
    motor->target_speed_pps = direction * (int32_t)motor->max_motion_speed_pps;
    motor->position_replan_pending = false;
    motor->state = STEPPER_STATE_POS_ACCEL;
    return Stepper_SetPulseSpeed(motor, start_speed, true);
}

Stepper_Status_t Stepper_Init(Stepper_t *motor)
{
    TIM_HandleTypeDef *htim;

    if (motor == 0 || motor->step_pwm == 0 || motor->step_pwm->htimx == 0 ||
        motor->dir_gpio == 0 || motor->min_speed_pps == 0U ||
        motor->max_speed_pps < motor->min_speed_pps) {
        return STEPPER_STATUS_INVALID_ARGUMENT;
    }

    htim = motor->step_pwm->htimx;
    Stepper_StopHardware(motor);
    htim->Instance->CR1 |= TIM_CR1_ARPE;
    if (motor->step_pwm->Channel == TIM_CHANNEL_1) {
        htim->Instance->CCMR1 |= TIM_CCMR1_OC1PE;
    } else if (motor->step_pwm->Channel == TIM_CHANNEL_2) {
        htim->Instance->CCMR1 |= TIM_CCMR1_OC2PE;
    } else if (motor->step_pwm->Channel == TIM_CHANNEL_3) {
        htim->Instance->CCMR2 |= TIM_CCMR2_OC3PE;
    } else if (motor->step_pwm->Channel == TIM_CHANNEL_4) {
        htim->Instance->CCMR2 |= TIM_CCMR2_OC4PE;
    } else {
        return STEPPER_STATUS_INVALID_ARGUMENT;
    }

    // STEP脉冲计数必须优先于1ms/10ms/20ms任务，防止PWM持续输出时漏计更新事件
    HAL_NVIC_SetPriority(motor->step_pwm->Tim_IRQn, 0U, 0U);
    NVIC_ClearPendingIRQ(motor->step_pwm->Tim_IRQn);
    HAL_NVIC_EnableIRQ(motor->step_pwm->Tim_IRQn);

    motor->initialized = true;
    motor->enabled = false;
    motor->pulse_running = false;
    motor->direction = 1;
    motor->position_pulses = 0;
    motor->target_position = 0;
    motor->move_total_pulses = 0U;
    motor->move_done_pulses = 0U;
    motor->move_remaining_pulses = 0U;
    motor->current_speed_pps = 0;
    motor->target_speed_pps = 0;
    motor->max_motion_speed_pps = 0U;
    motor->accel_pps2 = 0U;
    motor->decel_pps2 = 0U;
    motor->speed_q16 = 0;
    motor->target_speed_q16 = 0;
    motor->position_replan_pending = false;
    motor->last_status = STEPPER_STATUS_OK;
    motor->state = STEPPER_STATE_DISABLED;
    MyGPIO_WritePin(motor->dir_gpio, motor->positive_dir_level);
    if (motor->en_gpio != 0) {
        MyGPIO_WritePin(motor->en_gpio, !motor->enable_active_level);
    }
    return STEPPER_STATUS_OK;
}

void Stepper_Enable(Stepper_t *motor)
{
    if (motor == 0 || !motor->initialized) return;
    if (motor->en_gpio != 0) {
        MyGPIO_WritePin(motor->en_gpio, motor->enable_active_level);
    }
    motor->enabled = true;
    if (motor->state == STEPPER_STATE_DISABLED) {
        motor->state = STEPPER_STATE_IDLE;
    }
}

void Stepper_Disable(Stepper_t *motor)
{
    uint32_t primask;

    if (motor == 0) return;
    primask = Stepper_EnterCritical();
    Stepper_StopHardware(motor);
    if (motor->en_gpio != 0) {
        MyGPIO_WritePin(motor->en_gpio, !motor->enable_active_level);
    }
    motor->enabled = false;
    motor->speed_q16 = 0;
    motor->target_speed_q16 = 0;
    motor->target_speed_pps = 0;
    motor->position_replan_pending = false;
    motor->state = STEPPER_STATE_DISABLED;
    Stepper_ExitCritical(primask);
}

Stepper_Status_t Stepper_RunImmediate(Stepper_t *motor, int32_t signed_speed_pps)
{
    uint32_t primask;
    Stepper_Status_t status;

    if (motor == 0 || !motor->initialized) return STEPPER_STATUS_NOT_INITIALIZED;
    if (!motor->enabled) return STEPPER_STATUS_INVALID_ARGUMENT;
    if (signed_speed_pps == 0) {
        Stepper_StopImmediate(motor);
        return STEPPER_STATUS_OK;
    }
    status = Stepper_ValidateSpeed(motor, Stepper_AbsI32(signed_speed_pps));
    if (status != STEPPER_STATUS_OK) return status;

    primask = Stepper_EnterCritical();
    motor->target_position = motor->position_pulses;
    motor->move_total_pulses = 0U;
    motor->move_done_pulses = 0U;
    motor->move_remaining_pulses = 0U;
    motor->target_speed_pps = signed_speed_pps;
    motor->speed_q16 = signed_speed_pps * STEPPER_Q16_ONE;
    motor->target_speed_q16 = motor->speed_q16;
    motor->position_replan_pending = false;
    status = Stepper_SetPulseSpeed(motor, signed_speed_pps, true);
    if (status == STEPPER_STATUS_OK) {
        motor->state = STEPPER_STATE_SPEED_IMMEDIATE;
    }
    motor->last_status = status;
    Stepper_ExitCritical(primask);
    return status;
}

Stepper_Status_t Stepper_RunRamp(Stepper_t *motor, int32_t signed_target_pps,
                                uint32_t accel_pps2, uint32_t decel_pps2)
{
    uint32_t primask;
    Stepper_Status_t status;
    int8_t direction;

    if (motor == 0 || !motor->initialized) return STEPPER_STATUS_NOT_INITIALIZED;
    if (!motor->enabled || accel_pps2 == 0U || decel_pps2 == 0U) {
        return STEPPER_STATUS_INVALID_ARGUMENT;
    }
    if (signed_target_pps != 0) {
        status = Stepper_ValidateSpeed(motor, Stepper_AbsI32(signed_target_pps));
        if (status != STEPPER_STATUS_OK) return status;
    }

    primask = Stepper_EnterCritical();
    motor->target_position = motor->position_pulses;
    motor->move_total_pulses = 0U;
    motor->move_done_pulses = 0U;
    motor->move_remaining_pulses = 0U;
    motor->accel_pps2 = accel_pps2;
    motor->decel_pps2 = decel_pps2;
    motor->target_speed_pps = signed_target_pps;
    motor->target_speed_q16 = signed_target_pps * STEPPER_Q16_ONE;
    motor->position_replan_pending = false;
    motor->state = STEPPER_STATE_SPEED_RAMP;

    if (!motor->pulse_running && signed_target_pps == 0) {
        motor->speed_q16 = 0;
        motor->state = STEPPER_STATE_IDLE;
        status = STEPPER_STATUS_OK;
    } else if (!motor->pulse_running) {
        direction = Stepper_SignI32(signed_target_pps);
        motor->speed_q16 = direction * (int32_t)motor->min_speed_pps * STEPPER_Q16_ONE;
        status = Stepper_SetPulseSpeed(motor,
                                      direction * (int32_t)motor->min_speed_pps, true);
    } else {
        status = STEPPER_STATUS_OK;
    }
    motor->last_status = status;
    Stepper_ExitCritical(primask);
    return status;
}

Stepper_Status_t Stepper_MoveToConstant(Stepper_t *motor, int32_t target_position,
                                       uint32_t speed_pps)
{
    uint32_t primask;
    Stepper_Status_t status;
    int8_t direction;

    status = Stepper_ValidateSpeed(motor, speed_pps);
    if (status != STEPPER_STATUS_OK) return status;
    if (!motor->enabled) return STEPPER_STATUS_INVALID_ARGUMENT;

    primask = Stepper_EnterCritical();
    Stepper_StopHardware(motor);
    status = Stepper_SetMoveDistance(motor, target_position);
    if (status == STEPPER_STATUS_OK && motor->move_remaining_pulses > 0U) {
        direction = (target_position > motor->position_pulses) ? 1 : -1;
        motor->target_speed_pps = direction * (int32_t)speed_pps;
        motor->speed_q16 = motor->target_speed_pps * STEPPER_Q16_ONE;
        motor->target_speed_q16 = motor->speed_q16;
        motor->position_replan_pending = false;
        status = Stepper_SetPulseSpeed(motor, motor->target_speed_pps, true);
        if (status == STEPPER_STATUS_OK) motor->state = STEPPER_STATE_POS_CONSTANT;
    } else if (status == STEPPER_STATUS_OK) {
        motor->state = STEPPER_STATE_IDLE;
    }
    motor->last_status = status;
    Stepper_ExitCritical(primask);
    return status;
}

Stepper_Status_t Stepper_MoveByConstant(Stepper_t *motor, int32_t delta_pulses,
                                       uint32_t speed_pps)
{
    uint32_t primask;
    int64_t target;
    Stepper_Status_t status;

    if (motor == 0 || !motor->initialized) return STEPPER_STATUS_NOT_INITIALIZED;
    primask = Stepper_EnterCritical();
    target = (int64_t)motor->position_pulses + (int64_t)delta_pulses;
    if (target > INT32_MAX || target < INT32_MIN) {
        Stepper_ExitCritical(primask);
        return STEPPER_STATUS_POSITION_OVERFLOW;
    }
    status = Stepper_MoveToConstant(motor, (int32_t)target, speed_pps);
    Stepper_ExitCritical(primask);
    return status;
}

Stepper_Status_t Stepper_MoveToTrapezoid(Stepper_t *motor, int32_t target_position,
                                        uint32_t max_speed_pps,
                                        uint32_t accel_pps2, uint32_t decel_pps2)
{
    uint32_t primask;
    Stepper_Status_t status;
    int8_t desired_direction;

    status = Stepper_ValidateSpeed(motor, max_speed_pps);
    if (status != STEPPER_STATUS_OK) return status;
    if (!motor->enabled || accel_pps2 == 0U || decel_pps2 == 0U) {
        return STEPPER_STATUS_INVALID_ARGUMENT;
    }

    primask = Stepper_EnterCritical();
    motor->target_position = target_position;
    motor->max_motion_speed_pps = max_speed_pps;
    motor->accel_pps2 = accel_pps2;
    motor->decel_pps2 = decel_pps2;
    desired_direction = (target_position >= motor->position_pulses) ? 1 : -1;

    if (target_position == motor->position_pulses) {
        Stepper_FinishMotion(motor);
        status = STEPPER_STATUS_OK;
    } else if (motor->pulse_running && motor->direction != desired_direction) {
        status = Stepper_SetMoveDistance(motor, target_position);
        motor->position_replan_pending = true;
        motor->state = STEPPER_STATE_POS_DECEL;
    } else {
        status = Stepper_SetMoveDistance(motor, target_position);
        if (status == STEPPER_STATUS_OK) {
            if (!motor->pulse_running) {
                status = Stepper_StartTrapezoidFromCurrent(motor);
            } else {
                motor->position_replan_pending = false;
                motor->target_speed_pps = desired_direction * (int32_t)max_speed_pps;
                motor->target_speed_q16 = motor->target_speed_pps * STEPPER_Q16_ONE;
                motor->speed_q16 = motor->current_speed_pps * STEPPER_Q16_ONE;
                motor->state = (motor->move_remaining_pulses <= Stepper_GetBrakeDistance(motor))
                             ? STEPPER_STATE_POS_DECEL : STEPPER_STATE_POS_ACCEL;
            }
        }
    }
    motor->last_status = status;
    Stepper_ExitCritical(primask);
    return status;
}

Stepper_Status_t Stepper_MoveByTrapezoid(Stepper_t *motor, int32_t delta_pulses,
                                        uint32_t max_speed_pps,
                                        uint32_t accel_pps2, uint32_t decel_pps2)
{
    uint32_t primask;
    int64_t target;
    Stepper_Status_t status;

    if (motor == 0 || !motor->initialized) return STEPPER_STATUS_NOT_INITIALIZED;
    primask = Stepper_EnterCritical();
    target = (int64_t)motor->position_pulses + (int64_t)delta_pulses;
    if (target > INT32_MAX || target < INT32_MIN) {
        Stepper_ExitCritical(primask);
        return STEPPER_STATUS_POSITION_OVERFLOW;
    }
    status = Stepper_MoveToTrapezoid(motor, (int32_t)target, max_speed_pps,
                                     accel_pps2, decel_pps2);
    Stepper_ExitCritical(primask);
    return status;
}

void Stepper_StopImmediate(Stepper_t *motor)
{
    uint32_t primask;

    if (motor == 0 || !motor->initialized) return;
    primask = Stepper_EnterCritical();
    Stepper_FinishMotion(motor);
    motor->target_position = motor->position_pulses;
    motor->move_total_pulses = 0U;
    motor->move_done_pulses = 0U;
    motor->move_remaining_pulses = 0U;
    Stepper_ExitCritical(primask);
}

void Stepper_StopRamp(Stepper_t *motor, uint32_t decel_pps2)
{
    uint32_t primask;

    if (motor == 0 || !motor->initialized || decel_pps2 == 0U) return;
    if (!motor->pulse_running) {
        Stepper_StopImmediate(motor);
        return;
    }
    primask = Stepper_EnterCritical();
    motor->target_position = motor->position_pulses;
    motor->move_total_pulses = 0U;
    motor->move_done_pulses = 0U;
    motor->move_remaining_pulses = 0U;
    motor->decel_pps2 = decel_pps2;
    motor->target_speed_pps = 0;
    motor->target_speed_q16 = 0;
    motor->position_replan_pending = false;
    motor->state = STEPPER_STATE_SPEED_RAMP;
    Stepper_ExitCritical(primask);
}

static void Stepper_TickSpeedRamp(Stepper_t *motor)
{
    int32_t current = motor->speed_q16;
    int32_t target = motor->target_speed_q16;
    uint32_t accel_delta = Stepper_RampDeltaQ16(motor->accel_pps2);
    uint32_t decel_delta = Stepper_RampDeltaQ16(motor->decel_pps2);
    int8_t current_sign = Stepper_SignI32(current);
    int8_t target_sign = Stepper_SignI32(target);
    uint32_t delta;
    uint32_t output_speed;
    int32_t next;

    if (current == target) return;

    if (target == 0 || (current_sign != 0 && current_sign != target_sign)) {
        delta = decel_delta;
        if (Stepper_AbsI32(current) <= delta) {
            next = 0;
        } else {
            next = current - current_sign * (int32_t)delta;
        }
    } else {
        bool speeding_up = Stepper_AbsI32(target) > Stepper_AbsI32(current);
        delta = speeding_up ? accel_delta : decel_delta;
        if (Stepper_AbsI32(target - current) <= delta) {
            next = target;
        } else {
            next = current + target_sign * (int32_t)delta;
        }
    }

    if (next == 0) {
        Stepper_StopHardware(motor);
        motor->speed_q16 = 0;
        if (target == 0) {
            motor->state = STEPPER_STATE_IDLE;
            return;
        }
        current_sign = target_sign;
        next = current_sign * (int32_t)motor->min_speed_pps * STEPPER_Q16_ONE;
        motor->speed_q16 = next;
        Stepper_SetPulseSpeed(motor, current_sign * (int32_t)motor->min_speed_pps, true);
        return;
    }

    motor->speed_q16 = next;
    output_speed = Stepper_AbsI32(next / STEPPER_Q16_ONE);
    if (output_speed < motor->min_speed_pps) {
        output_speed = motor->min_speed_pps;
    }
    Stepper_SetPulseSpeed(motor, Stepper_SignI32(next) * (int32_t)output_speed, false);
}

static void Stepper_TickPositionPlan(Stepper_t *motor)
{
    int8_t direction = motor->direction;
    uint32_t accel_delta = Stepper_RampDeltaQ16(motor->accel_pps2);
    uint32_t decel_delta = Stepper_RampDeltaQ16(motor->decel_pps2);
    uint32_t speed_q16 = Stepper_AbsI32(motor->speed_q16);
    uint32_t min_q16 = motor->min_speed_pps * STEPPER_Q16_ONE;
    uint32_t max_q16 = motor->max_motion_speed_pps * STEPPER_Q16_ONE;
    uint32_t brake_distance;

    if (motor->move_remaining_pulses == 0U && !motor->position_replan_pending) {
        Stepper_FinishMotion(motor);
        return;
    }

    if (motor->position_replan_pending) {
        if (speed_q16 <= decel_delta || speed_q16 <= min_q16) {
            Stepper_StopHardware(motor);
            motor->position_replan_pending = false;
            if (Stepper_StartTrapezoidFromCurrent(motor) != STEPPER_STATUS_OK) {
                motor->state = STEPPER_STATE_ERROR;
            }
            return;
        }
        speed_q16 -= decel_delta;
        motor->speed_q16 = direction * (int32_t)speed_q16;
        Stepper_SetPulseSpeed(motor, direction * (int32_t)(speed_q16 / STEPPER_Q16_ONE), false);
        return;
    }

    brake_distance = Stepper_GetBrakeDistance(motor);
    if (motor->state != STEPPER_STATE_POS_DECEL &&
        motor->move_remaining_pulses <= brake_distance + 1U) {
        motor->state = STEPPER_STATE_POS_DECEL;
    }

    if (motor->state == STEPPER_STATE_POS_ACCEL) {
        if (speed_q16 + accel_delta >= max_q16) {
            speed_q16 = max_q16;
            motor->state = STEPPER_STATE_POS_CRUISE;
        } else {
            speed_q16 += accel_delta;
        }
    } else if (motor->state == STEPPER_STATE_POS_DECEL) {
        if (speed_q16 > min_q16 + decel_delta) {
            speed_q16 -= decel_delta;
        } else {
            speed_q16 = min_q16;
        }
    }

    motor->speed_q16 = direction * (int32_t)speed_q16;
    Stepper_SetPulseSpeed(motor, direction * (int32_t)(speed_q16 / STEPPER_Q16_ONE), false);
}

void Stepper_Tick1ms(Stepper_t *motor)
{
    if (motor == 0 || !motor->initialized || !motor->enabled) return;

    if (motor->state == STEPPER_STATE_SPEED_RAMP) {
        Stepper_TickSpeedRamp(motor);
    } else if (motor->state == STEPPER_STATE_POS_ACCEL ||
               motor->state == STEPPER_STATE_POS_CRUISE ||
               motor->state == STEPPER_STATE_POS_DECEL) {
        Stepper_TickPositionPlan(motor);
    }
}

void Stepper_PulseIRQHandler(Stepper_t *motor)
{
    if (motor == 0 || !motor->pulse_running) return;

    if ((motor->direction > 0 && motor->position_pulses == INT32_MAX) ||
        (motor->direction < 0 && motor->position_pulses == INT32_MIN)) {
        motor->last_status = STEPPER_STATUS_POSITION_OVERFLOW;
        motor->state = STEPPER_STATE_ERROR;
        Stepper_StopHardware(motor);
        return;
    }

    motor->position_pulses += motor->direction;

    if (Stepper_IsPositionState(motor->state)) {
        if (motor->position_replan_pending) {
            int64_t distance = (int64_t)motor->target_position -
                               (int64_t)motor->position_pulses;
            uint64_t magnitude = (distance < 0) ? (uint64_t)(-distance) :
                                                  (uint64_t)distance;
            motor->move_remaining_pulses = (magnitude > UINT32_MAX) ?
                                            UINT32_MAX : (uint32_t)magnitude;
            return;
        }
        if (motor->move_done_pulses < UINT32_MAX) {
            motor->move_done_pulses++;
        }
        if (motor->move_remaining_pulses > 0U) {
            motor->move_remaining_pulses--;
        }
        if (motor->move_remaining_pulses == 0U ||
            motor->position_pulses == motor->target_position) {
            motor->position_pulses = motor->target_position;
            Stepper_FinishMotion(motor);
        }
    }
}

int32_t Stepper_GetPosition(const Stepper_t *motor)
{
    return (motor == 0) ? 0 : motor->position_pulses;
}

Stepper_Status_t Stepper_SetPosition(Stepper_t *motor, int32_t position)
{
    if (motor == 0 || !motor->initialized) return STEPPER_STATUS_NOT_INITIALIZED;
    if (Stepper_IsBusy(motor)) return STEPPER_STATUS_BUSY;
    motor->position_pulses = position;
    motor->target_position = position;
    return STEPPER_STATUS_OK;
}

int32_t Stepper_GetSpeed(const Stepper_t *motor)
{
    return (motor == 0) ? 0 : motor->current_speed_pps;
}

Stepper_State_t Stepper_GetState(const Stepper_t *motor)
{
    return (motor == 0) ? STEPPER_STATE_ERROR : motor->state;
}

bool Stepper_IsBusy(const Stepper_t *motor)
{
    if (motor == 0) return false;
    return motor->state != STEPPER_STATE_DISABLED &&
           motor->state != STEPPER_STATE_IDLE &&
           motor->state != STEPPER_STATE_ERROR;
}

void Timer_Stepper1_Pulse_Callback(void)
{
    Stepper_PulseIRQHandler(&Stepper1);
}
