#include "kiss_telemetry.h"

uint8_t xdata aTxBuffer[20] = {0};

void telem_UART_Init(void)
{
    gpio_mode_set(P3,GPIO_ModePin_1,GPIO_Mode_Out_PP);

	DMA_UR1T_CFG = 0x81; 	//启用UR1T_DMA中断 数据优先级1,最低中断优先级
	DMA_UR1T_CR = 0x80;

	DMA_UR1T_AMTH = (uint16_t)(sizeof(aTxBuffer) - 1) >> 8;
	DMA_UR1T_AMT = (uint16_t)(sizeof(aTxBuffer) - 1);	//设置DMA传输数据长度20个字节

	DMA_UR1T_TXAH = (uint16_t)aTxBuffer >> 8;
	DMA_UR1T_TXAL = (uint16_t)aTxBuffer & 0xFF;	//设置DMA传输数据地址

	DMA_UR1_ITVH = 0x00;
	DMA_UR1_ITVL = 0x00;	//设置DMA传输速度

    aTxBuffer[16] = 0x00;
    aTxBuffer[17] = 0x00;
    aTxBuffer[18] = 0x80;
    aTxBuffer[19] = 0x7F;
}

void send_telem_DMA(void)
{
	DMA_UR1T_CR = 0xC0;	//启动UR1T_DMA传输
}

void makeTelemPackage(float temp, float voltage, float current, float consumption, float e_rpm)
{
    register uint8_t *p = aTxBuffer;
    register union_float_t value;
    value.Flt = voltage;
    p[0] = value.u8[3];
    p[1] = value.u8[2];
    p[2] = value.u8[1];
    p[3] = value.u8[0];

    value.Flt = current;
    p[4] = value.u8[3];
    p[5] = value.u8[2];
    p[6] = value.u8[1];
    p[7] = value.u8[0];

    value.Flt = consumption;
    p[8] =  value.u8[3];
    p[9] =  value.u8[2];
    p[10] = value.u8[1];
    p[11] = value.u8[0];

    value.Flt = e_rpm;
    p[12] = value.u8[3];
    p[13] = value.u8[2];
    p[14] = value.u8[1];
    p[15] = value.u8[0];


    // aTxBuffer[4] = (consumption >> 8) & 0xFF; // consumption
    // aTxBuffer[5] = consumption & 0xFF; //  in mah

    // aTxBuffer[6] = (e_rpm >> 8) & 0xFF; //
    // aTxBuffer[7] = e_rpm & 0xFF; // eRpM *100

    // aTxBuffer[8] = temp; // temperature
    // aTxBuffer[9] = get_crc8(aTxBuffer, 9);
}