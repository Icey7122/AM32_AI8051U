/*
 * stc8051u_it.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */

#ifndef INC_STC8051U_IT_H_
#define INC_STC8051U_IT_H_

#include "mcu.h"
#include "targets.h"

#define CMP_IRQHandler(val)				CMP_ISR(val)  INTERRUPT(P4INT_VECTOR) USING(2)
#define TWENTY_KHZ_IRQHandler(val)		TMR4_ISR(val) INTERRUPT(TMR4_VECTOR)
#define COM_TIMER_IRQHandler(val)		TMR0_ISR(val) INTERRUPT(TMR0_VECTOR) USING(1)
#define EXIT_IRQHandler(val)			EXIT3_ISR(val) INTERRUPT(INT3_VECTOR)

#endif /* INC_STC8051U_IT_H_ */
