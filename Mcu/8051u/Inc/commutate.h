/*
 * comparator.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */

#ifndef INC_COMMUTATE_H_
#define INC_COMMUTATE_H_

#include "mcu.h"

#define COMP_P46 0x00
#define COMP_P50 0x01
#define COMP_P51 0x02

#define ENABLE_RISE_INTERRUPT() 		P4IM0 |= 0x02
#define ENABLE_FALL_INTERRUPT()			P4IM0 &= ~0x02

#define getCompOutputLevel() 	(CMPRES)
#define maskPhaseInterrupts() 	P4INTE &= ~0x02
#define enableCompInterrupts() 	P4INTE |= 0x02

extern bool forward; // 正转
extern bool rising; // 上升沿
extern bool desync_check; // 失步检查
extern bool prop_brake_active; // 比例制动激活
extern bool old_routine; // 旧周期
extern bool zcfound; // 零交叉发现
extern bool do_once_sinemode;
extern uint8_t desync_happened; // 失步标志

extern int8_t step; // 换相步骤
extern uint8_t filter_level; // 滤波等级
extern uint8_t stuckcounter; // 卡住计数器
extern uint8_t bemfcounter; // BEMF计数器
extern uint8_t min_bemf_counts_up;
extern uint8_t min_bemf_counts_down;
extern uint8_t bemf_timeout; // BEMF超时
extern uint8_t bemf_timeout_happened; // BEMF超时标志
extern uint8_t bad_count; // 坏计数
extern uint8_t bad_count_threshold; // 坏计数阈值
extern uint8_t temp_advance; // 临时提前
extern uint8_t auto_advance_level; // 自动提前等级
extern uint8_t stepper_sine;
extern uint8_t changeover_step;

extern uint16_t lastzctime; // 当前零交叉时间
extern uint16_t thiszctime; // 当前零交叉时间
extern uint16_t zero_crosses; // 零交叉
extern uint16_t advance; // 提前
extern uint16_t waitTime; // 换相等待时间
extern uint16_t average_interval; // 平均间隔
extern uint16_t last_average_interval;
extern uint16_t commutation_interval; // 换相间隔
extern uint16_t commutation_intervals[6]; // 换相间隔数组
extern uint16_t last_commutation_interval;
extern uint16_t step_delay;

extern int16_t phase_A_position;
extern int16_t phase_B_position;
extern int16_t phase_C_position;

extern const int32_t pwmSin[];

extern void changeCompInput(void);
extern void commutate(void);
extern void interruptRoutine(void);
extern void PeriodElapsedCallback(void);
extern void zcfoundroutine(void);
extern void getBemfState(void);
extern void advanceincrement(void);
extern void startMotor(void);

#endif /* INC_COMMUTATE_H_ */
