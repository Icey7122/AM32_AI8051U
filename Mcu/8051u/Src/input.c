#include "main.h"
#include "input.h"
#include "targets.h"
#include "functions.h"
#include "eeprom.h"
#include "sounds.h"
#include "phaseouts.h"
#include "commutate.h"


// Flags
bool servoPwm = 0;
bool ushot = 0;
bool inputSet = 0; // 输入设置
bool brushed_direction_set = 0;
bool return_to_center = 0;
uint8_t play_tone_flag;

// Timer and buffer
uint16_t ic_timer_prescaler = 0;
uint8_t buffersize = 32;
uint16_t xdata dma_buffer[64] = { 0 };
uint16_t smallestnumber = 20000;
uint16_t average_signal_pulse;
uint16_t zero_input_count = 0;

// Input values
int16_t newinput;
int16_t adjusted_input;
int16_t input;
int16_t last_input;
int32_t input_override;

// Duty cycle values
int32_t duty_cycle;
int32_t duty_cycle_setpoint;
int32_t adjusted_duty_cycle;
int32_t last_duty_cycle;
int32_t prop_brake_duty_cycle;
uint8_t max_duty_cycle_change;

receiveFUNC_t *receiveDmaBuffer = servoDmaBuffer;

void transfercomplete(void) {

	if (!inputSet) {
		detectInput();
		receiveDmaBuffer();
		return;
	} else {
		if (ushot == 1) {
			computeUshotDMA();
			receiveDmaBuffer();
		}
		if (servoPwm == 1) {
			if (getInputPinState()) {
				buffersize = 3;
			} else {
				buffersize = 2;
				computeServoInput();
			}
			receiveDmaBuffer();
		}
		if (!armed) {
			if (adjusted_input == 0 && calibration_required == 0) { // note this in input..not newinput so it
																	// will be adjusted be main loop
				zero_input_count++;
			} else {
				zero_input_count = 0;
				if (adjusted_input > 1500) {
					if (getAbsDif(adjusted_input, last_input) > 50) {
						enter_calibration_counter = 0;
					} else {
						enter_calibration_counter++;
					}
					if (enter_calibration_counter > 50 && (!high_calibration_set)) {
						playBeaconTune3();
						calibration_required = 1;
						enter_calibration_counter = 0;
					}
					last_input = adjusted_input;
				}
			}
		}
	}
}

void detectInput(void) {
	static uint16_t j;
	static uint32_t lastnumber;
	lastnumber = dma_buffer[0];
	smallestnumber = 20000;
	average_signal_pulse = 0;
	for (j = 1; j < 31; j++) {
	    if (dma_buffer[j] - lastnumber > 0) {
	        if ((dma_buffer[j] - lastnumber) < smallestnumber) {
	            smallestnumber = dma_buffer[j] - lastnumber;
	        }
	        average_signal_pulse += (dma_buffer[j] - lastnumber);
	    }
	    lastnumber = dma_buffer[j];
	}
	average_signal_pulse = average_signal_pulse / 32;

	if (ushot == 1) {
	    checkUshot();
	}
	if (servoPwm == 1) {
		checkServo();
	}

	if (!ushot && !servoPwm) {
		checkServo();
		checkUshot();
	}
}

