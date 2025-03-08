#include "main.h"
#include "input.h"
#include "targets.h"
#include "functions.h"
#include "eeprom.h"
#include "sounds.h"
#include "phaseouts.h"
#include "commutate.h"


uint8_t servoPwm = 0;
uint8_t dshot = 0;
uint8_t ic_timer_prescaler = 0;
// uint8_t out_put = 0;

//detectInput
uint16_t xdata dma_buffer[64] = { 0 };
uint8_t buffersize = 32;
uint16_t smallestnumber = 20000;
uint16_t average_signal_pulse;
//input and input process
uint8_t input_ready = 0;
uint8_t inputSet = 0;					//输入设置
int16_t newinput;
int16_t adjusted_input;
int16_t input;
int16_t last_input;
int16_t input_override;
int32_t duty_cycle;
int32_t adjusted_duty_cycle;
int32_t last_duty_cycle;
uint8_t max_duty_cycle_change;
int32_t prop_brake_duty_cycle;
uint8_t brushed_direction_set = 0;
uint8_t return_to_center = 0;
uint16_t zero_input_counter = 0;

void receiveDshotDma(void) {
	// out_put = 0;

	PWMB_PSCRH = 0x00;
	PWMB_PSCRL = ic_timer_prescaler;
	PWMB_ARRH = 0xFF;
	PWMB_ARRL = 0xFF;

	PWMB_CCER1 = 0x01;

	PWMB_EGR = 0x01; 
}

