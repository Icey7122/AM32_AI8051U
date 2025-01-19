#include "commutate.h"
#include "eeprom.h"
#include "mcu.h"
#include "targets.h"
#include "sounds.h"
#include "phaseouts.h"
#include "input.h"
#include "stc8051u_it.h"
#include "peripherals.h"
#include "serial_telemetry.h"
#include "main.h"
#include "functions.h"
#include "adc.h"

#include "stdio.h"

#include "string.h"
#include "version.h"

uint8_t drive_by_rpm = 0;
uint8_t use_speed_control_loop = 0;
uint32_t MINIMUM_RPM_SPEED_CONTROL = 1000;
uint32_t MAXIMUM_RPM_SPEED_CONTROL = 50000;
// assign speed control PID values values are x10000
fastPID speedPid = { // commutation speed loop time
		{ 0 },		  //err
		{ 10 },       //kp
		{ 0 },        //ki
		{ 100 },      //kd
		{ 0 }, 		  //integral
		{ 0 },  	  //derivative
	    { 0 }, 		  //last_error
		{ 0 },        //pid_output
		{ 10000 },    //integral_limit
		{ 50000 }     //output_limit
};

fastPID currentPid = { // 1khz loop time
		{ 0 },        //err
		{ 400 },      //kp
		{ 0 },        //ki
		{ 1000 },     //kd
		{ 0 }, 		  //integral
		{ 0 },  	  //derivative
	    { 0 }, 		  //last_error
		{ 0 },        //pid_output
		{ 20000 },    //integral_limit
		{ 100000 }    //output_limit
};


fastPID stallPid = { // 1khz loop time
		{ 0 },  	  //err
		{ 1 },        //kp
		{ 0 },        //ki
		{ 50 },       //kd
		{ 0 }, 		  //integral
		{ 0 },  	  //derivative
	    { 0 }, 		  //last_error
		{ 0 },        //pid_output
		{ 10000 },    //integral_limit
		{ 50000 }     //output_limit
};

uint8_t armed = 0;
uint8_t running = 0;
uint8_t fast_accel = 0;

uint16_t e_rpm;
uint16_t k_erpm;
uint32_t e_com_time;
uint32_t target_e_com_time;

uint8_t maximum_throttle_change_ramp = 1;

uint8_t cell_counter = 0;	 		//电池计数器
uint16_t battery_voltage = 0; 		//电池电压
int16_t actual_current = 0;		//实际电流
float consumed_current = 0;		//消耗电流
int16_t current_limit_adjust;   //电流限制
uint8_t degrees_celsius;

float stall_protection_adjust;
uint16_t stall_protect_target_interval = TARGET_STALL_PROTECTION_INTERVAL;

uint16_t telem_ms_counter = 0;		//遥测计数器
uint8_t send_telemetry = 0;			//发送遥测标志
uint8_t telemetry_interval_ms = 30; 	//遥测间隔


uint8_t adc_counter = 0;
uint16_t ledcounter = 0;			//LED计数器
uint16_t onekhzcounter = 0;			//1KHz计数器
uint16_t twentykhzcounter = 0;		//20KHz计数器
uint16_t signaltimecounter = 0;		//信号计数器
uint16_t low_voltage_counter = 0;   //低电压计数器
uint16_t armed_timeout_counter = 0; //握手超时计数器


