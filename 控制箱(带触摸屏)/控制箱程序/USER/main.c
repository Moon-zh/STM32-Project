#include "includes.h"
#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "my_HDMI.h"
#include "G405tf.h"
#include "kz.h"

void	UploadState(void);

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
#define LED_TASK_PRIO       			6 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void LED_task(void *pdata);

//�����������ȼ�
#define HDMI_TASK_PRIO       			5 
//���������ջ��С
#define HDMI_STK_SIZE  		    		512
//�����ջ	
OS_STK HDMI_TASK_STK[HDMI_STK_SIZE];
//������
void HDMI_task(void *pdata);

//�����������ȼ�
#define Button_TASK_PRIO       			7 
//���������ջ��С
#define Button_STK_SIZE  		    	512
//�����ջ	
OS_STK Button_TASK_STK[Button_STK_SIZE];
//������
void Button_task(void *pdata);

//�����������ȼ�
#define AutoModel_TASK_PRIO       		3 
//���������ջ��С
#define AutoModel_STK_SIZE  		    1024
//�����ջ	
OS_STK AutoModel_TASK_STK[AutoModel_STK_SIZE];
//������
void AutoModel_task(void *pdata);

//�����������ȼ�
#define UpSky_TASK_PRIO       			4 
//���������ջ��С
#define UpSky_STK_SIZE  		    	512
//�����ջ	
OS_STK UpSky_TASK_STK[UpSky_STK_SIZE];
//������
void UpSky_task(void *pdata);

void	HDMI_Init()
{
	queue_reset();			//��ջ�����
	delay_ms(300);			//����ȴ�300ms
}

void	init()				//ϵͳ��ʼ��
{
	static qsize  size = 0;
	Model=0;
	SystemInit();
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�	
	comClearRxFifo(COM1);
	delay_ms(1500);
	bsp_InitUart();
	HDMI_Init();
	SetHandShake();			//����������
	delay_ms(100);
	size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //�ӻ������л�ȡһ��ָ��        
	if(size>0)//���յ�ָ��
	{
		ProcessMessage((PCTRL_MSG)cmd_buffer, size);//ָ���
	}
	while(!init_4G());
}

int 	main(void)
{	 
	init();
	OSInit();   
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}

//��ʼ����
void 	start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    					   
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(UpSky_task,(void *)0,(OS_STK*)&UpSky_TASK_STK[UpSky_STK_SIZE-1],UpSky_TASK_PRIO);
	if(Model)OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);
	else OSTaskCreate(Button_task,(void *)0,(OS_STK*)&Button_TASK_STK[Button_STK_SIZE-1],Button_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

void 	LED_task(void *pdata)		//LED����״ָ̬ʾ������
{	 
	while(1)
	{
		delay_ms(1000);
		LED0=!LED0;//��ʾϵͳ��������	
	}
}

u8 tabbutton[]={0x01,0x10,0x03,0xeb,0x00,0x01,0x02,0x00,0x01,0x43,0x8b};			//���ü̵���ģʽ

void	SetIoNormal()				//����IOģ��Ϊ����ģʽ
{
	u8 i,k;
	u16 crc;
	for(k=0;k<3;k++)
	{
		tabbutton[0]=0xFE;
		tabbutton[8]=0;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(110);
		tabbutton[0]=1;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(60);
	}
}

void	SetIoLinkage()				//����IOģ��Ϊ����ģʽ
{
	u16 crc;
	u8 i,k;
	for(k=0;k<3;k++)
	{
		tabbutton[0]=0xFE;
		tabbutton[8]=1;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(110);
		tabbutton[0]=1;
		crc=mc_check_crc16(tabbutton,9);
		tabbutton[9]=crc>>8;
		tabbutton[10]=crc&0xff;
		for(i=0;i<11;i++)
		comSendChar(COM4,tabbutton[i]);
		delay_ms(60);
	}
}

u8	sky_tab[]={0x01 ,0x45 ,0x00 ,0x00 ,0x00 ,0x0D ,0x02 ,0xff ,0xff ,0x85 ,0x13};	//���ؼĴ���

void	pdks(u16 data)		//�ϴ��̵���״̬����̨
{
	u16 crc;
	sky_tab[7]=data>>8;
	sky_tab[8]=data&0xff;
	crc=mc_check_crc16(sky_tab,9);
	sky_tab[9]=crc>>8;
	sky_tab[10]=crc&0xff;
	comSendBuf(COM1,sky_tab,11);
	delay_ms(100);
}

u8 tabks[]={0xfd,0x01,0x00,0x00,0x00,0x08,0x29,0xc3};								//�ϴ�����

void	UploadState()				//��ȡ���ϴ��̵���״̬
{
	u8 a[9];
	u16 crc;
	u16 ks_state=0;
	comClearRxFifo(COM4);
	tabks[0]=0xfd;					//��һ���̵����ĵ�ַ
	sendcmd_kz(tabks);
	COM4GetBuf(a,6);
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,4);
	if((a[4]==crc>>8)&&(a[5]==(crc&0xff)))ks_state=a[3];
	
	memset(a,0,8);
	delay_ms(100);
	tabks[0]=1;						//�ڶ����̵����ĵ�ַ
	sendcmd_kz(tabks);
	COM4GetBuf(a,6);			
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,4);
	if((a[4]==crc>>8)&&(a[5]==(crc&0xff)))ks_state=(ks_state<<8)|a[3];
	else ks_state<<=8;

	memset(a,0,8);
	pdks(ks_state);
	delay_ms(200);
}

u8	Auto_bz=1;			//�Զ�ģʽ������־

