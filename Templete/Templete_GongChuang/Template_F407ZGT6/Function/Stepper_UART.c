#include "Stepper_UART.h"
#include "dma.h"
#include "usart.h"
#include "string.h"

volatile bool rxFrameFlag = false;
volatile uint8_t rxCmd[STEPPER_UART_RX_BUFFER_SIZE] = {0};
volatile uint16_t rxCount = 0;

static void Stepper_UART_StartReceive(void)
{
	if (HAL_UARTEx_ReceiveToIdle_DMA(&STEPPER_UART_HUART,
										(uint8_t *)rxCmd,
										STEPPER_UART_RX_BUFFER_SIZE) == HAL_OK)
	{
		/* 步进电机回包按空闲帧处理，不需要半传输事件。 */
		__HAL_DMA_DISABLE_IT(STEPPER_UART_HUART.hdmarx, DMA_IT_HT);
	}
}

void Stepper_UART_Init(void)
{
	rxFrameFlag = false;
	rxCount = 0;
	memset((void *)rxCmd, 0, sizeof(rxCmd));
	Stepper_UART_StartReceive();
}

void Stepper_UART_RxCallback(uint16_t Size)
{
	if (Size > STEPPER_UART_RX_BUFFER_SIZE)
	{
		Size = STEPPER_UART_RX_BUFFER_SIZE;
	}

	rxCount = Size;
	rxFrameFlag = true;

	/* 普通模式下每收到一帧都需要重新启动DMA接收。 */
	Stepper_UART_StartReceive();
}

