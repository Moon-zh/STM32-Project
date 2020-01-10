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
#include "stmflash.h"
#include "FlashDivide.h"

#include	"iwdg.h" //�������Ź������ļ�
#include	"UserHmi.h"
#include	"cgq.h"//�����������ļ�


 
void	readflashthree(void);					//��flash�ж�ȡ��Ԫ������



/************************************************
 ALIENTEKս��STM32������ʵ��53
 UCOSIIʵ��3-��Ϣ���С��ź������������ʱ��  ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


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


//������
//�����������ȼ�
#define MASTER_TASK_PRIO       			4 
//���������ջ��С
#define MASTER_STK_SIZE  					512
//�����ջ	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//������
void MasterCtrl_task(void *pdata);


//wifi����
//�����������ȼ�
#define GPRS_TASK_PRIO       			5 
//���������ջ��С
#define GPRS_STK_SIZE  		 			1024
//�����ջ	
__align(8)OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//������
void network_task(void * pdata);


//��������������
//�����������ȼ�
#define HMI_TASK_PRIO       			8 
//���������ջ��С
#define HMI_STK_SIZE  		 			1024
//�����ջ	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//������
void Task_HMIMonitor(void * pdata);


//��Ԫ������
//�����������ȼ�
#define KEY_TASK_PRIO       			3 
//���������ջ��С
#define KEY_STK_SIZE  					1024
//�����ջ	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);


//LED����
//�����������ȼ�
#define LED_TASK_PRIO       			7 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);

//������ʪ�ȴ���������
//�����������ȼ�
#define CGQ_TR_TASK_PRIO       			11 
//���������ջ��С
#define CGQ_TR_STK_SIZE  		    		64
//�����ջ
OS_STK CGQ_TR_TASK_STK[CGQ_TR_STK_SIZE];
//������
void cgq_tr_task(void *pdata);


//��Ԫ���FLASH�洢��ַ
#define FLASH_THREE_ADDR  0X0802FE00
//////////////////////////////////////////////////////////////////////////////
    




int main(void)
{	 	
	char tiaoshi[180];
	u8 ts_flash;

	
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�

	//uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
  	BSP_InitIO();				//��ʼ��IO(��������)
  	GPS_Init();					//��ʼ��GPS��GPS��ڴ��ģ�
//	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
   	W25QXX_Init();     //W25QXX��ʼ��
   	bsp_InitUart();				//��ʼ������ RS485
		RTC_Init();	  			//RTC��ʼ��
	  HDMI_Init();    //��������ʼ��
	  TIM3_Int_Init(9999,7199);//10Khz�ļ���Ƶ�ʣ�������10000Ϊ1s
//	  IWDG_Init(4,1250);//�������Ź���ʼ�� ���Ƶ��Ϊ64,����ֵΪ1250,���ʱ��Ϊ2s		 explorer.exe
    /*
			����������Ź���������Ҫ�������ڵĹ���״̬��ʹ��flash���档
	  */

	
	/*����   Ԥ��flash�еĴ洢��Ϊ�ո�*/
   FlashReadIP(&ts_flash);//��ȡ�ǲ���֮ǰ���ù�flash
	if(ts_flash!=55)//֮ǰû����flash����д��0x5656
	{
			strcpy(&tiaoshi[0],"mingji"); //wifi����IoT
			strcpy(&tiaoshi[30],"mingji2015");//wifi����ag20190520
			strcpy(&tiaoshi[60],"192.168.7.101");   //IP��ַ  "192.168.7.101";
			strcpy(&tiaoshi[90],"255.255.255.0"); //��������  "255.255.255.0";
			strcpy(&tiaoshi[120],"192.168.7.1");  //���ص�ַ  	"192.168.7.1";
			strcpy(&tiaoshi[150],"114.114.114.114"); //����  "114.114.114.114";
			FlashWriteWiFi((u8 *)tiaoshi);//����FLASH����		
		
			ts_flash=55;
		  FlashWriteIP(&ts_flash);//��0x5656д��flash��������
	}
	
   //sec:�� min���� hour��ʱ day���� week������ mon���� year:��
	//SetRTC(0x05,0x14,0x17,0x30,0x2,0x7,0x19); //���� ���ô�����RTCʱ��


		
   	OSInit();  	 				//��ʼ��UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	  OSStart();	  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����
void start_task(void *pdata)
{
   OS_CPU_SR cpu_sr=0;	    	    
	 pdata = pdata; 
	
	
	OSStatInit();						//��ʼ��ͳ������.�������ʱ1��������
	
	readflashthree();//��flash�ж�ȡ��Ԫ������
	
	OS_ENTER_CRITICAL();				//�����ٽ���(�޷����жϴ��)  
	
	
	//wifiEMW����
 	OSTaskCreate(network_task,(void *)0,       
		(OS_STK*)&GPRS_TASK_STK[GPRS_STK_SIZE-1],
		GPRS_TASK_PRIO);	 
	
	//������
 	OSTaskCreate(MasterCtrl_task,(void *)0,
		(OS_STK*)&MASTER_TASK_STK[MASTER_STK_SIZE-1],
		MASTER_TASK_PRIO);	 	
	
	//��������������
 	OSTaskCreate(Task_HMIMonitor,(void *)0,
		(OS_STK*)&HMI_TASK_STK[HMI_STK_SIZE-1],
		HMI_TASK_PRIO);	 		
			   
	//��Ԫ������
 	OSTaskCreate(key_task,(void *)0,
		(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],
		KEY_TASK_PRIO);	 				

	//LED����
 	OSTaskCreate(led_task,(void *)0,
		(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],
		LED_TASK_PRIO);	

	//������ʪ�ȴ���������
	OSTaskCreate(cgq_tr_task,(void *)0,
		(OS_STK*)&CGQ_TR_TASK_STK[CGQ_TR_STK_SIZE-1],
		CGQ_TR_TASK_PRIO);		
 	
	
 	OSTaskSuspend(START_TASK_PRIO);		//������ʼ����.
	OS_EXIT_CRITICAL();					//�˳��ٽ���(���Ա��жϴ��)
}



