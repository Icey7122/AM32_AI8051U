#ifndef _MCU_H
#define _MCU_H

#include "STC8051U.H"
#include "intrins.h"
#include "nstdint.h"

#ifndef __VSCODE_C251__
#define INTERRUPT(x) interrupt x
#define USING(x) using x
#else
#define INTERRUPT(x)
#define USING(x)
#endif

#define __enable_irq() EA = 1
#define __disable_irq() EA = 0


#define  P0_MODE_IO_PU(Pin)				P0M1 &= ~(Pin), P0M0 &= ~(Pin)
#define  P1_MODE_IO_PU(Pin)				P1M1 &= ~(Pin), P1M0 &= ~(Pin)
#define  P2_MODE_IO_PU(Pin)				P2M1 &= ~(Pin), P2M0 &= ~(Pin)
#define  P3_MODE_IO_PU(Pin)				P3M1 &= ~(Pin), P3M0 &= ~(Pin)
#define  P4_MODE_IO_PU(Pin)				P4M1 &= ~(Pin), P4M0 &= ~(Pin)
#define  P5_MODE_IO_PU(Pin)				P5M1 &= ~(Pin), P5M0 &= ~(Pin)
#define  P6_MODE_IO_PU(Pin)				P6M1 &= ~(Pin), P6M0 &= ~(Pin)
#define  P7_MODE_IO_PU(Pin)				P7M1 &= ~(Pin), P7M0 &= ~(Pin)

#define  P0_MODE_IN_HIZ(Pin)			P0M1 |= (Pin), P0M0 &= ~(Pin)
#define  P1_MODE_IN_HIZ(Pin)			P1M1 |= (Pin), P1M0 &= ~(Pin)
#define  P2_MODE_IN_HIZ(Pin)			P2M1 |= (Pin), P2M0 &= ~(Pin)
#define  P3_MODE_IN_HIZ(Pin)			P3M1 |= (Pin), P3M0 &= ~(Pin)
#define  P4_MODE_IN_HIZ(Pin)			P4M1 |= (Pin), P4M0 &= ~(Pin)
#define  P5_MODE_IN_HIZ(Pin)			P5M1 |= (Pin), P5M0 &= ~(Pin)
#define  P6_MODE_IN_HIZ(Pin)			P6M1 |= (Pin), P6M0 &= ~(Pin)
#define  P7_MODE_IN_HIZ(Pin)			P7M1 |= (Pin), P7M0 &= ~(Pin)

#define  P0_MODE_OUT_OD(Pin)			P0M1 |= (Pin), P0M0 |= (Pin)
#define  P1_MODE_OUT_OD(Pin)			P1M1 |= (Pin), P1M0 |= (Pin)
#define  P2_MODE_OUT_OD(Pin)			P2M1 |= (Pin), P2M0 |= (Pin)
#define  P3_MODE_OUT_OD(Pin)			P3M1 |= (Pin), P3M0 |= (Pin)
#define  P4_MODE_OUT_OD(Pin)			P4M1 |= (Pin), P4M0 |= (Pin)
#define  P5_MODE_OUT_OD(Pin)			P5M1 |= (Pin), P5M0 |= (Pin)
#define  P6_MODE_OUT_OD(Pin)			P6M1 |= (Pin), P6M0 |= (Pin)
#define  P7_MODE_OUT_OD(Pin)			P7M1 |= (Pin), P7M0 |= (Pin)

#define  P0_MODE_OUT_PP(Pin)			P0M1 &= ~(Pin), P0M0 |= (Pin)
#define  P1_MODE_OUT_PP(Pin)			P1M1 &= ~(Pin), P1M0 |= (Pin)
#define  P2_MODE_OUT_PP(Pin)			P2M1 &= ~(Pin), P2M0 |= (Pin)
#define  P3_MODE_OUT_PP(Pin)			P3M1 &= ~(Pin), P3M0 |= (Pin)
#define  P4_MODE_OUT_PP(Pin)			P4M1 &= ~(Pin), P4M0 |= (Pin)
#define  P5_MODE_OUT_PP(Pin)			P5M1 &= ~(Pin), P5M0 |= (Pin)
#define  P6_MODE_OUT_PP(Pin)			P6M1 &= ~(Pin), P6M0 |= (Pin)
#define  P7_MODE_OUT_PP(Pin)			P7M1 &= ~(Pin), P7M0 |= (Pin)


