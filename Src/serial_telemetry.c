#include "serial_telemetry.h"

uint8_t update_crc8(uint8_t crc, uint8_t crc_seed)
{
    uint8_t crc_u, i;
    crc_u = crc;
    crc_u ^= crc_seed;
    for (i = 0; i < 8; i++)
        crc_u = (crc_u & 0x80) ? 0x7 ^ (crc_u << 1) : (crc_u << 1);
    return (crc_u);
}

uint8_t get_crc8(uint8_t* Buf, uint8_t BufLen)
{
    uint8_t crc = 0, i;
    for (i = 0; i < BufLen; i++)
        crc = update_crc8(Buf[i], crc);
    return (crc);
}

void makeTelemPackage(uint8_t temp, uint16_t voltage, uint16_t current, uint16_t consumption, uint16_t e_rpm)
{

    aTxBuffer[0] = (voltage >> 8) & 0xFF; // voltage hB
    aTxBuffer[1] = voltage & 0xFF; // voltage   lowB

    aTxBuffer[2] = (current >> 8) & 0xFF; // current
    aTxBuffer[3] = current & 0xFF; // divide by 10 for Amps

    aTxBuffer[4] = (consumption >> 8) & 0xFF; // consumption
    aTxBuffer[5] = consumption & 0xFF; //  in mah

    aTxBuffer[6] = (e_rpm >> 8) & 0xFF; //
    aTxBuffer[7] = e_rpm & 0xFF; // eRpM *100

    aTxBuffer[8] = temp; // temperature

    aTxBuffer[9] = get_crc8(aTxBuffer, 9);
}