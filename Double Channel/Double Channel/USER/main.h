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
#include "Emw3060.h"
#include "w25qxx.h"
#include "FlashDivide.h"
#include "cJSON.h"


#define 	CMD_AIRTEMP		0x01
#define		CMD_AIRHUMI 	0x00
#define		CMD_SOILTEMP	0x03
#define		CMD_SOILHUMI	0x12
#define		CMD_CO2			0x05
#define 	CMD_LIGTH		0x07
#define		CMD_EC			0x15
#define		EnvNum			2				//传感器最大上限
#define 	sensor_num		1				//传感器数量

extern char ProductKey1[20];
extern char DeviceName1[50];
extern char DeviceSecret1[50];

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
#define HDMI_STK_SIZE  		    		256
//任务堆栈	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//任务函数
void HDMI_task(void *pdata);

//设置任务优先级
#define IO_TASK_PRIO       				3 
//设置任务堆栈大小
#define IO_STK_SIZE  		    		128
//任务堆栈	
OS_STK IO_TASK_STK[IO_STK_SIZE];
//任务函数
void IO_task(void *pdata);

//设置任务优先级
#define Collection_TASK_PRIO       		4 
//设置任务堆栈大小
#define Collection_STK_SIZE  		    128
//任务堆栈	
OS_STK Collection_TASK_STK[Collection_STK_SIZE];
//任务函数
void Collection_task(void *pdata);

//设置任务优先级
#define SaveThree_TASK_PRIO       		5 
//设置任务堆栈大小
#define SaveThree_STK_SIZE  		    256
//任务堆栈	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//任务函数
void SaveThree_task(void *pdata);

//设置任务优先级
#define Model_TASK_PRIO       			6 
//设置任务堆栈大小
#define Model_STK_SIZE  		    	256
//任务堆栈	
__align(8)OS_STK Model_TASK_STK[Model_STK_SIZE];
//任务函数
void Model_task(void *pdata);

//设置任务优先级
#define UpyunWF_TASK_PRIO       		7 
//设置任务堆栈大小
#define UpyunWF_STK_SIZE  		    	1024
//任务堆栈	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//任务函数
void UpyunWF_task(void *pdata);

//设置任务优先级
#define Work_TASK_PRIO       			8 
//设置任务堆栈大小
#define Work_STK_SIZE  		    		256
//任务堆栈	
__align(8)OS_STK Work_TASK_STK[Work_STK_SIZE];
//任务函数
void Work_task(void *pdata);

//设置任务优先级
#define Plan_TASK_PRIO       			9 
//设置任务堆栈大小
#define Plan_STK_SIZE  		    		256
//任务堆栈	
OS_STK Plan_TASK_STK[Plan_STK_SIZE];
//任务函数
void Plan_task(void *pdata);

//设置任务优先级
#define Sewage_TASK_PRIO       			11 
//设置任务堆栈大小
#define Sewage_STK_SIZE  		    	256
//任务堆栈	
OS_STK Sewage_TASK_STK[Plan_STK_SIZE];
//任务函数
void Sewage_task(void *pdata);

#ifndef Env
#define Env
typedef struct
{
	u8	IO1;
	u8	IO2;
	u8	IO3;
	u8	IO4;
}IOSET;
#endif
IOSET IOSTATE;

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

#ifndef PLA
#define PLA
typedef struct
{
	u8	month;
	u8	day;
	u8	hour;
	u8	minute;
	u8	IrrMode;
	u8	Partition;
	u16	Irrtime;
	u8	State;
}RPLAN;
#endif

RPLAN	Plan;	

#ifndef Envc
#define Envc
typedef struct
{
	u16		airtemp;	//转为十进制后需要/10
	u16		airhumi;	//转为十进制后需要/10
	u16		soiltemp;	//转为十进制后需要/10
	u16		soilhumi;	//转为十进制后需要/10
	u16		CO2;
	u32		light;
	u16		EC;
}Environmental;
#endif

Environmental sensor[EnvNum];//传感器

#ifndef ELog
#define ELog
typedef struct
{
	u8		StartTime[5];	
	u8		StartMode;	
	u8		IrrMode;	
	u8		Partition;
	u16		Irrtime;
	u8		StopTime[5];
	u8		StopMode;
	u16		RemTime;
}SysLog;
#endif

SysLog	Log;			//系统日志

