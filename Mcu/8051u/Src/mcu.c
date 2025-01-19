#include "mcu.h"

union_int16_t union_16_T0;
union_int16_t union_16_T1;
union_int16_t union_16_T2;
union_int16_t union_16_T3;
union_int16_t union_16_T4;
union_int16_t union_16_T11;

void TMR0_RELOAD(uint16_t time) {
	time = 65536 - time;
	TH0 = time >> 8;
	TL0 = time;
}

void TMR1_RELOAD(uint16_t time) {
	time = 65536 - time;
	TH1 = time >> 8;
	TL1 = time;
}

void TMR2_RELOAD(uint16_t time) {
	time = 65536 - time;
	T2H = time >> 8;
	T2L = time;
}

void TMR3_RELOAD(uint16_t time) {
	time = 65536 - time;
	T3H = time >> 8;
	T3L = time;
}

void TMR4_RELOAD(uint16_t time) {
	time = 65536 - time;
	T4H = time >> 8;
	T4L = time;
}

void TMR11_RELOAD(uint16_t time) {
	time = 65536 - time;
	T11H = time >> 8;
	T11L = time;
}