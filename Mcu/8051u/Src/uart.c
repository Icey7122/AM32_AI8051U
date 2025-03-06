#include "stdio.h"
#include "uart.h"


void UART1_Init(void)
{
	SCON = 0xD0;		//9位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = BAUD_RATE_RELOAD;			//设置定时初始值
	T2H = BAUD_RATE_RELOAD >> 8;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	USARTCR2 = 0x06; 	//启动校验位，奇校验
}

#pragma FUNCTIONS (static)
char putchar(char c)
{
	SBUF = c;
	while (!TI);
	TI = 0;
	return c;
}
