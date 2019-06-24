#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  

#define MANUAL_MODE 1//手动模式
#define LOCAL_MODE	2//本地模式
#define NET_MODE	3//网络模式
//MASTER状态
#define WARTER_IDLE			0//浇水空闲状态
#define WARTER_WORK 		1//浇水工作状态
#define FERTILIZER_IDLE		0//施肥空闲
#define FERTILIZER_WORK		1//施肥工作
//消息
#define MSG_WARTERING		1//浇水
#define MSG_FERTILIZER		2//施肥
#define MSG_MODECHANGE		3//模式
#define MSG_RADIOTUBE		4//电磁阀
#define MSG_ZONEALARM		5//分区告警


#define USERCORE_TASK_CODE 	1//主任务
#define GPRS_TASK_CODE 		2//网络通信任务	
#define ZONE_TASK_CODE 		3//分区管理任务
#define IO_TASK_CODE 		4//主板IO任务
#define SCREEN_TASK_CODE 	5//触摸屏任务
#define TIMER_INTER_CODE	6//定时器

typedef struct
{
	u8 CmdType;
	u8 CmdSrc;
	u8 CmdData[2];
}MsgStruct;

typedef struct
{
	u8 PumpWFlag;	//水泵是否开启
	u8 PumpFFlag;	//肥泵是否开启
	u8 Passageway[6]; //通道阀位控制
	u8 Zone;		//分区阀控制
	u8 WorkHour;	//工作时长(h)
	u8 WorkMinute;	//工作时长(m)
	u8 WorkDay; 	//工作周几
	u8 StartHour;	//开始小时
	u8 StartMinute; //开始分钟
}
StrategyStruct;

extern u16 WarterRemainderTime ;//浇水剩余工作时间
extern u16 FertilizerRemainderTime ;//施肥剩余工作时间
extern u8 ZoneAlarm[2];

//extern u8 ModbusCoil[MAX_COIL_NUM/8];//modbus线圈，共40个

void MasterCtrl_task(void *pdata);
u8 SetStrategy(StrategyStruct *Strategy);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);

#endif
