#ifndef MAIN_H
#define MAIN_H

#include "functions.h"
#include "mcu.h"

extern uint8_t drive_by_rpm;
extern uint8_t use_speed_control_loop;
extern uint32_t MINIMUM_RPM_SPEED_CONTROL;
extern uint32_t MAXIMUM_RPM_SPEED_CONTROL;

extern fastPID xdata speedPid;
extern fastPID xdata currentPid;
extern fastPID xdata stallPid;

extern uint8_t  armed;
extern uint8_t  running;

extern uint16_t e_rpm;
extern uint16_t k_erpm;
extern uint32_t e_com_time;
extern uint32_t target_e_com_time;

extern uint8_t cell_counter;
extern uint16_t battery_voltage; 		
extern uint16_t actual_current;
extern uint16_t current_limit_adjust;
extern uint8_t degrees_celsius;

extern float stall_protection_adjust;
extern uint16_t stall_protect_target_interval;

extern uint16_t telem_ms_counter;		//遥测计数器
extern uint8_t send_telemetry;			//发送遥测标志
extern uint8_t telemetry_interval_ms; 	//遥测间隔

extern uint16_t ledcounter;			//LED计数器
extern uint16_t onekhzcounter;			//1KHz计数器
extern uint16_t twentykhzcounter;		//20KHz计数器
extern uint16_t signaltimecounter;		//信号计数器
extern uint16_t armed_timeout_counter; //握手超时计数器

extern void TwentyKhzRoutine(void);

#endif // !MAIN_H