#include "stdio.h"
#include "uart.h"

uint8_t xdata aTxBuffer[10] = {0};

void telem_UART1_Init(void)
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = BAUD_RATE_RELOAD;			//设置定时初始值
	T2H = BAUD_RATE_RELOAD >> 8;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时

	gpio_mode_set(P3,GPIO_ModePin_0,GPIO_Mode_AIN);
	gpio_mode_set(P3,GPIO_ModePin_1,GPIO_Mode_Out_PP);

	DMA_UR1T_CFG = 0x81; 	//启用UR1T_DMA中断 数据优先级1,最低中断优先级
	DMA_UR1T_CR = 0x80;

	DMA_UR1T_AMTH = 0x00;
	DMA_UR1T_AMT = 0x09;	//设置DMA传输数据长度9个字节

	DMA_UR1T_TXAH = (uint16_t)aTxBuffer >> 8;
	DMA_UR1T_TXAL = (uint16_t)aTxBuffer & 0xFF;	//设置DMA传输数据地址

	DMA_UR1_ITVH = 0x00;
	DMA_UR1_ITVL = 0x00;	//设置DMA传输速度
}

void send_telem_DMA(void)
{
	DMA_UR1T_CR |= 0x40;	//启动UR1T_DMA传输
}

#pragma FUNCTIONS (static)
char putchar(char c)
{
	SBUF = c;
	while (!TI);
	TI = 0;
	return c;
}