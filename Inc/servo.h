#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "mcu.h"

#define IC_IRQHandler(val)				PWMB_ISR(val) INTERRUPT(PWMB_VECTOR) USING(3)	

extern uint8_t calibration_required; 				// calibration required
extern uint8_t high_calibration_set; 				// high calibration set
extern uint8_t enter_calibration_counter; 			// enter calibration count	 
extern uint8_t high_calibration_counts; 			// high calibration counter
extern uint8_t low_calibration_counter; 			// low calibration counter
extern int16_t last_high_threshold; 				// last high threshold
extern int16_t servorawinput; 					// raw servo input
extern int16_t max_servo_deviation;	// max deviation between two servo pulses 

extern void UN_PWMB_Init(void);
extern void servoDmaBuffer(void);
extern void computeServoInput(void);
extern void checkServo(void);

#endif // !SIGNAL_H