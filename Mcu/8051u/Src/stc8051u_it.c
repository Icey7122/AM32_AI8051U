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
#include "serial_telemetry.h"

uint8_t interrupt_time = 0;


void CMP_IRQHandler(void)
{
	interruptRoutine();
	P4INTF = 0x00;		
}

void TWENTY_KHZ_IRQHandler(void)
{	
	TwentyKhzRoutine();
}

void COM_TIMER_IRQHandler(void)
{
	PeriodElapsedCallback();
}

void IC_IRQHandler(void)
{
    dma_buffer[interrupt_time++] = ((uint16_t)PWMB_CCR5H << 8 | PWMB_CCR5L);

	if (interrupt_time == buffersize / 2) {
		if(servoPwm){
            PWMB_CCER1 = 0x03;
		}
    }
	
	if (interrupt_time == buffersize) {
		transfercomplete();
		input_ready = 1;
        interrupt_time = 0;
    }
}


void ADC_DMA_IRQHandler(void)
{
	ADC_DMA_Callback();
	DMA_ADC_STA = 0x00;
}

void UART1_DMA_IRQHandler(void)
{
	DMA_UR1T_STA = 0x00;
}

