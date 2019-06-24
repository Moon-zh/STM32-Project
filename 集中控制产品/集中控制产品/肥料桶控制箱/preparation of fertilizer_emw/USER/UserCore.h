#ifndef __USERCORE_H
#define __USERCORE_H	

#include "sys.h"
#include "includes.h"  

#define MANUAL_MODE 	1//手动模式
#define LOCAL_MODE		2//本地模式
#define NET_MODE		3//网络模式

//WARTERADD状态
#define WARTERADD_IDLE				0//注水 空闲
#define WARTERADD_WAITESWITCH		1//注水 等电磁阀
//#define WARTERADD_WAITELEVEL		2//注水	等液位
#define WARTERADD_WAITEFULL			3//注水	水满
//#define WARTERADD_END				4//注水	结束
#define WATERADD_CANCEL				5//注水 取消
//WARTERADD消息
#define MSG_WATERADDSTART		1//注水
#define MSG_SWITCHOPEN			2//电磁阀打开
#define MSG_SWITCHERR			3//电磁阀失败
#define MSG_LEVEGET				4//获取到液位数据
#define MSG_LEVEERR				5//液位获取失败
#define MSG_WATERFULL			6//水满
#define MSG_WATERADDCANCEL		7//取消

//STIR状态
#define STIR_IDLE				0//搅拌 空闲
#define STIR_WATESWITCH_ON		1//搅拌 等电磁阀
#define STIR_VERIFYSWITCH_ON 	2//搅拌 验证电磁阀
#define STIR_WAITETIME 			3//搅拌 时间到
#define STIR_WATESWITCH_OFF		4//搅拌 等电磁阀
#define STIR_VERIFYSWITCH_OFF 	5//搅拌 验证电磁阀
#define STIR_END				6//搅拌 结束
//STIR 消息
#define MSG_STIRSTART			1//搅拌
#define MSG_SWITCHONOK			2//电磁阀开OK
#define MSG_SWITCHONERROR		3//电磁阀开错误
#define MSG_VERFYONOK			4//等待开验证 OK
#define MSG_VERFYONERR			5//等待开验证 ERR
#define MSG_TIMEOVER			6//时间到
#define MSG_SWITCHOFFOK			7//电磁阀关OK
#define MSG_SWITCHOFFERROR		8//电磁阀关错误
#define MSG_VERFYOFFOK			9//等待关验证 OK
#define MSG_VERFYOFFERR			10//等待关验证 ERR
#define MSG_STIRCANCEL			11//搅拌取消





#define GPRS_TASK_CODE 		2//网络通信任务	
#define ZONE_TASK_CODE 		3//分区管理任务
#define IO_TASK_CODE 		4//主板IO任务
#define SCREEN_TASK_CODE 	5//触摸屏任务
#define TIMER_INTER_CODE	6//定时器
#define ALARM_TASK_CODE		7//告警任务
#define WATERADD_TASK_CODE	8//
#define STIR_TASK_CODE		9//

//故障码
#define FAULT_SW1 0x01
#define FAULT_SW2 0x02
#define FAULT_SW3 0x04
#define FAULT_SW4 0x08
#define FAULT_SW5 0x10
#define FAULT_STIR1 0x20
#define FAULT_STIR2 0x40
#define FAULT_STIR3 0x80
#define FAULT_STIR4 0x100
#define FAULT_STIR5 0x200
#define FAULT_LEVEL1 0x400
#define FAULT_LEVEL2 0x800
#define FAULT_LEVEL3 0x1000
#define FAULT_LEVEL4 0x2000
#define FAULT_LEVEL5 0x4000

typedef struct
{
	u8 CmdType;
	u8 CmdSrc;
	u8 CmdData[2];
}MsgStruct;

extern u8 ControlMode ;//	控制模式：1：本地 0:网络
extern u8 WaterAddState1;
extern u8 WaterAddState2;
extern u8 WaterAddState3;
extern u8 WaterAddState4;
extern u8 WaterAddState5;

extern u8 StirState1;
extern u8 StirState2;
extern u8 StirState3;
extern u8 StirState4;
extern u8 StirState5;
extern u32 g_FaultCode;//故障码用于处理故障和报警使用

void WaterAdd_task(void *pdata);
void Stir_task(void *pdata);
u8 PackSendMasterQ(MsgStruct* MsgBlk);
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk);
u8 PackSendWaterAddQ(MsgStruct* MsgBlk);
u8 PackSendStirQ(MsgStruct* MsgBlk);

#endif