#define P0_PULL_UP_ENABLE(Pin)		P0PU |= (Pin)
#define P1_PULL_UP_ENABLE(Pin)		P1PU |= (Pin)
#define P2_PULL_UP_ENABLE(Pin)		P2PU |= (Pin)
#define P3_PULL_UP_ENABLE(Pin)		P3PU |= (Pin)
#define P4_PULL_UP_ENABLE(Pin)		P4PU |= (Pin)
#define P5_PULL_UP_ENABLE(Pin)		P5PU |= (Pin)
#define P6_PULL_UP_ENABLE(Pin)		P6PU |= (Pin)
#define P7_PULL_UP_ENABLE(Pin)		P7PU |= (Pin)

#define P0_PULL_UP_DISABLE(Pin)		P0PU &= ~(Pin)
#define P1_PULL_UP_DISABLE(Pin)		P1PU &= ~(Pin)
#define P2_PULL_UP_DISABLE(Pin)		P2PU &= ~(Pin)
#define P3_PULL_UP_DISABLE(Pin)		P3PU &= ~(Pin)
#define P4_PULL_UP_DISABLE(Pin)		P4PU &= ~(Pin)
#define P5_PULL_UP_DISABLE(Pin)		P5PU &= ~(Pin)
#define P6_PULL_UP_DISABLE(Pin)		P6PU &= ~(Pin)
#define P7_PULL_UP_DISABLE(Pin)		P7PU &= ~(Pin)


#define P0_PULL_DOWN_ENABLE(Pin)	P0PD |= (Pin)
#define P1_PULL_DOWN_ENABLE(Pin)	P1PD |= (Pin)
#define P2_PULL_DOWN_ENABLE(Pin)	P2PD |= (Pin)
#define P3_PULL_DOWN_ENABLE(Pin)	P3PD |= (Pin)
#define P4_PULL_DOWN_ENABLE(Pin)	P4PD |= (Pin)
#define P5_PULL_DOWN_ENABLE(Pin)	P5PD |= (Pin)
#define P6_PULL_DOWN_ENABLE(Pin)	P6PD |= (Pin)
#define P7_PULL_DOWN_ENABLE(Pin)	P7PD |= (Pin)

#define P0_PULL_DOWN_DISABLE(Pin)	P0PD &= ~(Pin)
#define P1_PULL_DOWN_DISABLE(Pin)	P1PD &= ~(Pin)
#define P2_PULL_DOWN_DISABLE(Pin)	P2PD &= ~(Pin)
#define P3_PULL_DOWN_DISABLE(Pin)	P3PD &= ~(Pin)
#define P4_PULL_DOWN_DISABLE(Pin)	P4PD &= ~(Pin)
#define P5_PULL_DOWN_DISABLE(Pin)	P5PD &= ~(Pin)
#define P6_PULL_DOWN_DISABLE(Pin)	P6PD &= ~(Pin)
#define P7_PULL_DOWN_DISABLE(Pin)	P7PD &= ~(Pin)

#define P0_ST_ENABLE(Pin)					P0NCS &= ~(Pin)
#define P1_ST_ENABLE(Pin)					P1NCS &= ~(Pin)
#define P2_ST_ENABLE(Pin)					P2NCS &= ~(Pin)
#define P3_ST_ENABLE(Pin)					P3NCS &= ~(Pin)
#define P4_ST_ENABLE(Pin)					P4NCS &= ~(Pin)
#define P5_ST_ENABLE(Pin)					P5NCS &= ~(Pin)
#define P6_ST_ENABLE(Pin)					P6NCS &= ~(Pin)
#define P7_ST_ENABLE(Pin)					P7NCS &= ~(Pin)

#define P0_ST_DISABLE(Pin)				P0NCS |= (Pin)
#define P1_ST_DISABLE(Pin)				P1NCS |= (Pin)
#define P2_ST_DISABLE(Pin)				P2NCS |= (Pin)
#define P3_ST_DISABLE(Pin)				P3NCS |= (Pin)
#define P4_ST_DISABLE(Pin)				P4NCS |= (Pin)
#define P5_ST_DISABLE(Pin)				P5NCS |= (Pin)
#define P6_ST_DISABLE(Pin)				P6NCS |= (Pin)
#define P7_ST_DISABLE(Pin)				P7NCS |= (Pin)

