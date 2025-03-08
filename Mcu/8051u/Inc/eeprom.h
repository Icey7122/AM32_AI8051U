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
#include "nstdbool.h"

#define		IAP_STANDBY()	IAP_CMD = 0		
#define		IAP_READ()		IAP_CMD = 1		
#define		IAP_WRITE()		IAP_CMD = 2		
#define		IAP_ERASE()		IAP_CMD = 3		

#define	IAP_ENABLE()		IAPEN = 1
#define	IAP_DISABLE()		IAP_CONTR = 0; IAP_CMD = 0; IAP_TRIG = 0; IAP_ADDRH = 0xFF; IAP_ADDRL = 0xFF

enum inputType {
	AUTO_IN, DSHOT_IN, SERVO_IN, SERIAL_IN
};

extern uint32_t eeprom_address;
extern uint8_t xdata eepromBuffer[176];

extern uint8_t BI_DIRECTION;  			
extern uint8_t DIR_REVERSED; 				
extern uint8_t USE_SIN_START;			
extern uint8_t COMP_PWM; 				
extern uint8_t VARIABLE_PWM; 			
extern uint8_t STUCK_ROTOR_PROTECTION;	
extern uint8_t ADVANCE_LEVEL; 			
extern uint16_t PWM_MAX_ARR;
extern uint16_t PWM_ARR;
extern int32_t min_startup_duty_cycle;
extern int32_t max_startup_duty_cycle;
extern int32_t minimum_duty_cycle;
extern int32_t maximum_duty_cycle;
extern uint16_t stall_protect_minimum_duty;
extern uint16_t MOTOR_KV;
extern uint8_t MOTOR_POLES;
extern uint8_t BRAKE_ON_STOP;
extern uint8_t STALL_PROTECTION;
extern uint8_t TLM_ON_INTERVAL;		
extern int16_t servo_low_threshold; 		
extern int16_t servo_high_threshold; 		
extern int16_t servo_neutral; 			
extern uint8_t servo_dead_band; 			
extern uint8_t LOW_VOLTAGE_CUTOFF;		
extern uint16_t low_cell_volt_cutoff;	
extern uint8_t RC_CAR_REVERSE; 		
extern uint8_t USE_HALL_SENSOR;  			
extern uint8_t sine_mode_changeover_thottle_level; 
extern uint8_t DRAG_BRAKE_STRENGTH; 		
extern uint8_t dead_time_override; 		
extern uint16_t throttle_max_at_low_rpm;
extern uint16_t throttle_max_at_high_rpm;
extern uint8_t TEMPERATURE_LIMIT; 			
extern uint16_t current_limit;			
extern uint8_t USE_CURRENT_LIMIT;		
extern uint8_t SINE_MODE_POWER;
extern uint8_t AUTO_ADVANCE; 		
extern uint8_t low_rpm_throttle_limit;
extern uint16_t low_rpm_level; 
extern uint16_t high_rpm_level; 
extern uint16_t reverse_speed_threshold;
extern uint8_t eeprom_layout_version;

extern uint8_t play_tone_flag;

bool save_flash_nolib(uint8_t *dat, uint32_t length, uint32_t add);
void read_flash_bin(uint8_t *dat, uint32_t add, uint32_t out_buff_len);
void loadEEpromSettings(void);
void saveEEpromSettings(void);

#endif /* INC_EEPROM_H_ */
