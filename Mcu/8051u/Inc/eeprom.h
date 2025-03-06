/*
 * eeprom.h
 *
 *  Created on: 2024年12月2日
 *      Author: 17263
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include "mcu.h"
#include "targets.h"

#define		IAP_STANDBY()	IAP_CMD = 0		
#define		IAP_READ()		IAP_CMD = 1		
#define		IAP_WRITE()		IAP_CMD = 2		
#define		IAP_ERASE()		IAP_CMD = 3		

#define	IAP_ENABLE()		IAPEN = 1
#define	IAP_DISABLE()		IAP_CONTR = 0; IAP_CMD = 0; IAP_TRIG = 0; IAP_ADDRH = 0xFF; IAP_ADDRL = 0xFF

enum inputType {
	AUTO_IN, DSHOT_IN, SERVO_IN, SERIAL_IN,EDTARM_IN
};

extern uint32_t eeprom_address;
extern uint8_t xdata eepromBuffer[176];

extern bool BI_DIRECTION; // 双向
extern bool DIR_REVERSED; // 方向反转
extern bool COMP_PWM; // 互补PWM
extern bool VARIABLE_PWM; // 可变PWM
extern bool STUCK_ROTOR_PROTECTION; // 堵转保护
extern bool BRAKE_ON_STOP;
extern bool TLM_ON_INTERVAL; // 遥测开启
extern bool LOW_VOLTAGE_CUTOFF; // 低电压保护
extern bool RC_CAR_REVERSE; // RC车模式
extern bool USE_HALL_SENSOR; // 有感模式
extern bool USE_CURRENT_LIMIT; // 电流限制
extern bool AUTO_ADVANCE; // 自动提前
extern bool LOW_RPM_THROTTLE_LIMIT;
extern uint8_t USE_SIN_START; // 正弦启动
extern uint8_t STALL_PROTECTION;

extern uint8_t ADVANCE_LEVEL; // 提前角度
extern uint8_t MOTOR_POLES;
extern uint8_t sine_mode_changeover_thottle_level; // 正弦模式切换油门等级
extern uint8_t DRAG_BRAKE_STRENGTH; // 拖动制动强度
extern uint8_t dead_time_override; // 死区时间
extern uint8_t TEMPERATURE_LIMIT; // 温度限制，255表示禁用
extern uint8_t SINE_MODE_POWER;
extern uint8_t eeprom_layout_version;

extern uint16_t PWM_MAX_ARR;
extern uint16_t PWM_ARR;
extern uint16_t stall_protect_minimum_duty;
extern uint16_t MOTOR_KV;
extern uint16_t low_cell_volt_cutoff; // 低电压保护电压
extern uint16_t throttle_max_at_low_rpm;
extern uint16_t throttle_max_at_high_rpm;
extern uint16_t current_limit; // 电流限制
extern uint16_t low_rpm_level; // 低转速等级，单位千转每分钟
extern uint16_t high_rpm_level; // 高转速等级
extern uint16_t reverse_speed_threshold;

extern int32_t min_startup_duty_cycle;
extern int32_t max_startup_duty_cycle;
extern int32_t minimum_duty_cycle;
extern int32_t maximum_duty_cycle;

extern int16_t servo_low_threshold; // 舵机低阈值
extern int16_t servo_high_threshold; // 舵机高阈值
extern int16_t servo_neutral; // 舵机中性
extern uint8_t servo_dead_band; // 舵机死区

bool save_flash_nolib(uint8_t *dat, uint32_t length, uint32_t add);
void read_flash_bin(uint8_t *dat, uint32_t add, uint32_t out_buff_len);
void loadEEpromSettings(void);
void saveEEpromSettings(void);

#endif /* INC_EEPROM_H_ */