#define P0_SPEED_LOW(Pin)					P0SR |= (Pin)
#define P1_SPEED_LOW(Pin)					P1SR |= (Pin)
#define P2_SPEED_LOW(Pin)					P2SR |= (Pin)
#define P3_SPEED_LOW(Pin)					P3SR |= (Pin)
#define P4_SPEED_LOW(Pin)					P4SR |= (Pin)
#define P5_SPEED_LOW(Pin)					P5SR |= (Pin)
#define P6_SPEED_LOW(Pin)					P6SR |= (Pin)
#define P7_SPEED_LOW(Pin)					P7SR |= (Pin)

#define P0_SPEED_HIGH(Pin)				P0SR &= ~(Pin)
#define P1_SPEED_HIGH(Pin)				P1SR &= ~(Pin)
#define P2_SPEED_HIGH(Pin)				P2SR &= ~(Pin)
#define P3_SPEED_HIGH(Pin)				P3SR &= ~(Pin)
#define P4_SPEED_HIGH(Pin)				P4SR &= ~(Pin)
#define P5_SPEED_HIGH(Pin)				P5SR &= ~(Pin)
#define P6_SPEED_HIGH(Pin)				P6SR &= ~(Pin)
#define P7_SPEED_HIGH(Pin)				P7SR &= ~(Pin)

#define P0_DRIVE_MEDIUM(Pin)			P0DR |= (Pin)
#define P1_DRIVE_MEDIUM(Pin)			P1DR |= (Pin)
#define P2_DRIVE_MEDIUM(Pin)			P2DR |= (Pin)
#define P3_DRIVE_MEDIUM(Pin)			P3DR |= (Pin)
#define P4_DRIVE_MEDIUM(Pin)			P4DR |= (Pin)
#define P5_DRIVE_MEDIUM(Pin)			P5DR |= (Pin)
#define P6_DRIVE_MEDIUM(Pin)			P6DR |= (Pin)
#define P7_DRIVE_MEDIUM(Pin)			P7DR |= (Pin)

#define P0_DRIVE_HIGH(Pin)				P0DR &= ~(Pin)
#define P1_DRIVE_HIGH(Pin)				P1DR &= ~(Pin)
#define P2_DRIVE_HIGH(Pin)				P2DR &= ~(Pin)
#define P3_DRIVE_HIGH(Pin)				P3DR &= ~(Pin)
#define P4_DRIVE_HIGH(Pin)				P4DR &= ~(Pin)
#define P5_DRIVE_HIGH(Pin)				P5DR &= ~(Pin)
#define P6_DRIVE_HIGH(Pin)				P6DR &= ~(Pin)
#define P7_DRIVE_HIGH(Pin)				P7DR &= ~(Pin)

#define P0_DIGIT_IN_ENABLE(Pin)		P0IE |= (Pin)
#define P1_DIGIT_IN_ENABLE(Pin)		P1IE |= (Pin)
#define P2_DIGIT_IN_ENABLE(Pin)		P2IE |= (Pin)
#define P3_DIGIT_IN_ENABLE(Pin)		P3IE |= (Pin)
#define P4_DIGIT_IN_ENABLE(Pin)		P4IE |= (Pin)
#define P5_DIGIT_IN_ENABLE(Pin)		P5IE |= (Pin)
#define P6_DIGIT_IN_ENABLE(Pin)		P6IE |= (Pin)
#define P7_DIGIT_IN_ENABLE(Pin)		P7IE |= (Pin)

#define P0_DIGIT_IN_DISABLE(Pin)	P0IE &= ~(Pin)
#define P1_DIGIT_IN_DISABLE(Pin)	P1IE &= ~(Pin)
#define P2_DIGIT_IN_DISABLE(Pin)	P2IE &= ~(Pin)
#define P3_DIGIT_IN_DISABLE(Pin)	P3IE &= ~(Pin)
#define P4_DIGIT_IN_DISABLE(Pin)	P4IE &= ~(Pin)
#define P5_DIGIT_IN_DISABLE(Pin)	P5IE &= ~(Pin)
#define P6_DIGIT_IN_DISABLE(Pin)	P6IE &= ~(Pin)
#define P7_DIGIT_IN_DISABLE(Pin)	P7IE &= ~(Pin)

