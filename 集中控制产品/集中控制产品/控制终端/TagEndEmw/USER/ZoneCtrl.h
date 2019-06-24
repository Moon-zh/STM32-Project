#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"
#include "UserCore.h"

typedef struct
{
	u8 CmdType;
}ZoneCommand;//����IO����

typedef struct
{	
	u8 Type;
	u8 TaskSta;
	u8 AskCnt;
	u8 RWSta;
	u8 Error;
	u8 ErrCnt;
}IOCtrlPara;//IO���Ʋ���

typedef struct
{
	u8 WaterBefore;//ǰ��ƫ��ʱ��
	u8 WaterAfter;//����ƫ��ʱ��
	u8 WaterTime;//ʩ�ʽ�ˮʱ��
	u8 DosingQty[5];//5�ַ�����/����
	u8 DosingSwitch;
	u32 ValveGroup;//��ʱ������,ֻ����ֵ1
}__attribute__((packed)) SlaveStruct;

typedef struct
{
	u8 State;
	u8 WaterBeforehour;
	u8 WaterBeforemin;
	u8 WaterBeforesec;
	u8 WaterAfterhour;
	u8 WaterAftermin;
	u8 WaterAftersec;
	u8 WaterTimehour;
	u8 WaterTimemin;
	u8 WaterTimesec;
	u8 PumpState;//0��1��
	u8 DosingSwitch;//ʩ��ͨ������
	u8 DosingQty[5];//5�ַ�����/����
	u32 ValveGroup;//��ʱ������,ֻ��һ����ֵ1
	u8 Alarm[22];
}  __attribute__((packed)) SlaveStateStruct;

//������������
#define START_CMD	0x01
#define STOP_CMD	0x02
#define ASK_CMD		0x03
#define CLEAR_CMD	0x04

//io���Ʋ�����д״̬
#define READSTA 0x01
#define WRITESTA 0x02
extern const u8 auchCRCHi[];
extern const u8 auchCRCLo[];
extern SlaveStruct SlavePara;
void ZoneCtrl_task(void *pdata);
void IO_READ_Input_Data(u8 id);
void IO_RESETALL_DATA(u8 id);
void IO_SET_DATA(u8 id,u8 dataadress,u8 action);
void IO_READ_Input_Data2(u8 id);
//void SetIOstate(u8 Num);
u16 CRC16(u8* puchMsg, u16 usDataLen);
void IO_RESETALL_DATA2(u8 id);
u8 PackSendZoneQ(MsgStruct* MsgBlk);
//u8 ZoneCtrl_Set(u8 Type,u8 Num);

#endif


