/*
 * peripherals.c
 *
 *  Created on: 2024年12月2日
 *      Author: 17263
 */
#include "peripherals.h"
#include "targets.h"

void initCorePeripherals(void) {

//	initAfterJump();
	configSystemClock();

	GPIO_Init();
	CMP_Init();

	PWMA_Init();
	TMR0_Init();
	TMR1_Init();
	TMR3_Init();
	TMR4_Init();
	TMR11_Init();
	UN_PWMB_Init();

#ifdef USE_SERIAL_TELEMETRY
	telem_UART_Init();
#endif
}

void initAfterJump(void) {
	CKCON = 0x00;
	WTST = 0;
	P_SW2 = 0x80;

	IAP_TPS = 40;
	RSTFLAG |= 0x04;
}

void configSystemClock(void){
	__disable_irq();

	// IRCBAND |= 0x03;
	// VRTRIM = 26;
	// IRTRIM = 107;

	USBCLK |= (1 << 7);
	USBCLK &= ~(3 << 5);
	USBCLK |= (2 << 5);
	_nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
	CLKSEL |= (1 << 7);
	CLKSEL |= (1 << 6);
	MCLKOCR = 0x01;
	IRC48MCR = 0x80;
	while (!(IRC48MCR & 1))
		;
	IRCBAND &= ~(3 << 6);
	IRCBAND |= (2 << 6);
	__enable_irq();
}

void GPIO_Init(void) {
	/* GPIO Ports Clock Enable */
	gpio_mode_set(P0,GPIO_ModePin_All,GPIO_Mode_IN_REST);
	gpio_mode_set(P1,GPIO_ModePin_All,GPIO_Mode_IN_REST);
	gpio_mode_set(P2,GPIO_ModePin_All,GPIO_Mode_IN_REST);
	gpio_mode_set(P3,GPIO_ModePin_All,GPIO_Mode_IN_REST);
	gpio_mode_set(P4,GPIO_ModePin_All,GPIO_Mode_IN_REST);

	gpio_mode_set(P0,GPIO_ModePin_All,GPIO_Mode_Out_REST);
	gpio_mode_set(P1,GPIO_ModePin_All,GPIO_Mode_Out_REST);
	gpio_mode_set(P2,GPIO_ModePin_All,GPIO_Mode_Out_REST);
	gpio_mode_set(P3,GPIO_ModePin_All,GPIO_Mode_Out_REST);
	gpio_mode_set(P4,GPIO_ModePin_All,GPIO_Mode_Out_REST);

}


void CMP_Init(void)
{
	gpio_mode_set(P4,GPIO_ModePin_1,GPIO_Mode_Out_PP);
	gpio_mode_set(P4,GPIO_ModePin_1,GPIO_Mode_Out_FAST);
	gpio_mode_set(P4,GPIO_ModePin_4 | GPIO_ModePin_5 | GPIO_ModePin_6,GPIO_Mode_AIN);
	gpio_mode_set(P5,GPIO_ModePin_1 | GPIO_ModePin_0 ,GPIO_Mode_AIN);

	CMPCR1 = 0;
	NIE = PIE = 0;
	CMPEN = CMPOE = 1;
	CMPCR2 = 47;			//improtant
	DISFLT = 0;
    // CMPCR2 &= ~(DISFLT);

	CMPO_S = 1;
	P4INTE |= 0x02;
	P4IM1 =  0x00;
	P4IM0 =  0x00;

	CMPEXCFG = 0x00;

	PINIPH = 0x10;
	PINIPL = 0x10;
}

void IWDG_Init(void) {
	WDT_CONTR &= ~0x07;
	WDT_CONTR |= 0x04;
	IDL_WDT = 1;
	EN_WDT = 1;
	CLR_WDT = 1;
}

void PWMA_Init(void)
{
	gpio_mode_set(P0,GPIO_ModePin_0 | GPIO_ModePin_2 | GPIO_ModePin_4 ,GPIO_Mode_Out_PP);
	gpio_mode_set(P0,GPIO_ModePin_1  | GPIO_ModePin_3 | GPIO_ModePin_5 ,GPIO_Mode_Out_PP);
	gpio_mode_set(P0,GPIO_ModePin_0 | GPIO_ModePin_2 | GPIO_ModePin_4 ,GPIO_Mode_Out_FAST);
	gpio_mode_set(P0,GPIO_ModePin_1  | GPIO_ModePin_3 | GPIO_ModePin_5 ,GPIO_Mode_Out_FAST);

	PWMA_PS = 0x55;
	PWMA_ENO = 0x00;
	PWMA_IOAUX = 0x00;
	PWMA_OISR = 0x00;

	PWMA_ARRH    = PWM_AUTORELOAD >> 8;
	PWMA_ARRL    = PWM_AUTORELOAD & 0xFF;


	PWMA_CCMR1 = 0x68;
	PWMA_CCER1 |= 0x05;

	PWMA_CCMR2 = 0x68;
	PWMA_CCER1 |= 0x50;

	PWMA_CCMR3 = 0x68;
	PWMA_CCER2 |= 0x05;

	PWMA_BKR   	= 0x80 | 0x08;
	PWMA_CR1 	= 0x81; //| 0x20;
	PWMA_EGR    = 0x01;
	
}

