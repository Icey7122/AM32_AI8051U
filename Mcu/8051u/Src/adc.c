#include "adc.h"
#include "main.h"
#include "functions.h"

uint16_t xdata adc_dma_buffer[ADC_CHANNEL_NUM * (ADC_TRANS_NUM + 2)] = {0};
uint16_t ADC_raw_volts;
uint16_t ADC_raw_current;

void ADC_Init(void) {

	gpio_mode_set(P1,GPIO_ModePin_6 | GPIO_ModePin_7,GPIO_Mode_AIN);

	ADC_POWER = 1;            //ADC POWER ON
	delayMillis(5);           //ADC POWER-UP DELAY
	RESFMT = 1;               //RESFMT(1):RIGHT-JUSTIFIED

	ADCCFG &= ~0x0f;		//SPEED(0)
	ADCTIM = 0x38;			//CSSETUP(0), CSHOLD(1), SMPDUTY(24)

	DMA_ADC_RXAH = (uint16_t)adc_dma_buffer >> 8;
	DMA_ADC_RXAL = (uint16_t)adc_dma_buffer & 0xFF;    //DMA_ADC BUFFER ADDRESS

	DMA_ADC_CFG2 = 0x0D;	//DMA_ADC Transfer 64 times

	DMA_ADC_CHSW0 = VOLTAGE_ADC_CHANNEL | CURRENT_ADC_CHANNEL;	//CH7 CH6
	DMA_ADC_CHSW1 = 0x00; 

	DMA_ADC_AMT = 0x00;
	DMA_ADC_AMTH = 0x00;    //设置ADC转换1次

	DMA_ADC_ITVH = 0x00;
	DMA_ADC_ITVL = 0x00;    //设置ADC速度

	DMA_ADC_CFG = 0x00;       //DMA INTERRUPT DISABLE 最低优先级，数据最低优先级

	DMA_ADC_CR = 0x80;      //DMA_ADC ENABLE
	DMA_ADC_CR |= 0x40;      //DMA_ADC ENABLE
}

void ADC_DMA_Callback(void)
{
	ADC_raw_current = adc_dma_buffer[CURRENT_ADC_PIN * (ADC_TRANS_NUM + 2) - 1];
	ADC_raw_volts = adc_dma_buffer[VOLTAGE_ADC_PIN * (ADC_TRANS_NUM + 2) - 1];
}

void Activate_ADC_DMA(void)
{
	DMA_ADC_CR |= 0x40;      //DMA_ADC ENABLE
}