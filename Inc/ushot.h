#ifndef USHOT_H
#define USHOT_H

#include "mcu.h"
#include "targets.h"

#define IC_UART_IRQHandler(val)			DMA_UR1R_ISR(val) INTERRUPT(DMA_UR1R_VECTOR) USING(3)

extern bool EDT_ARM_ENABLE;
extern bool EDT_ARMED;

extern uint8_t ushotcommand;
extern uint8_t last_ushot_command;
extern uint8_t last_command;
extern int16_t tocheck;

extern uint8_t command_counter;
extern uint8_t high_pin_counter;
extern uint16_t ushot_goodcounter;
extern uint16_t ushot_badcounter;

extern void UN_UART_Init(void);
extern void ushotDmaBuffer(void);
extern void computeUshotDMA(void);
extern void checkUshot(void);

#endif // !UART_H