#define	GPIO_ModePin_0		0x01	
#define	GPIO_ModePin_1		0x02	
#define	GPIO_ModePin_2		0x04	
#define	GPIO_ModePin_3		0x08	
#define	GPIO_ModePin_4		0x10	
#define	GPIO_ModePin_5		0x20	
#define	GPIO_ModePin_6		0x40	
#define	GPIO_ModePin_7		0x80	
#define	GPIO_ModePin_LOW	0x0F	
#define	GPIO_ModePin_HIGH	0xF0	
#define	GPIO_ModePin_All	0xFF

#define GPIO_Mode_IN_REST(port,pin) 		port##_ST_ENABLE(pin),port##_DIGIT_IN_ENABLE(pin)
#define GPIO_Mode_AIN(port,pin) 			port##_MODE_IN_HIZ(pin),port##_PULL_DOWN_DISABLE(pin),port##_ST_DISABLE(pin),port##_DIGIT_IN_DISABLE(pin)
#define GPIO_Mode_IN_FLOATING(port,pin) 	port##_MODE_IN_HIZ(pin),port##_PULL_DOWN_DISABLE(pin)
#define GPIO_Mode_IPU(port,pin) 			port##_MODE_IO_PU(pin),port##_PULL_DOWN_DISABLE(pin)
#define GPIO_Mode_IPD(port,pin) 			port##_MODE_IN_HIZ(pin),port##_PULL_DOWN_ENABLE(pin)

#define GPIO_Mode_Out_REST(port,pin) 		port##_SPEED_LOW(pin),port##_DRIVE_MEDIUM(pin)
#define GPIO_Mode_Out_FAST(port,pin) 		port##_SPEED_HIGH(pin),port##_DRIVE_HIGH(pin)
#define GPIO_Mode_Out_PP(port,pin) 			port##_MODE_OUT_PP(pin),port##_PULL_DOWN_DISABLE(pin)
#define GPIO_Mode_Out_OD(port,pin) 			port##_MODE_OUT_OD(pin),port##_PULL_DOWN_DISABLE(pin)

#define gpio_mode_set(port,pin,mode) 	mode(port,pin)
#define gpio_set(port,pin) 				pin = 1
#define gpio_clear(port,pin) 			pin = 0
#define gpio_read(port,pin) 			pin

typedef union 
{
	int8_t i8[2];
	uint8_t u8[2];
	int16_t i16;
	uint16_t u16;	
} union_int16_t;

extern union_int16_t union_16_T0;
extern union_int16_t union_16_T1;
extern union_int16_t union_16_T2;
extern union_int16_t union_16_T3;
extern union_int16_t union_16_T4;
extern union_int16_t union_16_T11;

#define TMR0_VALUE() (union_16_T0.u8[0] = TH0,union_16_T0.u8[1] = TL0,union_16_T0.u16)
#define TMR1_VALUE() (union_16_T1.u8[0] = TH1,union_16_T1.u8[1] = TL1,union_16_T1.u16)
#define TMR2_VALUE() (union_16_T2.u8[0] = T2H,union_16_T2.u8[1] = T2L,union_16_T2.u16)
#define TMR3_VALUE() (union_16_T3.u8[0] = T3H,union_16_T3.u8[1] = T3L,union_16_T3.u16)
#define TMR4_VALUE() (union_16_T4.u8[0] = T4H,union_16_T4.u8[1] = T4L,union_16_T4.u16)
#define TMR11_VALUE() (union_16_T11.u8[0] = T11H,union_16_T11.u8[1] = T11L,union_16_T11.u16)

#define TMR0_SET(time)		(TH0 = (uint16_t)time >> 8,TL0 = time)
#define TMR1_SET(time)		(TH1 = (uint16_t)time >> 8,TL1 = time)
#define TMR2_SET(time)		(T2H = (uint16_t)time >> 8,T2L = time)
#define TMR3_SET(time)		(T3H = (uint16_t)time >> 8,T3L = time)
#define TMR4_SET(time)		(T4H = (uint16_t)time >> 8,T4L = time)
#define TMR11_SET(time)		(T11H = (uint16_t)time >> 8,T11L = time)

extern void TMR0_RELOAD(uint16_t time);
extern void TMR1_RELOAD(uint16_t time);
extern void TMR2_RELOAD(uint16_t time);
extern void TMR3_RELOAD(uint16_t time);
extern void TMR4_RELOAD(uint16_t time);
extern void TMR11_RELOAD(uint16_t time);



#endif
