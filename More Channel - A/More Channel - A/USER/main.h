#ifndef __MAIN_H
#define __MAIN_H
#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "includes.h"
#include "check.h"
#include "stmflash.h"
//#include "Emw3060.h"
#include "NeowayN21.h"
#include "w25qxx.h"
#include "FlashDivide.h"
#include "cJSON.h"

extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];
extern char CCID[20];

#define FLASH_SAVE_ADDR  0X0802DE00
#define FLASH_THREE_ADDR 0X0802FE00

//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

//设置任务优先级
#define LED_TASK_PRIO       			1 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void LED_task(void *pdata);

//设置任务优先级
#define HDMI_TASK_PRIO       			2 
//设置任务堆栈大小
#define HDMI_STK_SIZE  		    		1536
//任务堆栈	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//任务函数
void HDMI_task(void *pdata);

//设置任务优先级
#define SaveThree_TASK_PRIO       		5 
//设置任务堆栈大小
#define SaveThree_STK_SIZE  		    256
//任务堆栈	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//任务函数
void SaveThree_task(void *pdata);

//设置任务优先级
#define Upyun_TASK_PRIO       			7 
//设置任务堆栈大小
#define Upyun_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//任务函数
void Upyun_task(void *pdata);

//设置任务优先级
#define Work_TASK_PRIO       			8 
//设置任务堆栈大小
#define Work_STK_SIZE  		    		256
//任务堆栈	
__align(8)OS_STK Work_TASK_STK[Work_STK_SIZE];
//任务函数
void Work_task(void *pdata);

//设置任务优先级
#define Water_TASK_PRIO       			3 
//设置任务堆栈大小
#define Water_STK_SIZE  		    	64
//任务堆栈	
__align(8)OS_STK Water_TASK_STK[Water_STK_SIZE];
//任务函数
void Water_task(void *pdata);

#ifndef TIM
#define TIM
typedef struct
{
	u32	year;
	u8	month;
	u8	day;
	u8	week;
	u8	hour;
	u8	minute;
	u8	second;
}RTC_TIME;
#endif

RTC_TIME STIME;			//系统时间	

#ifndef ELog
#define ELog
typedef struct
{
	u8		HouseNum;
	u8		StartTime[5];
	u8		StartMode;
	u8		StopTime[5];	
	u8		StopMode;	
	u8		IrrMode;
	u16		Irrtime;
	u8		UseWater;
	u8		FerNum;
	u8		SFerusage;
	u8		PFerusage;
}SysLog;
#endif

SysLog	Log;			//系统日志

#ifndef Run_parameters
#define	Run_parameters
typedef struct
{
	u8	Mu;				//亩数
	u8	AverageMu;		//亩均施肥量
	u8	SpecialFer;		//专用桶
	u8	Pre;			//前置时间
	u8	Behind;			//后置时间
	u8	Terminaladdr;	//终端地址
	u8	TerminalNum;	//终端通道
	u8	a,b,c;
}Run_P;
#endif

Run_P	Run_SetMessage;

u8	Concentration[8];	//肥料浓度

#define	FerSwitch		PBin(12)				//吸肥开关
#define	FerState		PBin(13)				//吸肥状态

#define	Stir1Open		IO8SWITCH|=1			//搅拌1打开
#define	Stir1Close		IO8SWITCH&=0xFE			//搅拌1关闭
#define	Stir2Open		IO8SWITCH|=2			//搅拌2打开
#define	Stir2Close		IO8SWITCH&=0xFD			//搅拌2关闭
#define	Solenoid1Open	IO8SWITCH|=4			//电磁阀1打开
#define	Solenoid1Close	IO8SWITCH&=0xFB			//电磁阀1关闭
#define	Solenoid2Open	IO8SWITCH|=8			//电磁阀2打开
#define	Solenoid2Close	IO8SWITCH&=0xF7			//电磁阀2关闭

#define	FerRun			IO_OutSet(1,1)			//吸肥
#define	FerStop			IO_OutSet(1,0)			//不吸肥
#define OpenLED			IO_OutSet(2,1)			//打开灯
#define CloseLED		IO_OutSet(2,0)			//关闭灯

#define	ManualModel		0						//手动模式
#define	localModel		1						//本地模式
#define	NetworkModel	2						//网络模式
#define	PlanModel		3						//计划模式

#define	RunState		(MCGS_Button&0x01)		//启动按钮
#define	SuspendButton	(MCGS_Button&0x02)		//暂停模式
#define	FerSButton		(MCGS_Button&0x04)		//肥料桶保存
#define	HouseRButton	(MCGS_Button&0x08)		//棚设置读取
#define	HouseSButton	(MCGS_Button&0x10)		//棚设置保存
#define	CechkSelf		(MCGS_Button&0x20)		//自检状态
#define	LogButton		(MCGS_Button&0x40)		//日志按钮
#define	Details			(MCGS_Button&0x80)		//详情按钮

