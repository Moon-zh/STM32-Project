#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"
#include "UserCore.h"

typedef struct
{
	u8 CmdType;
}ZoneCommand;//分区IO命令

typedef struct
{	
	u8 Type;
	u8 TaskSta;
	u8 AskCnt;
	u8 RWSta;
	u8 Error;
	u8 ErrCnt;
}IOCtrlPara;//IO控制参数

typedef struct
{
	u8 WaterBefore;//前置偏移时间
	u8 WaterAfter;//后置偏移时间
	u8 WaterTime;//施肥浇水时长
	u8 DosingQty[5];//5种肥用量/流量
	u8 DosingSwitch;
	u32 ValveGroup;//因时间问题,只传数值1
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
	u8 PumpState;//0关1开
	u8 DosingSwitch;//施肥通道开关
	u8 DosingQty[5];//5种肥用量/流量
	u32 ValveGroup;//因时间问题,只传一个数值1
	u8 Alarm[22];
}  __attribute__((packed)) SlaveStateStruct;

//分区命令类型
#define START_CMD	0x01
#define STOP_CMD	0x02
#define ASK_CMD		0x03
#define CLEAR_CMD	0x04

//io控制参数读写状态
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


