/*
 * stc8051u_it.c
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */
#include "input.h"
#include "main.h"
#include "stc8051u_it.h"
#include "adc.h"
#include "functions.h"
#include "phaseouts.h"
#include "commutate.h"
#include "peripherals.h"
#include "kiss_telemetry.h"

uint8_t ic_interrupt_counter = 0;

void CMP_IRQHandler(void)
{
	interruptRoutine();
	P4INTF = 0x00;		
}

void TWENTY_KHZ_IRQHandler(void)
{	
	tenKhzRoutine();
}

void COM_TIMER_IRQHandler(void)
{
	PeriodElapsedCallback();
}

void IC_IRQHandler(void)
{
	register PWM_TypeDef *PWMB = PWMB_ADDRESS;
	register union_int32_t *buffer = (union_int32_t *)dma_buffer + ic_interrupt_counter;

    buffer->u8[0] = PWMB->CCR1H;
    buffer->u8[1] = PWMB->CCR1L;
	buffer->u8[2] = PWMB->CCR2H;
	buffer->u8[3] = PWMB->CCR2L;
	ic_interrupt_counter++;
	
	if (ic_interrupt_counter == buffersize >> 1) {
		transfercomplete();
		ic_interrupt_counter = 0;
		INT3IF = 1;
    }
	PWMB->SR1 &= ~0x04;
}

void IC_UART_IRQHandler(void)
{
	transfercomplete();
	INT3IF = 1;
	DMA_UR1R_STA = 0x00;
}

void EXIT_IRQHandler(void)
{
	setInput();
}

void UART1_DMA_IRQHandler(void)
{
	DMA_UR1T_STA = 0x00;
}

