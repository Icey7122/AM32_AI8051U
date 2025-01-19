#include "sounds.h"
#include "targets.h"
#include "functions.h"
#include "eeprom.h"
#include "phaseouts.h"
#include "peripherals.h"

uint8_t beep_volume = 5;

void pause(uint16_t ms) {
	SET_DUTY_CYCLE_ALL(0);
	delayMillis(ms);
	SET_DUTY_CYCLE_ALL(beep_volume); // volume of the beep, (duty cycle) don't go
									 // above 25 out of 2000
}

void setVolume(uint8_t volume) {
	if (volume > 11) {
		volume = 11;
	}
	beep_volume = volume * 2; // volume variable from 0 - 11 equates to CCR value of 0-22
}

void setCaptureCompare(void) {
	SET_DUTY_CYCLE_ALL(beep_volume); // volume of the beep, (duty cycle) don't go
									 // above 25 out of 2000
}

void playBJNote(uint16_t freq, uint16_t bduration) { // hz and ms
	uint16_t timerOne_reload = PWM_AUTORELOAD;

	SET_PRESCALER_PWM(10);
	timerOne_reload = CPU_FREQUENCY_MHZ * 100000 / freq;

	SET_AUTO_RELOAD_PWM(timerOne_reload);
	SET_DUTY_CYCLE_ALL(beep_volume * timerOne_reload / PWM_AUTORELOAD); // volume of the beep, (duty cycle) don't
																		// go above 25 out of 2000
	delayMillis(bduration);
}

uint16_t getBlueJayNoteFrequency(uint8_t bjarrayfreq) {
	return 10000000 / (bjarrayfreq * 247 + 4000);
}

void playBlueJayTune(void) {
	uint16_t i;
	uint8_t full_time_count = 0;
	uint16_t duration;
	uint16_t frequency;
	(*comStep[2])();
	// read_flash_bin(blueJayTuneBuffer , eeprom_address + 48 , 128);
	for (i = 52; i < 176; i += 2) {
		RELOAD_WATCHDOG_COUNTER();
		signaltimecounter = 0;

		if (eepromBuffer[i] == 255) {
			full_time_count++;

		} else {
			if (eepromBuffer[i + 1] == 0) {
				duration = full_time_count * 254 + eepromBuffer[i];
				SET_DUTY_CYCLE_ALL(0);
				delayMillis(duration);
			} else {
				frequency = getBlueJayNoteFrequency(eepromBuffer[i + 1]);
				duration = ((full_time_count * 254 + eepromBuffer[i])
						* (100000 / frequency)) / 100;
				playBJNote(frequency, duration);
			}
			full_time_count = 0;
		}
	}
	allOff(); // turn all channels low again
	SET_PRESCALER_PWM(0); // set prescaler back to 0.
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	signaltimecounter = 0;
	RELOAD_WATCHDOG_COUNTER();
}

void playStartupTune(void) {
	uint8_t value = *(unsigned char far*)(eeprom_address + 48);
	__disable_irq();
	if (value != 0xFF) {
		playBlueJayTune();
	} else {
		SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
		setCaptureCompare();
		(*comStep[2])(); // activate a pwm channel
		SET_PRESCALER_PWM(55); // frequency of beep
		delayMillis(200); // duration of beep

		(*comStep[4])();
		SET_PRESCALER_PWM(40); // next beep is higher frequency
		delayMillis(200);

		(*comStep[5])();
		SET_PRESCALER_PWM(25); // higher again..
		delayMillis(200);

		allOff(); // turn all channels low again
		SET_PRESCALER_PWM(0); // set prescaler back to 0.
		signaltimecounter = 0;
	}

	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	__enable_irq();
}

