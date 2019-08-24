#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"

typedef struct
{
	u8 Enable;
	u8 CtrlNum;
	u8 CtrlType;
}ZoneCommand;//����IO����

typedef struct
{
	u8 IdleSta;
	u8 RWSta;
	u8 Error;
}IOCtrlPara;//IO���Ʋ���

#define ENVIRDATA_MAXNUM	21
//io���Ʋ�����д״̬
#define READSTA 0x01
#define WRITESTA 0x02
extern const u8 auchCRCHi[];
extern const u8 auchCRCLo[];
void ZoneCtrl_task(void *pdata);
void SetIOstate(u8 Num);
u16 CRC16(u8* puchMsg, u16 usDataLen);
u8 ZoneCtrl_Set(u8 Type,u8 Num);
//u8 PackSendZoneQ(MsgStruct* MsgBlk);
extern s16 EnvirData[ENVIRDATA_MAXNUM];
extern u8 UnframeEnvironment(void);
extern u8 UnfreameGreenhouseCtrl(void);
extern void ReadEnvironment(void);
extern void ReadGreenhoudeCtrl(void);
#endif


