#ifndef UART_H
#define UART_H

#include "mcu.h"
#include "targets.h"

#define BAUD_RATE_RELOAD 	(uint16_t)(0x10000 - ((CPU_FREQUENCY_MHZ * 1000000) / 4 / BAUD_RATE))

extern void UART1_Init(void);

#endif // !UART_H