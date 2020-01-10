#ifndef __CRC16_H
#define __CRC16_H

#include	"sys.h"


uint16_t crc16bitbybit(uint8_t *ptr, uint16_t len);
uint16_t crc16table(uint8_t *ptr, uint16_t len);
uint16_t crc16tablefast(uint8_t *ptr, uint16_t len);



#endif