void TMR0_Init(void)
{
	TM0PS = 19;
	AUXR |= 0x80;
	TMOD &= 0xF0;
	TMOD |= 0x01;
	TMR0_SET(0);
	TF0 = 0;
	TR0 = 1;
	ET0 = 1;
	PT0H = 1; PT0 = 1;
}

void TMR1_Init(void) {
	//非自动重装载模式
	TM1PS = 19;			//设置定时器时钟预分频 ( 注意:并非所有系列都有此寄存器,详情请查看数据手册 )
	AUXR |= 0x40;			//定时器时钟1T模式
	TMOD &= 0x0F;			//设置定时器模式
	TMOD |= 0x10;			//设置定时器模式
	TMR1_SET(0);
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
}

void TMR3_Init(void) {
	//自动重装载模式
	TM3PS = 39;			//设置定时器时钟预分频 ( 注意:并非所有系列都有此寄存器,详情请查看数据手册 )
	T4T3M |= 0x02;			//定时器时钟1T模式
	TMR3_SET(0);
	T4T3M |= 0x08;			//定时器3开始计时
}

void TMR4_Init(void) {
	//默认最低优先级
	//自动重装载寄存器
	TM4PS = 39;			//设置定时器时钟预分频 ( 注意:并非所有系列都有此寄存器,详情请查看数据手册 )
	T4T3M |= 0x20;			//定时器时钟1T模式
	TMR4_RELOAD(1000000 / LOOP_FREQUENCY_HZ);
	T4T3M |= 0x80;			//定时器4开始计时
	IE2 |= 0x40;			//使能定时器4中断

}

void TMR11_Init(void) {
	//非自动重装载模式
	T11CR &= 0xf3;			//选择时钟源 (系统时钟SYSclk)
	T11PS = 0xFF;				//设置定时器时钟预分频 ( 注意:并非所有系列都有此寄存器,详情请查看数据手册 )
	T11CR &= 0xEF;			//定时器时钟12T模式
	TMR11_SET(0);
	T11CR |= 0x80;			//定时器11开始计时
	T11CR &= 0xfd;			//禁止定时器中断
}


void UN_PWMB_Init(void) {

	gpio_mode_set(P2,GPIO_ModePin_1,GPIO_Mode_IN_FLOATING);

	PWMB_PS = 0x02;
	PWMB_PSCRH = 0x00;
	PWMB_PSCRL = 0x00;
	PWMB_BKR = 0xC0;
	PWMB_CCER1 = 0x00;
	// BWMA_SMCR = 0x40;
	PWMB_CCMR1 = 0x01;
	PWMB_CCER1 = 0x01;
	PWMB_ARRH = 0xFF;
	PWMB_ARRL = 0xFF;
	PWMB_IER |= 0x02;
	// BWMA_CCER1 = 0xDD;
	// BWMA_CCER2 = 0xDD;

	PPWMBH = 1;PPWMB = 0;

	PWMB_EGR = 0x01;
	PWMB_CR1 |= 0x01;
}

void generatePwmTimerEvent(void)
{	
	PWMA_EGR = 0x01;
}


void SET_PRESCALER_PWM(uint16_t presc)
{	
	PWMA_PSCRH = presc >> 8; 
	PWMA_PSCRL = presc;
	PWMA_EGR = 0x01;
} 				

void SET_AUTO_RELOAD_PWM(uint16_t relval)
{
	PWMA_ARRH = relval >> 8;
	PWMA_ARRL = relval;
}

void SET_DUTY_CYCLE_ALL(uint16_t newdc)
{
	PWMA_CCR1H = PWMA_CCR2H = PWMA_CCR3H = newdc >> 8;
	PWMA_CCR1L = PWMA_CCR2L = PWMA_CCR3L = newdc;
}

void setPWMCompare1(uint16_t compareone)
{
	PWMA_CCR1H = compareone >> 8;
	PWMA_CCR1L = compareone;
}

void setPWMCompare2(uint16_t comparetwo)
{
	PWMA_CCR2H = comparetwo >> 8;
	PWMA_CCR2L = comparetwo;
}

void setPWMCompare3(uint16_t comparethree)
{
	PWMA_CCR3H = comparethree >> 8;
	PWMA_CCR3L = comparethree;
}
