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
#define HDMI_STK_SIZE  		    		1536
//�����ջ	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//������
void HDMI_task(void *pdata);

//�����������ȼ�
#define SaveThree_TASK_PRIO       		5 
//���������ջ��С
#define SaveThree_STK_SIZE  		    256
//�����ջ	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//������
void SaveThree_task(void *pdata);

//�����������ȼ�
#define Upyun_TASK_PRIO       			7 
//���������ջ��С
#define Upyun_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//������
void Upyun_task(void *pdata);

//�����������ȼ�
#define Work_TASK_PRIO       			8 
//���������ջ��С
#define Work_STK_SIZE  		    		256
//�����ջ	
__align(8)OS_STK Work_TASK_STK[Work_STK_SIZE];
//������
void Work_task(void *pdata);

//�����������ȼ�
#define Water_TASK_PRIO       			3 
//���������ջ��С
#define Water_STK_SIZE  		    	64
//�����ջ	
__align(8)OS_STK Water_TASK_STK[Water_STK_SIZE];
//������
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

RTC_TIME STIME;			//ϵͳʱ��	

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

SysLog	Log;			//ϵͳ��־

#ifndef Run_parameters
#define	Run_parameters
typedef struct
{
	u8	Mu;				//Ķ��
	u8	AverageMu;		//Ķ��ʩ����
	u8	SpecialFer;		//ר��Ͱ
	u8	Pre;			//ǰ��ʱ��
	u8	Behind;			//����ʱ��
	u8	Terminaladdr;	//�ն˵�ַ
	u8	TerminalNum;	//�ն�ͨ��
	u8	a,b,c;
}Run_P;
#endif

Run_P	Run_SetMessage;

u8	Concentration[8];	//����Ũ��

#define	FerSwitch		PBin(12)				//���ʿ���
#define	FerState		PBin(13)				//����״̬

#define	Stir1Open		IO8SWITCH|=1			//����1��
#define	Stir1Close		IO8SWITCH&=0xFE			//����1�ر�
#define	Stir2Open		IO8SWITCH|=2			//����2��
#define	Stir2Close		IO8SWITCH&=0xFD			//����2�ر�
#define	Solenoid1Open	IO8SWITCH|=4			//��ŷ�1��
#define	Solenoid1Close	IO8SWITCH&=0xFB			//��ŷ�1�ر�
#define	Solenoid2Open	IO8SWITCH|=8			//��ŷ�2��
#define	Solenoid2Close	IO8SWITCH&=0xF7			//��ŷ�2�ر�

#define	FerRun			IO_OutSet(1,1)			//����
#define	FerStop			IO_OutSet(1,0)			//������
#define OpenLED			IO_OutSet(2,1)			//�򿪵�
#define CloseLED		IO_OutSet(2,0)			//�رյ�

#define	ManualModel		0						//�ֶ�ģʽ
#define	localModel		1						//����ģʽ
#define	NetworkModel	2						//����ģʽ
#define	PlanModel		3						//�ƻ�ģʽ

#define	RunState		(MCGS_Button&0x01)		//������ť
#define	SuspendButton	(MCGS_Button&0x02)		//��ͣģʽ
#define	FerSButton		(MCGS_Button&0x04)		//����Ͱ����
#define	HouseRButton	(MCGS_Button&0x08)		//�����ö�ȡ
#define	HouseSButton	(MCGS_Button&0x10)		//�����ñ���
#define	CechkSelf		(MCGS_Button&0x20)		//�Լ�״̬
#define	LogButton		(MCGS_Button&0x40)		//��־��ť
#define	Details			(MCGS_Button&0x80)		//���鰴ť

#define	CheckFer1		(MCGS_Check&0x01)		//�Լ����Ͱ��ť
#define	CheckFer2		(MCGS_Check&0x02)
#define	CheckFer3		(MCGS_Check&0x04)
#define	CheckFer4		(MCGS_Check&0x08)
#define	CheckFer5		(MCGS_Check&0x10)
#define	CheckFer6		(MCGS_Check&0x20)
#define	CheckFer7		(MCGS_Check&0x40)
#define	CheckFer8		(MCGS_Check&0x80)
#define	CheckFerM		(MCGS_Check2&0x01)		//�Լ����ʱ�
#define	CheckSolenoid	(MCGS_Check2&0x02)		//�Լ����
#define	CheckFlow		(MCGS_Check2&0x04)		//�Լ�������

