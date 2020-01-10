#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  

#define FERTILIZER1TIME	10   //施肥1阶段的浇清水时间
#define FERTILIZER3TIME	10   //施肥3阶段的浇清水时间

#define MANUAL_MODE 1//手动模式
#define LOCAL_MODE	2//本地模式
#define NET_MODE	3//网络模式

//MASTER状态
#define SYSTEM_IDLE			0//空闲状态
#define SYSTEM_WATER 		1//工作状态:浇清水

#define SYSTEM_FERTILIZER1 		2//施肥1阶段
#define SYSTEM_FERTILIZER2 		3//施肥2阶段
#define SYSTEM_FERTILIZER3 		4//施肥3阶段

//消息
#define MSG_START			1//启动
#define MSG_STOP			2//停止
#define MSG_TIMEOVER		3//倒计时结束


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
	u8 CmdData[6];
}MsgStruct;

typedef struct
{
	u8 WorkModel; //1:浇水 2:施肥
	u8 Zone;	//分区 按位
	u8 TimeH;
	u8 TimeM;
	u8 ImplementZone;//正在执行的区域
	u8 FerTimeH;
	u8 FerTimeM;
}WorkPara;

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
u8 GetSysState(void);

void rct_dueishi_cx(void);//RCT对时程序  查询网络时间，并将时间同步至RTC，每24小时同步一次

#endif
