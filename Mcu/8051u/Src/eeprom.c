/*
 * eeprom.c
 *
 *  Created on: 2024年12月2日
 *      Author: 17263
 */
#include "sounds.h"
#include "peripherals.h"
#include "eeprom.h"
#include "input.h"
#include "string.h"
#include "functions.h"
#include "ushot.h"
#include "version.h"

#define page_size 512                   // 512 bytes for STC8051U

uint32_t eeprom_address = EEPROM_START_ADD;
EEprom_t eepromBuffer;
// Flags
bool LOW_VOLTAGE_CUTOFF = 0; // 低电压保护
bool USE_HALL_SENSOR; // 有感模式
bool use_current_limit = 0; // 电流限制
bool low_rpm_throttle_limit= 1;

// Counters and Levels
uint8_t dead_time_override = DEAD_TIME; // 死区时间
uint8_t eeprom_layout_version = EEPROM_VERSION;

// Timing and Duty Cycles
uint16_t PWM_MAX_ARR = PWM_AUTORELOAD;
uint16_t pwm_arr = PWM_AUTORELOAD;
uint16_t stall_protect_minimum_duty = DEAD_TIME;
uint16_t motor_kv = 2000;
uint16_t low_cell_volt_cutoff = 330; // 低电压保护电压
uint16_t throttle_max_at_low_rpm = 400;
uint16_t throttle_max_at_high_rpm = 2000;
uint16_t low_rpm_level = 20; // 低转速等级，单位千转每分钟
uint16_t high_rpm_level = 70; // 高转速等级
uint16_t reverse_speed_threshold;

int32_t min_startup_duty = 120;
int32_t startup_max_duty_cycle = 200;
int32_t minimum_duty_cycle = DEAD_TIME;
int32_t duty_cycle_maximum = 2000;

// Servo Settings
int16_t servo_low_threshold = 1012; // 舵机低阈值
int16_t servo_high_threshold = 2000; // 舵机高阈值
int16_t servo_neutral = 1500; // 舵机中性
uint8_t servo_dead_band = 100; // 舵机死区

bool save_flash_nolib(uint8_t *dat, uint32_t length, uint32_t add) {
	static uint32_t i;

	IAP_ENABLE();
	// unlock flash
	// erase page if address even divisable by 512
	if ((add % page_size) == 0) {
		CMD_FAIL = 0;

		IAP_ERASE();

		IAP_ADDRE = (uint8_t) (add >> 16);
		IAP_ADDRH = (uint8_t) (add >> 8);
		IAP_ADDRL = (uint8_t) add;

		IAP_TRIG = 0x5A;
		IAP_TRIG = 0xA5;
		_nop_();
		_nop_();
		_nop_();
		_nop_();

		while (CMD_FAIL)
			;
	}

	IAP_WRITE();  			//宏调用, 送字节写命令

	for (i = 0; i < length; i++) {
		CMD_FAIL = 0;

		IAP_ADDRE = (uint8_t) ((add + i) >> 16);
		IAP_ADDRH = (uint8_t) ((add + i) >> 8);
		IAP_ADDRL = (uint8_t) (add + i);
		IAP_DATA = *(dat + i);

		IAP_TRIG = 0x5A;
		IAP_TRIG = 0xA5;
		_nop_();
		_nop_();
		_nop_();
		_nop_();

		while (CMD_FAIL)
			;
	}

	IAP_DISABLE();                      //关闭IAP

	return memcmp(dat, (uint8_t volatile far*)add, length) == 0;
}

void read_flash_bin(uint8_t *dat, uint32_t add, uint32_t out_buff_len) {
	memcpy(dat, (uint8_t volatile far*)add, out_buff_len);
}


