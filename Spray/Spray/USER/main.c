#include "main.h"

void	init()							//ϵͳ��ʼ��
{
	SystemInit();
	delay_init();	    	 		  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();		  					
	bsp_InitUart();
	comClearRxFifo(COM4);
	memset(g_RxBuf4,0,UART4_RX_BUF_SIZE);
	memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
	comClearRxFifo(COM5);
	delay_ms(1000);
	W25QXX_Init();				//�ⲿflash��ʼ��
	readflashthree();			//��ȡ��Ԫ��
}

int 	main(void)						//������
{	 
	u8 err;
	init();
	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	tmr1=OSTmrCreate(0,100,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,(u8 *)tmr1,&err);
	OSTmrStart(tmr1,&err);
	OSStart();
}

void 	start_task(void *pdata)			//�̳߳�ʼ��
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();						   
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);							//LED������
	OSTaskCreate(Main_task,(void *)0,(OS_STK*)&Main_TASK_STK[Main_STK_SIZE-1],Main_TASK_PRIO);						//��������
	OSTaskCreate(Upyun_task,(void *)0,(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],Upyun_TASK_PRIO);					//NB����
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);	//��Ԫ��洢����
	OSTaskSuspend(START_TASK_PRIO);	
	OS_EXIT_CRITICAL();				
}

void	Main_task(void	*pdata)			//��������
{
	while(1)
	{
		delay_ms(200);
		ttm=0;
		if(Run)
		{
			IO_OutSet(1,1);
			while((Time>0)&Run)delay_ms(200);
		}
		IO_OutSet(1,0);
	}
}

void	readset()						//��ȡ�����·�������
{
	u8 len;cJSON *json;
	u8 buf[250];char *jsa;
	if(n21_set)return;
	len=COM1GetBuf(buf,100);
	if(len>10)
	{
		delay_ms(200);COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"RCVPUB")[0]!='R'){memset(buf,0,200);comClearRxFifo(COM1);return;}
		json=cJSON_Parse(strstr((const char *)buf,"{"));
		Time=atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Timing")));
		Run=atoi(jsa=cJSON_Print(cJSON_GetObjectItem(json,"Switch")));
		cJSON_Delete(json);
		cJSON_free(jsa);
	}
	memset(buf,0,200);
}

void	Uptoaliyun()					//�ϴ��豸״̬
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\\\"Switch\\\":%d,",!(u8)Back);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Timing\\\":%d",Time);		for(;*b;b++)a[i++]=*b;
	Nsendok=sendN21(a);
}

void	Upccid()						//�ϱ�CCID
{            
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	Nsendok=sendN21(b);
}

void 	Upyun_task(void *pdata)			//�ϴ������� N21
{
	u8 i;u8 state=0;
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
		readset();						//��ȡ�������·�����
		if(state!=(!Back))state=(!Back),Uptoaliyun();
		if(nbup)
		{
			nbup=0;
			n21_set=1;
			comClearRxFifo(COM1);
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
			n21_set=0;
			Uptoaliyun();
		}
		if(!Nsendok){n21_set=1;N21_B=0;NeoWayN21_init();conN21();n21_set=0;N21_B=1;}	
	}
}

void 	LED_task(void *pdata)			//LED����
{	 
	u8 i=0;
	while(1)
	{
		delay_ms(200);
		if(LED_BZ)
		{
			LED0=!LED0;		
		}
		else
		{
			if(++i==5)
			i=0,LED0=!LED0;	
		}
	}
}

void	readthree()						//ʶ����Ԫ��
{
	u8 i;
	char hc[250];
	char *msg=hc;
	memset(hc,0,sizeof hc);
	i=0;
	if(strstr((const char *)g_RxBuf4,"m1:")[0]=='m')
	{
		msg=strstr((const char *)g_RxBuf4,"PK:");
		if(msg[1]=='K')
		{
			memset(ProductKey0,0,20);
			msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey0[i++]=*msg;	if(i>=20)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DN:");
		if(msg[1]=='N')
		{
			memset(DeviceName0,0,50);
			msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName0[i++]=*msg;	if(i>=50)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DS:");
		if(msg[1]=='S')
		{
			memset(DeviceSecret0,0,50);
			msg+=3;for(i=0;*msg;msg++)		{DeviceSecret0[i++]=*msg;if(i>=50)return ;}
		}
		sendflashthree();
		comSendBuf(COM4,g_RxBuf4,strlen((char *)g_RxBuf4));
		comClearRxFifo(COM4);
		memset(g_RxBuf4,0,UART4_RX_BUF_SIZE);
	}	
}

void 	SaveThree_task(void *pdata)		//��Ԫ������
{
	while(1)
	{
		delay_ms(200);
		readthree();							
	}
}