void setInput(void)	{
	if (eepromBuffer.eeppack.bi_direction) {
		if (ushot == 0) {
			if (eepromBuffer.eeppack.rc_car_reverse) {
				if (newinput > (1000 + (servo_dead_band << 1))) {
					if (forward == eepromBuffer.eeppack.dir_reversed) {
						adjusted_input = 0;
						//               if (running) {
						prop_brake_active = 1;
						if (return_to_center) {
							forward = !eepromBuffer.eeppack.dir_reversed;
							prop_brake_active = 0;
							return_to_center = 0;
						}
					}
					if (prop_brake_active == 0) {
						return_to_center = 0;
						adjusted_input = map(newinput, 1000 + (servo_dead_band << 1), 2000, 47, 2047);
					}
				}
				if (newinput < (1000 - (servo_dead_band << 1))) {
					if (forward == (!eepromBuffer.eeppack.dir_reversed)) {
						adjusted_input = 0;
						prop_brake_active = 1;
						if (return_to_center) {
							forward = eepromBuffer.eeppack.dir_reversed;
							prop_brake_active = 0;
							return_to_center = 0;
						}
					}
					if (prop_brake_active == 0) {
						return_to_center = 0;
						adjusted_input = map(newinput, 0, 1000 - (servo_dead_band << 1), 2047, 47);
					}
				}
				if (newinput >= (1000 - (servo_dead_band << 1)) && newinput <= (1000 + (servo_dead_band << 1))) {
					adjusted_input = 0;
					if (prop_brake_active) {
						prop_brake_active = 0;
						return_to_center = 1;
					}
				}
			} else {
				if (newinput > (1000 + (servo_dead_band << 1))) {
					if (forward == eepromBuffer.eeppack.dir_reversed) {
						if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
							forward = !eepromBuffer.eeppack.dir_reversed;
							zero_crosses = 0;
							old_routine = 1;
							maskPhaseInterrupts();
							brushed_direction_set = 0;
						} else {
							newinput = 1000;
						}
					}
					adjusted_input = map(newinput,1000 + (servo_dead_band << 1), 2000, 47, 2047);
				}
				if (newinput < (1000 - (servo_dead_band << 1))) {
					if (forward == (!eepromBuffer.eeppack.dir_reversed)) {
						if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
							zero_crosses = 0;
							old_routine = 1;
							forward = eepromBuffer.eeppack.dir_reversed;
							maskPhaseInterrupts();
							brushed_direction_set = 0;
						} else {
							newinput = 1000;
						}
					}
					adjusted_input = map(newinput, 0, 1000 - (servo_dead_band << 1), 2047, 47);
				}

				if (newinput >= (1000 - (servo_dead_band << 1)) && newinput <= (1000 + (servo_dead_band << 1))) {
					adjusted_input = 0;
					brushed_direction_set = 0;
				}
			}
		}

		if (ushot) {
			if (newinput > 1047) {

				if (forward == eepromBuffer.eeppack.dir_reversed) {
					if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
						forward = !eepromBuffer.eeppack.dir_reversed;
						zero_crosses = 0;
						old_routine = 1;
						maskPhaseInterrupts();
						brushed_direction_set = 0;
					} else {
						newinput = 0;
					}
				}
				adjusted_input = ((newinput - 1048) * 2 + 47) - reversing_dead_band;
			}
			if (newinput <= 1047 && newinput > 47) {
				if (forward == (!eepromBuffer.eeppack.dir_reversed)) {
					if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
						zero_crosses = 0;
						old_routine = 1;
						forward = eepromBuffer.eeppack.dir_reversed;
						maskPhaseInterrupts();
						brushed_direction_set = 0;
					} else {
						newinput = 0;
					}
				}
				adjusted_input = ((newinput - 48) * 2 + 47) - reversing_dead_band;
			}
			if (newinput < 48) {
				adjusted_input = 0;
				brushed_direction_set = 0;
			}
		}
	} else {
		adjusted_input = newinput;
	}
#ifndef BRUSHED_MODE
    if ((bemf_timeout_happened > bemf_timeout) && eepromBuffer.eeppack.stuck_rotor_protection) {
        allOff();
        maskPhaseInterrupts();
        input = 0;
        bemf_timeout_happened = 102;
#ifdef USE_RGB_LED
        GPIOB->BRR = LL_GPIO_PIN_8; // on red
        GPIOB->BSRR = LL_GPIO_PIN_5; //
        GPIOB->BSRR = LL_GPIO_PIN_3;
#endif
    } else {
#ifdef FIXED_DUTY_MODE
        input = FIXED_DUTY_MODE_POWER * 20 + 47;
#else
		if (eepromBuffer.eeppack.use_sine_start) {
			if (adjusted_input < 30) { // dead band ?
				input = 0;
			}
            if (adjusted_input > 30 && adjusted_input < (eepromBuffer.eeppack.sine_mode_changeover_thottle_level * 20)) {
                input = map(adjusted_input, 30,
                    (eepromBuffer.eeppack.sine_mode_changeover_thottle_level * 20), 47, 160);
            }
            if (adjusted_input >= (eepromBuffer.eeppack.sine_mode_changeover_thottle_level * 20)) {
                input = map(adjusted_input, (eepromBuffer.eeppack.sine_mode_changeover_thottle_level * 20),
                    2047, 160, 2047);
			}
		} else {
			if (use_speed_control_loop) {
				if (drive_by_rpm) {
					target_e_com_time = 60000000
							/ map(adjusted_input, 47, 2047, MINIMUM_RPM_SPEED_CONTROL,MAXIMUM_RPM_SPEED_CONTROL)
							/ (eepromBuffer.eeppack.motor_poles / 2);
					if (adjusted_input < 47) { // dead band ?
						input = 0;
						speedPid.error = 0;
						input_override = 0;
					} else {
						input = (uint16_t) (input_override / 10000); // speed control pid override
						if (input > 2047) {
							input = 2047;
						}
						if (input < 48) {
							input = 48;
						}
					}
				} else {

                    input = (uint16_t)(input_override / 10000); // speed control pid override
                    if (input > 2047) {
                        input = 2047;
                    }
                    if (input < 48) {
                        input = 48;
                    }
				}
			} else {
				input = adjusted_input;
			}
		}
#endif
	}