#define	FerState		PBin(12)				//吸肥状态
#define	Scram			PBin(13)				//急停
#define PressSwitch		PBin(15)				//压力开关
//#define	StirSwitch1		(IO8STATE&0x01)			//搅拌1开关
//#define	StirState1		(IO8STATE&0x02)			//搅拌1反馈
//#define	StirSwitch2		(IO8STATE&0x04)			//搅拌2开关
//#define	StirState2		(IO8STATE&0x08)			//搅拌2反馈
//#define	SolenoidSwitch1	(IO8STATE&0x10)			//电磁阀1开关
//#define	SolenoidState1	(IO8STATE&0x20)			//电磁阀1反馈
//#define	SolenoidSwitch2	(IO8STATE&0x40)			//电磁阀2开关
//#define	SolenoidState2	(IO8STATE&0x80)			//电磁阀2反馈

#define	StirState1		(IO8STATE&0x01)			//搅拌1反馈
#define	StirState2		(IO8STATE&0x02)			//搅拌2反馈
#define	SolenoidState1	(IO8STATE&0x04)			//电磁阀1反馈
#define	SolenoidState2	(IO8STATE&0x08)			//电磁阀2反馈
#define	SolenoidState3	(IO8STATE&0x10)			//电磁阀1开关
#define	SolenoidState4	(IO8STATE&0x20)			//电磁阀1反馈
#define	SolenoidState5	(IO8STATE&0x40)			//电磁阀2开关
#define	SolenoidState6	(IO8STATE&0x80)			//电磁阀2反馈

#define	Stir1Open		IO8SWITCH|=1			//搅拌1打开
#define	Stir1Close		IO8SWITCH&=0xFE			//搅拌1关闭
#define	Stir2Open		IO8SWITCH|=2			//搅拌2打开
#define	Stir2Close		IO8SWITCH&=0xFD			//搅拌2关闭
#define	Solenoid1Open	IO8SWITCH|=4			//电磁阀1打开
#define	Solenoid1Close	IO8SWITCH&=0xFB			//电磁阀1关闭
#define	Solenoid2Open	IO8SWITCH|=8			//电磁阀2打开
#define	Solenoid2Close	IO8SWITCH&=0xF7			//电磁阀2关闭
#define	Solenoid3Open	IO8SWITCH|=0x10			//电磁阀3打开
#define	Solenoid3Close	IO8SWITCH&=0xEF			//电磁阀3关闭
#define	Solenoid4Open	IO8SWITCH|=0x20			//电磁阀4打开
#define	Solenoid4Close	IO8SWITCH&=0xDF			//电磁阀4关闭
#define	Solenoid5Open	IO8SWITCH|=0x40			//电磁阀5打开
#define	Solenoid5Close	IO8SWITCH&=0xBF			//电磁阀5关闭
#define	Solenoid6Open	IO8SWITCH|=0x80			//电磁阀6打开
#define	Solenoid6Close	IO8SWITCH&=0x7F			//电磁阀6关闭

#define	FerRun			IO_OutSet(1,1)			//吸肥
#define	FerStop			IO_OutSet(1,0)			//不吸肥
#define OpenLED			IO_OutSet(2,1)			//打开灯
#define CloseLED		IO_OutSet(2,0)			//关闭灯
#define	SewageOpen		IO_OutSet(4,1)			//打开排污
#define	SewageClose		IO_OutSet(4,0)			//关闭排污

#define	ManualModel		0						//手动模式
#define	localModel		1						//本地模式
#define	NetworkModel	2						//网络模式
#define	PlanModel		3						//计划模式

#define	ManualButton	(MCGS_Button&0x01)		//手动按钮
#define	IrrMethod		(MCGS_Button&0x02)		//灌溉模式
#define	WifiButton		(MCGS_Button&0x04)		//wifi开关
#define	StartButton		(MCGS_Button&0x08)		//启动按钮
#define	PlanButton		(MCGS_Button&0x10)		//计划按钮
#define	RunState		(MCGS_Button&0x20)		//运行状态
#define	LogButton		(MCGS_Button&0x40)		//日志按钮
#define	SaveButton		(MCGS_Button&0x80)		//保存按钮

#define	ChoPartition1	(MCGS_Partition&0x01)	//分区一选择
#define	ChoPartition2	(MCGS_Partition&0x02)	//分区二选择
#define	ChoPartition3	(MCGS_Partition&0x04)	//分区三选择
#define	ChoPartition4	(MCGS_Partition&0x08)	//分区四选择
#define	ChoPartition5	(MCGS_Partition&0x10)	//分区五选择
#define	ChoPartition6	(MCGS_Partition&0x20)	//分区六选择
#define	DHCPButton		(MCGS_Partition&0x40)	//DHCP
#define	TimeButton		(MCGS_Partition&0x80)	//时间同步

#define	Fer2M			(MCGS_Fer2S&0x02)		//二号桶搅拌电机
#define	Auto			(MCGS_Fer2S&0x04)		//自动开关

