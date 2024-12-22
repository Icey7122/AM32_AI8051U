/*
 * comparator.c
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */
#include "input.h"
#include "main.h"
#include "eeprom.h"
#include "phaseouts.h"
#include "commutate.h"
#include "peripherals.h"

int8_t 	step = 0;				//换相步骤
uint8_t forward = 1; 			//正转
uint8_t rising = 0;				//上升沿
uint8_t filter_level = 0; 		//滤波等级
uint8_t desync_check; 			//失步检查
uint8_t desync_happened = 0; 	//失步标志
uint8_t prop_brake_active = 0; 	//比例制动激活
uint8_t old_routine = 0; 		//旧周期

uint8_t stuckcounter = 0; 		//卡住计数器
uint8_t bemfcounter = 0; 		//BEMF计数器
uint8_t min_bemf_counts_up = TARGET_MIN_BEMF_COUNTS;
uint8_t min_bemf_counts_down = TARGET_MIN_BEMF_COUNTS;
uint8_t bemf_timeout = 10;				//BEMF超时
uint8_t bemf_timeout_happened = 0;		//BEMF超时标志
uint8_t bad_count = 0; 			//坏计数
uint8_t bad_count_threshold = CPU_FREQUENCY_MHZ / 24; //坏计数阈值

uint8_t	zcfound = 0; 			//零交叉发现
uint16_t thiszctime;			//当前零交叉时间
uint16_t zero_crosses; 			//零交叉

uint8_t temp_advance = 2;		//临时提前
uint8_t auto_advance_level; 	//自动提前等级
uint16_t advance; 				//提前
uint16_t waitTime; 				//换相等待时间
uint16_t average_interval; 			//平均间隔
uint16_t last_average_interval;
uint16_t commutation_interval = 12500; 		//换相间隔
uint16_t commutation_intervals[6] = { 0 }; 	//换相间隔数组
uint16_t last_commutation_interval;		

uint16_t step_delay = 100;
uint8_t stepper_sine = 0;
uint8_t changeover_step = 4;
uint8_t do_once_sinemode;
int16_t phase_A_position;
int16_t phase_B_position;
int16_t phase_C_position;

int32_t xdata pwmSin[] = { 180, 183, 186, 189, 193, 196, 199, 202, 205, 208,
		211, 214, 217, 220, 224, 227, 230, 233, 236, 239, 242, 245, 247, 250,
		253, 256, 259, 262, 265, 267, 270, 273, 275, 278, 281, 283, 286, 288,
		291, 293, 296, 298, 300, 303, 305, 307, 309, 312, 314, 316, 318, 320,
		322, 324, 326, 327, 329, 331, 333, 334, 336, 337, 339, 340, 342, 343,
		344, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 355, 356, 357,
		357, 358, 358, 359, 359, 359, 360, 360, 360, 360, 360, 360, 360, 360,
		360, 359, 359, 359, 358, 358, 357, 357, 356, 355, 355, 354, 353, 352,
		351, 350, 349, 348, 347, 346, 344, 343, 342, 340, 339, 337, 336, 334,
		333, 331, 329, 327, 326, 324, 322, 320, 318, 316, 314, 312, 309, 307,
		305, 303, 300, 298, 296, 293, 291, 288, 286, 283, 281, 278, 275, 273,
		270, 267, 265, 262, 259, 256, 253, 250, 247, 245, 242, 239, 236, 233,
		230, 227, 224, 220, 217, 214, 211, 208, 205, 202, 199, 196, 193, 189,
		186, 183, 180, 177, 174, 171, 167, 164, 161, 158, 155, 152, 149, 146,
		143, 140, 136, 133, 130, 127, 124, 121, 118, 115, 113, 110, 107, 104,
		101, 98, 95, 93, 90, 87, 85, 82, 79, 77, 74, 72, 69, 67, 64, 62, 60, 57,
		55, 53, 51, 48, 46, 44, 42, 40, 38, 36, 34, 33, 31, 29, 27, 26, 24, 23,
		21, 20, 18, 17, 16, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 5, 4, 3, 3, 2, 2,
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 20, 21, 23, 24, 26, 27, 29, 31,
		33, 34, 36, 38, 40, 42, 44, 46, 48, 51, 53, 55, 57, 60, 62, 64, 67, 69,
		72, 74, 77, 79, 82, 85, 87, 90, 93, 95, 98, 101, 104, 107, 110, 113,
		115, 118, 121, 124, 127, 130, 133, 136, 140, 143, 146, 149, 152, 155,
		158, 161, 164, 167, 171, 174, 177
};

void commutate(void){
	if (forward) {
		step++;
		if (step > 5) {
			step = 0;
			desync_check = 1;
		}
		rising = step % 2;
	} else {
		step--;
		if (step < 0) {
			step = 5;
			desync_check = 1;
		}
		rising = !(step % 2);
	}
	__disable_irq();
	if (!prop_brake_active) {
		(*comStep[step])();
	}
	__enable_irq();

	if (rising) {
		ENABLE_FALL_INTERRUPT();
	} else {
		ENABLE_RISE_INTERRUPT();
	}

	if (average_interval > 1700) {
		old_routine = 1;
	}
	bemfcounter = 0;
	zcfound = 0;
	commutation_intervals[step] = commutation_interval;
}

