/*
 * phaseout.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */

#ifndef INC_PHASEOUT_H_
#define INC_PHASEOUT_H_

#include "mcu.h"

typedef void(STEP_FUNC_t)(void);

extern STEP_FUNC_t* comStep[6];

void phaseAPWM(void);
void phaseAFLOAT(void);
void phaseALOW(void);

void phaseBPWM(void);
void phaseBFLOAT(void);
void phaseBLOW(void);

void phaseCPWM(void);
void phaseCFLOAT(void);
void phaseCLOW(void);

extern void STEP_AB(void);
extern void STEP_CB(void);
extern void STEP_CA(void);
extern void STEP_BA(void);
extern void STEP_BC(void);
extern void STEP_AC(void);


extern void allOff(void);
extern void fullBrake(void);
extern void allpwm(void);
extern void twoChannelForward(void);
extern void twoChannelReverse(void);
extern void proportionalBrake(void);



#endif /* INC_PHASEOUT_H_ */
