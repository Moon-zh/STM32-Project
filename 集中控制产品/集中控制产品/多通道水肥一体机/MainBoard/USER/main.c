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
#include "UserCore.h" 
#include "PCHmi.h"
#include "FlashDivide.h"
#include "UserHmi.h"
#include "communication.h" 
#include "stmflash.h"
#include "Emw3060.h"
/************************************************
 ALIENTEKս��STM32������ʵ��53
 UCOSIIʵ��3-��Ϣ���С��ź������������ʱ��  ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
#define FLASH_THREE_ADDR  0X0802FE00


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
#define CHECK_TASK_PRIO       			9
//���������ջ��С
#define CHECK_STK_SIZE  					512
//�����ջ	
OS_STK CHECK_TASK_STK[CHECK_STK_SIZE];
//������
void CHECKCtrl_task(void *pdata);


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
#define MASTER_TASK_PRIO       			4 
//���������ջ��С
#define MASTER_STK_SIZE  					512
//�����ջ	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//������
void MasterCtrl_task(void *pdata);

//�ź���������
//�����������ȼ�
#define PCHMI_TASK_PRIO       			3 
//���������ջ��С
#define PCHMI_STK_SIZE  		 		512
//�����ջ	
OS_STK PCHMI_TASK_STK[PCHMI_STK_SIZE];
//������
void PCHmi_task(void *pdata);

//�ź���������
//�����������ȼ�
#define FERLOGIC_TASK_PRIO       			11
//���������ջ��С
#define FERLOGIC_STK_SIZE  		 		512
//�����ջ	
OS_STK FERLOGIC_TASK_STK[FERLOGIC_STK_SIZE];
//������
void FerLogic_task(void *pdata);
//GPRS����
//�����������ȼ�
#define GPRS_TASK_PRIO       			5 
//���������ջ��С
#define GPRS_STK_SIZE  		 			1024
//�����ջ	
OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//������
void network_task(void * pdata);

//��������
//�����������ȼ�
#define HMI_TASK_PRIO       			8 
//���������ջ��С
#define HMI_STK_SIZE  		 			256
//�����ջ	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//������
void Task_HMIMonitor(void * pdata);


//����ɨ������
//�����������ȼ�
#define KEY_TASK_PRIO       			2 
//���������ջ��С
#define KEY_STK_SIZE  					516
//�����ջ	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);

u8 RemoteQInit(void); 
extern CtaskRecordunion ctask_record;//��¼����
extern u8 task_record_buff[1500]; //���ڴ洢100�������¼
	u8 count=0;
	u8 flag_record=0;
u8 Current_task_num=1;//��ǰ�����
extern Targetred ctargetrcord;//��ǰ�����¼
extern u8 set_Pressbutonn[2];//����ѹ�����ؼ�����
extern FlashSet flashset;//���ò�������
//////////////////////////////////////////////////////////////////////////////
    
OS_EVENT * msg_key;			//���������¼���	  
OS_EVENT * q_msg;			//��Ϣ����
OS_TMR   * tmr1;			//�����ʱ��1
OS_TMR   * tmr2;			//�����ʱ��2
OS_TMR   * tmr3;			//�����ʱ��3
OS_FLAG_GRP * flags_key;	//�����ź�����
void * MsgGrp[256];			//��Ϣ���д洢��ַ,���֧��256����Ϣ


//�����ʱ��3�Ļص�����				  	   
void tmr3_callback(OS_TMR *ptmr,void *p_arg) 
{	
	//u8* p;	 
} 


 int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	//uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
  	BSP_InitIO();				//��ʼ��IO
  	GPS_Init();					//��ʼ��GPS
//	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
   	W25QXX_Init();
   	bsp_InitUart();				//��ʼ��RS485
	 //TIM4_Config();//���ö�ʱ��4
	OSInit();  	 				//��ʼ��UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
//	u8 err;	    	    

	pdata = pdata; 	
	RTC_Init();
	OSStatInit();						//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();				//�����ٽ���(�޷����жϴ��)   
 	RemoteQInit();	
	FlashReadPara((u8*)flashset.FlashPara,0,76);
	FlashReadtaskrecord(task_record_buff,ADDR_TASKRECORD ,1600);
	FlashReadtargetecord(ctargetrcord.target,67);//����Ϊ��ȡ�������˴�Ϊ�˴�ͷ��ʼ
	if(ctargetrcord.trecord.currenttarget_id==0xff)
  {
		ctargetrcord.trecord.curtargetnum=0; //����û�����0��ʼ
	ctargetrcord.trecord.curtarget_mode=0;
	}
	else
	{
		ctargetrcord.trecord.curtarget_mode=0;
		ctask_record.record.task_num=ctargetrcord.trecord.curtargetnum;
	}
	W25QXX_Read( set_Pressbutonn, ADDR_SETBUTONN, 2 );
//	FlashReadtargetecord(ctargetrcord.target,51);
//  if(ctargetrcord.trecord.curtargetnum>100) //��ֹ�����
//		ctargetrcord.trecord.curtargetnum=0;
//	ctask_record.record.task_num=ctargetrcord.trecord.curtargetnum;//��ȡ���洢������ű��⸲��
//	FlashReadFixedPara(g_u8SysTemPassword,22,8);
//OSTaskCreate(ZoneCtrl_task,(void *)0,
//		(OS_STK*)&ZONE_TASK_STK[ZONE_STK_SIZE-1],
//		ZONE_TASK_PRIO);	 	
//	
 	OSTaskCreate(key_task,(void *)0,
		(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],
		KEY_TASK_PRIO);	 				

 	OSTaskCreate(network_task,(void *)0,
		(OS_STK*)&GPRS_TASK_STK[GPRS_STK_SIZE-1],
		GPRS_TASK_PRIO);	 
	
 	OSTaskCreate(FerLogic_task,(void *)0,
		(OS_STK*)&FERLOGIC_TASK_STK[FERLOGIC_STK_SIZE-1],
		FERLOGIC_TASK_PRIO);		
 	OSTaskCreate(MasterCtrl_task,(void *)0,
		(OS_STK*)&MASTER_TASK_STK[MASTER_STK_SIZE-1],
		MASTER_TASK_PRIO);	 	
	
 	OSTaskCreate(CHECKCtrl_task,(void *)0,
		(OS_STK*)&CHECK_TASK_STK[CHECK_STK_SIZE-1],
		CHECK_TASK_PRIO);	 	  
 	OSTaskCreate(Task_HMIMonitor,(void *)0,
		(OS_STK*)&HMI_TASK_STK[HMI_STK_SIZE-1],
		HMI_TASK_PRIO);	
// 	OSTaskCreate(PCHmi_task,(void *)0,
//		(OS_STK*)&PCHMI_TASK_STK[PCHMI_STK_SIZE-1],
//		PCHMI_TASK_PRIO);	
	  TIM3_Int_Init(9999,7199);//10Khz�ļ���Ƶ�ʣ�������10000Ϊ1s  
// 	OSTaskSuspend(START_TASK_PRIO);		//������ʼ����.
	OS_EXIT_CRITICAL();					//�˳��ٽ���(���Ա��жϴ��)
	while(1)
	{
		delay_ms(500);
//		writetagend(1,1);
		target_updat();
//
//	board2com( 1,1,0x0f);		
//		count++;
//		switch(count)
//	  {
//			case 1:
//			board2com( 1,1,0x0f);

//			break;
//			case 2:
//			board2com( 1,0,0x07);
//		
//			break;
//			case 3:
//			board2com( 1,0,0x03);
//		
//			break;
//			case 4:
//			board2com( 1,0,0x01);
//		
//			break;	
//			case 5:
//			board2com( 1,0,0);
//			count=0;		
//			break;				
//			default:
//			break;
//		
//		}
//		//���������¼����
//		if((ctask_record.record.task_num<30)&&(flag_record==0))
//		{
//		 ctask_record.record.task_num++;
//		 ctask_record.record.start_year=calendar.w_year;
//     ctask_record.record.start_month=calendar.w_month;	
//		 ctask_record.record.start_day=calendar.w_date;
//		 ctask_record.record.start_hour=calendar.hour;
//     ctask_record.record.start_minitue=calendar.min;
//     ctask_record.record.start_second=calendar.sec;
//     ctask_record.record.irrigated_mode=1; //��ˮ���		 
//		 ctask_record.record.irrigated_time=120;//���ʱ��
//		 ctask_record.record.fertilizing_amount=100;
//		 ctask_record.record.stop_mode++;
//		 Save_TaskRecord(ctask_record.record.task_num,0);
//		}
//		if(ctask_record.record.task_num==30)
//		{
//			flag_record=1;
//		}
//		if(count>0)
//		{
//			Save_TaskRecord(count,1);
//			count=0;
//		}
	
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
}
void	sendflashthree(u8 group)				//���ڴ�д�����õ���Ԫ������
{
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)
	STMFLASH_Write(group*100+FLASH_THREE_ADDR,(u16*)"PK:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+4,(u16*)ProductKey0,sizeof(ProductKey0));
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+24,(u16*)"DN:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+28,(u16*)DeviceName0,sizeof(DeviceName0));
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+78,(u16*)"DS:0",4);
	STMFLASH_Write(group*100+FLASH_THREE_ADDR+82,(u16*)DeviceSecret0,sizeof(DeviceSecret0));
	readflashthree();
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}
extern uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];
void	readthree()								//��COM3 485���ڶ�ȡ�Ƿ���������Ԫ������
{
	u8 i;
	char hc[250];
	char *msg=hc;
	u8 buf[250];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	i=0;
//	len=COM3GetBuf(buf,200);
	memcpy(buf,g_RxBuf3,200);
//	if(len>10)
//	{
		if(strstr((const char *)buf,"m1")[0]=='m')
		{
			OSTaskSuspend(CHECK_TASK_PRIO);
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
			memset(g_RxBuf3,0,sizeof g_RxBuf3);
			comClearRxFifo(COM3);
			comSendBuf(COM3,buf,strlen((char *)buf));
			memset(buf,0,sizeof buf);
			memset(hc,0,sizeof buf);
		}
//	}
}
void key_task(void *pdata)
{	
	readflashthree();
	while(1)
	{
		delay_ms(1000);
		readthree();					//�ж��Ƿ�ɨ��ǹ������Ԫ������
	}
}

