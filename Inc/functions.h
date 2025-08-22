/*
 * function.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */

#ifndef INC_FUNCTIONS_H_
#define INC_FUNCTIONS_H_

#include "mcu.h"
#include "targets.h"

typedef struct fastPID {
	int32_t error;
	uint32_t Kp;
	uint32_t Ki;
	uint32_t Kd;
	int32_t integral;
	int32_t derivative;
	int32_t last_error;
	int32_t pid_output;
	int32_t integral_limit;
	int32_t output_limit;
} fastPID;

extern uint8_t update_crc8(uint8_t crc, uint8_t crc_seed);
extern uint8_t get_crc8(uint8_t* Buf, uint8_t BufLen);
extern void delayMicros(uint32_t micros);
extern void delayMillis(uint32_t millis);
extern uint32_t getAbsDif(int16_t number1, int16_t number2);
extern float doPidCalculations(struct fastPID *pidnow, int actual, int target);
extern int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

#endif /* INC_FUNCTIONS_H_ */
