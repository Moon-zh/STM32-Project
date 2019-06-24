#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_WARTERPROG				0x0000	//浇水程序起始地址
#define ADDR_DOSINGPROG				0x1000	//施肥程序起始地址
#define ADDR_IRRIGATIONPROG			0x2000	//灌溉程序起始地址
#define ADDR_FERTILIZERAMOUNT		0x3000	//用肥量起始地址
#define ADDR_FIXEDPARA				0x4000	//固定参数起始地址
#define ADDR_PARA					0x5000	//参数起始地址
#define ADDR_TASKRECORD   0x6000  //任务保存记录
#define ADDR_SETBUTONN    0X7000 //未完成任务地址
#define ADDR_UNCOMPLET    0X8000 //未完成任务地址
#define ADDR_PASSWORD     0X9000 //密码保存地址
#define ADDR_PARASET	  0xa000	//WiFi参数地址
#define ADDR_DHCPSET	  0xa0a0	//WiFi参数地址

void FlashReadWaterProg(u8* _pBuf,u8 addr,u8 Num);
void FlashWriteWaterProg(u8* _pBuf,u8 Num);
void FlashReadDosingProg(u8* _pBuf,u8 addr,u8 Num);
void FlashWriteDosingProg(u8* _pBuf,u8 Num);
void FlashReadIrrigationProg(u8* _pBuf,u8 addr,u8 Num);
void FlashWriteIrrigationProg(u8* _pBuf,u8 Num);
void FlashReadFertilizerAmount(u8* _pBuf,u8 addr,u8 Num);
void FlashWriteFertilizerAmount(u8* _pBuf,u8 Num);
void FlashReadFixedPara(u8* _pBuf,u8 addr,u8 Num);
void FlashWriteFixedPara(u8* _pBuf,u8 Num);
void FlashReadPara(u8* _pBuf,u8 addr,u8 Num);
void FlashWritePara(u8* _pBuf,u8 Num);
void FlashReadtaskrecord(u8* _pBuf,u32 addr,u16 Num);
void FlashWritetaskrecord(u8* _pBuf,u32 addr,u16 Num);
void FlashReadtargetecord(u8* _pBuf,u16 Num);
void FlashWritetargetrecord(u8* _pBuf,u16 Num);
void FlashReadWiFi(u8* _pBuf);
void FlashWriteWiFi(u8* _pBuf);
void FlashReadDHCP(u8* _pBuf);
void FlashWriteDHCP(u8* _pBuf);

#endif
