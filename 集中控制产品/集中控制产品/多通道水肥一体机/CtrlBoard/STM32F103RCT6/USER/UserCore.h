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
//MOCHINE状态机
#define MOCHINE_IDLE    0 //机器空闲状态
#define MOCHINE_ACTION  1 //机器执行状态
#define MOCHINE_ALARM   2 //机器报警状态
#define MOCHINE_STOP    3 //机器停止状态
//消息
#define MSG_START      0X10  //任务启动消息
#define MSG_ACTION_WATER_ON      0X11  // 浇水电磁阀开启
#define MSG_ACTION_WATER_OFF      0X12  // 浇水电磁阀关闭
#define MSG_ACTION_ERTILIZER_ON      0X13  // 施肥电磁阀开启
#define MSG_ACTION_ERTILIZER_OFF      0X14  //施肥电磁阀关闭
#define MSG_ALARM			      0X15						//报警信息
#define MSG_STOP			      0X16						//停止消息	
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
#define IO_SEND_COUNT_MAX    20 //暂定为1000次 调试用
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
enum MOCHINEerrorstate
{
		MOCH_E_STATE_START_ON =1, //启动失败  
		MOCH_E_STATE_WATER_ON =2, //灌水开关启动失败
		MOCH_E_STATE_FERT_ON  =3, //施肥开启失败
		MOCH_E_STATE_FLOW_ON  =4 //流量计读取错误


};
typedef struct
{
u8 FertilizerOnID; //施肥泵开启ID 看8路IO设置的ID 因为只对应一个模块在设备组装完成之后可以将其固定
u8 FertilizerIoNumber;//施肥电磁阀要开启的路数 从1-5按顺序开启
u8 FertilizerIoStatrNum; //当前开启施肥泵的路数
u8 FertilizerIoSucess; //施肥阀开启成功开始计时
u8 WaterOnID;			//对应的要开启的棚区浇水ID 4路IO的ID
u8 WaterOnSucess; //浇水阀开启成功开始计时
u16 Fertilizertime; //施肥总时间分钟
u16 FertilizerCurrenttime;//当前施肥总时间
u16 FertilizerCurrenttimeing;//施肥进行的时间	
u16 WaterTime;		//浇水总时间
u16 WaterTimeing;	//浇水进行的时间 
u8 Io_on_Off_Sucess;//io开启循环标志
u8 Io_Step; //io开启步骤	 由于共用同一个485只能一个一个开启
u32	lastFlowmeter1;//流量计1的数值 要上传
u32	lastFlowmeter2;//流量计2的数值 要上传
u32	lastFlowmeter3;//流量计3的数值 要上传
u32	lastFlowmeter4;//流量计4的数值 要上传
u32	lastFlowmeter5;//流量计5的数值 要上传
u16 u16MOCHINEerrorstate;	//系统错误状态
}
MOCHINEStruct;
extern u16 WarterRemainderTime ;//浇水剩余工作时间
extern u16 FertilizerRemainderTime ;//施肥剩余工作时间
extern u8 ZoneAlarm[2];

//extern u8 ModbusCoil[MAX_COIL_NUM/8];//modbus线圈，共40个

void MasterCtrl_task(void *pdata);
u8 SetStrategy(StrategyStruct *Strategy);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);

#endif
