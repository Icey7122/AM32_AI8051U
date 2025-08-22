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
uint8_t high_calibration_counts = 0; 			// high calibration counter
uint8_t low_calibration_counter = 0; 			// low calibration counter
int16_t last_high_threshold = 0; 				// last high threshold
int16_t servorawinput; 					// raw servo input
int16_t max_servo_deviation = 250;	// max deviation between two servo pulses


void UN_PWMB_Init(void) {
	register PWM_TypeDef *PWMB = PWMB_ADDRESS;

	gpio_mode_set(INPUT_PIN_PORT,INPUT_MODE_PIN,GPIO_Mode_IN_FLOATING);
	
	PWMB_PS = INPUT_PS;
	PWMB->PSCRH = 0x00;
	PWMB->PSCRL = CPU_FREQUENCY_MHZ - 1;
	PWMB->CCER1 = 0x00;
	PWMB->CCER2 = 0x00;


	PWMB->CCMR1 = 0x01;
	PWMB->CCMR2 = 0x02;
	PWMB->CCER1 = 0x31;

	PWMB->ARRH = 0xFF;
	PWMB->ARRL = 0xFF;
	PWMB->IER = 0x04;

	PPWMBH = 1;PPWMB = 0;

	PWMB->EGR = 0x01;
	PWMB->CR1 = 0x01;
}

void servoDmaBuffer(void) {
	register PWM_TypeDef *PWMB = PWMB_ADDRESS;
	PWMB->PSCRH = ic_timer_prescaler >> 8;
	PWMB->PSCRL = ic_timer_prescaler;
	PWMB->EGR = 0x01; 
}

void computeServoInput(void) {
	if (((dma_buffer[1] - dma_buffer[0]) > 800) && ((dma_buffer[1] - dma_buffer[0]) < 2200)) {
		signaltimeout = 0;
		if (calibration_required) {
			if (!high_calibration_set) {
				if (high_calibration_counts == 0) {
					last_high_threshold = dma_buffer[1] - dma_buffer[0];
				}
				high_calibration_counts++;
				if (getAbsDif(last_high_threshold, servo_high_threshold) > 50) {
					calibration_required = 0;
				} else {
					servo_high_threshold = ((7 * servo_high_threshold + (dma_buffer[1] - dma_buffer[0])) >> 3);
					if (high_calibration_counts > 50) {
						servo_high_threshold = servo_high_threshold - 25;
						eepromBuffer.eeppack.servo.high_threshold = (servo_high_threshold - 1750) / 2;
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
					eepromBuffer.eeppack.servo.low_threshold = (servo_low_threshold - 750) / 2;
					calibration_required = 0;
					saveEEpromSettings();
					low_calibration_counter = 0;
					playChangedTone();
				}
			}
			signaltimeout = 0;
		} else {
			if (eepromBuffer.eeppack.bi_direction) {
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
			signaltimeout = 0;
		}
	} else {
		zero_input_count = 0; // reset if out of range
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
		receiveDmaBuffer = servoDmaBuffer;
	}
}
