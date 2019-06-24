#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_WARTERTIME				0x0000	//灌溉程序起始地址
#define ADDR_FERTILIZERTIME			0x1000	//程序源码起始地址

void FlashReadWaterTime(u8* _pBuf);
void FlashWriteWaterTime(u8* _pBuf);
void FlashReadFertilizerTime(u8* _pBuf);
void FlashWriteFertilizerTime(u8* _pBuf);

#endif