void TwentyKhzRoutine(void)	{ //finish
	twentykhzcounter++;
	onekhzcounter++;
	ledcounter++;
	if (!armed) {
		if (cell_counter == 0) {
			if (inputSet) {
				if (adjusted_input == 0) {
					armed_timeout_counter++;
					if (armed_timeout_counter > LOOP_FREQUENCY_HZ) { // one second
						if (zero_input_counter > 30) {
							armed = 1;
							if ((cell_counter == 0) && LOW_VOLTAGE_CUTOFF) {
								static uint8_t i;
								cell_counter = battery_voltage / 370;
								for (i = 0; i < cell_counter; i++) {
									playInputTune();
									delayMillis(100);
									RELOAD_WATCHDOG_COUNTER();
								}
							} else {
								playInputTune();
							}
							if (!servoPwm) {
								RC_CAR_REVERSE = 0;
							}
						} else {
							inputSet = 0;
							armed_timeout_counter = 0;
						}
					}
				} else {
					armed_timeout_counter = 0;
				}
			}
		}
	}

	if (TLM_ON_INTERVAL) {
		telem_ms_counter++;
		if (telem_ms_counter > telemetry_interval_ms * 20) {
			send_telemetry = 1;
			telem_ms_counter = 0;
		}
	}

	if (!stepper_sine) {
		if (old_routine && running) {
			maskPhaseInterrupts();
			getBemfState();
			if (!zcfound) {
				if (rising) {
					if (bemfcounter > min_bemf_counts_up) {
						zcfound = 1;
						zcfoundroutine();
					}
				} else {
					if (bemfcounter > min_bemf_counts_down) {
						zcfound = 1;
						zcfoundroutine();
					}
				}
			}
		}

		if (onekhzcounter > PID_LOOP_DIVIDER) { // 1khz PID loop
			onekhzcounter = 0;
			if (USE_CURRENT_LIMIT && running) {
				current_limit_adjust -= doPidCalculations(&currentPid, actual_current, current_limit * 100) / 10000;
				if (current_limit_adjust < minimum_duty_cycle) {
					current_limit_adjust = minimum_duty_cycle;
				}
				if (current_limit_adjust > 2000) {
					current_limit_adjust = 2000;
				}
			}

			if (STALL_PROTECTION && running) { // this boosts throttle as the rpm gets lower, for crawlers
												// and rc cars only, do not use for multirotors.
				stall_protection_adjust += (doPidCalculations(&stallPid,commutation_interval, stall_protect_target_interval)) / 10000;
				if (stall_protection_adjust > 150) {
					stall_protection_adjust = 150;
				}
				if (stall_protection_adjust <= 0) {
					stall_protection_adjust = 0;
				}
			}
			
			if (use_speed_control_loop && running) {
				input_override += doPidCalculations(&speedPid, e_com_time,target_e_com_time) / 10000;
				if (input_override > 2047) {
					input_override = 2047;
				}
				if (input_override < 0) {
					input_override = 0;
				}
				if (zero_crosses < 100) {
					speedPid.integral = 0;
				}
			}
		}

		if (maximum_throttle_change_ramp) {
			if (zero_crosses < 150 || last_duty_cycle < 150) {
				max_duty_cycle_change = RAMP_SPEED_STARTUP;
			} else {
				if (average_interval > 500) {
					max_duty_cycle_change = RAMP_SPEED_LOW_RPM;
				} else {
					max_duty_cycle_change = RAMP_SPEED_HIGH_RPM;
				}
			}

			if ((duty_cycle - last_duty_cycle) > max_duty_cycle_change) {
				duty_cycle = last_duty_cycle + max_duty_cycle_change;
				if (commutation_interval > 500) {
					fast_accel = 1;
					temp_advance = ADVANCE_LEVEL;
				} else {
					fast_accel = 0;
				}
			} else if ((last_duty_cycle - duty_cycle) > max_duty_cycle_change) {
				duty_cycle = last_duty_cycle - max_duty_cycle_change;
				fast_accel = 0;
				temp_advance = ADVANCE_LEVEL;
			} else {
				if (duty_cycle < 300 && commutation_interval < 300) {
					temp_advance = ADVANCE_LEVEL;
				} else {
					temp_advance = ADVANCE_LEVEL;
				}

				fast_accel = 0;
			}
		}
		if ((armed && running) && duty_cycle > 47) {
			if (VARIABLE_PWM) {
			}
			adjusted_duty_cycle = ((duty_cycle * PWM_ARR) / 2000) + 1;
		} else {

			if (prop_brake_active) {
				adjusted_duty_cycle = PWM_MAX_ARR - ((prop_brake_duty_cycle * PWM_ARR) / 2000) + 1;
			} else {
				adjusted_duty_cycle = ((duty_cycle * PWM_ARR) / 2000);
			}
		}
		
		last_duty_cycle = duty_cycle;
		SET_AUTO_RELOAD_PWM(PWM_ARR);
		SET_DUTY_CYCLE_ALL(adjusted_duty_cycle);
	}
	signaltimecounter++;
}


