#include "input.h"
#include "ushot.h"
#include "main.h"
#include "eeprom.h"
#include "commutate.h"

bool EDT_ARM_ENABLE;
bool EDT_ARMED = 0;

uint8_t ushotcommand;
uint8_t last_ushot_command;
uint8_t last_command;
int16_t tocheck = 0;

uint8_t command_counter;
uint8_t high_pin_counter = 0;
uint16_t ushot_goodcounter = 0;
uint16_t ushot_badcounter = 0;

void UN_UART_Init(void)
{
	gpio_mode_set(P3,GPIO_ModePin_0,GPIO_Mode_IN_FLOATING);

	DMA_UR1R_CFG = 0x8A; 	//启用UR1R_DMA中断 数据优先级2,中断优先级2
	DMA_UR1R_CR = 0x80; 	//运行UR1R_DMA
	
	DMA_UR1R_AMTH = 0x00;
	DMA_UR1R_AMT = 0x04;	//设置DMA传输数据长度5个字节

	DMA_UR1R_DONEH = 0x00;
	DMA_UR1R_DONE = 0x00;

	DMA_UR1R_RXAH = (uint16_t)dma_buffer >> 8;
	DMA_UR1R_RXAL = (uint16_t)dma_buffer & 0xFF;	//设置DMA传输数据地址

}

void ushotDmaBuffer(void) {
    DMA_UR1R_CR = 0xA1;
}

void computeUshotDMA(void) {
    register union_float_t tocheck_flt;
    if ((DMA_UR1R_STA & 0x02) == 0) {
		signaltimecounter = 0;

        if (!armed) {
            if (send_telemetry == 0) {
                if (getInputPinState()) { // if the pin is high for 100 checks between
                                          // signal pulses its inverted
					high_pin_counter++;
                    if (high_pin_counter > 100) {
                        send_telemetry = 1;
                    }
                }
            }
        }
        
        tocheck_flt.u8[0] = ((uint8_t *)dma_buffer)[3];
        tocheck_flt.u8[1] = ((uint8_t *)dma_buffer)[2];
        tocheck_flt.u8[2] = ((uint8_t *)dma_buffer)[1];
        tocheck_flt.u8[3] = ((uint8_t *)dma_buffer)[0];
        tocheck = tocheck_flt.Flt;
        

        if ((USARTCR2 & 0x01) == 0) {
            signaltimecounter = 0;
            ushot_goodcounter++;
			if (((uint8_t *)dma_buffer)[4] & 0x80) {
				send_telemetry = 1;
			}
            if (tocheck > 47) {
                if (EDT_ARMED) {
                    newinput = tocheck;
                    ushotcommand = 0;
                    command_counter = 0;
                    return;
                }
            }
            if ((tocheck <= 47) && (tocheck > 0)) {
                newinput = 0;
                ushotcommand = tocheck; //  todo
            }
            if (tocheck == 0) {
                if (EDT_ARM_ENABLE) {
                    EDT_ARMED = 0;
                }

                newinput = 0;
                ushotcommand = 0;
                command_counter = 0;
            }

            if ((ushotcommand > 0) && (running == 0) && armed) {
                if (ushotcommand != last_command) {
                    last_command = ushotcommand;
                    command_counter = 0;
                }
                if (ushotcommand < 5) { // beacons
                    command_counter = 6; // go on right away
                }
                command_counter++;
                if (command_counter >= 6) {
                    command_counter = 0;
                    switch (ushotcommand) { // todo
                    case 1:
                        play_tone_flag = 1;
                        break;
                    case 2:
                        play_tone_flag = 2;
                        break;
                    case 3:
                        play_tone_flag = 3;
                        break;
                    case 4:
                        play_tone_flag = 4;
                        break;
                    case 5:
                        play_tone_flag = 5;
                        break;
                    case 7:
                        eepromBuffer[17] = 0;
                        forward = 1 - eepromBuffer[17];
                        //	play_tone_flag = 1;
                        break;
                    case 8:
						eepromBuffer[17] = 1;
						forward = 1 - eepromBuffer[17];
                        //	play_tone_flag = 2;
                        break;
                    case 9:
                        eepromBuffer[18] = 0;
                        break;
                    case 10:
                        eepromBuffer[18] = 1;
                        break;
                    case 12:
                        saveEEpromSettings();
                        play_tone_flag = 1 + eepromBuffer[17];
                        //	NVIC_SystemReset();
                        break;
                    case 13:
                        // ushot_extended_telemetry = 1;
                        // send_extended_ushot = 0b111000000000;
                        if (EDT_ARM_ENABLE == 1) {
                            EDT_ARMED = 1;
                        }
                        break;
                    case 14:
                        // ushot_extended_telemetry = 0;
                        // send_extended_ushot = 0b111011111111;
                        //	make_ushot_package();
                        break;
                    case 20:
                        forward = 1 - eepromBuffer[17];
                        break;
                    case 21:
                        forward = eepromBuffer[17];
                        break;
                    }
                    last_ushot_command = ushotcommand;
                    ushotcommand = 0;
                }
            }
        } else {
            USARTCR2 &= ~0x01;
            ushot_badcounter++;
        }
    }
}

void checkUshot(void) {
	register PWM_TypeDef *PWMB = PWMB_ADDRESS;
    if ((smallestnumber >= 1) && (smallestnumber <= 8)) {
        ushot = 1;
        inputSet = 1;
		PWMB->IER = 0x00;
		receiveDmaBuffer = ushotDmaBuffer;
    }
}
