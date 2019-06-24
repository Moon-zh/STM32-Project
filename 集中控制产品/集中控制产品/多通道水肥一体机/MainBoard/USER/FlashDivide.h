#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_WARTERPROG				0x0000	//��ˮ������ʼ��ַ
#define ADDR_DOSINGPROG				0x1000	//ʩ�ʳ�����ʼ��ַ
#define ADDR_IRRIGATIONPROG			0x2000	//��ȳ�����ʼ��ַ
#define ADDR_FERTILIZERAMOUNT		0x3000	//�÷�����ʼ��ַ
#define ADDR_FIXEDPARA				0x4000	//�̶�������ʼ��ַ
#define ADDR_PARA					0x5000	//������ʼ��ַ
#define ADDR_TASKRECORD   0x6000  //���񱣴��¼
#define ADDR_SETBUTONN    0X7000 //δ��������ַ
#define ADDR_UNCOMPLET    0X8000 //δ��������ַ
#define ADDR_PASSWORD     0X9000 //���뱣���ַ
#define ADDR_PARASET	  0xa000	//WiFi������ַ
#define ADDR_DHCPSET	  0xa0a0	//WiFi������ַ

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
