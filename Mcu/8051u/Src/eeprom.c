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

#define page_size 512                   // 512 bytes for STC8051U

uint32_t eeprom_address = EEPROM_START_ADD;
uint8_t xdata eepromBuffer[176] = { 0 };

// Flags
bool BI_DIRECTION = 0; // 双向
bool DIR_REVERSED; // 方向反转
bool COMP_PWM = 1; // 互补PWM
bool VARIABLE_PWM = 1; // 可变PWM
bool STUCK_ROTOR_PROTECTION = 1; // 堵转保护
bool BRAKE_ON_STOP = 0;
bool TLM_ON_INTERVAL = 0; // 遥测开启
bool LOW_VOLTAGE_CUTOFF = 0; // 低电压保护
bool RC_CAR_REVERSE = 0; // RC车模式
bool USE_HALL_SENSOR; // 有感模式
bool USE_CURRENT_LIMIT = 0; // 电流限制
bool AUTO_ADVANCE = 0; // 自动提前
bool LOW_RPM_THROTTLE_LIMIT = 1;
uint8_t USE_SIN_START = 1; // 正弦启动
uint8_t STALL_PROTECTION = 1;

// Counters and Levels
uint8_t ADVANCE_LEVEL = 2; // 提前角度
uint8_t MOTOR_POLES = 14;
uint8_t sine_mode_changeover_thottle_level = 25; // 正弦模式切换油门等级
uint8_t DRAG_BRAKE_STRENGTH = 5; // 拖动制动强度
uint8_t dead_time_override = DEAD_TIME; // 死区时间
uint8_t TEMPERATURE_LIMIT = 255; // 温度限制，255表示禁用
uint8_t SINE_MODE_POWER = 5;
uint8_t eeprom_layout_version = 2;

// Timing and Duty Cycles
uint16_t PWM_MAX_ARR = PWM_AUTORELOAD;
uint16_t PWM_ARR = PWM_AUTORELOAD;
uint16_t stall_protect_minimum_duty = DEAD_TIME;
uint16_t MOTOR_KV = 2000;
uint16_t low_cell_volt_cutoff = 330; // 低电压保护电压
uint16_t throttle_max_at_low_rpm = 400;
uint16_t throttle_max_at_high_rpm = 2000;
uint16_t current_limit = 0; // 电流限制
uint16_t low_rpm_level = 20; // 低转速等级，单位千转每分钟
uint16_t high_rpm_level = 70; // 高转速等级
uint16_t reverse_speed_threshold;