#define	CheckFer1		(MCGS_Check&0x01)		//自检肥料桶按钮
#define	CheckFer2		(MCGS_Check&0x02)
#define	CheckFer3		(MCGS_Check&0x04)
#define	CheckFer4		(MCGS_Check&0x08)
#define	CheckFer5		(MCGS_Check&0x10)
#define	CheckFer6		(MCGS_Check&0x20)
#define	CheckFer7		(MCGS_Check&0x40)
#define	CheckFer8		(MCGS_Check&0x80)
#define	CheckFerM		(MCGS_Check2&0x01)		//自检吸肥泵
#define	CheckSolenoid	(MCGS_Check2&0x02)		//自检分区
#define	CheckFlow		(MCGS_Check2&0x04)		//自检流量阀

#define	TimeButton		(MCGS_Partition&0x80)	//时间同步

#define	MCGS_SSID			10					//wifi名称
#define	MCGS_PASSWORD		74					//密码
#define	MCGS_IP				138					//IP地址
#define	MCGS_NETWORK		202					//子网掩码
#define	MCGS_GATEWAY		266					//网关
#define	MCGS_DNS			330					//DNS

u8	MCGS_Button=0,MCGS_Partition=0,MCGS_Fer=0;
u8	MCGS_Check=0,MCGS_Check2=0;					//触摸屏按键，触摸屏分区，肥料桶开启状态(用于报警) 自检按键
u8	HC_Partition,HC_IrrMode;					//触摸屏更新缓存

u8	IO8SWITCH=0;	//IO8输出状态
u8	MODEL=0;		//当前模式
u16	IrrTime=0;		//设置的灌溉时常
u16	Remaining=0;	//倒计时时间
u8	Net=0;			//是否为网络启动

u8	Irrsign=0;		//灌溉时长读取标志
u8	Remsign=0;		//倒计时更新标志
u8	Cursing=0;		//分区更新标志
u8	ReadNet=0;		//读取网络参数
u8	IO8Set=0;		//IO8更新标志
u8	Err=0;			//异常标志
u8	Error=0;		//异常类型
u32	LogP=0;			//Log存储地址标志
u8	ChoMode=1;		//启动模式选择
u8	GetTime=0;		//获取触摸屏时间标志
u8	LogPage=0;		//日志页码
u8	LogDe=0;		//日志详情号
u8	TimePopup=0;	//时间同步弹窗是否弹出
u8	SetRun=0;		//非本地启动重载标志
u16	htime=0;		//手动计时
u8	Logwait=0;		//等待日志写入

u8	HDMI_READ_START=0;	//读取运行所需参数标志
u8	HDMI_SET_STOP=0;	//触摸屏返回主界面
u8	stage=0;		//工作阶段
u8	PreTime=0;		//前置建立时间
u8	BehindTime=0;	//后置冲洗时间
u16 Actual_Water=0;	//实际用水量
u16 Actual_Fer=0;	//实际用肥量
u16 Actual_FerP=0;	//实际用肥量
u8	HouseNum=0;		//棚号
u8	PublicFer=0;	//公用桶选中
u8	checknum=0;		//自检电磁阀
u16	checkflow=0;	//自检流量
u16	Mathflow=0;		//目标流量
u16	NeedFlow=0;		//需要流量
u8	checkfer=0;		//自检肥料桶电磁阀
u32	water0=0;		//总水表
u32	water1=0;		//水表一
u32	water2=0;		//水表二
u32	thiswater0=0;	//本次开始用水量
u32	thiswater1=0;	//本次开始专用肥
u32	thiswater2=0;	//本次开始公用肥
u8	watertime=0;	//用于流量计算
u16	water1flow=0;	//水表一流量

u8	n21_set=1;		//N21初始化完成
u8	nbup=0;			//上传标志
u8	NET=0;			//网络控制标志

union	Logaddr
{
	u8	mem[4];
	u32	mem32;
}Logmem;			//当前日志地址

union	pressure
{
	float	f;
	u16		I[2];
}pr;

u8	LED_BZ=0,SysTime=0;		//LED显示切换标志，校时标志
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned int  up_time=15;	//数据上传频率

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];

u8	Nsendok=0;
u8	rema=0;					//运行倒计时标志
u8	ttm=0,upenv=0,up_state=0;					//分钟计时器，传感器上传时间计时，设备状态上传标志
OS_TMR   * tmr1;           						//软件定时器1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)	//软件定时器1回调函数
{
    if(++ttm==6)
	{
		htime++;ttm=0;							//
		if(RunState)nbup=1,Remaining--;			//Remsign=1;				//灌溉倒计时
	}
}

void	readflashthree()				//读取三元组
{
	char buf[150];
	char *msg=buf;
	u8 i;
	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,70);
	if(buf[1]=='K')
	{
		memset(ProductKey0,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey0[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName0,0,50);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceName0[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret0,0,50);
		msg=buf+78+4;for(i=0;*msg;msg++)	DeviceSecret0[i++]=*msg;
	}
}

void	sendflashthree()				//写入三元组
{
	LED_BZ=1;
	STMFLASH_Write(FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
	STMFLASH_Write(FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
	STMFLASH_Write(FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+82,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	readflashthree();
	LED_BZ=0;
}
void	SetIO8(u8 addr,u8 value);
#include "IO.h"
#include "Interactive.h"
#include "Work.h"
#endif
