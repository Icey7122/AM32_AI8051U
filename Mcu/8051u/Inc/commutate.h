/*
 * comparator.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */

#ifndef INC_COMMUTATE_H_
#define INC_COMMUTATE_H_

#include "mcu.h"

#define COMP_P46 0x00
#define COMP_P50 0x01
#define COMP_P51 0x02

#define ENABLE_RISE_INTERRUPT() 		P4IM0 |= 0x02
#define ENABLE_FALL_INTERRUPT()			P4IM0 &= ~0x02

#define getCompOutputLevel() 	(CMPRES)
#define maskPhaseInterrupts() 	P4INTE &= ~0x02
#define enableCompInterrupts() 	P4INTE |= 0x02

extern int8_t  step;
extern uint8_t forward;
extern uint8_t rising;
extern uint8_t filter_level;
extern uint8_t desync_check;
extern uint8_t desync_happened;
extern uint8_t prop_brake_active;
extern uint8_t old_routine;

extern uint8_t stuckcounter;
extern uint8_t bemfcounter;
extern uint8_t min_bemf_counts_up;
extern uint8_t min_bemf_counts_down; 
extern uint8_t bemf_timeout;
extern uint8_t bemf_timeout_happened;
extern uint8_t bad_count;
extern uint8_t bad_count_threshold;

extern uint8_t	zcfound;
extern uint16_t thiszctime;
extern uint16_t zero_crosses;

extern uint8_t temp_advance;
extern uint8_t auto_advance_level;
extern uint16_t advance;
extern uint16_t waitTime;
extern uint16_t average_interval;
extern uint16_t last_average_interval;
extern uint16_t commutation_interval;
extern uint16_t commutation_intervals[6];
extern uint16_t last_commutation_interval;		

extern uint16_t step_delay;
extern uint8_t stepper_sine;
extern uint8_t changeover_step;
extern uint8_t do_once_sinemode;
extern int16_t phase_A_position;
extern int16_t phase_B_position; 
extern int16_t phase_C_position;

extern const int32_t pwmSin[];

extern void commutate(void);
extern void interruptRoutine(void);
extern void PeriodElapsedCallback(void);
extern void zcfoundroutine(void);
extern void getBemfState(void);
extern void advanceincrement(void);
extern void startMotor(void);

#endif /* INC_COMMUTATE_H_ */
