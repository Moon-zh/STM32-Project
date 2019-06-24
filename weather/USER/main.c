#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "rtc.h" 		 	 
#include "string.h"
#include "Includes.h"
#include "Rs485.h"
#include "led.h"
#include "ZoneCtrl.h"
#include "timer.h"
#include "IO_BSP.h"
#include "W25qxx.h"
#include "GPS.h"
#include "Emw3060.h"
#include "NeowayN21.h"
#include "stmflash.h"
#include "LED_dz.h"
 
/************************************************
 ALIENTEKս��STM32������ʵ��53
 UCOSIIʵ��3-��Ϣ���С��ź������������ʱ��  ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

extern Environmental sensor;

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//LED����
//�����������ȼ�
#define LED_TASK_PRIO       			7 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);

//�¶Ȳɼ�����
//�����������ȼ�
#define ZONE_TASK_PRIO       		 	6
//���������ջ��С
#define ZONE_STK_SIZE  				256
//�����ջ	
OS_STK ZONE_TASK_STK[ZONE_STK_SIZE];
//������
void Temperature_task(void *pdata);

//������
//�����������ȼ�
#define MASTER_TASK_PRIO       			13 
//���������ջ��С
#define MASTER_STK_SIZE  					256
//�����ջ	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//������
void MasterCtrl_task(void *pdata);

//�ź���������
//�����������ȼ�
#define FLAGS_TASK_PRIO       			9
//���������ջ��С
#define FLAGS_STK_SIZE  		 		56
//�����ջ	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//������
void flags_task(void *pdata);

//GPRS����
//�����������ȼ�
#define GPRS_TASK_PRIO       			12 
//���������ջ��С
#define GPRS_STK_SIZE  		 			512
//�����ջ	
OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//������
void gprs_task(void * pdata);

//��������
//�����������ȼ�
#define HMI_TASK_PRIO       			8 
//���������ջ��С
#define HMI_STK_SIZE  		 			512
//�����ջ	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//������
void Task_HMIMonitor(void * pdata);


//����ɨ������
//�����������ȼ�
#define KEY_TASK_PRIO       			11 
//���������ջ��С
#define KEY_STK_SIZE  					56
//�����ջ	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);

//�����������ȼ�
#define UpyunWF_TASK_PRIO       		2 
//���������ջ��С
#define UpyunWF_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//������
void UpyunWF_task(void *pdata);

//�����������ȼ�
#define Upyun_TASK_PRIO       			1 
//���������ջ��С
#define Upyun_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK Upyun_TASK_STK[Upyun_STK_SIZE];
//������
void Upyun_task(void *pdata);

#define LED_DZ_TASK_PRIO       			3 
//���������ջ��С
#define LED_DZ_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK LED_DZ_TASK_STK[LED_DZ_STK_SIZE];
//������
void LED_DZ_task(void *pdata);

#define SaveThree_TASK_PRIO       			4 
//���������ջ��С
#define SaveThree_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//������
void SaveThree_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////
    
OS_EVENT * msg_key;			//���������¼���	  
OS_EVENT * q_msg;			//��Ϣ����
OS_TMR   * tmr1;			//�����ʱ��1
OS_TMR   * tmr2;			//�����ʱ��2
OS_TMR   * tmr3;			//�����ʱ��3
OS_FLAG_GRP * flags_key;	//�����ź�����
void * MsgGrp[256];			//��Ϣ���д洢��ַ,���֧��256����Ϣ

u8	N21BZ=0;			//Ϊ�˱���N21���ͳ�ͻ���ж��Ƿ�N21�ڷ�������
u8	READBZ=1;			//�ж��Ƿ���һ���ϴ����ڽ��������ڱ����Ƿ��ϴ�
u16	time=0;
u8	LED_BZ=0;
unsigned char printf_num=1;	//printf ����ָ���־ 1ָ��uart1 2ָ��uart2
unsigned char N21_B=0;		//NBģ���ʼ����ɱ�־
unsigned char Emw_B=0;		//wifiģ���ʼ����ɱ�־
unsigned char emw_set=1;	//wifi�������̻߳����־
unsigned char n21_set=1;	//N21�������̻߳����־
unsigned int  up_time=15;		//�����ϴ�Ƶ��
unsigned int  up_wartime=2;	//�����ϴ�Ƶ��

extern char ssid[20];
extern char password[20];

extern char ProductKey1[20];
extern char DeviceName1[50];
extern char DeviceSecret1[50];

extern char *ProductKeyw;
extern char *DeviceNamew;
extern char *DeviceSecretw;

//��Ȩ��Ϣ
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;

extern char CCID[25];


#define FLASH_SAVE_ADDR  0X0802DE00
#define FLASH_THREE_ADDR  0X0802FE00

void	readflashthree(void);
int main(void)
{	 		    
	SystemInit();
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	bsp_InitUart();				//��ʼ��RS485
	OSInit();  	 				//��ʼ��UCOSII
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  
//	readflashthree();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void	uptodz()						//����LED������ʾ����
{
	setdz();
	setprogram1(sensor);
	setprogram2(sensor);
}

void 	LED_DZ_task(void *pdata)		//LED������ʾ����
{	 
	u8 i;
	while(1)
	{
		delay_ms(200);
		if(++i==80)
		{
			i=0;
			uptodz();
		}	
	}
}

//��ʼ����
void 	start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
//	u8 err;	    	    
	pdata = pdata; 	
//	RTC_Init();
//	OSStatInit();						//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();				//�����ٽ���(�޷����жϴ��)    
	//delay_ms(1000);
 OSTaskCreate(led_task,(void *)0,
		(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],
		LED_TASK_PRIO);		
	
 	OSTaskCreate(ZoneCtrl_task,(void *)0,
		(OS_STK*)&ZONE_TASK_STK[ZONE_STK_SIZE-1],
		ZONE_TASK_PRIO);	
	
	OSTaskCreate(UpyunWF_task,(void *)0,
		(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],
		UpyunWF_TASK_PRIO);
	
	OSTaskCreate(Upyun_task,(void *)0,
	(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],
	Upyun_TASK_PRIO);

	OSTaskCreate(LED_DZ_task,(void *)0,
		(OS_STK*)&LED_DZ_TASK_STK[LED_DZ_STK_SIZE-1],
		LED_DZ_TASK_PRIO);

	OSTaskCreate(SaveThree_task,(void *)0,
		(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],
		SaveThree_TASK_PRIO);
	
 	OSTaskSuspend(START_TASK_PRIO);		//������ʼ����.
	OS_EXIT_CRITICAL();					//�˳��ٽ���(���Ա��жϴ��)
}

void	readflashthree()				//��flash�ж�ȡ��Ԫ������
{
	char buf[150];
	char *msg=buf;
	u8 i;
	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,55);
//	if((buf[0]==0)|(buf[0]==0xff))return;
	if(buf[1]=='K')
	{
		memset(ProductKey0,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName0,0,20);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceName[i++]=*msg;
	}
	if(buf[59]=='S')
	{
		memset(DeviceSecret0,0,40);
		msg=buf+62;for(i=0;*msg;msg++)	DeviceSecret[i++]=*msg;
	}
	
	STMFLASH_Read(100+FLASH_THREE_ADDR,(u16*)buf,70);
//	if((buf[0]==0)|(buf[0]==0xff))return;
	if(buf[1]=='K')
	{
		memset(ProductKey1,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKeyw[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName1,0,50);
		msg=buf+24+4;for(i=0;*msg;msg++)	DeviceNamew[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret1,0,50);
		msg=buf+78+4;for(i=0;*msg;msg++)	DeviceSecretw[i++]=*msg;
	}

}

void	sendflashthree(u8 group)		//���ڴ�д�����õ���Ԫ������
{
	
	LED_BZ=1;
	if(group==1)
	{
		STMFLASH_Write(FLASH_THREE_ADDR,(u16*)"PK:0",4);
		STMFLASH_Write(FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
		STMFLASH_Write(FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
		STMFLASH_Write(FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
		STMFLASH_Write(FLASH_THREE_ADDR+58,(u16*)"DS:0",4);
		STMFLASH_Write(FLASH_THREE_ADDR+62,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	}
	else
	{
		STMFLASH_Write(100+FLASH_THREE_ADDR,(u16*)"PK:0",4);
		STMFLASH_Write(100+FLASH_THREE_ADDR+4,(u16*)ProductKey1,sizeof(ProductKey1));
		STMFLASH_Write(100+FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
		STMFLASH_Write(100+FLASH_THREE_ADDR+28,(u16*)DeviceName1,sizeof(DeviceName1));
		STMFLASH_Write(100+FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
		STMFLASH_Write(100+FLASH_THREE_ADDR+82,(u16*)DeviceSecret1,sizeof(DeviceSecret1));
	}
	readflashthree();
	LED_BZ=0;
}

void	sendflash(void);
void	readflash()						//��flash��ȡ��������
{
	u16 data;
	u16 buf[50];
	u8 i,k;

	memset(buf,0,sizeof buf);
	STMFLASH_Read(400+FLASH_SAVE_ADDR+200,(u16*)&data,1);	//�Ƿ��ǳ���
	if(data!=0xaaaa){sendflash(); return;}

	STMFLASH_Read(400+FLASH_SAVE_ADDR,buf,50);				//��ȡ��������
	
	STMFLASH_Read(450+FLASH_SAVE_ADDR,buf,20);				//��ȡssid
	for(k=i=0;i<20;i+=2,k++)
	{
		ssid[i]=buf[k]>>8;
		ssid[i+1]=buf[k]&0xff;
	}

	STMFLASH_Read(500+FLASH_SAVE_ADDR,buf,20);				//��ȡpassword
	for(k=i=0;i<20;i+=2,k++)
	{
		password[i]=buf[k]>>8;
		password[i+1]=buf[k]&0xff;
	}
	
	STMFLASH_Read(610+FLASH_SAVE_ADDR,(u16*)&data,1);		//��ȡ�ϱ�ʱ��
	if(data>=1&&data<0xffff)up_time=data;
	
	STMFLASH_Read(620+FLASH_SAVE_ADDR,(u16*)&data,1);		//��ȡ�����ϱ�ʱ��
	if(data>=1&&data<0xffff)up_wartime=data;			
}

void	sendflash()						//�洢�������õ�flash
{
	u16 bz=0xaaaa;
	u16 buf[50];
	u8 i,k;
	
	STMFLASH_Write(400+FLASH_SAVE_ADDR,(u16*)buf,48);			//д�뱨������
	delay_ms(200);
	STMFLASH_Write(400+FLASH_SAVE_ADDR+200,(u16*)&bz,2);		//д�������־
	delay_ms(100);
	STMFLASH_Write(610+FLASH_SAVE_ADDR,(u16*)&up_time,2);		//д���ϱ�ʱ��
	STMFLASH_Write(620+FLASH_SAVE_ADDR,(u16*)&up_wartime,2);	//д�뱨���ϱ�ʱ��

	
	for(k=i=0;i<20;i++)
	{
		buf[i]=ssid[k++]<<8;
		buf[i]=buf[i]|ssid[k++];
	}
	
	STMFLASH_Write(450+FLASH_SAVE_ADDR,(u16*)buf,20);			//д��ssid

	for(k=i=0;i<20;i++)
	{
		buf[i]=password[k++]<<8;
		buf[i]=buf[i]|password[k++];
	}
	
	STMFLASH_Write(500+FLASH_SAVE_ADDR,(u16*)buf,20);			//д��password
	delay_ms(200);
	readflash();
}

void	readthree()						//��COM3 485���ڶ�ȡ�Ƿ���������Ԫ������
{
	u8 len,i;
	char hc[250];
	char *msg=hc;
	u8 buf[100];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	len=0;i=0;
	len=COM5GetBuf(buf,99);
	if(len>50)
	{
		if(strstr((const char *)buf,"m1")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey0,0,20);
				msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey[i++]=*msg;	if(i>=20)return ;}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName0,0,50);
				msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName[i++]=*msg;	if(i>=50)return ;}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret0,0,50);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecret[i++]=*msg;if(i>=50)return ;}
			}
			len=1;
		}
		else if(strstr((const char *)buf,"m2")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey1,0,20);
				msg+=3;for(i=0;*msg!=',';msg++)	{ProductKeyw[i++]=*msg;	if(i>=20)return ;}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName1,0,50);
				msg+=3;for(i=0;*msg!=',';msg++)	{DeviceNamew[i++]=*msg;	if(i>=50)return ;}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret1,0,50);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecretw[i++]=*msg;if(i>=50)return ;}
			}
			len=2;
		}
		else if(strstr((const char *)buf,"wf")[0]=='w')
		{
			
		}
		else return;	
		sendflashthree(len);
		comSendBuf(COM5,buf,strlen((char *)buf));
		comClearRxFifo(COM5);
		memset(buf,0,sizeof buf);
		memset(hc,0,sizeof buf);
	}
}

void	readset(u8 t)					//��ȡ�����·�������
{
	u8 len;
	u8 buf[250];
	char c[100];
	char *msg=c;
	if(n21_set)goto qw;
	len=COM1GetBuf(buf,100);
	if(len>10)
	{
		delay_ms(200);COM1GetBuf(buf,100);			//��ȡ���豸�·�������
		if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,200);memset(c,0,200);comClearRxFifo(COM1);return;}
		msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=							(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
		msg=strstr((const char *)buf,"up_wartime:");	if(msg[0]=='u')up_wartime=						(msg[11]-0x30)*10000+(msg[12]-0x30)*1000+(msg[13]-0x30)*100+(msg[14]-0x30)*10+(msg[15]-0x30);
		
		sendflash();
		comClearRxFifo(COM1);

		if(t)OSTaskSuspend(UpyunWF_TASK_PRIO);

		if(t)OSTaskResume(UpyunWF_TASK_PRIO);
	}
	else
	{
qw:		if(emw_set)return;
		len=COM2GetBuf(buf,100);
		if(len>10)
		{
			delay_ms(200);COM2GetBuf(buf,100);		//��ȡ���豸�·�������
			if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,200);memset(c,0,200);comClearRxFifo(COM2);return;}
			msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=							(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
			msg=strstr((const char *)buf,"up_wartime:");	if(msg[0]=='u')up_wartime=						(msg[11]-0x30)*10000+(msg[12]-0x30)*1000+(msg[13]-0x30)*100+(msg[14]-0x30)*10+(msg[15]-0x30);
			
			sendflash();
			comClearRxFifo(COM2);
			
			if(!t)OSTaskSuspend(Upyun_TASK_PRIO);

			if(!t)OSTaskResume(Upyun_TASK_PRIO);
			
		}
	}
	memset(buf,0,200);
	memset(c,0,200);
}

u8 Nsendok=0;
void	Upccid()										//�ϱ�CCID
{
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	Nsendok=sendN21(b,0);
}
void	Uptoaliyun_wifi()	//�ϴ����������ݵ������� WIFI
{
	char a[400];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	
//	sensor.temp=(rand()%100)*10;
//	sensor.humi=(rand()%100)*10;
//	sensor.pm25=rand()%1000;
//	sensor.co2=rand()%1000;
//	sensor.vol=rand()%1000;
//	sensor.pm10=rand()%1000;
//	sensor.windspeed=rand()%10;
//	sensor.winddir=rand()%16;
//	sensor.rain=rand()%30;
//	sensor.snow=rand()%1;
//	sensor.light=rand()%100000;
//	sensor.bmp=rand()%10000;
	
	sprintf(b,"\"airtemp_1\":%2.1f,",(float)(sensor.temp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumi_1\":%2.1f,",(float)(sensor.humi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"pm25_1\":%d,",sensor.pm25);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2_1\":%d,",sensor.co2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"sound_1\":%d,",sensor.vol);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"pm10_1\":%d,",sensor.pm10);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"windspeed_1\":%d,",sensor.windspeed);					for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"winddir_1\":%d,",sensor.winddir);						for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"rain_1\":%d,",sensor.rain);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"snow_1\":%d,",sensor.snow);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"light_1\":%d,",sensor.light);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"bmp_1\":%d,",sensor.bmp);								for(;*b;b++)a[i++]=*b;
	
	sendEmw(a,0);
}

void	Uptoaliyun()			//�ϴ����������ݵ�������
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	
	sprintf(b,"\\\"airtemp_1\\\":%2.1f,",(float)(sensor.temp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"airhumi_1\\\":%2.1f,",(float)(sensor.humi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"pm25_1\\\":%d,",sensor.pm25);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"CO2_1\\\":%d,",sensor.co2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"sound_1\\\":%d,",sensor.vol);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"pm10_1\\\":%d,",sensor.pm10);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"windspeed_1\\\":%2.1f,",(float)(sensor.windspeed/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"winddir_1\\\":%d,",sensor.winddir);						for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"rain_1\\\":%d,",sensor.rain);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"snow_1\\\":%d,",sensor.snow);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"light_1\\\":%d,",sensor.light);							for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"bmp_1\\\":%d,",sensor.bmp);								for(;*b;b++)a[i++]=*b;
	
	Nsendok=sendN21(a,0);
}
//void	Uptoaliyun_wifi2()	//�ϴ����������ݵ�������
//{
//	char a[400];
//	char c[250];
//	char *b=c;
//	u8 i;i=0;
//	memset(a,0,sizeof a);
//	sprintf(b,"\"airtemp_1\":%2.1f,",(float)(sensor.temp/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"airhumi_1\":%2.1f,",(float)(sensor.humi/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"pm25_1\":%d,",sensor.pm25);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"CO2_1\":%d,",sensor.co2);								for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"sound_1\":%d,",sensor.vol);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"pm10_1\":%d,",sensor.pm10);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"windspeed_1\":%d,",sensor.windspeed);					for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"winddir_1\":%d,",sensor.winddir);						for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"rain_1\":%d,",sensor.rain);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"snow_1\":%d,",sensor.snow);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"light_1\":%d,",sensor.light);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\"bmp_1\":%d,",sensor.bmp);								for(;*b;b++)a[i++]=*b;
//	
//	sendEmw2(a,0);
//}

extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
void 	Upyun_task(void *pdata)							//�ϴ������� N21
{
	u8 i;
	u8 buf[20];
	NeoWayN21_init();					//N21��ʼ�� ע��
	conN21();							//N21���ӵ�������
	for(i=0;i<3;i++)					//�ȴ�N21��ȫ��ʼ����
	delay_ms(1000);
	Upccid();							//�ϱ�CCID
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	N21_B=1;n21_set=0;
	while(1)
	{
		delay_ms(200);
		if(((++time)/250)>=up_time)					//���ͼ����ÿ��ֻ����һ��
		{
			n21_set=1;
			comClearRxFifo(COM1);
			memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			printf_num=1;
			printf("AT\r\n");
			delay_ms(100);
			printf_num=1;
			printf("AT\r\n");
			delay_ms(300);
			COM1GetBuf(buf,15);
			if(strchr((const char *)buf,'K')[0]!='K')
			{
				n21_set=1;
				N21_B=0;
				NeoWayN21_init();					//N21��ʼ�� ע��
				conN21();							//N21���ӵ�������
				memset(buf,0,10);
				n21_set=0;
				N21_B=1;
				memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			}
			time=0;
			Uptoaliyun();
			n21_set=0;
		}
		if(!Nsendok){n21_set=1;N21_B=0;NeoWayN21_init();conN21();n21_set=0;N21_B=1;}
		readset(0);						//��ȡ�������·�����	
	}
}

void 	UpyunWF_task(void *pdata)						//�ϴ������� WIFI
{
	u16 i;u8 error=0;u8 buf[100];
	Emw3060_init();
	Emw3060_con();
	Emw_B=1;emw_set=0;
	while(1)
	{
		delay_ms(200);
		readset(1);										//��ȡ�������·�����
		comClearRxFifo(COM2);
		printf_num=2;
		printf("AT+WJAPS\r");
		delay_ms(200);
		COM2GetBuf(buf,45);
		if(strstr((const char *)buf,"STATION_UP")[0]!='S')
		{
			if(++error==10){emw_set=1;Emw_B=0;Emw3060_init();Emw3060_con();Emw_B=1;emw_set=0;}
			continue;
		}
		else error=0;		
		if(((++i)/250)>=up_time){i=0;Uptoaliyun_wifi();}
	}
}

void 	SaveThree_task(void *pdata)						//��Ԫ��洢����
{
	while(1)
	{
		delay_ms(200);
		readthree();							//�ж��Ƿ�ɨ��ǹ������Ԫ������
	}
}

char *str()
{
	static char a[250];
	char c[150];
	char *b=c;

//	sprintf(b,"\\\"airtemp_1\\\":%2.1f,",(float)(temp/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"airhumi_1\\\":%2.1f,",(float)(humi/10.0));		for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"pm25_1\\\":%d,",pm25);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"CO2_1\\\":%d,",co2);								for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"sound_1\\\":%d,",vol);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"pm10_1\\\":%d,",pm10);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"windspeed_1\\\":%d,",windspeed);					for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"winddir_1\\\":%d,",winddir);						for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"rain_1\\\":%d,",rain);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"snow_1\\\":%d,",snow);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"light_1\\\":%d,",light);							for(;*b;b++)a[i++]=*b;
//	sprintf(b,"\\\"bmp_1\\\":%d,",bmp);								for(;*b;b++)a[i++]=*b;
	b=a;
	return b;
}


//LED����
void led_task(void *pdata)
{
	u8 i=0;
	while(1)
	{
		delay_ms(200);
		if(LED_BZ)
		{
			LED0=!LED0;		//��ʾϵͳ����д����Ԫ��
		}
		else
		{
			if(++i==5)
			i=0,LED0=!LED0;	//��ʾϵͳ��������
		}
		switch(Emwled)
		{
			case 0:			//�ر�ָʾ
				EmwLED1=0;
				EmwLED2=0;
				break;
			case 1:			//��������wifi
				EmwLED1=!EmwLED1;
				break;
			case 2:			//�������Ӱ���  ��  �����ϱ�����
				EmwLED1=1;
				EmwLED2=!EmwLED2;
				break;
			case 3:			//�ϱ����
				EmwLED2=0;
				break;
		}
//		switch(Emwled2)
//		{
//			case 0:
//				Emw2LED1=0;
//				Emw2LED2=0;
//				break;
//			case 1:
//				Emw2LED1=!Emw2LED1;
//				break;
//			case 2:
//				Emw2LED1=1;
//				Emw2LED2=!Emw2LED2;
//				break;
//			case 3:
//				Emw2LED2=0;
//				break;
//		}
	}				 
}
//����������
void touch_task(void *pdata)
{	  	
//	u32 cpu_sr;
// 	u16 lastpos[2];		//���һ�ε����� 
	while(1)
	{
		delay_ms(5);	 
	}
}     
//������Ϣ��ʾ����
void qmsgshow_task(void *pdata)
{
}
//������
void main_task(void *pdata)
{							 
//	u32 key=0;	
//	u8 err;	
//	u8 alfredTest[3] = {0X41,0X54,0X0D};
	
	//comSendBuf(COM3,alfredTest,sizeof(alfredTest) );
// 	u8 tmr2sta=1;	//�����ʱ��2����״̬   
// 	u8 tmr3sta=0;	//�����ʱ��3����״̬
//	u8 flagsclrt=0;	//�ź�������ʾ���㵹��ʱ   
	//tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100msִ��һ��
 	while(1)
	{
		delay_ms(300);
		//comSendBuf(COM3,alfredTest,sizeof(alfredTest) );

			
	}
}		   
//�ź�������������
void flags_task(void *pdata)
{	
//	u16 flags;	
//	u8 err;	    						 
	while(1)
	{
		delay_ms(50);
 	}
}
   		    
//����ɨ������
void key_task(void *pdata)
{	
	while(1)
	{
 		delay_ms(10);
	}
}