void transfercomplete(void) {

	if (inputSet == 0) {
		detectInput();
		receiveDshotDma();
		return;
	}
	if (inputSet == 1) {

		// if (dshot_telemetry) {
		// 	if (out_put) {
		// 	    make_dshot_package(e_com_time);
		// 	    computeDshotDMA();
		// 	    receiveDshotDma();
		// 	    return;
		// 	} else {
		// 	    sendDshotDma();
		// 	    return;
		// 	}
		// } else {

			// if (dshot == 1) {
			//     computeDshotDMA();
			//     receiveDshotDma();
			// }
			if (servoPwm == 1) {
				if (getInputPinState()) {
					buffersize = 3;
				} else {
					buffersize = 2;
					computeServoInput();
				}
				receiveDshotDma();
			}
		// }
		if (!armed) {
			// if (dshot && (average_count < 8) && (zero_input_counter > 5)) {
			// 	average_count++;
			// 	average_packet_length = average_packet_length
			// 			+ (dma_buffer[31] - dma_buffer[0]);
			// 	if (average_count == 8) {
			// 		dshot_frametime_high = (average_packet_length >> 3)
			// 				+ (average_packet_length >> 7);
			// 		dshot_frametime_low = (average_packet_length >> 3)
			// 				- (average_packet_length >> 7);
			// 	}
			// }
			if (adjusted_input == 0 && calibration_required == 0) { // note this in input..not newinput so it
																	// will be adjusted be main loop
				zero_input_counter++;
			} else {
				zero_input_counter = 0;
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

	if (dshot == 1) {
	    // checkDshot();
	}
	if (servoPwm == 1) {
		checkServo();
	}

	if (!dshot && !servoPwm) {
		// checkDshot();
		checkServo();
	}
}

void setInput(void)	{
	if (BI_DIRECTION) {
		if (dshot == 0) {
			if (RC_CAR_REVERSE) {
				if (newinput > (1000 + (servo_dead_band << 1))) {
					if (forward == DIR_REVERSED) {
						adjusted_input = 0;
						//               if (running) {
						prop_brake_active = 1;
						if (return_to_center) {
							forward = 1 - DIR_REVERSED;
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
					if (forward == (1 - DIR_REVERSED)) {
						adjusted_input = 0;
						prop_brake_active = 1;
						if (return_to_center) {
							forward = DIR_REVERSED;
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
					if (forward == DIR_REVERSED) {
						if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
							forward = 1 - DIR_REVERSED;
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
					if (forward == (1 - DIR_REVERSED)) {
						if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
							zero_crosses = 0;
							old_routine = 1;
							forward = DIR_REVERSED;
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

		if (dshot) {
			if (newinput > 1047) {

				if (forward == DIR_REVERSED) {
					if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
						forward = 1 - DIR_REVERSED;
						zero_crosses = 0;
						old_routine = 1;
						maskPhaseInterrupts();
						brushed_direction_set = 0;
					} else {
						newinput = 0;
					}
				}
				adjusted_input = ((newinput - 1048) * 2 + 47) - 1;
			}
			if (newinput <= 1047 && newinput > 47) {
				if (forward == (1 - DIR_REVERSED)) {
					if (((commutation_interval > reverse_speed_threshold) && (duty_cycle < 200)) || stepper_sine) {
						zero_crosses = 0;
						old_routine = 1;
						forward = DIR_REVERSED;
						maskPhaseInterrupts();
						brushed_direction_set = 0;
					} else {
						newinput = 0;
					}
				}
				adjusted_input = ((newinput - 48) * 2 + 47) - 1;
			}
			if (newinput < 48) {
				adjusted_input = 0;
				brushed_direction_set = 0;
			}
		}
	} else {
		adjusted_input = newinput;
	}

	if ((bemf_timeout_happened > bemf_timeout) && STUCK_ROTOR_PROTECTION) {
		allOff();
		maskPhaseInterrupts();
		input = 0;
		bemf_timeout_happened = 102;
	} else {

		if (USE_SIN_START) {
			if (adjusted_input < 30) { // dead band ?
				input = 0;
			}
			if (adjusted_input > 30 && adjusted_input < (sine_mode_changeover_thottle_level * 20)) {
				input = map(adjusted_input, 30,(sine_mode_changeover_thottle_level * 20), 47, 160);
			}
			if (adjusted_input >= (sine_mode_changeover_thottle_level * 20)) {
				input = map(adjusted_input,(sine_mode_changeover_thottle_level * 20), 2047, 160, 2047);
			}
		} else {
			if (use_speed_control_loop) {
				if (drive_by_rpm) {
					target_e_com_time = 60000000
							/ map(adjusted_input, 47, 2047, MINIMUM_RPM_SPEED_CONTROL,MAXIMUM_RPM_SPEED_CONTROL)
							/ (MOTOR_POLES / 2);
					if (adjusted_input < 47) { // dead band ?
						input = 0;
						speedPid.error = 0;
						input_override = 0;
					} else {
						input = (uint16_t) input_override; // speed control pid override
						if (input_override > 2047) {
							input = 2047;
						}
						if (input_override < 48) {
							input = 48;
						}
					}
				} else {

					input = input_override; // speed control pid override
					if (input_override > 2047) {
						input = 2047;
					}
					if (input_override < 48) {
						input = 48;
					}
				}
			} else {
				input = adjusted_input;
			}
		}
	}

	if (!stepper_sine) {
		if ((input >= 47 + (80 * USE_SIN_START)) && armed) {
			if (running == 0) {
				allOff();
				if (!old_routine) {
					startMotor();
				}
				running = 1;
				last_duty_cycle = min_startup_duty_cycle;
			}

			if (USE_SIN_START) {
				duty_cycle = map(input, 137, 2047, minimum_duty_cycle + 40, 2000);
			} else {
				duty_cycle = map(input, 47, 2047, minimum_duty_cycle,2000);
			}

			if (!RC_CAR_REVERSE) {
				prop_brake_active = 0;
			}
		}

		if (input < 47 + (80 * USE_SIN_START)) {
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

			if (!COMP_PWM) {
				duty_cycle = 0;
				if (!running) {
					old_routine = 1;
					zero_crosses = 0;
					if (BRAKE_ON_STOP) {
						fullBrake();
					} else {
						if (!prop_brake_active) {
							allOff();
						}
					}
				}
				if (RC_CAR_REVERSE && prop_brake_active) {

					prop_brake_duty_cycle = (getAbsDif(1000, newinput) + 1000);
					if (prop_brake_duty_cycle >= (PWM_MAX_ARR - 1)) {
						fullBrake();
					} else {
						proportionalBrake();
					}

				}
			} else {
				if (!running) {
					old_routine = 1;
					zero_crosses = 0;
					bad_count = 0;
					if (BRAKE_ON_STOP) {
						if (!USE_SIN_START) {
							prop_brake_duty_cycle = (1980) + DRAG_BRAKE_STRENGTH * 2;
							proportionalBrake();
							prop_brake_active = 1;
						}
					} else {
						allOff();
					}
					duty_cycle = 0;
				}

				phase_A_position = ((step) * 60) + 180;
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

				if (USE_SIN_START == 1) {
					stepper_sine = 1;
				}
				duty_cycle = 0;
			}
		}
		if (!prop_brake_active) {
			if (input >= 47 && (zero_crosses < (30U >> STALL_PROTECTION))) {
				if (duty_cycle < min_startup_duty_cycle) {
					duty_cycle = min_startup_duty_cycle;
				}
				if (duty_cycle > max_startup_duty_cycle) {
					duty_cycle = max_startup_duty_cycle;
				}
			}

			if (duty_cycle > maximum_duty_cycle) {
				duty_cycle = maximum_duty_cycle;
			}
			
			if (USE_CURRENT_LIMIT) {
				if (duty_cycle > current_limit_adjust) {
					duty_cycle = current_limit_adjust;
				}
			}

			if (stall_protection_adjust > 0 && input > 47) {
				duty_cycle = duty_cycle + (int16_t) stall_protection_adjust;
			}
		}
	}
}

void resetInputCaptureTimer(void) {
	PWMB_PSCRH = 0x00;
	PWMB_PSCRL = 0x00;
	PWMB_ARRH = 0xFF;
	PWMB_ARRL = 0xFF;

	PWMB_CCER1 = 0x01;

	PWMB_EGR = 0x01; 
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
