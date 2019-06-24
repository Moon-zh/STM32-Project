#ifndef __ALARM_H
#define __ALARM_H
#include "sys.h"
#include "includes.h"  
#include "UserCore.h"

#define ALARMMSG_LEVEL_OPEN 		0x01//Һλ�жϿ���
#define ALARMMSG_LEVEL_CLOSE 		0x02//Һλ�ж�ֹͣ
#define ALARMMSG_STIRTIME_OPEN	 	0x03//����ʱ���жϿ���
#define ALARMMSG_STIRTIME_CLOSE	 	0x04//����ʱ���ж�ֹͣ

#define ALARMCOLLPERIO	20//Һλ�ɼ�����20S
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
extern u16 NeedFertilizer[5];//��Ҫ��ӵķ���
extern u8 BucketPercent[5];//Ũ�ȱ���
extern u16 BucketLimit[5];//Һλ����

u8 PackSendAlarmQ(MsgStruct* MsgBlk);

void CaluculateFertilizer(void);
void GetFlashData(void);
void FertilizerCalculate(void);
void ParaApplication(void);

#endif