void interruptRoutine(void) {
	static uint8_t i;
	if (average_interval > 125) {
		if ((INTERVAL_TIMER_COUNT < 125) && (duty_cycle < 600)&& (zero_crosses < 500)) { // should be impossible, desync?exit anyway
			return;
		}
		stuckcounter++; // stuck at 100 interrupts before the main loop happens
						// again.
		if (stuckcounter > 100) {
			maskPhaseInterrupts();
			zero_crosses = 0;
			return;
		}
	}
	for (i = 0; i < filter_level; i++) {
		if (getCompOutputLevel() == rising) {
			return;
		}
	}
	__disable_irq();
	maskPhaseInterrupts();
	thiszctime = INTERVAL_TIMER_COUNT;
	SET_INTERVAL_TIMER_COUNT(0);

	SET_AND_ENABLE_COM_INT(waitTime+1);// enable COM_TIMER interrupt
	__enable_irq();
}


void PeriodElapsedCallback(void) {
	DISABLE_COM_TIMER_INT(); // disable interrupt
	commutate();
	commutation_interval = (3 * commutation_interval + thiszctime) >> 2;
	if (!AUTO_ADVANCE) {
		advance = (commutation_interval >> 3) * temp_advance; // 60 divde 8 7.5 degree increments
	} else {
		advance = (commutation_interval * auto_advance_level) >> 6; // 60 divde 64 0.9375 degree increments
	}
	waitTime = (commutation_interval >> 1) - advance;
	if (!old_routine) {
		enableCompInterrupts();
	}
	if (zero_crosses < 10000) {
		zero_crosses++;
	}
}

void zcfoundroutine(void) { // only used in polling mode, blocking routine.
	thiszctime = INTERVAL_TIMER_COUNT;
	SET_INTERVAL_TIMER_COUNT(0);
	commutation_interval = (thiszctime + (3 * commutation_interval)) / 4;
	advance = (commutation_interval >> 3) * 2; //   7.5 degree increments
	waitTime = commutation_interval / 2 - advance;

	while ((INTERVAL_TIMER_COUNT) < (waitTime)) {
		if (zero_crosses < 5) {
			break;
		}
	}
	TMR0_RELOAD(waitTime + 1); // enable COM_TIMER interrupt
	commutate();
	bemfcounter = 0;
	bad_count = 0;

	zero_crosses++;
	if (STALL_PROTECTION || RC_CAR_REVERSE) {
		if (zero_crosses >= 20 && commutation_interval <= 2000) {
			old_routine = 0;
			enableCompInterrupts(); // enable interrupt
		}
	} else {
		if (commutation_interval < 1300) {
			old_routine = 0;
			enableCompInterrupts(); // enable interrupt
		}
	}
}

void getBemfState(void) {
	static uint8_t current_state = 0;
	current_state = !getCompOutputLevel(); // polarity reversed
	if (rising) {
		if (current_state) {
			bemfcounter++;
		} else {
			bad_count++;
			if (bad_count > bad_count_threshold) {
				bemfcounter = 0;
			}
		}
	} else {
		if (!current_state) {
			bemfcounter++;
		} else {
			bad_count++;
			if (bad_count > bad_count_threshold) {
				bemfcounter = 0;
			}
		}
	}
}

void advanceincrement(void) {
	static int32_t xdata compareone;
	static int32_t xdata comparetwo;
	static int32_t xdata comparethree;
	if (forward) {
		phase_A_position++;
		if (phase_A_position > 359) {
			phase_A_position = 0;
		}
		phase_B_position++;
		if (phase_B_position > 359) {
			phase_B_position = 0;
		}
		phase_C_position++;
		if (phase_C_position > 359) {
			phase_C_position = 0;
		}
	} else {
		phase_A_position--;
		if (phase_A_position < 0) {
			phase_A_position = 359;
		}
		phase_B_position--;
		if (phase_B_position < 0) {
			phase_B_position = 359;
		}
		phase_C_position--;
		if (phase_C_position < 0) {
			phase_C_position = 359;
		}
	}

	compareone = (((2 * pwmSin[phase_A_position] / SINE_DIVIDER) + DEAD_TIME) * PWM_MAX_ARR / 2000) * SINE_MODE_POWER / 10;
	comparetwo = (((2 * pwmSin[phase_B_position] / SINE_DIVIDER) + DEAD_TIME) * PWM_MAX_ARR / 2000) * SINE_MODE_POWER / 10;
	comparethree = (((2 * pwmSin[phase_C_position] / SINE_DIVIDER) + DEAD_TIME) * PWM_MAX_ARR / 2000) * SINE_MODE_POWER / 10;

	setPWMCompare1(compareone);
	setPWMCompare2(comparetwo);
	setPWMCompare3(comparethree);
}

void startMotor(void) {
	if (running == 0) {
		commutate();
		commutation_interval = 10000;
		SET_INTERVAL_TIMER_COUNT(5000);
		running = 1;
	}
	enableCompInterrupts();
}
