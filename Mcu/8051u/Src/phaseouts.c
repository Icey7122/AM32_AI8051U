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

STEP_FUNC_t* comStep[6] = {STEP_AB,STEP_CB,STEP_CA,STEP_BA,STEP_BC,STEP_AC};

///////////////////////////////////////////////PHASE A////////////////////////////////////////////////////
void phaseAPWM(void) {
	if (!COMP_PWM) {
		PWMA_ENO &= ~0x02;
		gpio_clear(PHASE_A_GPIO_PORT_LOW,PHASE_A_GPIO_LOW);
	} else {
		PWMA_ENO |= 0x02;
	}
	PWMA_ENO |= 0x01;
}

void phaseAFLOAT(void) {
	PWMA_ENO &= ~0x03;
	gpio_clear(PHASE_A_GPIO_PORT_LOW,PHASE_A_GPIO_LOW);
	gpio_clear(PHASE_A_GPIO_PORT_HIGH,PHASE_A_GPIO_HIGH);
}

void phaseALOW(void) {
	PWMA_ENO &= ~0x03;
	gpio_set(PHASE_A_GPIO_PORT_LOW,PHASE_A_GPIO_LOW);
	gpio_clear(PHASE_A_GPIO_PORT_HIGH,PHASE_A_GPIO_HIGH);
}


///////////////////////////////////////////////PHASE B////////////////////////////////////////////////////
void phaseBPWM(void) {
	if (!COMP_PWM) { // for future
		PWMA_ENO &= ~0x08;
		gpio_clear(PHASE_B_GPIO_PORT_LOW,PHASE_B_GPIO_LOW);
	} else {
		PWMA_ENO |= 0x08;
	}
	PWMA_ENO |= 0x04;
}

void phaseBFLOAT(void) {
	PWMA_ENO &= ~0x0C;
	gpio_clear(PHASE_B_GPIO_PORT_LOW,PHASE_B_GPIO_LOW);
	gpio_clear(PHASE_B_GPIO_PORT_HIGH,PHASE_B_GPIO_HIGH);
}

void phaseBLOW(void) {
	PWMA_ENO &= ~0x0C;
	gpio_set(PHASE_B_GPIO_PORT_LOW,PHASE_B_GPIO_LOW);
	gpio_clear(PHASE_B_GPIO_PORT_HIGH,PHASE_B_GPIO_HIGH);
}

///////////////////////////////////////////////PHASE C////////////////////////////////////////////////////

void phaseCPWM(void) {
	if (!COMP_PWM) {
		PWMA_ENO &= ~0x20;
		gpio_clear(PHASE_C_GPIO_PORT_LOW,PHASE_C_GPIO_LOW);
	} else {
		PWMA_ENO |= 0x20;
	}
	PWMA_ENO |= 0x10;
}

void phaseCFLOAT(void) {
	PWMA_ENO &= ~0x30;
	gpio_clear(PHASE_C_GPIO_PORT_LOW,PHASE_C_GPIO_LOW);
	gpio_clear(PHASE_C_GPIO_PORT_HIGH,PHASE_C_GPIO_HIGH);
}

void phaseCLOW(void) {
	PWMA_ENO &= ~0x30;
	gpio_set(PHASE_C_GPIO_PORT_LOW,PHASE_C_GPIO_LOW);
	gpio_clear(PHASE_C_GPIO_PORT_HIGH,PHASE_C_GPIO_HIGH);
}

///////////////////////////////////////////////PHASE STEP////////////////////////////////////////////////////

void STEP_AB(void)
{
	phaseCFLOAT();
	phaseBLOW();
	phaseAPWM();

	CMPEXCFG = PHASE_C_COMP;
}


void STEP_CB(void)
{
	phaseAFLOAT();
	phaseBLOW();
	phaseCPWM();

	CMPEXCFG = PHASE_A_COMP;
}



void STEP_CA(void)
{
	phaseBFLOAT();
	phaseALOW();
	phaseCPWM();

	CMPEXCFG = PHASE_B_COMP;
}


void STEP_BA(void)
{
	phaseCFLOAT();
	phaseALOW();
	phaseBPWM();

	CMPEXCFG = PHASE_C_COMP;
}



void STEP_BC(void)
{
	phaseAFLOAT();
	phaseCLOW();
	phaseBPWM();

	CMPEXCFG = PHASE_A_COMP;
}



void STEP_AC(void)
{
	phaseBFLOAT();
	phaseCLOW();
	phaseAPWM();

	CMPEXCFG = PHASE_B_COMP;
}



void allOff(void) {
	phaseAFLOAT();
	phaseBFLOAT();
	phaseCFLOAT();
}



void fullBrake(void) { // full braking shorting all low sides
	phaseALOW();
	phaseBLOW();
	phaseCLOW();
}


void allpwm(void) { // for stepper_sine
	phaseAPWM();
	phaseBPWM();
	phaseCPWM();
}


void twoChannelForward(void) {
	phaseAPWM();
	phaseBLOW();
	phaseCPWM();
}

void twoChannelReverse(void) {
	phaseALOW();
	phaseBPWM();
	phaseCLOW();
}


void proportionalBrake(void) { // alternate all channels between braking (ABC LOW)
// and coasting (ABC float) put lower channel into
// alternate mode and turn upper OFF for each
// channel
// turn all HIGH channels off for ABC

	PWMA_ENO = 0x2A;
	gpio_clear(PHASE_A_GPIO_PORT_HIGH,PHASE_A_GPIO_HIGH);
	gpio_clear(PHASE_B_GPIO_PORT_HIGH,PHASE_B_GPIO_HIGH);
	gpio_clear(PHASE_C_GPIO_PORT_HIGH,PHASE_C_GPIO_HIGH);
}