void playBrushedStartupTune(void) {
	__disable_irq();
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	setCaptureCompare();
	(*comStep[0])(); // activate a pwm channel
	SET_PRESCALER_PWM(40); // frequency of beep
	delayMillis(300); // duration of beep
	(*comStep[1])(); // activate a pwm channel
	SET_PRESCALER_PWM(30); // frequency of beep
	delayMillis(300); // duration of beep
	(*comStep[2])();// activate a pwm channel
	SET_PRESCALER_PWM(25); // frequency of beep
	delayMillis(300); // duration of beep
	(*comStep[3])();
	SET_PRESCALER_PWM(20); // higher again..
	delayMillis(300);
	allOff(); // turn all channels low again
	SET_PRESCALER_PWM(0); // set prescaler back to 0.
	signaltimecounter = 0;
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	__enable_irq();
}

void playDuskingTune(void) {
	setCaptureCompare();
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	(*comStep[1])(); // activate a pwm channel
	SET_PRESCALER_PWM(60); // frequency of beep
	delayMillis(200); // duration of beep
	SET_PRESCALER_PWM(55); // next beep is higher frequency
	delayMillis(150);
	SET_PRESCALER_PWM(50); // higher again..
	delayMillis(150);
	SET_PRESCALER_PWM(45); // frequency of beep
	delayMillis(100); // duration of beep
	SET_PRESCALER_PWM(50); // next beep is higher frequency
	delayMillis(100);
	SET_PRESCALER_PWM(55); // higher again..
	delayMillis(100);
	SET_PRESCALER_PWM(25); // higher again..
	delayMillis(200);
	SET_PRESCALER_PWM(55); // higher again..
	delayMillis(150);
	allOff(); // turn all channels low again
	SET_PRESCALER_PWM(0); // set prescaler back to 0.
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
}

void playInputTune2(void) {
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	__disable_irq();
	RELOAD_WATCHDOG_COUNTER();
	SET_PRESCALER_PWM(60);
	setCaptureCompare();
	(*comStep[0])();
	delayMillis(75);
	SET_PRESCALER_PWM(80);
	delayMillis(75);
	SET_PRESCALER_PWM(90);
	RELOAD_WATCHDOG_COUNTER();
	delayMillis(75);
	allOff();
	SET_PRESCALER_PWM(0);
	signaltimecounter = 0;
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	__enable_irq();
}

void playInputTune(void) {
	__disable_irq();
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	RELOAD_WATCHDOG_COUNTER();
	SET_PRESCALER_PWM(80);
	setCaptureCompare();
	(*comStep[2])();
	delayMillis(100);
	SET_PRESCALER_PWM(70);
	delayMillis(100);
	SET_PRESCALER_PWM(40);
	delayMillis(100);
	allOff();
	SET_PRESCALER_PWM(0);
	signaltimecounter = 0;
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	__enable_irq();
}

void playDefaultTone(void) {
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	SET_PRESCALER_PWM(50);
	setCaptureCompare();
	(*comStep[2])();
	delayMillis(150);
	RELOAD_WATCHDOG_COUNTER();
	SET_PRESCALER_PWM(30);
	delayMillis(150);
	allOff();
	SET_PRESCALER_PWM(0);
	signaltimecounter = 0;
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
}

void playChangedTone(void) {
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	SET_PRESCALER_PWM(40);
	setCaptureCompare();
	(*comStep[1])();
	delayMillis(150);
	RELOAD_WATCHDOG_COUNTER();
	SET_PRESCALER_PWM(80);
	delayMillis(150);
	allOff();
	SET_PRESCALER_PWM(0);
	signaltimecounter = 0;
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
}

void playBeaconTune3(void) {
	static int8_t i;
	SET_AUTO_RELOAD_PWM(PWM_AUTORELOAD);
	__disable_irq();
	setCaptureCompare();
	for (i = 119; i > 0; i = i - 2) {
		RELOAD_WATCHDOG_COUNTER();
		(*comStep[i / 20])();
		SET_PRESCALER_PWM(10 + (i / 2));
		delayMillis(10);
	}
	allOff();
	SET_PRESCALER_PWM(0);
	signaltimecounter = 0;
	SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	__enable_irq();
}
