#ifndef __STEPPER_UART_H
#define __STEPPER_UART_H

#include "usart.h"
#include "stdbool.h"

#define STEPPER_UART_HUART             (huart3)
#define STEPPER_UART_RX_BUFFER_SIZE    255U

extern volatile bool rxFrameFlag;
extern volatile uint8_t rxCmd[STEPPER_UART_RX_BUFFER_SIZE];
extern volatile uint16_t rxCount;

void Stepper_UART_Init(void);

void Stepper_UART_RxCallback(uint16_t Size);

#endif
