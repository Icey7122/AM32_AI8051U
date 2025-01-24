#ifndef UART_H
#define UART_H

#include "mcu.h"
#include "targets.h"

#define BAUD_RATE_RELOAD (uint16_t)(0x10000 - ((CPU_FREQUENCY_MHZ * 1000000) / 4 / BAUD_RATE))

#define UART1_DMA_IRQHandler(val)				UART1_DMA_ISR(val) INTERRUPT(DMA_UR1T_VECTOR)

extern uint8_t xdata aTxBuffer[10];

extern void telem_UART1_Init(void);
extern void send_telem_DMA(void);


#endif // !UART_H