void loadEEpromSettings(void) {
	read_flash_bin(eepromBuffer.buffer, eeprom_address, sizeof(eepromBuffer.buffer));

	if (eepromBuffer.eeppack.advance_level > 3) {
        eepromBuffer.eeppack.advance_level = 2;
    }

	if (eepromBuffer.eeppack.pwm_frequency < 49 && eepromBuffer.eeppack.pwm_frequency > 7) {
		if (eepromBuffer.eeppack.pwm_frequency < 49 && eepromBuffer.eeppack.pwm_frequency > 23) {
			PWM_MAX_ARR = map(eepromBuffer.eeppack.pwm_frequency, 24, 48, PWM_AUTORELOAD,PWM_AUTORELOAD / 2);
		}
		if (eepromBuffer.eeppack.pwm_frequency < 24 && eepromBuffer.eeppack.pwm_frequency > 11) {
			PWM_MAX_ARR = map(eepromBuffer.eeppack.pwm_frequency, 12, 24, PWM_AUTORELOAD * 2,PWM_AUTORELOAD);
		}
		if (eepromBuffer.eeppack.pwm_frequency < 12 && eepromBuffer.eeppack.pwm_frequency > 7) {
			PWM_MAX_ARR = map(eepromBuffer.eeppack.pwm_frequency, 7, 16, PWM_AUTORELOAD * 3,PWM_AUTORELOAD / 2 * 3);
		}
		SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
	} else {
		pwm_arr = PWM_AUTORELOAD;
		SET_AUTO_RELOAD_PWM(pwm_arr);
	}

	if (eepromBuffer.eeppack.startup_power < 151 && eepromBuffer.eeppack.startup_power > 49) {
		if(!eepromBuffer.eeppack.comp_pwm){
			min_startup_duty = (eepromBuffer.eeppack.startup_power) * 2;
			minimum_duty_cycle = (eepromBuffer.eeppack.startup_power / 2);
			stall_protect_minimum_duty = minimum_duty_cycle + 10;			
		}else{
			min_startup_duty = (eepromBuffer.eeppack.startup_power) *2 ;
            minimum_duty_cycle = (eepromBuffer.eeppack.startup_power / 2);
            stall_protect_minimum_duty = minimum_duty_cycle + 10;
		}

	} else {
		min_startup_duty = 150;
		minimum_duty_cycle = (min_startup_duty / 2) + 10;
	}

	motor_kv = (eepromBuffer.eeppack.motor_kv * 40) + 20;
#ifdef THREE_CELL_MAX
		motor_kv =  motor_kv / 2;
#endif
	setVolume(2);

    if (eepromBuffer.eeppack.eeprom_version > 0) { // these commands weren't introduced until eeprom version 1.
#ifdef CUSTOM_RAMP

#else
        if (eepromBuffer.eeppack.beep_volume > 11) {
            setVolume(5);
        } else {
            setVolume(eepromBuffer.eeppack.beep_volume);
        }
#endif
        servo_low_threshold = (eepromBuffer.eeppack.servo.low_threshold * 2) + 750; // anything below this point considered 0
        servo_high_threshold = (eepromBuffer.eeppack.servo.high_threshold * 2) + 1750;
        ; // anything above this point considered 2000 (max)
        servo_neutral = (eepromBuffer.eeppack.servo.neutral) + 1374;
        servo_dead_band = eepromBuffer.eeppack.servo.dead_band;

        if (eepromBuffer.eeppack.low_voltage_cut_off == 0x01) {
            LOW_VOLTAGE_CUTOFF = 1;
        } else {
            LOW_VOLTAGE_CUTOFF = 0;
        }

        low_cell_volt_cutoff = eepromBuffer.eeppack.low_cell_volt_cutoff + 250; // 2.5 to 3.5 volts per cell range

#ifndef HAS_HALL_SENSORS
        eepromBuffer.eeppack.use_hall_sensors = 0;
#endif

        if (eepromBuffer.eeppack.sine_mode_changeover_thottle_level < 5 || eepromBuffer.eeppack.sine_mode_changeover_thottle_level > 25) { // sine mode changeover 5-25 percent throttle
            eepromBuffer.eeppack.sine_mode_changeover_thottle_level = 5;
        }
        if (eepromBuffer.eeppack.drag_brake_strength == 0 || eepromBuffer.eeppack.drag_brake_strength > 10) { // drag brake 1-10
            eepromBuffer.eeppack.drag_brake_strength = 10;
        }

        if (eepromBuffer.eeppack.driving_brake_strength == 0 || eepromBuffer.eeppack.driving_brake_strength > 9) { // motor brake 1-9
            eepromBuffer.eeppack.driving_brake_strength = 10;
        }

        if(eepromBuffer.eeppack.driving_brake_strength < 10){
            dead_time_override = DEAD_TIME + (150 - (eepromBuffer.eeppack.driving_brake_strength * 10));
            if (dead_time_override > 200) {
                dead_time_override = 200;
            }
			min_startup_duty = min_startup_duty + dead_time_override;
			minimum_duty_cycle = minimum_duty_cycle + dead_time_override;
			throttle_max_at_low_rpm = throttle_max_at_low_rpm + dead_time_override;
			startup_max_duty_cycle = startup_max_duty_cycle + dead_time_override;
#ifdef STMICRO
        	TIM1->BDTR |= dead_time_override;
#endif
#ifdef ARTERY
        	TMR1->brk |= dead_time_override;
#endif
#ifdef GIGADEVICES
        	TIMER_CCHP(TIMER0) |= dead_time_override;
#endif
#ifdef WCH
            TIM1->BDTR |= dead_time_override;
#endif
#ifdef STC
			PWMA_DTR |= dead_time_override;
#endif
        }
        if (eepromBuffer.eeppack.limits.temperature < 70 || eepromBuffer.eeppack.limits.temperature > 140) {
            eepromBuffer.eeppack.limits.temperature = 255;
        }

        if (eepromBuffer.eeppack.limits.current > 0 && eepromBuffer.eeppack.limits.current < 100) {
            use_current_limit = 1;
        }
        
        if (eepromBuffer.eeppack.sine_mode_power == 0 || eepromBuffer.eeppack.sine_mode_power > 10) {
            eepromBuffer.eeppack.sine_mode_power = 5;
        }

        // unsinged int cant be less than 0
        if (eepromBuffer.eeppack.input_type < 10) {
            switch (eepromBuffer.eeppack.input_type) {
            case AUTO_IN:
                ushot = 0;
                servoPwm = 0;
                EDT_ARMED = 1;
                break;
            case DSHOT_IN:
                ushot = 1;
                EDT_ARMED = 1;
                break;
            case SERVO_IN:
                servoPwm = 1;
                break;
            case SERIAL_IN:
                break;
            case EDTARM_IN:
                EDT_ARM_ENABLE = 1;
                EDT_ARMED = 0;
                ushot = 1;
                break;
            };
        } else {
            ushot = 0;
            servoPwm = 0;
            EDT_ARMED = 1;
        }
        if (motor_kv < 300) {
            low_rpm_throttle_limit = 0;
        }
        low_rpm_level = motor_kv / 100 / (32 / eepromBuffer.eeppack.motor_poles);

        high_rpm_level = motor_kv / 12 / (32 / eepromBuffer.eeppack.motor_poles);				
    }
	reverse_speed_threshold = map(motor_kv, 300, 3000, 1000, 500);
}

void saveEEpromSettings(void) {
    eepromBuffer.eeppack.eeprom_version = eeprom_layout_version;
    save_flash_nolib(eepromBuffer.buffer, sizeof(eepromBuffer.buffer), eeprom_address);
}


