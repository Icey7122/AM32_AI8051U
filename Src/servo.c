#include "main.h"
#include "eeprom.h"
#include "sounds.h"
#include "targets.h"
#include "servo.h"

#include "functions.h"
#include "input.h"

uint8_t calibration_required; 				// calibration required
uint8_t high_calibration_set = 0; 				// high calibration set
uint8_t enter_calibration_counter = 0; 			// enter calibration count	
uint8_t high_calibration_counter = 0; 			// high calibration counter
uint8_t low_calibration_counter = 0; 			// low calibration counter
int16_t last_high_threshold = 0; 				// last high threshold
int16_t servorawinput; 					// raw servo input
int16_t max_servo_deviation = 250;	// max deviation between two servo pulses

void computeServoInput(void) {
	if (((dma_buffer[1] - dma_buffer[0]) > 800) && ((dma_buffer[1] - dma_buffer[0]) < 2200)) {
		signaltimecounter = 0;
		if (calibration_required) {
			if (!high_calibration_set) {
				if (high_calibration_counter == 0) {
					last_high_threshold = dma_buffer[1] - dma_buffer[0];
				}
				high_calibration_counter++;
				if (getAbsDif(last_high_threshold, servo_high_threshold) > 50) {
					calibration_required = 0;
				} else {
					servo_high_threshold = ((7 * servo_high_threshold + (dma_buffer[1] - dma_buffer[0])) >> 3);
					if (high_calibration_counter > 50) {
						servo_high_threshold = servo_high_threshold - 25;
						eepromBuffer[33] = (servo_high_threshold - 1750) / 2;
						high_calibration_set = 1;
						playDefaultTone();
					}
				}
				last_high_threshold = servo_high_threshold;
			}
			if (high_calibration_set) {
				if (dma_buffer[1] - dma_buffer[0] < 1250) {
					low_calibration_counter++;
					servo_low_threshold = ((7 * servo_low_threshold + (dma_buffer[1] - dma_buffer[0])) >> 3);
				}
				if (low_calibration_counter > 75) {
					servo_low_threshold = servo_low_threshold + 25;
					eepromBuffer[32] = (servo_low_threshold - 750) / 2;
					calibration_required = 0;
					saveEEpromSettings();
					low_calibration_counter = 0;
					playChangedTone();
				}
			}
			signaltimecounter = 0;
		} else {
			if (BI_DIRECTION) {
				if (dma_buffer[1] - dma_buffer[0] <= servo_neutral) {
					servorawinput = map((dma_buffer[1] - dma_buffer[0]),servo_low_threshold, servo_neutral, 0, 1000);
				} else {
					servorawinput = map((dma_buffer[1] - dma_buffer[0]),servo_neutral + 1, servo_high_threshold, 1001,2000);
				}
			} else {
				servorawinput = map((dma_buffer[1] - dma_buffer[0]),servo_low_threshold, servo_high_threshold, 47, 2047);
				if (servorawinput <= 48) {
					servorawinput = 0;
				}
			}
			signaltimecounter = 0;
		}
	} else {
		zero_input_counter = 0; // reset if out of range
	}

	if (servorawinput - newinput > max_servo_deviation) {
		newinput += max_servo_deviation;
	} else if (newinput - servorawinput > max_servo_deviation) {
		newinput -= max_servo_deviation;
	} else {
		newinput = servorawinput;
	}

}

void checkServo(void) {
	if (smallestnumber > 200 && smallestnumber < 20000) {
		servoPwm = 1;
		ic_timer_prescaler = CPU_FREQUENCY_MHZ - 1;
		buffersize = 2;
		inputSet = 1;
	}
}