void 	HDMI_task(void *pdata)		//Һ�����������
{	  
	static qsize  size = 0;
	SetIoNormal();
	while(1)
	{
		delay_ms(200);
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //�ӻ������л�ȡһ��ָ��        
		if(size>0)//���յ�ָ��
		{
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//ָ���
		}
		if(ControlModel&Auto_bz)	//�ж������Զ�ģʽ
		{
			Auto_bz=0;
			OSTaskCreate(AutoModel_task,(void *)0,(OS_STK*)&AutoModel_TASK_STK[AutoModel_STK_SIZE-1],AutoModel_TASK_PRIO);
		}
//		if(SaveButton==1)			//���ð�ť���£���ȡ���õ�ֵ
//		{
//			GetControlValue(2,5);
//			delay_ms(50);
//			GetControlValue(2,7);
//			delay_ms(50);
//			GetControlValue(2,6);
//			delay_ms(50);
//			GetControlValue(2,8);
//			delay_ms(50);
//			SaveButton=0;
//		}
	}
}


u8	Button_bz=0;
void Button_task(void *pdata)		//�޴�����������
{
	SetIoLinkage();					//���ü̵���Ϊ����ģʽ	
	ControlModel=1;
	Auto_bz=0;
	UploadState();
	OSTaskCreate(AutoModel_task,(void *)0,(OS_STK*)&AutoModel_TASK_STK[AutoModel_STK_SIZE-1],AutoModel_TASK_PRIO);
	Button_bz=1;
	while(1)
	{
		delay_ms(200);
	}
}

void	controlks(u8 *data)			//�ж���ָ̨��̵���ִ�ж���	
{
	if(data[0]!=1)return;
	if(data[1]==5)
	{
		switch(data[3])
		{
			case 0:
					switch(data[4])
					{
						case 0xff:
								curtainopen();
								break;
						case 0:
								curtainclose();
								break;
						default:break;
					}
					break;
			case 2:
					switch(data[4])
					{
						case 0:
								fan1stop();
								break;
						case 0xff:
								fan1open();
								break;
						default:break;
					}
					break;
			case 3:
					switch(data[4])
					{
						case 0:
								fan1stop();
								break;
						case 0xff:
								fan1close();
								break;
						default:break;
					}
					break;
			case 4:
					switch(data[4])
					{
						case 0:
								fan2stop();
								break;
						case 0xff:
								fan2open();
								break;
						default:break;
					}
					break;
			case 5:
					switch(data[4])
					{
						case 0:
								fan2stop();
								break;
						case 0xff:
								fan2close();
								break;
						default:break;
					}
					break;
			case 6:
					switch(data[4])
					{
						case 0:
								fan3stop();
								break;
						case 0xff:
								fan3open();
								break;
						default:break;
					}
					break;
			case 7:
					switch(data[4])
					{
						case 0:
								fan3stop();
								break;
						case 0xff:
								fan3close();
								break;
						default:break;
					}
					break;
			case 8:
					switch(data[4])
					{
						case 0:
								filllight1open();
								break;
						case 0xff:
								filllight1close();
								break;
						default:break;
					}
					break;
			case 9:
					switch(data[4])
					{
						case 0:
								filllight2open();
								break;
						case 0xff:
								filllight2close();
								break;
						default:break;
					}
					break;
			case 0X0A:
					switch(data[4])
					{
						case 0:
								filllight3open();
								break;
						case 0xff:
								filllight3close();
								break;
						default:break;
					}
					break;		
			case 0x0B:
					switch(data[4])
					{
						case 0:
								moregasopen();
								break;
						case 0xff:
								moregasclose();
								break;
						default:break;
					}
					break;	
			case 0X0C:
					switch(data[4])
					{
						case 0:
								sprayopen();
								break;
						case 0xff:
								sprayclose();
								break;
						default:break;
					}
					break;
					
			default:break;
		}
	}
	delay_ms(100);
}

static u8 ks_tab[8];	//�̵�������ָ��洢
static u8 ks_bz=0;		//�ж��Ƿ�ı�̵���״̬

void	readks()					//�ж��Ƿ����̨�յ��̵�������ָ��
{
	u8 b[21];
	u8 a[8];
	u16 crc;
	u8 len;
	len=COM1GetBuf(b,20);
	if(len<6)return;
	if(b[1]==0x45)
	{
		for(len=0;len<8;len++)
		{
			a[len]=b[len+8];
		}
	}
	else
	{
		for(len=0;len<8;len++)
		{
			a[len]=b[len];
		}
	}
	comClearRxFifo(COM1);
	crc=mc_check_crc16(a,6);
	if((a[6]==crc>>8)&&(a[7]==(crc&0xff)))
	{
		for(;len>0;len--)ks_tab[len]=a[len];
		ks_tab[0]=a[0];
		ks_bz=1;
	}
}

void AutoModel_task(void *pdata)	//�Զ�ģʽ
{
	while(1)
	{
		delay_ms(200);
		readks();					//���������Ƿ���Զ��ң������
		if(!ControlModel)			//�л����ֶ�ģʽ
		{
			Auto_bz=1;
			OSTaskSuspend(AutoModel_TASK_PRIO);
		}	
	}
}

void	UpSky_task(void *pdata)
{
	while(1)
	{
		delay_ms(200);
		if(!ControlModel)UploadState();	//�Զ�ģʽ�²������ϴ�����
		if(Button_bz)UploadState();
		if(ks_bz)					//�ж��Ƿ���Ҫ�ı�̵���״̬
		{
			ks_bz=0;
			controlks(ks_tab);
			delay_ms(100);
			UploadState();
		}	
	}
}
