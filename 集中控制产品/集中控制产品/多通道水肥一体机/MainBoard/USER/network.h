#ifndef NETWORK_H
#define NETWORK_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"

typedef struct
{
	u16 pre_flushtime;//前置冲洗时间
	u16 fer_irrtimr; //施肥时间
	u16 post_flushtimr;	//后置冲洗时间
	u16 AferPer;	//A肥百分比
	u16 BferPer;	//B肥百分比
	u16 CferPer;	//C肥百分比
	u16 DferPer;	//D肥百分比
  	u16 are_fer;	//亩均用肥KG
}__attribute__((packed))IrrProStruct;
typedef struct
{
	u16 PartitionState[16];//16分区状态
	u16 DeviceState;//设备状态 
	u16 TimeCountDown;//倒计时
	u16 IrrProj;//灌溉程序号
	u16 BucketFerPer[4];	//肥料罐内肥料浓度
	u16 PartitionAre[16]; //分区面积
	//u16 post_flushtimr;	//任务开始月
	IrrProStruct CFIP;	//任务开始月
	IrrProStruct S1IP;	//任务开始时
	IrrProStruct S2IP;	//任务开始分
	IrrProStruct S3IP;	//任务开始秒
  	IrrProStruct S4IP;//灌溉模式
}__attribute__((packed))NetReadStruct;

typedef union
{
	NetReadStruct NetRead;
	s16 Remote_Read[79];
}__attribute__((packed))RemoteReadStruct;
typedef struct
{
	u16 PartitionEnable[16];
	u16 IrrType;
	u16 partitionSel[16];
	u16 IrrTime;
	u16 IrrPioj;
	u16 Switch;
}__attribute__((packed))NetSetStruct;

typedef union
{
	NetSetStruct NetSet;
	s16 Remote_Para[36];
}__attribute__((packed))RemoteSetStruct;

extern const u8* SetText[];
extern const u8* ReadText[];
extern RemoteReadStruct ReRead;
extern RemoteSetStruct ReSet;

#endif
