/*
 * phaseout.c
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */
#include "targets.h"
#include "peripherals.h"
#include "phaseouts.h"
#include "eeprom.h"
#include "commutate.h"

phaseFUNC_t* comStep[6] = {phaseAB,phaseCB,phaseCA,phaseBA,phaseBC,phaseAC};

///////////////////////////////////////////////PHASE STEP////////////////////////////////////////////////////
void phaseAB(void)
{
// 	A:PWM  B:LOW  C:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x85;	
	} else {
		PWMA->CCER1 = 0x81;	
	}
	PWMA->CCER2 = 0x00;
	PWMA->EGR = 0x20;
	CMPEXCFG = PHASE_C_COMP;
}


void phaseCB(void)
{
// 	C:PWM  B:LOW  A:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER2 = 0x05;	
	} else {
		PWMA->CCER2 = 0x01;	
	}
	PWMA->CCER1 = 0x80;
	PWMA->EGR = 0x20;
	CMPEXCFG = PHASE_A_COMP;
}



void phaseCA(void)
{
// 	C:PWM  A:LOW  B:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER2 = 0x05;	
	} else {
		PWMA->CCER2 = 0x01;	
	}
	PWMA->CCER1 = 0x08;
	PWMA->EGR = 0x20;
	CMPEXCFG = PHASE_B_COMP;
}


void phaseBA(void)
{
// 	B:PWM  A:LOW  C:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x58;	
	} else {
		PWMA->CCER1 = 0x18;	
	}
	PWMA->CCER2 = 0x00;
	PWMA->EGR = 0x20;
	CMPEXCFG = PHASE_C_COMP;
}



void phaseBC(void)
{
// 	B:PWM  C:LOW  A:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x50;	
	} else {
		PWMA->CCER1 = 0x10;	
	}
	PWMA->CCER2 = 0x08;
	PWMA->EGR = 0x20;
	CMPEXCFG = PHASE_A_COMP;
}



void phaseAC(void)
{
// 	A:PWM  C:LOW  B:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x05;	
	} else {
		PWMA->CCER1 = 0x01;	
	}
	PWMA->CCER2 = 0x08;
	PWMA->EGR = 0x20;
	CMPEXCFG = PHASE_B_COMP;
}



void allOff(void) {
// 	A:FLOAT  B:FLOAT  C:FLOAT
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	PWMA->CCER1 = 0x00;
	PWMA->CCER2 = 0x00;	
	PWMA->EGR = 0x20;
}



void fullBrake(void) { // full braking shorting all low sides
// 	A:LOW  B:LOW  C:LOW
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	PWMA->CCER1 = 0x88;
	PWMA->CCER2 = 0x08;	
	PWMA->EGR = 0x20;
}


void allpwm(void) { // for stepper_sine
// 	A:PWM  B:PWM  C:PWM
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x55;	
		PWMA->CCER2 = 0x05;
	} else {
		PWMA->CCER1 = 0x11;	
		PWMA->CCER2 = 0x01;
	}
	PWMA->EGR = 0x20;
}


void twoChannelForward(void) {
// 	A:PWM  B:LOW  C:PWM
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x85;	
		PWMA->CCER2 = 0x05;
	} else {
		PWMA->CCER1 = 0x81;	
		PWMA->CCER2 = 0x01;
	}
	PWMA->EGR = 0x20;
}

void twoChannelReverse(void) {
// 	A:LOW  B:PWM  C:LOW
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	if(COMP_PWM){
		PWMA->CCER1 = 0x58;	
		PWMA->CCER2 = 0x08;
	} else {
		PWMA->CCER1 = 0x18;	
		PWMA->CCER2 = 0x08;
	}
	PWMA->EGR = 0x20;
}


void proportionalBrake(void) { // alternate all channels between braking (ABC LOW)
// and coasting (ABC float) put lower channel into
// alternate mode and turn upper OFF for each
// channel
// turn all HIGH channels off for ABC
	register PWM_TypeDef *PWMA = PWMA_ADDRESS;
	PWMA->CCER1 = 0xCC;	
	PWMA->CCER2 = 0x0C;
	PWMA->EGR = 0x20;
}





