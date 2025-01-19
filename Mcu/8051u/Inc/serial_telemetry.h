#ifndef SERIAL_TELEMETRY_H
#define SERIAL_TELEMETRY_H

#include "mcu.h"
#include "uart.h"


extern uint8_t update_crc8(uint8_t crc, uint8_t crc_seed);
extern uint8_t get_crc8(uint8_t* Buf, uint8_t BufLen);
extern void makeTelemPackage(uint8_t temp, uint16_t voltage, uint16_t current, uint16_t consumption, uint16_t e_rpm); 


#endif // SERIAL_TELEMETRY_H