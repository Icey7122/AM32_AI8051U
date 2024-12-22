/*
 * target.h
 *
 *  Created on: 2024年12月1日
 *      Author: 17263
 */
#ifndef INC_TARGET_H_
#define INC_TARGET_H_

#ifndef USE_MAKE

#define AM32REF_8051U

#endif

#ifdef AM32REF_8051U

#define FILE_NAME 					"AM32REF_8051U"
#define FIRMWARE_NAME 				"AM32 Ref-ESC"
#define DEAD_TIME 					20
#define HARDWARE_GROUP_A
#define MILLIVOLT_PER_AMP 			65
#define CURRENT_OFFSET 				0
#define TARGET_VOLTAGE_DIVIDER 		110
#define VOLTAGE_ADC_CHANNEL
#define VOLTAGE_ADC_PIN 			11//P11
#define CURRENT_ADC_CHANNEL
#define CURRENT_ADC_PIN 			12//P12
// #define USE_SERIAL_TELEMETRY
#define TARGET_STALL_PROTECTION_INTERVAL 9000

#endif

#ifndef FIRMWARE_NAME
/* if you get this then you have forgotten to add the section for your target above */
#error "Missing defines for target"
#endif

#ifndef TARGET_VOLTAGE_DIVIDER
#define TARGET_VOLTAGE_DIVIDER 110
#endif

#ifndef SINE_DIVIDER
#define SINE_DIVIDER 2
#endif

#ifndef MILLIVOLT_PER_AMP
#define MILLIVOLT_PER_AMP 20
#endif

#ifndef CURRENT_OFFSET
#define CURRENT_OFFSET 0
#endif

#ifndef TARGET_STALL_PROTECTION_INTERVAL
#define TARGET_STALL_PROTECTION_INTERVAL 6500
#endif

#ifndef RAMP_SPEED_STARTUP
#define RAMP_SPEED_STARTUP 1 // adjusted 2.14 to match duty cycle change between mcu targets.
#endif

#ifndef RAMP_SPEED_LOW_RPM // below commutation interval of 250us
#define RAMP_SPEED_LOW_RPM 4
#endif

#ifndef RAMP_SPEED_HIGH_RPM
#define RAMP_SPEED_HIGH_RPM 16
#endif

#ifdef HARDWARE_GROUP_A

#define MCU_8051U
#define INPUT_PIN 					P21
#define INPUT_PIN_PORT 				P2
#define INPUT_MODE_PIN				GPIO_ModePin_1

#define PHASE_A_GPIO_HIGH 			P00
#define PHASE_A_GPIO_PORT_HIGH 		P0
#define PHASE_A_MODE_GPIO_HIGH		GPIO_ModePin_0
#define PHASE_A_GPIO_LOW 			P01
#define PHASE_A_GPIO_PORT_LOW 		P0
#define PHASE_A_MODE_GPIO_LOW		GPIO_ModePin_1

#define PHASE_B_GPIO_HIGH 			P02
#define PHASE_B_GPIO_PORT_HIGH 		P0
#define PHASE_B_MODE_GPIO_HIGH		GPIO_ModePin_2
#define PHASE_B_GPIO_LOW 			P03
#define PHASE_B_GPIO_PORT_LOW 		P0
#define PHASE_B_MODE_GPIO_LOW		GPIO_ModePin_3

#define PHASE_C_GPIO_HIGH 			P04
#define PHASE_C_GPIO_PORT_HIGH 		P0
#define PHASE_C_MODE_GPIO_HIGH		GPIO_ModePin_4
#define PHASE_C_GPIO_LOW 			P05
#define PHASE_C_GPIO_PORT_LOW 		P0
#define PHASE_C_MODE_GPIO_LOW		GPIO_ModePin_5

#define PHASE_A_COMP COMP_P46
#define PHASE_B_COMP COMP_P51
#define PHASE_C_COMP COMP_P50

#endif

#ifdef MCU_8051U

#define CPU_FREQUENCY_MHZ 			40
#define APPLICATION_ADDRESS 		0xFF1000
#define EEPROM_START_ADD 			(uint32_t)0xFFF800
#define PWM_CLOCK_MHZ 				CPU_FREQUENCY_MHZ

#define CMP_IRQHandler(val)				CMP_ISR(val) interrupt P4INT_VECTOR using 3
#define TWENTY_KHZ_IRQHandler(val)		TMR4_ISR(val) interrupt TMR4_VECTOR using 1
#define COM_TIMER_IRQHandler(val)		TMR0_ISR(val) interrupt TMR0_VECTOR using 3
#define IC_IRQHandler(val)				PWMB_ISR(val) interrupt PWMB_VECTOR	using 2	


#define PWM_FREQUNCY_HZ				24000
#define PWM_AUTORELOAD 				((PWM_CLOCK_MHZ * 1000000 / PWM_FREQUNCY_HZ) - 1)

#define TARGET_MIN_BEMF_COUNTS 		2
#define USE_ADC
#ifndef CURRENT_ADC_PIN
#define CURRENT_ADC_PIN 			
#define CURRENT_ADC_CHANNEL 		
#endif
#ifndef VOLTAGE_ADC_PIN
#define VOLTAGE_ADC_PIN 			
#define VOLTAGE_ADC_CHANNEL 		
#endif
#define DSHOT_PRIORITY_THRESHOLD 	70

#endif

#define NVIC_SystemReset() 	IAP_CONTR = 0x20

#ifndef LOOP_FREQUENCY_HZ
#define LOOP_FREQUENCY_HZ 20000
#endif

#define PID_LOOP_DIVIDER (LOOP_FREQUENCY_HZ / 1000)


#endif /* INC_TARGET_H_ */