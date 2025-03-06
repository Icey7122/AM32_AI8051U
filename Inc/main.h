#ifndef MAIN_H
#define MAIN_H

#include "functions.h"
#include "mcu.h"

extern bool armed;
extern bool send_telemetry; // 发送遥测标志
extern bool maximum_throttle_change_ramp;
extern uint8_t drive_by_rpm;
extern uint8_t use_speed_control_loop;
extern uint8_t running;
extern uint8_t fast_accel;

extern uint32_t MINIMUM_RPM_SPEED_CONTROL;
extern uint32_t MAXIMUM_RPM_SPEED_CONTROL;

extern fastPID speedPid; // commutation speed loop time
extern fastPID currentPid; // 1khz loop time
extern fastPID stallPid; // 1khz loop time

extern uint16_t e_rpm;
extern uint16_t k_erpm;
extern uint32_t e_com_time;
extern uint32_t target_e_com_time;

extern uint8_t cell_counter; // 电池计数器
extern uint16_t battery_voltage; // 电池电压
extern int16_t actual_current; // 实际电流
extern float consumed_current; // 消耗电流
extern int16_t current_limit_adjust; // 电流限制
extern uint8_t degrees_celsius;

extern float stall_protection_adjust;
extern uint16_t stall_protect_target_interval; // 失速保护目标间隔

extern uint16_t telem_ms_counter; // 遥测计数器
extern uint8_t telemetry_interval_ms; // 遥测间隔

extern uint16_t ledcounter; // LED计数器
extern uint16_t onekhzcounter; // 1KHz计数器
extern uint16_t twentykhzcounter; // 20KHz计数器
extern uint16_t signaltimecounter; // 信号计数器
extern uint16_t low_voltage_counter; // 低电压计数器
extern uint16_t armed_timeout_counter; // 握手超时计数器

extern void TwentyKhzRoutine(void);

#endif // !MAIN_H