static void checkDeviceInfo(void) {
#define DEVINFO_MAGIC1 0x5925e3da
#define DEVINFO_MAGIC2 0x4eb863d9

	const struct devinfo {
		uint32_t magic1;
		uint32_t magic2;
		const uint8_t deviceInfo[9];
	} *devinfo = (struct devinfo*) (0xFF1000 - 32);

	if (devinfo->magic1 != DEVINFO_MAGIC1 || devinfo->magic2 != DEVINFO_MAGIC2) {
		// bootloader does not support this feature, nothing to do
		return;
	}
	// change eeprom_address based on the code in the bootloaders device info
	switch (devinfo->deviceInfo[4]) {
	case 0x1f:
		eeprom_address = 0xFF7C00;
		break;
	case 0x35:
		eeprom_address = 0xFFF800;
		break;
	case 0x2b:
		eeprom_address = 0xFFF800;
		break;
	}

// 	// TODO: check pin code and reboot to bootloader if incorrect
}
/**********************************************/
void main(void)
{
	initAfterJump();

	checkDeviceInfo();

	initCorePeripherals();

	// enableCorePeripherals();

	loadEEpromSettings();


	if (VERSION_MAJOR != eepromBuffer[3] || VERSION_MINOR != eepromBuffer[4]) {
		eepromBuffer[3] = VERSION_MAJOR;
		eepromBuffer[4] = VERSION_MINOR;
		strncpy((char*) &eepromBuffer[5], FIRMWARE_NAME, 12);
		saveEEpromSettings();
	}

	if (DIR_REVERSED == 1) {
		forward = 0;
	} else {
		forward = 1;
	}

	PWM_ARR = PWM_MAX_ARR;

	if (!COMP_PWM) {
		USE_SIN_START = 0; // sine start requires complementary pwm.
	}

	if (RC_CAR_REVERSE) { // overrides a whole lot of things!
		throttle_max_at_low_rpm = 1000;
		BI_DIRECTION = 1;
		USE_SIN_START = 0;
		low_rpm_throttle_limit = 1;
		VARIABLE_PWM = 0;
		// stall_protection = 1;
		COMP_PWM = 0;
		STUCK_ROTOR_PROTECTION = 0;
		minimum_duty_cycle = minimum_duty_cycle + 50;
		stall_protect_minimum_duty = stall_protect_minimum_duty + 50;
		min_startup_duty_cycle = min_startup_duty_cycle + 50;
	}

	playStartupTune();

	zero_input_counter = 0;

	IWDG_Init();

	RELOAD_WATCHDOG_COUNTER();

	receiveDshotDma();
	if (drive_by_rpm) {
		use_speed_control_loop = 1;
	}

	setInputPullUp();

	while (1)
	{
		if(input_ready){
			setInput();
			input_ready = 0;
		}
		
		if(zero_crosses < 5){
	  		min_bemf_counts_up = TARGET_MIN_BEMF_COUNTS * 2;
			min_bemf_counts_down = TARGET_MIN_BEMF_COUNTS * 2;
		}else{
	 		min_bemf_counts_up = TARGET_MIN_BEMF_COUNTS;
			min_bemf_counts_down = TARGET_MIN_BEMF_COUNTS;
		}

		RELOAD_WATCHDOG_COUNTER();

		if (VARIABLE_PWM) {
			PWM_ARR = map(commutation_interval,96,200,PWM_MAX_ARR / 2,PWM_MAX_ARR);
		}

		if (signaltimecounter > (LOOP_FREQUENCY_HZ >> 1)) { // half second timeout when armed;
			static uint8_t i;
			if (armed) {
				allOff();
				armed = 0;
				input = 0;
				inputSet = 0;
				zero_input_counter = 0;
				SET_DUTY_CYCLE_ALL(0);
				resetInputCaptureTimer();
				for (i = 0; i < 64; i++) {
					dma_buffer[i] = 0;
				}
				NVIC_SystemReset();
			}
			if (signaltimecounter > LOOP_FREQUENCY_HZ << 1) { // 2 second when not armed
				allOff();
				armed = 0;
				input = 0;
				inputSet = 0;
				zero_input_counter = 0;
				SET_DUTY_CYCLE_ALL(0);
				resetInputCaptureTimer();
				for (i = 0; i < 64; i++) {
					dma_buffer[i] = 0;
				}
				NVIC_SystemReset();
			}
		}

		if (twentykhzcounter > LOOP_FREQUENCY_HZ) { // 1s sample interval 10000
			consumed_current = (float) actual_current / 360 + consumed_current;
		// 	switch (dshot_extended_telemetry) {

		// 	case 1:
		// 		send_extended_dshot = (uint16_t) 2 << 8 | degrees_celsius;
		// 		dshot_extended_telemetry = 2;
		// 		break;
		// 	case 2:
		// 		send_extended_dshot = (uint16_t) 6 << 8
		// 				| (uint8_t) actual_current / 50;
		// 		dshot_extended_telemetry = 3;
		// 		break;
		// 	case 3:
		// 		send_extended_dshot = (uint16_t) 4 << 8
		// 				| (uint8_t) (battery_voltage / 25);
		// 		dshot_extended_telemetry = 1;
		// 		break;
		// 	}
			twentykhzcounter = 0;
		}

		if ((zero_crosses > 1000) || (adjusted_input == 0)) {
			bemf_timeout_happened = 0;
		}
		if (zero_crosses > 100 && adjusted_input < 200) {
			bemf_timeout_happened = 0;
		}
		if (USE_SIN_START && adjusted_input < 160) {
			bemf_timeout_happened = 0;
		}

		if (adjusted_input < 150) { // startup duty cycle should be low enough to not burn motor
			bemf_timeout = 100;
		} else {
			bemf_timeout = 10;
		}

		average_interval = e_com_time / 3;

		if (desync_check && zero_crosses > 10) {
			if ((getAbsDif(last_average_interval, average_interval) > average_interval >> 1) && (average_interval < 2000)) { // throttle resitricted before zc 20.
				zero_crosses = 0;
				desync_happened++;
				if ((!BI_DIRECTION && (input > 47)) || commutation_interval > 1000) {
					running = 0;
				}
				old_routine = 1;
				if (zero_crosses > 100) {
					average_interval = 5000;
				}
				last_duty_cycle = min_startup_duty_cycle / 2;
			}
			desync_check = 0;
			last_average_interval = average_interval;
		}

		// if (dshot_telemetry && (commutation_interval > DSHOT_PRIORITY_THRESHOLD)) {
		// 	PPWMBH = 1;
		// 	PPWMB = 1;

		// 	PCMPH = 1;
		// 	PCMP = 0;

		// 	// NVIC_SetPriority(IC_DMA_IRQ_NAME, 0);
		// 	// NVIC_SetPriority(COM_TIMER_IRQ, 1);
		// 	// NVIC_SetPriority(COMPARATOR_IRQ, 1);
		// } else {
		// 	PPWMBH = 1;
		// 	PPWMB = 0;

		// 	PCMPH = 1;
		// 	PCMP = 1;
		// 	// NVIC_SetPriority(IC_DMA_IRQ_NAME, 1);
		// 	// NVIC_SetPriority(COM_TIMER_IRQ, 0);
		// 	// NVIC_SetPriority(COMPARATOR_IRQ, 0);
		// }

		if (send_telemetry) {
			makeTelemPackage(degrees_celsius, battery_voltage, (uint16_t)consumed_current, 0, e_rpm);
			send_telem_DMA();
			send_telemetry = 0;
		}

		adc_counter++;
		if (adc_counter > 200) { // for adc and telemetry
			Activate_ADC_DMA();
			degrees_celsius;
			battery_voltage = (uint32_t)((7 * battery_voltage) + (((float)ADC_raw_volts * 5000 / 4095 * TARGET_VOLTAGE_DIVIDER) / 100)) >> 3;
			actual_current = (((float)ADC_raw_current * 5000 / 41) - (CURRENT_OFFSET * 100)) / (MILLIVOLT_PER_AMP);
			if (actual_current < 0) {
				actual_current = 0;
			}
			if (LOW_VOLTAGE_CUTOFF) {
				if (battery_voltage < (cell_counter * low_cell_volt_cutoff)) {
					low_voltage_counter++;
					if (low_voltage_counter > (20000 - (stepper_sine * 900))) {
						input = 0;
						allOff();
						maskPhaseInterrupts();
						running = 0;
						zero_input_counter = 0;
						armed = 0;
					}
				} else {
					low_voltage_counter = 0;
				}
			}
			adc_counter = 0;
		}

		stuckcounter = 0;
		if(stepper_sine == 0) {
			e_rpm = running * (600000 / e_com_time); // in tens of rpm
			k_erpm = e_rpm / 10; // ecom time is time for one electrical revolution in microseconds

			if (low_rpm_throttle_limit) { // some hardware doesn't need this, its on
										  // by default to keep hardware / motors
										  // protected but can slow down the response
										  // in the very low end a little.
				maximum_duty_cycle = map(k_erpm, low_rpm_level, high_rpm_level,
						throttle_max_at_low_rpm, throttle_max_at_high_rpm); // for more performance lower the
																			// high_rpm_level, set to a
																			// consvervative number in source.
			} else {
				maximum_duty_cycle = 2000;
			}

			if (degrees_celsius > TEMPERATURE_LIMIT) {	
				maximum_duty_cycle = map(degrees_celsius,TEMPERATURE_LIMIT - 10, TEMPERATURE_LIMIT + 10,throttle_max_at_high_rpm / 2, 1);
			}

			if (zero_crosses < 100 && commutation_interval > 500) {
				filter_level = 12;
			} else {
				filter_level = map(average_interval, 100, 500, 3, 12);
			}

			if (commutation_interval < 50) {
				filter_level = 2;
			}

			if(AUTO_ADVANCE) {
				auto_advance_level = map(duty_cycle, 100, 2000, 13, 23);
			}
			
			if (INTERVAL_TIMER_COUNT > 45000 && running == 1) {
				bemf_timeout_happened++;

				maskPhaseInterrupts();
				old_routine = 1;
				if (duty_cycle < 48) {
					running = 0;
					commutation_interval = 5000;
				}
				zero_crosses = 0;
				zcfoundroutine();
			}
		} else { // stepper sine
			if (input > 48 && armed) {
				if (input > 48 && input < 137) { // sine wave stepper
					if (do_once_sinemode) {
						// disable commutation interrupt in case set
						DISABLE_COM_TIMER_INT();
						maskPhaseInterrupts();
						SET_DUTY_CYCLE_ALL(0);
						allpwm();
						do_once_sinemode = 0;
					}
					advanceincrement();
					step_delay = map(input, 48, 120, 7000 / MOTOR_POLES, 810 / MOTOR_POLES);
					delayMicros(step_delay);
					e_rpm = 600 / step_delay; // in hundreds so 33 e_rpm is 3300 actual erpm
				} else {
					do_once_sinemode = 1;
					advanceincrement();
					if (input > 200) {
						phase_A_position = 0;
						step_delay = 80;
					}
					delayMicros(step_delay);
					if (phase_A_position == 0) {
						stepper_sine = 0;
						running = 1;
						old_routine = 1;
						commutation_interval = 9000;
						average_interval = 9000;
						last_average_interval = average_interval;
						SET_INTERVAL_TIMER_COUNT(9000);
						zero_crosses = 20;
						prop_brake_active = 0;
						step = changeover_step;
						// comStep(step);// rising bemf on a same as position 0.
						if (STALL_PROTECTION) {
							last_duty_cycle = stall_protect_minimum_duty;
						}
						commutate();
						generatePwmTimerEvent();
					}
				}
			} else {
				do_once_sinemode = 1;
				if (BRAKE_ON_STOP) {
					duty_cycle = (PWM_MAX_ARR - 19) + DRAG_BRAKE_STRENGTH * 2;
					adjusted_duty_cycle = PWM_MAX_ARR - ((duty_cycle * PWM_ARR) / PWM_MAX_ARR) + 1;
					proportionalBrake();
					SET_DUTY_CYCLE_ALL(adjusted_duty_cycle);
					prop_brake_active = 1;
				} else {
					SET_DUTY_CYCLE_ALL(0);
					allOff();
				}
				e_rpm = 0;
			}
		}
	}
}



