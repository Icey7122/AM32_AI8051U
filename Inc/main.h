#ifndef MAIN_H
#define MAIN_H

#include "functions.h"
#include "mcu.h"

extern bool armed;
extern bool crawler_mode;
extern bool send_telemetry; // 发送遥测标志
extern bool send_esc_info_flag;
extern bool maximum_throttle_change_ramp;
extern uint8_t drive_by_rpm;
extern uint8_t use_speed_control_loop;
extern uint8_t running;
extern uint8_t fast_accel;
extern int8_t startup_boost;
extern int8_t reversing_dead_band;
extern uint16_t enter_sine_angle;

extern uint32_t MINIMUM_RPM_SPEED_CONTROL;
extern uint32_t MAXIMUM_RPM_SPEED_CONTROL;

extern fastPID speedPid; // commutation speed loop time
extern fastPID currentPid; // 1khz loop time
extern fastPID stallPid; // 1khz loop time

extern uint16_t e_rpm;
extern uint16_t k_erpm;
extern uint32_t e_com_time;
extern uint32_t target_e_com_time;

extern uint8_t cell_count; // 电池计数器
extern uint16_t battery_voltage; // 电池电压
extern int16_t actual_current; // 实际电流
extern float consumed_current; // 消耗电流
extern int16_t use_current_limit_adjust; // 电流限制
extern uint8_t degrees_celsius;

extern float stall_protection_adjust;
extern uint16_t stall_protect_target_interval; // 失速保护目标间隔

extern uint16_t telem_ms_count; // 遥测计数器
extern uint8_t telemetry_interval_ms; // 遥测间隔

extern uint8_t adc_counter;
extern uint16_t ledcounter; // LED计数器
extern uint16_t one_khz_loop_counter; // 1KHz计数器
extern uint16_t tenkhzcounter; // 20KHz计数器
extern uint16_t signaltimeout; // 信号计数器
extern uint16_t low_voltage_count; // 低电压计数器
extern uint16_t armed_timeout_count; // 握手超时计数器

extern void tenKhzRoutine(void);

#endif // !MAIN_H