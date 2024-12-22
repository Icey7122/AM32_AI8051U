/*
 * function.c
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */
#include "functions.h"


int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
    if (x < in_min) {
        x = in_min;
    }
    if (x > in_max) {
        x = in_max;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float doPidCalculations(struct fastPID *pidnow, int actual, int target) {

	pidnow->error = actual - target;
	pidnow->integral = pidnow->integral + pidnow->error * pidnow->Ki;
	if (pidnow->integral > pidnow->integral_limit) {
		pidnow->integral = pidnow->integral_limit;
	}
	if (pidnow->integral < -pidnow->integral_limit) {
		pidnow->integral = -pidnow->integral_limit;
	}

	pidnow->derivative = pidnow->Kd * (pidnow->error - pidnow->last_error);
	pidnow->last_error = pidnow->error;

	pidnow->pid_output = pidnow->error * pidnow->Kp + pidnow->integral
			+ pidnow->derivative;

	if (pidnow->pid_output > pidnow->output_limit) {
		pidnow->pid_output = pidnow->output_limit;
	}
	if (pidnow->pid_output < -pidnow->output_limit) {
		pidnow->pid_output = -pidnow->output_limit;
	}
	return pidnow->pid_output;
}

int16_t getAbsDif(int16_t number1, int16_t number2){
	int16_t result = number1 - number2;
    if (result < 0) {
        result = -result;
    }
    return result;
}

void delayMicros(uint32_t micros) {
	T3R = 0;
	TMR3_RELOAD(micros);
	T3IF = 0;
	T3R = 1;
	while (!T3IF)
		;
}

void delayMillis(uint32_t millis) {
	T11CR &= 0x7F;
	TMR11_RELOAD(millis * 13);
	T11CR |= 0x80;
	T11CR &= 0xFE;
	while (!(T11CR & 0x01))
		;
}
