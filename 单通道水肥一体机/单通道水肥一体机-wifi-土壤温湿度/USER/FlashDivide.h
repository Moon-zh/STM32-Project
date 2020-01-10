#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  




#define ADDR_PARASET				0x3000	//WiFi������ַ
#define ADDR_IP			      	0x0040	//WIFI��IP��ַ
#define ADDR_SJ  		      	0x0042	//ʱ��ĵ�ַ
#define ADDR_DHCP           0x0043  //DHCP���ò�������ĵ�ַ
#define ADDR_ZDMS_QDYZ           0x0044  //�Զ�ģʽ�� ������ֵ
#define ADDR_ZDMS_TZYZ           0x0045  //�Զ�ģʽ�� ֹͣ��ֵ



void FlashReadWaterTime(u8* _pBuf);
void FlashWriteWaterTime(u8* _pBuf);

void FlashReadWiFi(u8* _pBuf);
void FlashWriteWiFi(u8* _pBuf);

void FlashReadIP(u8* _pBuf);
void FlashWriteIP(u8* _pBuf);

void FlashReadSJ(u8* _pBuf);
void FlashWriteSJ(u8* _pBuf);

void FlashReadDHCP(u8* _pBuf);
void FlashWriteDHCP(u8* _pBuf);



void FlashReadZDMS_QDYZ(u8* _pBuf);
void FlashWriteZDMS_QDYZ(u8* _pBuf);

void FlashReadZDMS_TZYZ(u8* _pBuf);
void FlashWriteZDMS_TZYZ(u8* _pBuf);

#endif
