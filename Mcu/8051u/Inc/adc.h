#ifndef ADC_H
#define ADC_H

#include "mcu.h"


#define ADC_TRANS_NUM  64
#define ADC_CHANNEL_NUM 2

#define ADC_DMA_IRQHandler(val)				ADC_DMA_ISR(val) interrupt DMA_ADC_VECTOR

extern uint16_t xdata adc_dma_buffer[];
extern uint16_t ADC_raw_volts;
extern uint16_t ADC_raw_current;

extern void ADC_DMA_Callback(void);
extern void Activate_ADC_DMA(void);

#endif // !ADC_H