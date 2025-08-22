/*
 * periherals.h
 *
 *  Created on: 2024年12月2日
 *      Author: 17263
 */

#ifndef INC_PERIPHERALS_H_
#define INC_PERIPHERALS_H_

#include "mcu.h"

#define COM_TIMER_COUNT(value) 					TMR0_RELOAD(value)
#define INTERVAL_TIMER_COUNT() 					TMR1_VALUE()
#define RELOAD_WATCHDOG_COUNTER() 				(CLR_WDT = 1)
#define DISABLE_COM_TIMER_INT() 				(ET0 = 0)
#define ENABLE_COM_TIMER_INT() 					(ET0 = 1)
#define SET_AND_ENABLE_COM_INT(time) 			(TMR0_RELOAD(time),ET0 = 1)			//使能定时器0中断
#define SET_INTERVAL_TIMER_COUNT(intertime) 	TMR1_SET(intertime)

extern void SET_PRESCALER_PWM(uint16_t presc);
extern void SET_AUTO_RELOAD_PWM(uint16_t relval);
extern void SET_DUTY_CYCLE_ALL(uint16_t newdc);
extern void setPWMCompare1(uint16_t compareone);
extern void setPWMCompare2(uint16_t comparetwo);
extern void setPWMCompare3(uint16_t comparethree);
extern void generatePwmTimerEvent(void);


extern void initCorePeripherals(void);
extern void initAfterJump(void);
extern void configSystemClock(void);

extern void GPIO_Init(void);
extern void CMP_Init(void);
extern void IWDG_Init(void);

extern void PWMA_Init(void);
extern void TMR0_Init(void);
extern void TMR1_Init(void);
extern void TMR3_Init(void);
extern void TMR4_Init(void);
extern void TMR11_Init(void);
extern void EXIT3_Init(void);


#endif /* INC_PERIPHERALS_H_ */
