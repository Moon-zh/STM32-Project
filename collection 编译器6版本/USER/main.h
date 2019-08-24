#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "includes.h"
#include "check.h"
#include "stmflash.h"
#include "Emw3060.h"
#include "NeowayN21.h"

extern char ProductKey1[20];
extern char DeviceName1[50];
extern char DeviceSecret1[50];

//��Ȩ��Ϣ
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char CCID[25];

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
#define HDMI_STK_SIZE  		    		512
//�����ջ	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//������
void HDMI_task(void *pdata);

//�����������ȼ�
#define Upyun_TASK_PRIO       			3 
//���������ջ��С
#define Upyun_STK_SIZE  		    	1024
//�����ջ	
OS_STK Upyun_TASK_STK[Upyun_STK_SIZE] __attribute__((aligned (8)));
//������
void Upyun_task(void *pdata);

//�����������ȼ�
#define Alarm_TASK_PRIO       			4 
//���������ջ��С
#define Alarm_STK_SIZE  		    	1024
//�����ջ	
OS_STK Alarm_TASK_STK[Alarm_STK_SIZE] __attribute__((aligned (8)));
//������
void Alarm_task(void *pdata);

//�����������ȼ�
#define SaveThree_TASK_PRIO       		5 
//���������ջ��С
#define SaveThree_STK_SIZE  		    1024
//�����ջ	
OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//������
void SaveThree_task(void *pdata);

//�����������ȼ�
#define Collection_TASK_PRIO       		6 
//���������ջ��С
#define Collection_STK_SIZE  		    256
//�����ջ	
OS_STK Collection_TASK_STK[Collection_STK_SIZE];
//������
void Collection_task(void *pdata);

//�����������ȼ�
#define UpyunWF_TASK_PRIO       		7 
//���������ջ��С
#define UpyunWF_STK_SIZE  		    	1024
//�����ջ	
OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE] __attribute__((aligned (8)));
//������
void UpyunWF_task(void *pdata);


#define 	CMD_AIRTEMP		0x01
#define		CMD_AIRHUMI 	0x00
#define		CMD_SOILTEMP	0x03
#define		CMD_SOILHUMI	0x02
#define		CMD_CO2			0x05
#define 	CMD_LIGTH		0x07
#define		CMD_EC			0x15
#define		EnvNum			5		//�������������

#define 	Alarm_airtemp 	800		//���±���ֵ
#define 	Alarm_airhumi 	900		//����ʪ�ȱ���ֵ
#define 	Alarm_light 	10000	//��ǿ�ȱ���ֵ
#define 	Alarm_CO2 		800		//CO2����ֵ
	
#define 	Alarm_soiltemp 	700		//�����¶ȱ���ֵ
#define 	Alarm_soilhumi 	900		//����ʪ�ȱ���ֵ
#define 	Alarm_EC 		100		//�絼��ת�ٱ���ֵ

#define 	sensor_num		1		//����������

#ifndef Env
#define Env
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

u8	N21BZ=0;			//Ϊ�˱���N21���ͳ�ͻ���ж��Ƿ�N21�ڷ�������
u8	LED_BZ=0;
unsigned char printf_num=1;	//printf ����ָ���־ 1ָ��uart1 2ָ��uart2
unsigned char N21_B=0;		//NBģ���ʼ����ɱ�־
unsigned char Emw_B=0;		//wifiģ���ʼ����ɱ�־
unsigned char emw_set=1;	//wifi�������̻߳����־
unsigned char n21_set=1;	//N21�������̻߳����־
unsigned int  up_time=15;	//�����ϴ�Ƶ��
unsigned int  up_wartime=20;//�����ϴ�Ƶ��
unsigned int  sendok=1;		//WIFI�ϱ�������־
unsigned int  Nsendok=1;	//NB�ϱ�������־

#include "my_HDMI.h"

extern 	uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];
extern 	uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];
void	UpSetAlarm_wifi(Waring data,u8 group);
void	UpSetAlarm(Waring data,u8 group);
void	readflash(void);
void	sendflash(void);

Environmental sensor[EnvNum];							//���������������������ڴ�

u8 firstarm=1;
u8 wifiup=0,nbup=0,warup;								//ģ���ϴ���־
u8 ttm=0,wifi=0,nb=0,wart;								//��ʱ������ �ϴ���־
