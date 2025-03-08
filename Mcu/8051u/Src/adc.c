#include "adc.h"
#include "main.h"
#include "functions.h"

uint16_t xdata adc_dma_buffer[ADC_CHANNEL_NUM * (ADC_TRANS_NUM + 2)] = {0};
uint16_t ADC_raw_volts;
uint16_t ADC_raw_current;

void ADC_DMA_Callback(void)
{
	ADC_raw_current = adc_dma_buffer[CURRENT_ADC_PIN * (ADC_TRANS_NUM + 2) - 1];
	ADC_raw_volts = adc_dma_buffer[VOLTAGE_ADC_PIN * (ADC_TRANS_NUM + 2) - 1];
}

void Activate_ADC_DMA(void)
{
	DMA_ADC_CR |= 0x40;      //DMA_ADC ENABLE
}