#define	Manual_Par1		(MCGS_Manual&0x01)
#define	Manual_Par2		(MCGS_Manual&0x02)
#define	Manual_Par3		(MCGS_Manual&0x04)
#define	Manual_Par4		(MCGS_Manual&0x08)
#define	Manual_Par5		(MCGS_Manual&0x10)
#define	Manual_Par6		(MCGS_Manual&0x20)
#define	Manual_Fer1		(MCGS_Manual&0x40)
#define	Manual_Fer2		(MCGS_Manual&0x80)

#define	MCGS_SSID			10					//wifi名称
#define	MCGS_PASSWORD		74					//密码
#define	MCGS_IP				138					//IP地址
#define	MCGS_NETWORK		202					//子网掩码
#define	MCGS_GATEWAY		266					//网关
#define	MCGS_DNS			330					//DNS

u8	MCGS_Button=0,MCGS_Partition=0,MCGS_Fer2S=0,MCGS_Manual,MCGS_FM;	//触摸屏按键，触摸屏分区
u8	HC_Partition,HC_IrrMode;					//触摸屏更新缓存

u8	SOILERROR=0;	//土壤传感器错误标识
u8	AIRERROR=0;		//空气传感器错误标识
u8	IO8STATE=0;		//IO8光耦状态
u8	IO8SWITCH=0;	//IO8输出状态
u8	MODEL=1;		//当前模式
u16	IrrTime=0;		//设置的灌溉时常
u16	Remaining=0;	//倒计时时间
u8	Net=0;			//是否为网络启动
u8	Current=0;		//分区

u8	Irrsign=0;		//灌溉时常读取标志
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
u8	TimePopup=0;	//时间同步弹窗是否弹出
u8	ReadPlan=0;		//读取计划
u16	PlanAddr=0;		//计划地址
u8	Planing=0;		//任务执行中
u8	SetRun=0;		//非本地启动重载标志
u16	htime=0;		//手动计时
u8	Logwait=0;		//等待日志写入
u8	Humiup=0;		//湿度上限
u8	Humidown=0;		//湿度下限
u8	sewage_space=0;	//排污间隔
u8	sewage_time=0;	//排污时长
u8	space=0;		//间隔及时
u8	sw_time=0;		//时长计时
u16	EC=0;			//目标EC
u16	NeedFlow=0;		//目标流量
u8	FerB=0;			//吸肥标志

union	Logaddr
{
	u8	mem[4];
	u32	mem32;
}Logmem;			//当前日志地址

u8	LED_BZ=0,SysTime=0;		//LED显示切换标志，校时标志
unsigned char printf_num=1;	//printf 串口指向标志 1指向uart1 2指向uart2
unsigned char Emw_B=0;		//wifi模组初始化完成标志
unsigned char emw_set=1;	//wifi报警与线程互斥标志
unsigned int  up_time=15;	//数据上传频率

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];

u8	waitPartition[6];		//分区排序列表

extern 	char ssid[20];
extern 	char password[20];
extern 	char ip[20];
extern 	char network[20];
extern 	char gateway[20];
extern 	char dns[20];
extern 	u8 DHCP;

void	Uptoaliyun_wifi_Env(Environmental data,u8 group);
//void	sendflash(void);
//void	readflash(void);

u8	rema=0;					//运行倒计时标志
u8	ttm=0,upenv=0,up_state=0;					//分钟计时器，传感器上传时间计时，设备状态上传标志
OS_TMR   * tmr1;           						//软件定时器1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)	//软件定时器1回调函数
{
    if(++ttm==6)
	{
		htime++;upenv++;ttm=0;					//传感器，系统计时
		if(rema)Remaining--,Remsign=1;			//灌溉倒计时
		space++;
	}
	sw_time+=10;
}

void	readflashthree()				//读取三元组
{
	char buf[150];
	char *msg=buf;
	u8 i;
	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,70);
	if(buf[1]=='K')
	{
		memset(ProductKey1,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey1[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName1,0,50);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceName1[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret1,0,50);
		msg=buf+78+4;for(i=0;*msg;msg++)	DeviceSecret1[i++]=*msg;
	}
}

void	sendflashthree()				//写入三元组
{
	LED_BZ=1;
	STMFLASH_Write(FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+4,(u16*)ProductKey1,sizeof(ProductKey1));
	STMFLASH_Write(FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+28,(u16*)DeviceName1,sizeof(DeviceName1));
	STMFLASH_Write(FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(FLASH_THREE_ADDR+82,(u16*)DeviceSecret1,sizeof(DeviceSecret1));
	readflashthree();
	LED_BZ=0;
}

#include "Sensor.h"
#include "Interactive.h"
#include "IO.h"
void	Sewage(u8 i)
{
	if(i)SewageOpen;
	else SewageClose;
}
#endif
