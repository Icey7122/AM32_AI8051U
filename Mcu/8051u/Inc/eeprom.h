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

typedef union EEprom_u {
    struct {
        uint8_t reserved_0; //0
        uint8_t eeprom_version; //1
        uint8_t reserved_1; //2
        struct {        
            uint8_t major; //3
            uint8_t minor; //4
        } version;
        char firmware_name[12]; //5-16
        uint8_t dir_reversed; // 17
        uint8_t bi_direction; // 18
        uint8_t use_sine_start; // 19
        uint8_t comp_pwm; // 20
        uint8_t variable_pwm; // 21
        uint8_t stuck_rotor_protection; // 22
        uint8_t advance_level; // 23
        uint8_t pwm_frequency; // 24
        uint8_t startup_power; // 25
        uint8_t motor_kv; // 26
        uint8_t motor_poles; // 27
        uint8_t brake_on_stop; // 28
        uint8_t stall_protection; // 29
        uint8_t beep_volume; // 30
        uint8_t telemetry_on_interval; // 31
        struct {
            uint8_t low_threshold; // 32
            uint8_t high_threshold; // 33
            uint8_t neutral; // 34
            uint8_t dead_band; // 35
        } servo;
        uint8_t low_voltage_cut_off; // 36
        uint8_t low_cell_volt_cutoff; // 37
        uint8_t rc_car_reverse; // 38
        uint8_t use_hall_sensors; // 39
        uint8_t sine_mode_changeover_thottle_level; // 40
        uint8_t drag_brake_strength; // 41
        uint8_t driving_brake_strength; // 42
        struct {
            uint8_t temperature; // 43
            uint8_t current; // 44
        } limits;
        uint8_t sine_mode_power; // 45
        uint8_t input_type; // 46
        uint8_t auto_advance; // 47
        uint8_t tune[128]; // 48-175
        struct {
            uint8_t can_node; // 176
            uint8_t esc_index; // 177
            uint8_t require_arming; // 178
            uint8_t telem_rate; // 179
            uint8_t require_zero_throttle; // 180
            uint8_t filter_hz; // 181
            uint8_t debug_rate; // 182
            uint8_t term_enable; // 183
            uint8_t reserved[8]; // 184-191
        } can;
    }eeppack;
    uint8_t buffer[192];
} EEprom_t;

extern uint32_t eeprom_address;
extern EEprom_t eepromBuffer;


extern bool LOW_VOLTAGE_CUTOFF; // 低电压保护
extern bool USE_HALL_SENSOR; // 有感模式
extern bool use_current_limit; // 电流限制
extern bool low_rpm_throttle_limit;

extern uint8_t dead_time_override; // 死区时间
extern uint8_t eeprom_layout_version;

extern uint16_t PWM_MAX_ARR;
extern uint16_t pwm_arr;
extern uint16_t stall_protect_minimum_duty;
extern uint16_t motor_kv;
extern uint16_t low_cell_volt_cutoff; // 低电压保护电压
extern uint16_t throttle_max_at_low_rpm;
extern uint16_t throttle_max_at_high_rpm;
extern uint16_t low_rpm_level; // 低转速等级，单位千转每分钟
extern uint16_t high_rpm_level; // 高转速等级
extern uint16_t reverse_speed_threshold;

extern int32_t min_startup_duty;
extern int32_t startup_max_duty_cycle;
extern int32_t minimum_duty_cycle;
extern int32_t duty_cycle_maximum;

extern int16_t servo_low_threshold; // 舵机低阈值
extern int16_t servo_high_threshold; // 舵机高阈值
extern int16_t servo_neutral; // 舵机中性
extern uint8_t servo_dead_band; // 舵机死区

bool save_flash_nolib(uint8_t *dat, uint32_t length, uint32_t add);
void read_flash_bin(uint8_t *dat, uint32_t add, uint32_t out_buff_len);
void loadEEpromSettings(void);
void saveEEpromSettings(void);

#endif /* INC_EEPROM_H_ */
