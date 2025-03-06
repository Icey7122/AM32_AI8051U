/*
 * phaseout.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */

#ifndef INC_PHASEOUT_H_
#define INC_PHASEOUT_H_

#include "mcu.h"

typedef void(phaseFUNC_t)(void);

extern phaseFUNC_t* comStep[6];

extern void phaseAB(void);
extern void phaseCB(void);
extern void phaseCA(void);
extern void phaseBA(void);
extern void phaseBC(void);
extern void phaseAC(void);


extern void allOff(void);
extern void fullBrake(void);
extern void allpwm(void);
extern void twoChannelForward(void);
extern void twoChannelReverse(void);
extern void proportionalBrake(void);



#endif /* INC_PHASEOUT_H_ */
