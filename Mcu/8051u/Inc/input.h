#ifndef INPUT_H
#define INPUT_H

#include "mcu.h"
#include "servo.h"
#include "ushot.h"

typedef void(receiveFUNC_t)(void);

extern bool servoPwm;
extern bool ushot;
extern bool inputSet; // 输入设置
extern bool brushed_direction_set;
extern bool return_to_center;
extern uint8_t play_tone_flag;

extern uint16_t ic_timer_prescaler;
extern uint8_t buffersize;
extern uint16_t xdata dma_buffer[64];
extern uint16_t smallestnumber;
extern uint16_t average_signal_pulse;
extern uint16_t zero_input_counter;

extern int16_t newinput;
extern int16_t adjusted_input;
extern int16_t input;
extern int16_t input_override;

extern int32_t duty_cycle;
extern int32_t adjusted_duty_cycle;
extern int32_t last_duty_cycle;
extern int32_t prop_brake_duty_cycle;
extern uint8_t max_duty_cycle_change;

extern receiveFUNC_t *receiveDmaBuffer;

extern void transfercomplete(void);
extern void setInput(void);
extern void detectInput(void);
extern void resetInputCaptureTimer(void);
#define getInputPinState()  (INPUT_PIN)
extern void setInputPolarityRising(void);
#define setInputPullDown()  gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IPD)
#define setInputPullUp()  gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IPU)
extern void enableHalfTransferInt(void);
#define setInputPullNone()  gpio_mode_set(INPUT_PIN_PORT, INPUT_MODE_PIN, GPIO_Mode_IN_FLOATING)

#endif // !INPUT_H