#endif
#ifndef BRUSHED_MODE
	if (!stepper_sine && armed) {
		if ((input >= 47 + (80 * eepromBuffer.eeppack.use_sine_start))) {
			if (running == 0) {
				allOff();
				if (!old_routine) {
					startMotor();
				}
				running = 1;
				last_duty_cycle = min_startup_duty;
			}

			if (eepromBuffer.eeppack.use_sine_start) {
				duty_cycle_setpoint = map(input, 137, 2047, minimum_duty_cycle + 40, 2000);
			} else {
				duty_cycle_setpoint = map(input, 47, 2047, minimum_duty_cycle,2000);
			}

			if (!eepromBuffer.eeppack.rc_car_reverse) {
				prop_brake_active = 0;
			}
		}

		if (input < 47 + (80 * eepromBuffer.eeppack.use_sine_start)) {
			if (play_tone_flag != 0) {
				switch (play_tone_flag) {
				case 1:
					playDefaultTone();
					break;
				case 2:
					playChangedTone();
					break;
				case 3:
					playBeaconTune3();
					break;
				case 4:
					playInputTune2();
					break;
				case 5:
					playDefaultTone();
					break;
				}
				play_tone_flag = 0;
			}

			if (!eepromBuffer.eeppack.comp_pwm) {
				duty_cycle_setpoint = 0;
				if (!running) {
					old_routine = 1;
					zero_crosses = 0;
					if (eepromBuffer.eeppack.brake_on_stop) {
						fullBrake();
					} else {
						if (!prop_brake_active) {
							allOff();
						}
					}
				}
				if (eepromBuffer.eeppack.rc_car_reverse && prop_brake_active) {
#ifndef PWM_ENABLE_BRIDGE
					prop_brake_duty_cycle = (getAbsDif(1000, newinput) + 1000);
					if (prop_brake_duty_cycle >= (PWM_MAX_ARR - 1)) {
						fullBrake();
					} else {
						proportionalBrake();
					}
#endif
				}
			} else {
				if (!running) {
					old_routine = 1;
					zero_crosses = 0;
					bad_count = 0;
					if (eepromBuffer.eeppack.brake_on_stop) {
						if (!eepromBuffer.eeppack.use_sine_start) {
#ifndef PWM_ENABLE_BRIDGE
							prop_brake_duty_cycle = (1980) + eepromBuffer.eeppack.drag_brake_strength * 2;
							proportionalBrake();
							prop_brake_active = 1;
#else
                            // todo add proportional braking for pwm/enable style bridge.
#endif
						}
					} else {
						allOff();
					}
					duty_cycle_setpoint = 0;
				}

				phase_A_position = ((step) * 60) + enter_sine_angle;
				if (phase_A_position > 359) {
					phase_A_position -= 360;
				}
				phase_B_position = phase_A_position + 119;
				if (phase_B_position > 359) {
					phase_B_position -= 360;
				}
				phase_C_position = phase_A_position + 239;
				if (phase_C_position > 359) {
					phase_C_position -= 360;
				}

				if (eepromBuffer.eeppack.use_sine_start == 1) {
					stepper_sine = 1;
				}
				duty_cycle_setpoint = 0;
			}
		}
		if (!prop_brake_active) {
			if (input >= 47 && (zero_crosses < (uint32_t)((uint32_t)30 >> eepromBuffer.eeppack.stall_protection))) {
				if (duty_cycle_setpoint < min_startup_duty) {
					duty_cycle_setpoint = min_startup_duty;
				}
				if (duty_cycle_setpoint > startup_max_duty_cycle) {
					duty_cycle_setpoint = startup_max_duty_cycle;
				}
			}

			if (duty_cycle_setpoint > duty_cycle_maximum) {
				duty_cycle_setpoint = duty_cycle_maximum;
			}
			
			if (use_current_limit) {
				if (duty_cycle_setpoint > use_current_limit_adjust) {
					duty_cycle_setpoint = use_current_limit_adjust;
				}
			}

			if (stall_protection_adjust > 0 && input > 47) {
				duty_cycle_setpoint = duty_cycle_setpoint + (stall_protection_adjust/10000);
			}
		}
	}
#endif
}

void resetInputCaptureTimer(void) {
	register PWM_TypeDef *PWMB = PWMB_ADDRESS;
	PWMB->PSCRH = 0x00;
	PWMB->PSCRL = 0x00;
	PWMB->IER = 0x04;
	PWMB->EGR = 0x01; 
	receiveDmaBuffer = servoDmaBuffer;
}

// uint8_t getInputPinState(void) {
// 	return INPUT_PIN;
// }

void setInputPolarityRising(void) {

}

// void setInputPullDown(void) {
// 	gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IPD);
// }

// void setInputPullUp(void) {
// 	gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IPU);
// }

void enableHalfTransferInt(void) {
	
}  

// void setInputPullNone(void) {
// 	gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IN_FLOATING);
// }
