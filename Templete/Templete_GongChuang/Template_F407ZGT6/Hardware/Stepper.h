#ifndef __STEPPER_H
#define __STEPPER_H

#include "MySystem.h"

typedef enum
{
    STEPPER_STATE_DISABLED = 0,
    STEPPER_STATE_IDLE,
    STEPPER_STATE_SPEED_IMMEDIATE,
    STEPPER_STATE_SPEED_RAMP,
    STEPPER_STATE_POS_CONSTANT,
    STEPPER_STATE_POS_ACCEL,
    STEPPER_STATE_POS_CRUISE,
    STEPPER_STATE_POS_DECEL,
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
    MyPWM_Typedef *step_pwm;
    MyGPIO_Typedef *dir_gpio;
    MyGPIO_Typedef *en_gpio;
    uint8_t positive_dir_level;
    uint8_t enable_active_level;
    uint16_t dir_setup_us;
    uint32_t min_speed_pps;
    uint32_t max_speed_pps;

    volatile Stepper_State_t state;
    volatile Stepper_Status_t last_status;
    volatile bool initialized;
    volatile bool enabled;
    volatile bool pulse_running;
    volatile int8_t direction;

    volatile int32_t position_pulses;
    volatile int32_t target_position;
    volatile uint32_t move_total_pulses;
    volatile uint32_t move_done_pulses;
    volatile uint32_t move_remaining_pulses;

    volatile int32_t current_speed_pps;
    volatile int32_t target_speed_pps;
    volatile uint32_t max_motion_speed_pps;
    volatile uint32_t accel_pps2;
    volatile uint32_t decel_pps2;

    int32_t speed_q16;
    int32_t target_speed_q16;
    volatile bool position_replan_pending;
} Stepper_t;

extern Stepper_t Stepper1;

Stepper_Status_t Stepper_Init(Stepper_t *motor);
void Stepper_Enable(Stepper_t *motor);
void Stepper_Disable(Stepper_t *motor);

Stepper_Status_t Stepper_RunImmediate(Stepper_t *motor, int32_t signed_speed_pps);
Stepper_Status_t Stepper_RunRamp(Stepper_t *motor, int32_t signed_target_pps,
                                uint32_t accel_pps2, uint32_t decel_pps2);

Stepper_Status_t Stepper_MoveToConstant(Stepper_t *motor, int32_t target_position,
                                       uint32_t speed_pps);
Stepper_Status_t Stepper_MoveByConstant(Stepper_t *motor, int32_t delta_pulses,
                                       uint32_t speed_pps);

Stepper_Status_t Stepper_MoveToTrapezoid(Stepper_t *motor, int32_t target_position,
                                        uint32_t max_speed_pps,
                                        uint32_t accel_pps2, uint32_t decel_pps2);
Stepper_Status_t Stepper_MoveByTrapezoid(Stepper_t *motor, int32_t delta_pulses,
                                        uint32_t max_speed_pps,
                                        uint32_t accel_pps2, uint32_t decel_pps2);

void Stepper_StopImmediate(Stepper_t *motor);
void Stepper_StopRamp(Stepper_t *motor, uint32_t decel_pps2);
void Stepper_Tick1ms(Stepper_t *motor);
void Stepper_PulseIRQHandler(Stepper_t *motor);

int32_t Stepper_GetPosition(const Stepper_t *motor);
Stepper_Status_t Stepper_SetPosition(Stepper_t *motor, int32_t position);
int32_t Stepper_GetSpeed(const Stepper_t *motor);
Stepper_State_t Stepper_GetState(const Stepper_t *motor);
bool Stepper_IsBusy(const Stepper_t *motor);

#endif