int32_t min_startup_duty_cycle = 120;
int32_t max_startup_duty_cycle = 200;
int32_t minimum_duty_cycle = DEAD_TIME;
int32_t maximum_duty_cycle = 2000;

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
	read_flash_bin(eepromBuffer, eeprom_address, 176);

	if (eepromBuffer[17] == 0x01) {
		DIR_REVERSED = 1;
	} else {
		DIR_REVERSED = 0;
	}
	if (eepromBuffer[18] == 0x01) {
		BI_DIRECTION = 1;
	} else {
		BI_DIRECTION = 0;
	}
	if (eepromBuffer[19] == 0x01) {
		USE_SIN_START = 1;
		//	 min_startup_duty = sin_mode_min_s_d;
	}else{
		USE_SIN_START = 0;
	}
	if (eepromBuffer[20] == 0x01) {
		COMP_PWM = 1;
	} else {
		COMP_PWM = 0;
	}
	if (eepromBuffer[21] == 0x01) {
		VARIABLE_PWM = 1;
	} else {
		VARIABLE_PWM = 0;
	}
	if (eepromBuffer[22] == 0x01) {
		STUCK_ROTOR_PROTECTION = 1;
	} else {
		STUCK_ROTOR_PROTECTION = 0;
	}
	if (eepromBuffer[23] < 4) {
		ADVANCE_LEVEL = eepromBuffer[23];
	} else {
		ADVANCE_LEVEL = 2; // * 7.5 increments
	}

	if (eepromBuffer[24] < 49 && eepromBuffer[24] > 7) {
		if (eepromBuffer[24] < 49 && eepromBuffer[24] > 23) {
			PWM_MAX_ARR = map(eepromBuffer[24], 24, 48, PWM_AUTORELOAD,PWM_AUTORELOAD / 2);
		}
		if (eepromBuffer[24] < 24 && eepromBuffer[24] > 11) {
			PWM_MAX_ARR = map(eepromBuffer[24], 12, 24, PWM_AUTORELOAD * 2,PWM_AUTORELOAD);
		}
		if (eepromBuffer[24] < 12 && eepromBuffer[24] > 7) {
			PWM_MAX_ARR = map(eepromBuffer[24], 7, 16, PWM_AUTORELOAD * 3,PWM_AUTORELOAD / 2 * 3);
		}
		SET_AUTO_RELOAD_PWM(PWM_MAX_ARR);
		//   throttle_max_at_high_rpm = TIMER1_MAX_ARR;
		//   duty_cycle_maximum = TIMER1_MAX_ARR;
	} else {
		PWM_ARR = PWM_AUTORELOAD;
		SET_AUTO_RELOAD_PWM(PWM_ARR);
	}

	if (eepromBuffer[25] < 151 && eepromBuffer[25] > 49) {
		min_startup_duty_cycle = (eepromBuffer[25]);
		minimum_duty_cycle = (eepromBuffer[25] / 3);
		stall_protect_minimum_duty = minimum_duty_cycle + 10;
	} else {
		min_startup_duty_cycle = 150;
		minimum_duty_cycle = (min_startup_duty_cycle / 2) + 10;
	}

	MOTOR_KV = (eepromBuffer[26] * 40) + 20;

	MOTOR_POLES = eepromBuffer[27];

	if (eepromBuffer[28] == 0x01) {
		BRAKE_ON_STOP = 1;
	} else {
		BRAKE_ON_STOP = 0;
	}

	if (eepromBuffer[29] == 0x01) {
		STALL_PROTECTION = 1;
	} else {
		STALL_PROTECTION = 0;
	}
	setVolume(2);
	if (eepromBuffer[1] > 0) { // these commands weren't introduced until eeprom version 1.
		if (eepromBuffer[30] > 11) {
			setVolume(5);
		} else {
			setVolume(eepromBuffer[30]);
		}

		if (eepromBuffer[31] == 0x01) {
			TLM_ON_INTERVAL = 1;
		} else {
			TLM_ON_INTERVAL = 0;
		}
		servo_low_threshold = (eepromBuffer[32] * 2) + 750; // anything below this point considered 0
		servo_high_threshold = (eepromBuffer[33] * 2) + 1750;
 		// anything above this point considered 2000 (max)
		servo_neutral = (eepromBuffer[34]) + 1374;
		servo_dead_band = eepromBuffer[35];

		if (eepromBuffer[36] == 0x01) {
			LOW_VOLTAGE_CUTOFF = 1;
		} else {
			LOW_VOLTAGE_CUTOFF = 0;
		}

		low_cell_volt_cutoff = eepromBuffer[37] + 250; // 2.5 to 3.5 volts per cell range

		if (eepromBuffer[38] == 0x01) {
			RC_CAR_REVERSE = 1;
		} else {
			RC_CAR_REVERSE = 0;
		}

		if (eepromBuffer[39] == 0x01) {
#ifdef HAS_HALL_SENSORS
            USE_HALL_SENSOR = 1;
#else
			USE_HALL_SENSOR = 0;
#endif
		} else {
			USE_HALL_SENSOR = 0;
		}

		if (eepromBuffer[40] > 4 && eepromBuffer[40] < 26) { // sine mode changeover 5-25 percent throttle
			sine_mode_changeover_thottle_level = eepromBuffer[40];
		}

		if (eepromBuffer[41] > 0 && eepromBuffer[41] < 11) { // drag brake 1-10
			DRAG_BRAKE_STRENGTH = eepromBuffer[41];
		}

		if (eepromBuffer[42] > 0 && eepromBuffer[42] < 10) { //driving_brake_strength:motor brake 1-9
			dead_time_override = DEAD_TIME + (150 - (eepromBuffer[42] * 10));
			if (dead_time_override > 200) {
				dead_time_override = 200;
			}
			min_startup_duty_cycle = eepromBuffer[25] + dead_time_override;
			minimum_duty_cycle = eepromBuffer[25] / 2 + dead_time_override;
			throttle_max_at_low_rpm = throttle_max_at_low_rpm + dead_time_override;
			max_startup_duty_cycle = max_startup_duty_cycle + dead_time_override;
			if (PWMA_DTR <= 127) {
				PWMA_DTR = dead_time_override;
			} else if (PWMA_DTR > 127) {
				PWMA_DTR = 0x80 | ((dead_time_override - 128) >> 1);
			}
		}

		if (eepromBuffer[43] >= 70 && eepromBuffer[43] <= 140) {
			TEMPERATURE_LIMIT = eepromBuffer[43];
		}

		if (eepromBuffer[44] > 0 && eepromBuffer[44] < 100) {
			current_limit = eepromBuffer[44];
			USE_CURRENT_LIMIT = 1;
		}

		if (eepromBuffer[45] > 0 && eepromBuffer[45] < 11) {
			SINE_MODE_POWER = eepromBuffer[45];
		}

		if (eepromBuffer[46] < 10) {
			switch (eepromBuffer[46]) {
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

		if (eepromBuffer[47] == 0x01) {
			AUTO_ADVANCE = 1;
		} else {
			AUTO_ADVANCE = 0;
		}

		if (MOTOR_KV < 300) {
			LOW_RPM_THROTTLE_LIMIT = 0;
		}else{
			LOW_RPM_THROTTLE_LIMIT = 1;
		}

		low_rpm_level = MOTOR_KV / 100 / (32 / MOTOR_POLES);
		high_rpm_level = MOTOR_KV / 12 / (32 / MOTOR_POLES);
	}
	reverse_speed_threshold = map(MOTOR_KV, 300, 3000, 1000, 500);
	//   reverse_speed_threshold = 200;
//    if (!comp_pwm) {
//        bi_direction = 0;
//    }
}

void saveEEpromSettings(void) {

	eepromBuffer[1] = eeprom_layout_version;
	if (DIR_REVERSED == 1) {
		eepromBuffer[17] = 0x01;
	} else {
		eepromBuffer[17] = 0x00;
	}
	if (BI_DIRECTION == 1) {
		eepromBuffer[18] = 0x01;
	} else {
		eepromBuffer[18] = 0x00;
	}
	if (USE_SIN_START == 1) {
		eepromBuffer[19] = 0x01;
	} else {
		eepromBuffer[19] = 0x00;
	}

	if (COMP_PWM == 1) {
		eepromBuffer[20] = 0x01;
	} else {
		eepromBuffer[20] = 0x00;
	}
	if (VARIABLE_PWM == 1) {
		eepromBuffer[21] = 0x01;
	} else {
		eepromBuffer[21] = 0x00;
	}
	if (STUCK_ROTOR_PROTECTION == 1) {
		eepromBuffer[22] = 0x01;
	} else {
		eepromBuffer[22] = 0x00;
	}
	eepromBuffer[23] = ADVANCE_LEVEL;
	save_flash_nolib(eepromBuffer, 176, eeprom_address);
}


