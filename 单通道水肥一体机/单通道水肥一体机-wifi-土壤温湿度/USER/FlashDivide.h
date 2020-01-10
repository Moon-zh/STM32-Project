#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  




#define ADDR_PARASET				0x3000	//WiFi参数地址
#define ADDR_IP			      	0x0040	//WIFI的IP地址
#define ADDR_SJ  		      	0x0042	//时间的地址
#define ADDR_DHCP           0x0043  //DHCP设置参数保存的地址
#define ADDR_ZDMS_QDYZ           0x0044  //自动模式下 启动阈值
#define ADDR_ZDMS_TZYZ           0x0045  //自动模式下 停止阈值



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
