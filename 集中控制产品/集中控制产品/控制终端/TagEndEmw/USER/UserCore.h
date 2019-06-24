#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  
#define STATE_START		            0x00
#define STATE_1		                0x01
#define STATE_2		                0x02
#define STATE_3		                0x03
#define STATE_4		                0x04
#define STATE_END		            0x05
#define CMD_RETURN_REAL_TIME_DATA      0x06
#define MANUAL_MODE 1//手动模式
#define LOCAL_MODE	2//本地模式
#define NET_MODE	3//网络模式
//MASTER状态
#define MASTER_IDLE		0//空闲
#define MASTER_WORK		1//工作
#define MASTER_ERROR	2//异常
#define MASTER_END		3//结束
#define MASTER_CANCLE	4//取消

//消息
#define MSG_START      	0X10  	//任务启动消息
#define MSG_NEXTTIME	0X11 	//下一次
#define MSG_ALARM		0X12	//报警信息
#define MSG_ERROR		0X13	//错误信息
#define MSG_STOP		0X14	//停止消息	
#define MSG_END			0X15	//结束消息	
#define MSG_SINGEND		0X16	//单次任务结束消息
#define MSG_CANCLEOK	0X17	//取消完成消息


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
	u8 Before;//施肥前浇水时间
	u8 Total;//总浇水时间
	u8 After;//施肥后浇水时间
}WaterProgStruct;
typedef struct
{
	u8 Ch1Ratio;
	u8 Ch2Ratio;
	u8 Ch3Ratio;
	u8 Ch4Ratio;
	u8 Ch5Ratio;
}DosingProgStruct;
typedef struct
{
	u8  Fertilizer1Percent;
	u8  Fertilizer2Percent;
	u8  Fertilizer3Percent;
	u8  Fertilizer4Percent;
	u8  Fertilizer5Percent;
}PercentStruct;
typedef struct
{
	//按照天设置
	u32 ActionDay;
	u8 ActionType[32];	//32天每天的动作类型 浇水 施肥 无动作
	//按照8容量设计
	u8 ValveGroup[8];	//灌溉组
	u8 RunTimeNum[8];	//浇水程序号
	u8 DosingProg[8];	//施肥程序号
	//按照4容量设计
	u8 StartTime[4];//开始时间
	u8 CycleTimes[4];//循环次数
	u8 CyclePperiod[4];//循环周期
}ProgramStruct;
typedef struct
{
	u8 ActionTypeIndex;//动作类型索引
	
	u8 ValveGroupIndex;//灌溉组索引
	u8 RunTimeNumIndex;//浇水程序号索引
//	u8 DosingProgIndex;//施肥程序号索引

	u8 StartTimeIndex;//开始时间索引
	u8 CycleTimesIndex;//循环次数索引
	u8 CyclePperiodIndex;//循环周期索引

	u8 WaterProgIndex;//浇水程序号索引
	u8 DosingProgIndex;//配肥程序号索引
	u8 IrrigationProgIndex;//灌溉程序号索引
}MasterStruct;

extern u16 Hmi_Buf[10];
extern u8 Master_State;
void MasterCtrl_task(void *pdata);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);

#endif
