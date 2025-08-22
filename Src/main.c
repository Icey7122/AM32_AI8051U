#include "main.h"
#include "commutate.h"
#include "eeprom.h"
#include "mcu.h"
#include "targets.h"
#include "sounds.h"
#include "phaseouts.h"
#include "input.h"
#include "stc8051u_it.h"
#include "peripherals.h"
#include "kiss_telemetry.h"
#include "functions.h"
#include "adc.h"

#include "stdio.h"
#include "string.h"
#include "version.h"

// Flags
bool armed = 0;
bool crawler_mode = 0;
bool send_telemetry = 0; // 发送遥测标志
bool send_esc_info_flag = 0;
bool maximum_throttle_change_ramp = 1;
uint8_t drive_by_rpm = 0;
uint8_t use_speed_control_loop = 0;
uint8_t running = 0;
uint8_t fast_accel = 0;
int8_t startup_boost = 50;
int8_t reversing_dead_band = 1;
uint16_t enter_sine_angle = 180;

// Speed Control
uint32_t MINIMUM_RPM_SPEED_CONTROL = 1000;
uint32_t MAXIMUM_RPM_SPEED_CONTROL = 100000;

// PID Controllers
fastPID speedPid = { // commutation speed loop time
        { 0 },		  // err
        { 10 },       // kp
        { 0 },        // ki
        { 100 },      // kd
        { 0 }, 		  // integral
        { 0 },  	  // derivative
        { 0 }, 		  // last_error
        { 0 },        // pid_output
        { 10000 },    // integral_limit
        { 50000 }     // output_limit
};

fastPID currentPid = { // 1khz loop time
        { 0 },        // err
        { 400 },      // kp
        { 0 },        // ki
        { 1000 },     // kd
        { 0 }, 		  // integral
        { 0 },  	  // derivative
        { 0 }, 		  // last_error
        { 0 },        // pid_output
        { 20000 },    // integral_limit
        { 100000 }    // output_limit
};

fastPID stallPid = { // 1khz loop time
        { 0 },  	  // err
        { 1 },        // kp
        { 0 },        // ki
        { 50 },       // kd
        { 0 }, 		  // integral
        { 0 },  	  // derivative
        { 0 }, 		  // last_error
        { 0 },        // pid_output
        { 10000 },    // integral_limit
        { 50000 }     // output_limit
};

// RPM and Timing
uint16_t e_rpm;
uint16_t k_erpm;
uint32_t e_com_time;
uint32_t target_e_com_time;

// Battery and Current
uint8_t cell_count = 0; // 电池计数器
uint16_t battery_voltage = 0; // 电池电压
int16_t actual_current = 0; // 实际电流
float consumed_current = 0; // 消耗电流
int16_t use_current_limit_adjust; // 电流限制
uint8_t degrees_celsius;

// Stall Protection
float stall_protection_adjust;
uint16_t stall_protect_target_interval = TARGET_STALL_PROTECTION_INTERVAL;

// Telemetry
uint16_t telem_ms_count = 0; // 遥测计数器
uint8_t telemetry_interval_ms = 30; // 遥测间隔

// Counters
uint8_t adc_counter = 0;
uint16_t ledcounter = 0; // LED计数器
uint16_t one_khz_loop_counter = 0; // 1KHz计数器
uint16_t tenkhzcounter = 0; // 20KHz计数器
uint16_t signaltimeout = 0; // 信号计数器
uint16_t low_voltage_count = 0; // 低电压计数器
uint16_t armed_timeout_count = 0; // 握手超时计数器