#define	TimeButton		(MCGS_Partition&0x80)	//ʱ��ͬ��

#define	MCGS_SSID			10					//wifi����
#define	MCGS_PASSWORD		74					//����
#define	MCGS_IP				138					//IP��ַ
#define	MCGS_NETWORK		202					//��������
#define	MCGS_GATEWAY		266					//����
#define	MCGS_DNS			330					//DNS

u8	MCGS_Button=0,MCGS_Partition=0,MCGS_Fer=0;
u8	MCGS_Check=0,MCGS_Check2=0;					//����������������������������Ͱ����״̬(���ڱ���) �Լ찴��
u8	HC_Partition,HC_IrrMode;					//���������»���

u8	IO8SWITCH=0;	//IO8���״̬
u8	MODEL=0;		//��ǰģʽ
u16	IrrTime=0;		//���õĹ��ʱ��
u16	Remaining=0;	//����ʱʱ��
u8	Net=0;			//�Ƿ�Ϊ��������

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
u8	LogDe=0;		//��־�����
u8	TimePopup=0;	//ʱ��ͬ�������Ƿ񵯳�
u8	SetRun=0;		//�Ǳ����������ر�־
u16	htime=0;		//�ֶ���ʱ
u8	Logwait=0;		//�ȴ���־д��

u8	HDMI_READ_START=0;	//��ȡ�������������־
u8	HDMI_SET_STOP=0;	//����������������
u8	stage=0;		//�����׶�
u8	PreTime=0;		//ǰ�ý���ʱ��
u8	BehindTime=0;	//���ó�ϴʱ��
u16 Actual_Water=0;	//ʵ����ˮ��
u16 Actual_Fer=0;	//ʵ���÷���
u16 Actual_FerP=0;	//ʵ���÷���
u8	HouseNum=0;		//���
u8	PublicFer=0;	//����Ͱѡ��
u8	checknum=0;		//�Լ��ŷ�
u16	checkflow=0;	//�Լ�����
u16	Mathflow=0;		//Ŀ������
u16	NeedFlow=0;		//��Ҫ����
u8	checkfer=0;		//�Լ����Ͱ��ŷ�
u32	water0=0;		//��ˮ��
u32	water1=0;		//ˮ��һ
u32	water2=0;		//ˮ���
u32	thiswater0=0;	//���ο�ʼ��ˮ��
u32	thiswater1=0;	//���ο�ʼר�÷�
u32	thiswater2=0;	//���ο�ʼ���÷�
u8	watertime=0;	//������������
u16	water1flow=0;	//ˮ��һ����

u8	n21_set=1;		//N21��ʼ�����
u8	nbup=0;			//�ϴ���־
u8	NET=0;			//������Ʊ�־

union	Logaddr
{
	u8	mem[4];
	u32	mem32;
}Logmem;			//��ǰ��־��ַ

union	pressure
{
	float	f;
	u16		I[2];
}pr;

u8	LED_BZ=0,SysTime=0;		//LED��ʾ�л���־��Уʱ��־
unsigned char printf_num=1;	//printf ����ָ���־ 1ָ��uart1 2ָ��uart2
unsigned int  up_time=15;	//�����ϴ�Ƶ��

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];

u8	Nsendok=0;
u8	rema=0;					//���е���ʱ��־
u8	ttm=0,upenv=0,up_state=0;					//���Ӽ�ʱ�����������ϴ�ʱ���ʱ���豸״̬�ϴ���־
OS_TMR   * tmr1;           						//�����ʱ��1
void tmr1_callback(OS_TMR *ptmr,void *p_arg)	//�����ʱ��1�ص�����
{
    if(++ttm==6)
	{
		htime++;ttm=0;							//
		if(RunState)nbup=1,Remaining--;			//Remsign=1;				//��ȵ���ʱ
	}
}

void	readflashthree()				//��ȡ��Ԫ��
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

void	sendflashthree()				//д����Ԫ��
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
