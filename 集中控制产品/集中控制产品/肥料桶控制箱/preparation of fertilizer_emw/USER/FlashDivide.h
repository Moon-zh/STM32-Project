#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_WARTERTIME				0x0000	//��ȳ�����ʼ��ַ
#define ADDR_FERTILIZERTIME			0x1000	//����Դ����ʼ��ַ
#define ADDR_PERSENT				0x2000	//����Դ����ʼ��ַ
#define ADDR_LOW				0x3000	//����Դ����ʼ��ַ
#define ADDR_D				0x4000	//����Դ����ʼ��ַ
#define ADDR_PARASET		0x5000	//WiFi������ַ
#define ADDR_DHCPSET	  	0xa0a0	//WiFi������ַ

void FlashReadWaterTime(u8* _pBuf);
void FlashWriteWaterTime(u8* _pBuf);
void FlashReadFertilizerTime(u8* _pBuf);
void FlashWriteFertilizerTime(u8* _pBuf);
void FlashReadWaterADD(u8* _pBuf);
void FlashWriteWaterADD(u8* _pBuf);
void FlashReadStir(u8* _pBuf);
void FlashWriteStir(u8* _pBuf);
void FlashReadPersent(u8* _pBuf);
void FlashWritePersent(u8* _pBuf);
void FlashReadLow(u8* _pBuf);
void FlashWriteLow(u8* _pBuf);
void FlashReadD(u8* _pBuf);
void FlashWriteD(u8* _pBuf);
void FlashReadWiFi(u8* _pBuf);
void FlashWriteWiFi(u8* _pBuf);
void FlashReadDHCP(u8* _pBuf);
void FlashWriteDHCP(u8* _pBuf);
#endif