void tenKhzRoutine(void){
	duty_cycle = duty_cycle_setpoint;
	tenkhzcounter++;
	ledcounter++;
	one_khz_loop_counter++;
	if (!armed) {
		if (cell_count == 0) {
			if (inputSet) {
				if (adjusted_input == 0) {
					armed_timeout_count++;
					if (armed_timeout_count > LOOP_FREQUENCY_HZ) { // one second
						if (zero_input_count > 30) {
							armed = 1;
#ifdef USE_LED_STRIP
                            //	send_LED_RGB(0,0,0);
                            delayMicros(1000);
                            send_LED_RGB(0, 255, 0);
#endif
#ifdef USE_RGB_LED
                            GPIOB->BRR = LL_GPIO_PIN_3; // turn on green
                            GPIOB->BSRR = LL_GPIO_PIN_8; // turn on green
                            GPIOB->BSRR = LL_GPIO_PIN_5;
#endif
							if ((cell_count == 0) && LOW_VOLTAGE_CUTOFF) {
								static uint8_t i;
								cell_count = battery_voltage / 370;
								for (i = 0; i < cell_count; i++) {
									playInputTune();
									delayMillis(100);
									RELOAD_WATCHDOG_COUNTER();
								}
							} else {
#ifdef MCU_AT415
								play_tone_flag = 4;
#else
								playInputTune();
#endif
							}
							if (!servoPwm) {
								eepromBuffer.eeppack.rc_car_reverse = 0;
							}
						} else {
							inputSet = 0;
							armed_timeout_count = 0;
						}
					}
				} else {
					armed_timeout_count = 0;
				}
			}
		}
	}

	if (eepromBuffer.eeppack.telemetry_on_interval) {
        telem_ms_count++;
        if (telem_ms_count > ((telemetry_interval_ms - 1 + eepromBuffer.eeppack.telemetry_on_interval) * 20)) {
            // telemetry_on_interval = 1 is a boolean, but it can also be 2 or more to indicate an identifier
            // by making the interval just slightly different with an unique identifier, we can guarantee that many ESCs can communicate on just one signal
            // there will be some collisions but not as many as if two ESCs always tried to talk at once.
            send_telemetry = 1;
            telem_ms_count = 0;
        }
	}

#ifndef BRUSHED_MODE

	if (!stepper_sine) {
#ifndef CUSTOM_RAMP
		if (old_routine && running) {
			// send_LED_RGB(255, 0, 0);
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
#endif
		if (one_khz_loop_counter > PID_LOOP_DIVIDER) { // 1khz PID loop
			one_khz_loop_counter = 0;
            if (use_current_limit && running) {
                use_current_limit_adjust -= (int16_t)(doPidCalculations(&currentPid, actual_current,
                                                          eepromBuffer.eeppack.limits.current * 2 * 100)
                    / 10000);
                if (use_current_limit_adjust < minimum_duty_cycle) {
                    use_current_limit_adjust = minimum_duty_cycle;
                }
                if (use_current_limit_adjust > 2000) {
                    use_current_limit_adjust = 2000;
                }
            }

			if (eepromBuffer.eeppack.stall_protection && running) { // this boosts throttle as the rpm gets lower, for crawlers
												// and rc cars only, do not use for multirotors.
				stall_protection_adjust += (doPidCalculations(&stallPid, commutation_interval,
                                                stall_protect_target_interval));
                if (stall_protection_adjust > 150UL * 10000) {
                    stall_protection_adjust = 150UL * 10000;
                }
                if (stall_protection_adjust <= 0) {
                    stall_protection_adjust = 0;
                }
			}
			if (use_speed_control_loop && running) {
                input_override += doPidCalculations(&speedPid, e_com_time, target_e_com_time);
                if (input_override > 2047UL * 10000) {
                    input_override = 2047UL * 10000;
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
			//	max_duty_cycle_change = map(k_erpm, low_rpm_level,
            // high_rpm_level, 1, 40);
#ifdef VOLTAGE_BASED_RAMP
            uint16_t voltage_based_max_change = map(battery_voltage, 800, 2200, 10, 1);
            if (average_interval > 200) {
                max_duty_cycle_change = voltage_based_max_change;
            } else {
                max_duty_cycle_change = voltage_based_max_change * 3;
            }
#else
            if (zero_crosses < 150 || last_duty_cycle < 150) {   
                max_duty_cycle_change = RAMP_SPEED_STARTUP;
            } else {
                if (average_interval > 500) {
                    max_duty_cycle_change = RAMP_SPEED_LOW_RPM;
                } else {
                    max_duty_cycle_change = RAMP_SPEED_HIGH_RPM;
                }
            }
#endif
#ifdef CUSTOM_RAMP
   //         max_duty_cycle_change = eepromBuffer[30];
#endif
            if ((duty_cycle - last_duty_cycle) > max_duty_cycle_change) {
                duty_cycle = last_duty_cycle + max_duty_cycle_change;
                if (commutation_interval > 500) {
                    fast_accel = 1;
					temp_advance = eepromBuffer.eeppack.advance_level;
                } else {
                    fast_accel = 0;
                }

            } else if ((last_duty_cycle - duty_cycle) > max_duty_cycle_change) {
                duty_cycle = last_duty_cycle - max_duty_cycle_change;
                fast_accel = 0;
				temp_advance = eepromBuffer.eeppack.advance_level;
            } else {
				if(duty_cycle < 300 && commutation_interval < 300){
					temp_advance = eepromBuffer.eeppack.advance_level;
				}else{
					temp_advance =  eepromBuffer.eeppack.advance_level;
				}

                fast_accel = 0;
            }
		}
		if ((armed && running) && input > 47) {
			if (eepromBuffer.eeppack.variable_pwm) {
			}
			adjusted_duty_cycle = ((duty_cycle * pwm_arr) / 2000) + 1;

		} else {

			if (prop_brake_active) {
				adjusted_duty_cycle = PWM_MAX_ARR - ((prop_brake_duty_cycle * pwm_arr) / 2000) + 1;
			} else {
				adjusted_duty_cycle = ((duty_cycle * pwm_arr) / 2000);
			}
		}
		last_duty_cycle = duty_cycle;
		SET_AUTO_RELOAD_PWM(pwm_arr);
		SET_DUTY_CYCLE_ALL(adjusted_duty_cycle);
	}
#endif // ndef brushed_mode
#if defined(FIXED_DUTY_MODE) || defined(FIXED_SPEED_MODE)
    if (getInputPinState()) {
        signaltimeout++;
        if (signaltimeout > LOOP_FREQUENCY_HZ) {
            NVIC_SystemReset();
        }
    } else {
        signaltimeout = 0;
    }
#else
    signaltimeout++;

#endif
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


    if (VERSION_MAJOR != eepromBuffer.eeppack.version.major || 
		VERSION_MINOR != eepromBuffer.eeppack.version.minor || 
		eeprom_layout_version > eepromBuffer.eeppack.eeprom_version) {
        eepromBuffer.eeppack.version.major = VERSION_MAJOR;
        eepromBuffer.eeppack.version.minor = VERSION_MINOR;
       	strncpy(eepromBuffer.eeppack.firmware_name, FIRMWARE_NAME, 12);
        saveEEpromSettings();
    }

	if (eepromBuffer.eeppack.dir_reversed == 1) {
		forward = 0;
	} else {
		forward = 1;
	}

	pwm_arr = PWM_MAX_ARR;

	if (!eepromBuffer.eeppack.comp_pwm) {
		eepromBuffer.eeppack.use_sine_start = 0; // sine start requires complementary pwm.
	}

	if (eepromBuffer.eeppack.rc_car_reverse) { // overrides a whole lot of things!
		throttle_max_at_low_rpm = 1000;
		eepromBuffer.eeppack.bi_direction = 1;
		eepromBuffer.eeppack.use_sine_start = 0;
		low_rpm_throttle_limit= 1;
		eepromBuffer.eeppack.variable_pwm = 0;
		// eepromBuffer.eeppack.stall_protection = 1;
		eepromBuffer.eeppack.comp_pwm = 0;
		eepromBuffer.eeppack.stuck_rotor_protection = 0;
        minimum_duty_cycle = minimum_duty_cycle + 50;
        stall_protect_minimum_duty = stall_protect_minimum_duty + 50;
        min_startup_duty = min_startup_duty + 50;
	}

#ifdef MCU_F031
    GPIOF->BSRR = LL_GPIO_PIN_6; // uncomment to take bridge out of standby mode
                                 // and set oc level
    GPIOF->BRR = LL_GPIO_PIN_7; // out of standby mode
    GPIOA->BRR = LL_GPIO_PIN_11;
#endif
#ifdef MCU_G031
    GPIOA->BRR = LL_GPIO_PIN_11;
    GPIOA->BSRR = LL_GPIO_PIN_12;    // Pa12 attached to enable on dev board
#endif
#ifdef USE_LED_STRIP
    send_LED_RGB(125, 0, 0);
#endif

#ifdef USE_CRSF_INPUT
    inputSet = 1;
    playStartupTune();
    MX_IWDG_Init();
    LL_IWDG_ReloadCounter(IWDG);
#else
#if defined(FIXED_DUTY_MODE) || defined(FIXED_SPEED_MODE)
    MX_IWDG_Init();
    RELOAD_WATCHDOG_COUNTER();
    inputSet = 1;
    armed = 1;
    adjusted_input = 48;
    newinput = 48;
		comStep(2);
#ifdef FIXED_SPEED_MODE
    use_speed_control_loop = 1;
    eepromBuffer.use_sine_start = 0;
    target_e_com_time = 60000000 / FIXED_SPEED_MODE_RPM / (eepromBuffer.motor_poles / 2);
    input = 48;
#endif

#else
#ifdef BRUSHED_MODE
    // bi_direction = 1;
    commutation_interval = 5000;
    eepromBuffer.use_sine_start = 0;
    maskPhaseInterrupts();
    playBrushedStartupTune();
#else
#ifdef MCU_AT415
    play_tone_flag = 5;
#else
    playStartupTune();
#endif
#endif
	zero_input_count = 0;
	IWDG_Init();
	RELOAD_WATCHDOG_COUNTER();
#ifdef GIMBAL_MODE
    eepromBuffer.bi_direction = 1;
    eepromBuffer.use_sine_start = 1;
#endif

#ifdef USE_ADC_INPUT
    armed_count_threshold = 5000;
    inputSet = 1;
#else
	receiveDmaBuffer();
	if (drive_by_rpm) {
		use_speed_control_loop = 1;
	}
#endif

#endif // end fixed duty mode ifdef
#endif // end crsf input

#ifdef MCU_F051
    MCU_Id = DBGMCU->IDCODE &= 0xFFF;
    REV_Id = DBGMCU->IDCODE >> 16;

    if (REV_Id >= 4096) {
        temperature_offset = 0;
    } else {
        temperature_offset = 230;
    }

#endif
#ifdef NEUTRONRC_G071
    setInputPullDown();
#else
	setInputPullUp();
#endif

#ifdef USE_STARTUP_BOOST
	min_startup_duty = min_startup_duty + 200 + ((eepromBuffer.eeppack.pwm_frequency * 100)/24);
	minimum_duty_cycle = minimum_duty_cycle + 50 + ((eepromBuffer.eeppack.pwm_frequency * 50 )/24);
	startup_max_duty_cycle = startup_max_duty_cycle + 400;
#endif

	while (1)
	{	
		e_com_time = (commutation_intervals[0] + commutation_intervals[1] + 
				  	  commutation_intervals[2] + commutation_intervals[3] + 
				  	  commutation_intervals[4] + commutation_intervals[5] + 4) >> 1;
#if defined(FIXED_DUTY_MODE) || defined(FIXED_SPEED_MODE)
        setInput();
#endif

#ifdef NEED_INPUT_READY
        if (input_ready) {
            processDshot();
            input_ready = 0;
        }
#endif
		if(zero_crosses < 5){
	  		min_bemf_counts_up = TARGET_MIN_BEMF_COUNTS * 2;
			min_bemf_counts_down = TARGET_MIN_BEMF_COUNTS * 2;
		}else{
	 		min_bemf_counts_up = TARGET_MIN_BEMF_COUNTS;
			min_bemf_counts_down = TARGET_MIN_BEMF_COUNTS;
		}

		RELOAD_WATCHDOG_COUNTER();

		if (eepromBuffer.eeppack.variable_pwm) {
			pwm_arr = map(commutation_interval,96,200,PWM_MAX_ARR / 2,PWM_MAX_ARR);
		}
        if (eepromBuffer.eeppack.variable_pwm == 2) {      // uses automatic range   
			if(average_interval < 250 && average_interval > 100){
				pwm_arr = average_interval * (CPU_FREQUENCY_MHZ/9);
			}
			if(average_interval < 100 && average_interval > 0){
				pwm_arr = 100 * (CPU_FREQUENCY_MHZ/9);
			}
			if((average_interval >= 250) || (average_interval == 0)){
				pwm_arr = 250 * (CPU_FREQUENCY_MHZ/9);
			} 
        }
		if (signaltimeout > (LOOP_FREQUENCY_HZ >> 1)) { // half second timeout when armed;
			static uint8_t i;
			if (armed) {
				allOff();
				armed = 0;
				input = 0;
				inputSet = 0;
				zero_input_count = 0;
				SET_DUTY_CYCLE_ALL(0);
				resetInputCaptureTimer();
				for (i = 0; i < 64; i++) {
					dma_buffer[i] = 0;
				}
				NVIC_SystemReset();
			}
			if (signaltimeout > LOOP_FREQUENCY_HZ << 1) { // 2 second when not armed
				allOff();
				armed = 0;
				input = 0;
				inputSet = 0;
				zero_input_count = 0;
				SET_DUTY_CYCLE_ALL(0);
				resetInputCaptureTimer();
				for (i = 0; i < 64; i++) {
					dma_buffer[i] = 0;
				}
				NVIC_SystemReset();
			}
		}
#ifdef USE_CUSTOM_LED
        if ((input >= 47) && (input < 1947)) {
            if (ledcounter > (2000 >> forward)) {
                GPIOB->BSRR = LL_GPIO_PIN_3;
            } else {
                GPIOB->BRR = LL_GPIO_PIN_3;
            }
            if (ledcounter > (4000 >> forward)) {
                ledcounter = 0;
            }
        }
        if (input > 1947) {
            GPIOB->BSRR = LL_GPIO_PIN_3;
        }
        if (input < 47) {
            GPIOB->BRR = LL_GPIO_PIN_3;
        }
#endif

		if (tenkhzcounter > LOOP_FREQUENCY_HZ) { // 1s sample interval 10000
			consumed_current = (float) actual_current / 360.0 + consumed_current;
			tenkhzcounter = 0;
		}

#ifndef BRUSHED_MODE

		if ((zero_crosses > 1000) || (adjusted_input == 0)) {
			bemf_timeout_happened = 0;
		}
		if (zero_crosses > 100 && adjusted_input < 200) {
			bemf_timeout_happened = 0;
		}
		if (eepromBuffer.eeppack.use_sine_start && adjusted_input < 160) {
			bemf_timeout_happened = 0;
		}

        if (crawler_mode) {
            if (adjusted_input < 400) {
                bemf_timeout_happened = 0;
            }
        } else {
            if (adjusted_input < 150) { // startup duty cycle should be low enough to not burn motor
                bemf_timeout = 100;
            } else {
                bemf_timeout = 10;
            }
        }
#endif
        average_interval = e_com_time / 3;
        if (desync_check && zero_crosses > 10) {
            if ((getAbsDif(last_average_interval, average_interval) > average_interval >> 1) && (average_interval < 2000)) { // throttle resitricted before zc 20.
                zero_crosses = 0;
                desync_happened++;
                if ((!eepromBuffer.eeppack.bi_direction && (input > 47)) || commutation_interval > 1000) {
                    running = 0;
                }
                old_routine = 1;
                if (zero_crosses > 100) {
                    average_interval = 5000;
                }
                last_duty_cycle = min_startup_duty / 2;
            }
            desync_check = 0;
            //	}
            last_average_interval = average_interval;
        }

#if !defined(MCU_G031) && !defined(NEED_INPUT_READY)
        // if (dshot_telemetry && (commutation_interval > DSHOT_PRIORITY_THRESHOLD)) {
        //      NVIC_SetPriority(IC_DMA_IRQ_NAME, 0);
        //      NVIC_SetPriority(COM_TIMER_IRQ, 1);
        //      NVIC_SetPriority(COMPARATOR_IRQ, 1);
        //  } else {
        //      NVIC_SetPriority(IC_DMA_IRQ_NAME, 1);
        //      NVIC_SetPriority(COM_TIMER_IRQ, 0);
        //      NVIC_SetPriority(COMPARATOR_IRQ, 0);
        //  }
#endif

		if (send_telemetry) {
#ifdef USE_SERIAL_TELEMETRY
			makeTelemPackage(degrees_celsius, battery_voltage, actual_current,consumed_current, e_rpm);
			send_telem_DMA();
			send_telemetry = 0;
#endif
		}else if (send_esc_info_flag){
			
		}

		adc_counter++;
		if (DMA_ADC_STA) { // for adc and telemetry
#if defined(STMICRO)
            ADC_DMA_Callback();
            LL_ADC_REG_StartConversion(ADC1);
            converted_degrees = __LL_ADC_CALC_TEMPERATURE(3300, ADC_raw_temp, LL_ADC_RESOLUTION_12B);
#endif
#ifdef MCU_GDE23
            ADC_DMA_Callback();
            // converted_degrees = (1.43 - ADC_raw_temp * 3.3 / 4096) * 1000 / 4.3 + 25;
            converted_degrees = ((int32_t)(357.5581395348837f * (1 << 16)) - ADC_raw_temp * (int32_t)(0.18736373546511628f * (1 << 16))) >> 16;
            adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
#endif
#ifdef ARTERY
            ADC_DMA_Callback();
            adc_ordinary_software_trigger_enable(ADC1, TRUE);
            converted_degrees = getConvertedDegrees(ADC_raw_temp);
#endif
#ifdef WCH
            startADCConversion( );
            converted_degrees = getConvertedDegrees(ADC_raw_temp);
#endif
#ifdef STC
			ADC_DMA_Callback();
#endif
			degrees_celsius;
			battery_voltage = (uint32_t)((7 * battery_voltage) + (((float)ADC_raw_volts * 5000 / 4095 * TARGET_VOLTAGE_DIVIDER) / 100)) >> 3;
			actual_current = (((float)ADC_raw_current * 5000 / 41) - (CURRENT_OFFSET * 100)) / (MILLIVOLT_PER_AMP);
			if (actual_current < 0) {
				actual_current = 0;
			}
			if (LOW_VOLTAGE_CUTOFF) {
				if (battery_voltage < (cell_count * low_cell_volt_cutoff)) {
					low_voltage_count++;
					if (low_voltage_count > (20000 - (stepper_sine * 900))) {
						input = 0;
						allOff();
						maskPhaseInterrupts();
						running = 0;
						zero_input_count = 0;
						armed = 0;
					}
				} else {
					low_voltage_count = 0;
				}
			}
            adc_counter = 0;
			DMA_ADC_STA = 0x00;
			Activate_ADC_DMA();
#ifdef USE_ADC_INPUT
            if (ADC_raw_input < 10) {
                zero_input_count++;
            } else {
                zero_input_count = 0;
            }
#endif
		}
#ifdef USE_ADC_INPUT
        signaltimeout = 0;
        ADC_smoothed_input = (((10 * ADC_smoothed_input) + ADC_raw_input) / 11);
        newinput = ADC_smoothed_input / 2;
        if (newinput > 2000) {
            newinput = 2000;
        }
#endif
		stuckcounter = 0;
		if(stepper_sine == 0) {
			e_rpm = running * (600000 / e_com_time); // in tens of rpm
			k_erpm = e_rpm / 10; // ecom time is time for one electrical revolution in microseconds

			if (low_rpm_throttle_limit) { // some hardware doesn't need this, its on
										  // by default to keep hardware / motors
										  // protected but can slow down the response
										  // in the very low end a little.
				duty_cycle_maximum = map(k_erpm, low_rpm_level, high_rpm_level,
						throttle_max_at_low_rpm, throttle_max_at_high_rpm); // for more performance lower the
																			// high_rpm_level, set to a
																			// consvervative number in source.
			} else {
				duty_cycle_maximum = 2000;
			}

			if (degrees_celsius > eepromBuffer.eeppack.limits.temperature) {	
              	duty_cycle_maximum = map(degrees_celsius, eepromBuffer.eeppack.limits.temperature - 10, eepromBuffer.eeppack.limits.temperature + 10,
                	throttle_max_at_high_rpm / 2, 1);
			}

			if (zero_crosses < 100 && commutation_interval > 500) {
				filter_level = 12;
			} else {
				filter_level = map(average_interval, 100, 500, 3, 12);
			}

			if (commutation_interval < 50) {
				filter_level = 2;
			}

            if (eepromBuffer.eeppack.auto_advance) {
              	auto_advance_level = map(duty_cycle, 100, 2000, 13, 23);
            }
            /**************** old routine*********************/
#ifdef CUSTOM_RAMP
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
#endif
			if (INTERVAL_TIMER_COUNT() > 45000 && running == 1) {
				bemf_timeout_happened++;

				maskPhaseInterrupts();
				old_routine = 1;
				if (input < 48) {
					running = 0;
					commutation_interval = 5000;
				}
				zero_crosses = 0;
				zcfoundroutine();
			}
		} else { // stepper sine

#ifdef GIMBAL_MODE
            step_delay = 300;
            maskPhaseInterrupts();
            allpwm();
            if (newinput > 1000) {
                desired_angle = map(newinput, 1000, 2000, 180, 360);
            } else {
                desired_angle = map(newinput, 0, 1000, 0, 180);
            }
            if (current_angle > desired_angle) {
                forward = 1;
                advanceincrement();
                delayMicros(step_delay);
                current_angle--;
            }
            if (current_angle < desired_angle) {
                forward = 0;
                advanceincrement();
                delayMicros(step_delay);
                current_angle++;
            }
#else
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
					step_delay = map(input, 48, 120, 7000 / eepromBuffer.eeppack.motor_poles, 810 / eepromBuffer.eeppack.motor_poles);
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
						if (eepromBuffer.eeppack.stall_protection) {
							last_duty_cycle = stall_protect_minimum_duty;
						}
						commutate();
						generatePwmTimerEvent();
					}
				}
			} else {
				do_once_sinemode = 1;
				if (eepromBuffer.eeppack.brake_on_stop) {
#ifndef PWM_ENABLE_BRIDGE
					duty_cycle = (PWM_MAX_ARR - 19) + eepromBuffer.eeppack.drag_brake_strength * 2;
					adjusted_duty_cycle = PWM_MAX_ARR - ((duty_cycle * pwm_arr) / PWM_MAX_ARR) + 1;
					proportionalBrake();
					SET_DUTY_CYCLE_ALL(adjusted_duty_cycle);
					prop_brake_active = 1;
#else
                    // todo add braking for PWM /enable style bridges.
#endif
				} else {
					SET_DUTY_CYCLE_ALL(0);
					allOff();
				}
				e_rpm = 0;
			}
#endif // gimbal mode
		}
#ifdef BRUSHED_MODE
        runBrushedLoop();
#endif
#if DRONECAN_SUPPORT
	DroneCAN_update();
#endif
	}
}