//LED����
void led_task(void *pdata)
{
	
	while(1)
	{

		delay_ms(10);

	}									 
}


//������ʪ�ȴ���������
void cgq_tr_task(void *pdata)
{
	
  FlashReadZDMS_QDYZ(&Guangai.qdyzbz);//��flash�ж�ȡ������ֵ
	if(Guangai.qdyzbz==0XFF)
	{
			Guangai.qdyzbz=16;
			FlashWriteZDMS_QDYZ(&Guangai.qdyzbz);//��flash��д��������ֵ		
	}
	
  FlashReadZDMS_TZYZ(&Guangai.tzyzbz);//��flash�ж�ȡֹͣ��ֵ
	if(Guangai.tzyzbz==0XFF)
	{
			Guangai.tzyzbz=24;
			FlashWriteZDMS_TZYZ(&Guangai.tzyzbz);//��flash��д��ֹͣ��ֵ		
	}	
	
	while(1)
	{		
		delay_ms(10);
		
    CGQ_cx();//����������
	}									 
}	   



void	readflashthree(void)						//��flash�ж�ȡ��Ԫ������
{
	char buf[150];
	char *msg=buf;
	u8 i;
 	STMFLASH_Read(FLASH_THREE_ADDR,(u16*)buf,75);
	if((buf[0]==0)|(buf[0]==0xff))return;
	if(buf[1]=='K')
	{
		memset(ProductKey0,0,20);
		msg=buf+4;for(i=0;*msg;msg++)		ProductKey[i++]=*msg;
	}
	if(buf[25]=='N')
	{
		memset(DeviceName0,0,20);
		msg=buf+25+3;for(i=0;*msg;msg++)	DeviceName[i++]=*msg;
	}
	if(buf[79]=='S')
	{
		memset(DeviceSecret0,0,40);
		msg=buf+79+3;for(i=0;*msg;msg++)	DeviceSecret[i++]=*msg;
	}
	
	//STMFLASH_Read(100+FLASH_THREE_ADDR,(u16*)buf,50);
	//if((buf[0]==0)||(buf[0]==0xff))return;
//	if(buf[1]=='K')
//	{
//		memset(ProductKey1,0,20);
//		msg=buf+4;for(i=0;*msg;msg++)		ProductKeyw[i++]=*msg;
//	}
//	if(buf[25]=='N')
//	{
//		memset(DeviceName1,0,20);
//		msg=buf+25+4;for(i=0;*msg;msg++)	DeviceNamew[i++]=*msg;
//	}
//	if(buf[49]=='S')
//	{
//		memset(DeviceSecret1,0,40);
//		msg=buf+49+4;for(i=0;*msg;msg++)	DeviceSecretw[i++]=*msg;
//	}
//	printf("%s,%s,%s",ProductKey0,DeviceName0,DeviceSecret0);
}


void	sendflashthree(u8 group)				//���ڴ�д�����õ���Ԫ������
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)
	//LED_BZ=1;
	STMFLASH_Write(group*100+FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+82,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	readflashthree();
	//LED_BZ=0;
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
//	disconN21();
//	delay_ms(2000);
//	conN21();
//	printf("%s,%s,%s",ProductKey0,DeviceName0,DeviceSecret0);
}


void	readthree()								//��COM3 485���ڶ�ȡ�Ƿ���������Ԫ������
{
	u8 len,i;
	char hc[250];
	char *msg=hc;
	u8 buf[250];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	len=0;i=0;
	len=COM3GetBuf(buf,200);
	if(len>10)
	{
		if(strstr((const char *)buf,"m1")[0]=='m')
		{
			msg=strstr((const char *)buf,"PK:");
			if(msg[1]=='K')
			{
				memset(ProductKey0,0,20);
				msg+=3;
				for(i=0;*msg!=',';msg++)	
				{
					ProductKey[i++]=*msg;	
					if(i>=20)return ;
				}
			}
			msg=strstr((const char *)buf,"DN:");
			if(msg[1]=='N')
			{
				memset(DeviceName0,0,50);
				msg+=3;
				for(i=0;*msg!=',';msg++)	
				{
					DeviceName[i++]=*msg;	
					if(i>=50)
						return ;
				}
			}
			msg=strstr((const char *)buf,"DS:");
			if(msg[1]=='S')
			{
				memset(DeviceSecret0,0,40);
				msg+=3;
				for(i=0;*msg;msg++)		
				{
					DeviceSecret[i++]=*msg;
					if(i>=40)return ;
				}
			}
			sendflashthree(0);
			comSendBuf(COM3,buf,strlen((char *)buf));
			comClearRxFifo(COM3);
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
			len=1;
		}
		/*else if(strstr((const char *)buf,"m2")[0]=='m')
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
				memset(DeviceSecret1,0,40);
				msg+=3;for(i=0;*msg;msg++)		{DeviceSecretw[i++]=*msg;if(i>=40)return ;}
			}
			comSendBuf(COM3,buf,strlen((char *)buf));
			comClearRxFifo(COM3);
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
			len=2;
		}*/
		else return;
		
	}
}


//��Ԫ������
void key_task(void *pdata)
{	
	while(1)
	{
		delay_ms(200);
		readthree();					//�ж��Ƿ�ɨ��ǹ������Ԫ������  	//��COM3 485���ڶ�ȡ�Ƿ���������Ԫ������
	}
}
