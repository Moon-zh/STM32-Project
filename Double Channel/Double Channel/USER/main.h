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
#define		EnvNum			2				//�������������
#define 	sensor_num		1				//����������

extern char ProductKey1[20];
extern char DeviceName1[50];
extern char DeviceSecret1[50];

#define FLASH_SAVE_ADDR  0X0802DE00
#define FLASH_THREE_ADDR 0X0802FE00

//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//�����������ȼ�
#define LED_TASK_PRIO       			1 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void LED_task(void *pdata);

//�����������ȼ�
#define HDMI_TASK_PRIO       			2 
//���������ջ��С
#define HDMI_STK_SIZE  		    		256
//�����ջ	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//������
void HDMI_task(void *pdata);

//�����������ȼ�
#define IO_TASK_PRIO       				3 
//���������ջ��С
#define IO_STK_SIZE  		    		128
//�����ջ	
OS_STK IO_TASK_STK[IO_STK_SIZE];
//������
void IO_task(void *pdata);

//�����������ȼ�
#define Collection_TASK_PRIO       		4 
//���������ջ��С
#define Collection_STK_SIZE  		    128
//�����ջ	
OS_STK Collection_TASK_STK[Collection_STK_SIZE];
//������
void Collection_task(void *pdata);

//�����������ȼ�
#define SaveThree_TASK_PRIO       		5 
//���������ջ��С
#define SaveThree_STK_SIZE  		    256
//�����ջ	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//������
void SaveThree_task(void *pdata);

//�����������ȼ�
#define Model_TASK_PRIO       			6 
//���������ջ��С
#define Model_STK_SIZE  		    	256
//�����ջ	
__align(8)OS_STK Model_TASK_STK[Model_STK_SIZE];
//������
void Model_task(void *pdata);

//�����������ȼ�
#define UpyunWF_TASK_PRIO       		7 
//���������ջ��С
#define UpyunWF_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//������
void UpyunWF_task(void *pdata);

//�����������ȼ�
#define Work_TASK_PRIO       			8 
//���������ջ��С
#define Work_STK_SIZE  		    		256
//�����ջ	
__align(8)OS_STK Work_TASK_STK[Work_STK_SIZE];
//������
void Work_task(void *pdata);

//�����������ȼ�
#define Plan_TASK_PRIO       			9 
//���������ջ��С
#define Plan_STK_SIZE  		    		256
//�����ջ	
OS_STK Plan_TASK_STK[Plan_STK_SIZE];
//������
void Plan_task(void *pdata);

//�����������ȼ�
#define Sewage_TASK_PRIO       			11 
//���������ջ��С
#define Sewage_STK_SIZE  		    	256
//�����ջ	
OS_STK Sewage_TASK_STK[Plan_STK_SIZE];
//������
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

RTC_TIME STIME;			//ϵͳʱ��

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
	u16		airtemp;	//תΪʮ���ƺ���Ҫ/10
	u16		airhumi;	//תΪʮ���ƺ���Ҫ/10
	u16		soiltemp;	//תΪʮ���ƺ���Ҫ/10
	u16		soilhumi;	//תΪʮ���ƺ���Ҫ/10
	u16		CO2;
	u32		light;
	u16		EC;
}Environmental;
#endif

Environmental sensor[EnvNum];//������

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

SysLog	Log;			//ϵͳ��־

#define	FerState		PBin(12)				//����״̬
#define	Scram			PBin(13)				//��ͣ
#define PressSwitch		PBin(15)				//ѹ������
//#define	StirSwitch1		(IO8STATE&0x01)			//����1����
//#define	StirState1		(IO8STATE&0x02)			//����1����
//#define	StirSwitch2		(IO8STATE&0x04)			//����2����
//#define	StirState2		(IO8STATE&0x08)			//����2����
//#define	SolenoidSwitch1	(IO8STATE&0x10)			//��ŷ�1����
//#define	SolenoidState1	(IO8STATE&0x20)			//��ŷ�1����
//#define	SolenoidSwitch2	(IO8STATE&0x40)			//��ŷ�2����
//#define	SolenoidState2	(IO8STATE&0x80)			//��ŷ�2����

#define	StirState1		(IO8STATE&0x01)			//����1����
#define	StirState2		(IO8STATE&0x02)			//����2����
#define	SolenoidState1	(IO8STATE&0x04)			//��ŷ�1����
#define	SolenoidState2	(IO8STATE&0x08)			//��ŷ�2����
#define	SolenoidState3	(IO8STATE&0x10)			//��ŷ�1����
#define	SolenoidState4	(IO8STATE&0x20)			//��ŷ�1����
#define	SolenoidState5	(IO8STATE&0x40)			//��ŷ�2����
#define	SolenoidState6	(IO8STATE&0x80)			//��ŷ�2����

#define	Stir1Open		IO8SWITCH|=1			//����1��
#define	Stir1Close		IO8SWITCH&=0xFE			//����1�ر�
#define	Stir2Open		IO8SWITCH|=2			//����2��
#define	Stir2Close		IO8SWITCH&=0xFD			//����2�ر�
#define	Solenoid1Open	IO8SWITCH|=4			//��ŷ�1��
#define	Solenoid1Close	IO8SWITCH&=0xFB			//��ŷ�1�ر�
#define	Solenoid2Open	IO8SWITCH|=8			//��ŷ�2��
#define	Solenoid2Close	IO8SWITCH&=0xF7			//��ŷ�2�ر�
#define	Solenoid3Open	IO8SWITCH|=0x10			//��ŷ�3��
#define	Solenoid3Close	IO8SWITCH&=0xEF			//��ŷ�3�ر�
#define	Solenoid4Open	IO8SWITCH|=0x20			//��ŷ�4��
#define	Solenoid4Close	IO8SWITCH&=0xDF			//��ŷ�4�ر�
#define	Solenoid5Open	IO8SWITCH|=0x40			//��ŷ�5��
#define	Solenoid5Close	IO8SWITCH&=0xBF			//��ŷ�5�ر�
#define	Solenoid6Open	IO8SWITCH|=0x80			//��ŷ�6��
#define	Solenoid6Close	IO8SWITCH&=0x7F			//��ŷ�6�ر�

#define	FerRun			IO_OutSet(1,1)			//����
#define	FerStop			IO_OutSet(1,0)			//������
#define OpenLED			IO_OutSet(2,1)			//�򿪵�
#define CloseLED		IO_OutSet(2,0)			//�رյ�
#define	SewageOpen		IO_OutSet(4,1)			//������
#define	SewageClose		IO_OutSet(4,0)			//�ر�����

#define	ManualModel		0						//�ֶ�ģʽ
#define	localModel		1						//����ģʽ
#define	NetworkModel	2						//����ģʽ
#define	PlanModel		3						//�ƻ�ģʽ

#define	ManualButton	(MCGS_Button&0x01)		//�ֶ���ť
#define	IrrMethod		(MCGS_Button&0x02)		//���ģʽ
#define	WifiButton		(MCGS_Button&0x04)		//wifi����
#define	StartButton		(MCGS_Button&0x08)		//������ť
#define	PlanButton		(MCGS_Button&0x10)		//�ƻ���ť
#define	RunState		(MCGS_Button&0x20)		//����״̬
#define	LogButton		(MCGS_Button&0x40)		//��־��ť
#define	SaveButton		(MCGS_Button&0x80)		//���水ť

#define	ChoPartition1	(MCGS_Partition&0x01)	//����һѡ��
#define	ChoPartition2	(MCGS_Partition&0x02)	//������ѡ��
#define	ChoPartition3	(MCGS_Partition&0x04)	//������ѡ��
#define	ChoPartition4	(MCGS_Partition&0x08)	//������ѡ��
#define	ChoPartition5	(MCGS_Partition&0x10)	//������ѡ��
#define	ChoPartition6	(MCGS_Partition&0x20)	//������ѡ��
#define	DHCPButton		(MCGS_Partition&0x40)	//DHCP
#define	TimeButton		(MCGS_Partition&0x80)	//ʱ��ͬ��

#define	Fer2M			(MCGS_Fer2S&0x02)		//����Ͱ������
#define	Auto			(MCGS_Fer2S&0x04)		//�Զ�����

#define	Manual_Par1		(MCGS_Manual&0x01)
#define	Manual_Par2		(MCGS_Manual&0x02)
#define	Manual_Par3		(MCGS_Manual&0x04)
#define	Manual_Par4		(MCGS_Manual&0x08)
#define	Manual_Par5		(MCGS_Manual&0x10)
#define	Manual_Par6		(MCGS_Manual&0x20)
#define	Manual_Fer1		(MCGS_Manual&0x40)
#define	Manual_Fer2		(MCGS_Manual&0x80)

#define	MCGS_SSID			10					//wifi����
#define	MCGS_PASSWORD		74					//����
#define	MCGS_IP				138					//IP��ַ
#define	MCGS_NETWORK		202					//��������
#define	MCGS_GATEWAY		266					//����
#define	MCGS_DNS			330					//DNS

u8	MCGS_Button=0,MCGS_Partition=0,MCGS_Fer2S=0,MCGS_Manual,MCGS_FM;	//����������������������
u8	HC_Partition,HC_IrrMode;					//���������»���

u8	SOILERROR=0;	//���������������ʶ
u8	AIRERROR=0;		//���������������ʶ
u8	IO8STATE=0;		//IO8����״̬
u8	IO8SWITCH=0;	//IO8���״̬
u8	MODEL=1;		//��ǰģʽ
u16	IrrTime=0;		//���õĹ��ʱ��
u16	Remaining=0;	//����ʱʱ��
u8	Net=0;			//�Ƿ�Ϊ��������
u8	Current=0;		//����

u8	Irrsign=0;		//���ʱ����ȡ��־
u8	Remsign=0;		//����ʱ���±�־
u8	Cursing=0;		//�������±�־
u8	ReadNet=0;		//��ȡ�������
u8	IO8Set=0;		//IO8���±�־
u8	Err=0;			//�쳣��־
u8	Error=0;		//�쳣����
u32	LogP=0;			//Log�洢��ַ��־
u8	ChoMode=1;		//����ģʽѡ��
u8	GetTime=0;		//��ȡ������ʱ���־
u8	LogPage=0;		//��־ҳ��
u8	TimePopup=0;	//ʱ��ͬ�������Ƿ񵯳�
u8	ReadPlan=0;		//��ȡ�ƻ�
u16	PlanAddr=0;		//�ƻ���ַ
u8	Planing=0;		//����ִ����
u8	SetRun=0;		//�Ǳ����������ر�־
u16	htime=0;		//�ֶ���ʱ
u8	Logwait=0;		//�ȴ���־д��
u8	Humiup=0;		//ʪ������
u8	Humidown=0;		//ʪ������
u8	sewage_space=0;	//���ۼ��
u8	sewage_time=0;	//����ʱ��
u8	space=0;		//�����ʱ
u8	sw_time=0;		//ʱ����ʱ
u16	EC=0;			//Ŀ��EC
u16	NeedFlow=0;		//Ŀ������
u8	FerB=0;			//���ʱ�־

union	Logaddr
{
	u8	mem[4];
	u32	mem32;
}Logmem;			//��ǰ��־��ַ

u8	LED_BZ=0,SysTime=0;		//LED��ʾ�л���־��Уʱ��־
unsigned char printf_num=1;	//printf ����ָ���־ 1ָ��uart1 2ָ��uart2
unsigned char Emw_B=0;		//wifiģ���ʼ����ɱ�־
unsigned char emw_set=1;	//wifi�������̻߳����־
unsigned int  up_time=15;	//�����ϴ�Ƶ��

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];

u8	waitPartition[6];		//���������б�

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

u8	rema=0;					//���е���ʱ��־
u8	ttm=0,upenv=0,up_state=0;					//���Ӽ�ʱ�����������ϴ�ʱ���ʱ���豸״̬�ϴ���־
OS_TMR   * tmr1;           						//�����ʱ��1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)	//�����ʱ��1�ص�����
{
    if(++ttm==6)
	{
		htime++;upenv++;ttm=0;					//��������ϵͳ��ʱ
		if(rema)Remaining--,Remsign=1;			//��ȵ���ʱ
		space++;
	}
	sw_time+=10;
}

void	readflashthree()				//��ȡ��Ԫ��
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

void	sendflashthree()				//д����Ԫ��
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
