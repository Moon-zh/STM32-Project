#ifndef __ALARM_H
#define __ALARM_H
#include "sys.h"
#include "includes.h"  
#include "UserCore.h"

#define ALARMMSG_LEVEL_OPEN 		0x01//液位判断开启
#define ALARMMSG_LEVEL_CLOSE 		0x02//液位判断停止
#define ALARMMSG_STIRTIME_OPEN	 	0x03//搅拌时间判断开启
#define ALARMMSG_STIRTIME_CLOSE	 	0x04//搅拌时间判断停止

#define ALARMCOLLPERIO	20//液位采集周期20S
typedef struct
{
	u8 LevelFlag;
	u16 DestLevel;
	u16 LastLevel;
}LevelParam;

extern u8 	StartLevelColl[5];
extern u8	_20SecCnt[5];
extern u8 	CollPerioF[5];

extern u8 	StartStirTime[5];
extern u8  _60SecCnt[5];
extern u8 	StirTimeF[5];

extern u16 	LevelData[5];
extern u8 	StirTimeMin[5];

extern u16  SetLevelValue[5];
extern u8  SetStirTime[5];
extern u16 NeedFertilizer[5];//需要添加的肥料
extern u8 BucketPercent[5];//浓度比例
extern u16 BucketLimit[5];//液位限制

u8 PackSendAlarmQ(MsgStruct* MsgBlk);

void CaluculateFertilizer(void);
void GetFlashData(void);
void FertilizerCalculate(void);
void ParaApplication(void);

#endif
