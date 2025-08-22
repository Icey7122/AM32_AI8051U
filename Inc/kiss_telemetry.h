#ifndef SERIAL_TELEMETRY_H
#define SERIAL_TELEMETRY_H

#include "mcu.h"
#include "uart.h"

#define UART1_DMA_IRQHandler(val)				UART1_DMA_ISR(val) INTERRUPT(DMA_UR1T_VECTOR)

extern uint8_t xdata aTxBuffer[];

extern void makeTelemPackage(float temp, float voltage, float current, float consumption, float e_rpm); 
extern void send_telem_DMA(void);
extern void telem_UART_Init(void);

#endif // SERIAL_TELEMETRY_H