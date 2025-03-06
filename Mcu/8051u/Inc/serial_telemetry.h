#ifndef SERIAL_TELEMETRY_H
#define SERIAL_TELEMETRY_H

#include "mcu.h"
#include "uart.h"

#define UART1_DMA_IRQHandler(val)				UART1_DMA_ISR(val) INTERRUPT(DMA_UR1T_VECTOR)

extern uint8_t xdata aTxBuffer[];

extern uint8_t update_crc8(uint8_t crc, uint8_t crc_seed);
extern uint8_t get_crc8(uint8_t* Buf, uint8_t BufLen);
extern void makeTelemPackage(float temp, float voltage, float current, float consumption, float e_rpm); 
extern void send_telem_DMA(void);
extern void telem_UART_Init(void);

#endif // SERIAL_TELEMETRY_H