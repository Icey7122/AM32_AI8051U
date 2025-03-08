#ifndef INPUT_H
#define INPUT_H

#include "mcu.h"
#include "servo.h"

extern uint8_t servoPwm;
extern uint8_t dshot;
extern uint8_t ic_timer_prescaler;

extern uint16_t xdata dma_buffer[64];
extern uint8_t buffersize;
extern uint16_t smallestnumber;
extern uint16_t average_signal_pulse;

extern uint8_t input_ready;
extern uint8_t inputSet;					
extern int16_t newinput;
extern int16_t adjusted_input;
extern int16_t input;
extern int16_t input_override;
extern int32_t duty_cycle;
extern int32_t adjusted_duty_cycle;
extern int32_t last_duty_cycle;
extern uint8_t max_duty_cycle_change;
extern int32_t prop_brake_duty_cycle;
extern uint8_t brushed_direction_set;
extern uint8_t return_to_center;
extern uint16_t zero_input_counter;

extern void receiveDshotDma(void);
extern void transfercomplete(void);
extern void setInput(void);
extern void detectInput(void);
extern void resetInputCaptureTimer(void);
// uint8_t getInputPinState(void);
#define getInputPinState()  (INPUT_PIN)
extern void setInputPolarityRising(void);
// extern void setInputPullDown(void);
// extern void setInputPullUp(void);
#define setInputPullDown()  gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IPD)
#define setInputPullUp()  gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IPU)
extern void enableHalfTransferInt(void);
// extern void setInputPullNone(void); 
#define setInputPullNone()  gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IN_FLOATING)

#endif // !INPUT_H