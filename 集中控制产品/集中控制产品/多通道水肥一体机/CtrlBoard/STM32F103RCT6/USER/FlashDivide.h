#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_WARTERTIME				0x0000	//��ȳ�����ʼ��ַ
#define ADDR_FERTILIZERTIME			0x1000	//����Դ����ʼ��ַ

void FlashReadWaterTime(u8* _pBuf);
void FlashWriteWaterTime(u8* _pBuf);
void FlashReadFertilizerTime(u8* _pBuf);
void FlashWriteFertilizerTime(u8* _pBuf);

#endif
