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
	FlashReadMem(Logmem.mem);	//��ȡ��־��ַ
	readflashthree();			//��ȡ��Ԫ��
	ReadNet=1;					//��ȡ�������
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
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);	//��Ԫ��洢����
	OSTaskCreate(UpyunWF_task,(void *)0,(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],UpyunWF_TASK_PRIO);			//WIFI����
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);						//����������
	OSTaskCreate(IO_task,(void *)0,(OS_STK*)&IO_TASK_STK[IO_STK_SIZE-1],IO_TASK_PRIO);								//IO����
	OSTaskCreate(Model_task,(void *)0,(OS_STK*)&Model_TASK_STK[Model_STK_SIZE-1],Model_TASK_PRIO);					//ģʽʶ�� �� �ֶ�ģʽ ����
	OSTaskCreate(Work_task,(void *)0,(OS_STK*)&Work_TASK_STK[Work_STK_SIZE-1],Work_TASK_PRIO);						//�������
	OSTaskCreate(Plan_task,(void *)0,(OS_STK*)&Plan_TASK_STK[Plan_STK_SIZE-1],Plan_TASK_PRIO);						//�ƻ�����
	OSTaskCreate(Collection_task,(void *)0,(OS_STK*)&Collection_TASK_STK[Collection_STK_SIZE-1],Collection_TASK_PRIO);//�ɼ�����������
	OSTaskSuspend(START_TASK_PRIO);	
	OS_EXIT_CRITICAL();				
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
		switch(Emwled)
		{
			case 0:
				EmwLED1=0;
				EmwLED2=0;
				break;
			case 1:
				EmwLED1=!EmwLED1;
				break;
			case 2:
				EmwLED1=1;
				EmwLED2=!EmwLED2;
				break;
			case 3:
				EmwLED2=0;
				break;
		}
	}
}

void 	HDMI_task(void *pdata)			//�������������
{	  
	u16 LogAddr;
	u8 i;
	while(1)
	{
		delay_ms(200);
		HDMI_Check_Button();										//��ȡ��Ļ����״̬
		if(Irrsign==1)HDMI_Check_Irrigation_time(),Irrsign=0;		//��ȡ���ʱ��
		if(Remsign==1)HDMI_Set_Remaining_time(Remaining),Remsign=0;	//���õ���ʱʱ��
		if(Cursing==1)HDMI_Set_Current_Partition(Current),Cursing=0;//���ù�ȷ���
		if(ReadNet==1)												//��ȡ��Ļ�������
		{
			HDMI_Check_Net(MCGS_SSID,ssid);
			HDMI_Check_Net(MCGS_PASSWORD,password);
			HDMI_Check_Net(MCGS_IP,ip);
			HDMI_Check_Net(MCGS_NETWORK,network);
			HDMI_Check_Net(MCGS_GATEWAY,gateway);
			HDMI_Check_Net(MCGS_DNS,dns);
			HDMI_Check_Acquisition_Frequency();
			ReadNet=0;
		}
		if(TimeButton)SysTime=1,HDMI_Set_Partition(MCGS_Partition&0x7F);//����ʱ��ͬ��
		if(SysTime==2)HDMI_SetSysTime(),SysTime=0;						//����ʱ��ͬ��
		if(Err)HDMI_Set_Error(),Err=0;									//��������
		if(GetTime)HDMI_Check_SysTime(),GetTime=0;						//��ȡ������ʱ��
		if(ReadPlan)HDMI_Check_Plan(PlanAddr),ReadPlan=0;				//��ȡ�ƻ�����
		
		if(SetRun)														//������Ļ���� ���ڼƻ�����
		{
			HDMI_Set_Partition(HC_Partition);
			HDMI_Set_Button(HC_IrrMode);
			SetRun=0;
		}
		
		if(LogButton)					//��־��ѯ��ť����
		{
			HDMI_Check_LogPage();		//��ѯ��ǰҳ��
			LogAddr=(LogPage-1)*200;	//ָ���Ӧ�洢��ַ
			for(i=0;i<10;i++)			//��ʾ10����־
			{
				while(Logwait)delay_ms(200);						//�ȴ��ⲿflash�������
				Logwait=1;
				FlashReadLog(&Log.StartTime[0],LogAddr+i*20);		//��ȡ��־
				Logwait=0;
				HDMI_Set_Log((LogPage-1)*10+i+1,0x190+(i*17));		//��ʾ��־
			}
		}
	}
}

void 	UpyunWF_task(void *pdata)		//WIFI����
{
	u8 buf[50];u8 error=0,tt=0;
	delay_ms(1000);
	while(!WifiButton)delay_ms(200);	//WIFI���ش�
	while(ReadNet==1)delay_ms(200);		//���������ȡ���
	do	
	{
		Emw3060_init();
	}while(!Emw3060_con());
	Emw_B=1;emw_set=0;
	Uptoaliyun_wifi_Env(sensor[0],0);
	while(1)
	{
		delay_ms(200);
		if(WifiButton)
		{
			if(error==255)				//�ж��Ƿ�����������WIFI
			{
				do	
				{
					Emw3060_init();
				}while(!Emw3060_con());
				error=0;
			}
			readset();					//��ȡ���·���ָ��
			if(tt!=ttm)					//10����һ��WIFI�Ƿ�����
			{
				tt=ttm;
				printf_num=1;
				printf("AT+WJAPS\r");
				delay_ms(50);
				COM1GetBuf(buf,45);
				if(strstr((const char *)buf,"STATION_UP")[0]!='S')
				{
					if(++error==10)		//��������
					{
						emw_set=1;Emw_B=0;
						do	
						{
							Emw3060_init();
						}while(!Emw3060_con());
						Emw_B=1;emw_set=0;
					
					}
					continue;
				}
				else error=0;
			}
			if(upenv>=up_time){Uptoaliyun_wifi_Env(sensor[0],0);upenv=0;}	//�������ϴ�
			if(up_state)Uptoaliyun_wifi_State(),up_state=0;					//�豸״̬�ϴ�
			if(SysTime==1)ReadNetTime(),SysTime=2;							//ͬ������ʱ��
		}
		else	if(error!=255){Emw_B=0;Emw3060_init();error=255;Emwled=0;}	//�ر�WIFI����
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
		OSTaskSuspend(Collection_TASK_PRIO);
		msg=strstr((const char *)g_RxBuf4,"PK:");
		if(msg[1]=='K')
		{
			memset(ProductKey1,0,20);
			msg+=3;for(i=0;*msg!=',';msg++)	{ProductKey1[i++]=*msg;	if(i>=20)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DN:");
		if(msg[1]=='N')
		{
			memset(DeviceName1,0,50);
			msg+=3;for(i=0;*msg!=',';msg++)	{DeviceName1[i++]=*msg;	if(i>=50)return ;}
		}
		msg=strstr((const char *)g_RxBuf4,"DS:");
		if(msg[1]=='S')
		{
			memset(DeviceSecret1,0,50);
			msg+=3;for(i=0;*msg;msg++)		{DeviceSecret1[i++]=*msg;if(i>=50)return ;}
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

void 	IO_task(void *pdata)			//IO��������
{
	while(1)
	{
		delay_ms(200);
		CheckIO8();								//��ȡIO8״̬
		if(IO8Set)SetIO8(IO8SWITCH),IO8Set=0;	//����IO8���
	}
}

void 	Collection_task(void *pdata)	//�ɼ�����
{
	while(1)
	{
		delay_ms(2000);
		ReadValue();					//��ȡ����������
	}
}

void 	Model_task(void *pdata)			//ģʽʶ������	��	�ֶ�����
{
	u8	State=0;
	u8	state=0;
	u8	i;
	SysLog HLog;
	while(1)
	{
		delay_ms(200);
		if((!FerSwitch)|StirSwitch1|StirSwitch2|SolenoidSwitch1|SolenoidSwitch2)MODEL=ManualModel,ChoMode=0;	//�κο�����Ϊ��Ԥ��Ϊ�ֶ�ģʽ
		else if(MODEL==ManualModel)MODEL=localModel;		//���п��ض����Զ���ʱ  ��֮ǰ���ֶ�ģʽ���Ϊ����ģʽ
		if(MODEL==ManualModel)
		{
			if((State!=IO8STATE)|(state!=FerState))
			{
				HLog.Irrtime=0;								//���üƻ�ʱ��Ϊ0
				TimePopup=1;								//������ʱ��ͬ������
				for(i=0;i<10;i++)delay_ms(1000);			//�ȴ�10��������豸�����
				State=IO8STATE;								//����IO״̬
				state=FerState;
				up_state=1;									//�ϴ��豸״̬
				
				if(Emw_B)									//��ȡ��ǰʱ��  ���������
				{
					SysTime=1;
					while(SysTime)delay_ms(200);
					HLog.StartTime[0]=STIME.year%100;
					HLog.StartTime[1]=STIME.month;
					HLog.StartTime[2]=STIME.day;
					HLog.StartTime[3]=STIME.hour;
					HLog.StartTime[4]=STIME.minute;
				}
				else	
				{
					GetTime=1;while(GetTime)delay_ms(200);
					HLog.StartTime[0]=STIME.year%100;
					HLog.StartTime[1]=STIME.month;
					HLog.StartTime[2]=STIME.day;
					HLog.StartTime[3]=STIME.hour;
					HLog.StartTime[4]=STIME.minute;
				}
				
				HLog.StartMode=0;							//��¼��־	������ʽ
				HLog.IrrMode=FerState;						//��¼��־	��ȷ�ʽ
				if(ChoPartition1)							//��¼��־	��ȷ���
				{
					if(ChoPartition2)Log.Partition=3;
					else HLog.Partition=1;
				}
				else HLog.Partition=2;
				htime=0;									//�����ֶ�������
				while((State==IO8STATE)&(state==FerState))delay_ms(200);	//����⵽IO״̬�ı�� ����������־��¼
				HLog.Irrtime=htime+1;						//��¼��־	���ʱ��
				HLog.RemTime=0;								//��¼��־	������ʾʱ�����	
				if(Emw_B)									//��ȡ��ǰʱ��  ���������
				{
					SysTime=1;
					while(SysTime)delay_ms(200);
					HLog.StopTime[0]=STIME.year%100;
					HLog.StopTime[1]=STIME.month;
					HLog.StopTime[2]=STIME.day;
					HLog.StopTime[3]=STIME.hour;
					HLog.StopTime[4]=STIME.minute;
				}
				else	
				{
					GetTime=1;while(GetTime)delay_ms(200);
					HLog.StopTime[0]=STIME.year%100;
					HLog.StopTime[1]=STIME.month;
					HLog.StopTime[2]=STIME.day;
					HLog.StopTime[3]=STIME.hour;
					HLog.StopTime[4]=STIME.minute;
				}
				HLog.StopMode=0;							//��¼��־	ֹͣģʽ
				while(Logwait)delay_ms(200);				//�ȴ��ⲿflash�������
				Logwait=1;OSTaskSuspend(HDMI_TASK_PRIO);
				FlashWriteLog(&HLog.StartTime[0],Logmem.mem32,17);	//��־д��flash
				if(Logmem.mem32==20*100)Logmem.mem32=0;				//��־�洢��ַ����
				Logmem.mem32+=20;FlashWriteMem(Logmem.mem);			//��־��ַ����
				Logwait=0;OSTaskResume(HDMI_TASK_PRIO);
				up_state=1;									//�ϴ��豸״̬
				TimePopup=1;
			}
		}
	}
}

void 	Plan_task(void *pdata)			//�ƻ�����
{
	u8	addr=0;
	while(1)
	{
		delay_ms(200);
		if(!PlanButton)continue;		//�ƻ������Ƿ��
		if(!TimePopup)					//��ǰ״̬����
		{
			if(!Planing)				//û��������ִ��
			{
				GetTime=1;while(GetTime)delay_ms(200);		//��ȡ������ʱ��
				PlanAddr=0x23B+addr;
				ReadPlan=1;while(ReadPlan)delay_ms(200);	//��ȡ�ƻ�����
				if((STIME.month==Plan.month)&(STIME.day==Plan.day)&(STIME.hour==Plan.hour)&(STIME.minute==Plan.minute)&(Plan.State==0))	//�Ƿ���Ҫִ������
				{
					if(Plan.Irrtime<21)goto next;			//�ƻ�����ʱ��С��21���Ӳ�ִ��
					Planing=1;								//װ��������в���
					IrrTime=Plan.Irrtime;					//��������ʱ��
					HC_IrrMode=MCGS_Button&0XFE;			//���ô�������ؼĴ���״̬
					HC_Partition=Plan.Partition;
					HC_IrrMode|=Plan.IrrMode|0x28;
					SetRun=1;
					while(SetRun)delay_ms(200);				//�ȴ�������ִ�н���
					Net=0;
					ChoMode=3;								//ģʽΪ�ƻ�����ģʽ
					while(Planing)delay_ms(200);			//����ִ�н���
					OSTaskSuspend(HDMI_TASK_PRIO);		delay_ms(100);
					HDMI_Set_Plan(0x242+addr);			delay_ms(100);
					OSTaskResume(HDMI_TASK_PRIO);
				}
next:			addr+=9;				//������һ������
				if(addr>=99)addr=0;
			}
		}
	}
}

void	Judge_Partition()				//��������������ȶ���
{
	u8 i;
	i=0;
	memset(waitPartition,0,sizeof(waitPartition));	//�������ȷ�������
	if(ChoPartition1)waitPartition[i++]=1;			//װ�ش���ȷ���
	if(ChoPartition2)waitPartition[i++]=2;
	if(ChoPartition3)waitPartition[i++]=3;
	if(ChoPartition4)waitPartition[i++]=4;
	if(ChoPartition5)waitPartition[i++]=5;
	if(ChoPartition6)waitPartition[i]=6;
}

void 	Work_task(void *pdata)			//�������
{
	u8 ATPartition,first,error;
	while(1)
	{
	 	delay_ms(200);
		first=0;								//����Ͱ�Ƿ���	0δ���� 1�ѿ���
		ATPartition=0;							//��ǰ��������������
		if(MODEL==ManualModel)continue;			//�ֶ�ģʽ����ѭ��
		if(RunState)							//������ť����
		{
			TimePopup=1;						//��ֹʱ��ͬ������
			if((ChoMode!=2)&(ChoMode!=3))ChoMode=1;
			if(Net==0&ChoMode!=3)				//�����Ƿ�Ϊ��������  �ж��Ƿ���Ҫ������
			{
				if(Irrsign==0)Irrsign=1;		//��ȡ���õ�ʱ��
				while(Irrsign==1)delay_ms(20);
			}
			if(Emw_B)							//��ȡ��ǰʱ��  ���������
			{
				SysTime=1;
				while(SysTime)delay_ms(200);
				Log.StartTime[0]=STIME.year%100;
				Log.StartTime[1]=STIME.month;
				Log.StartTime[2]=STIME.day;
				Log.StartTime[3]=STIME.hour;
				Log.StartTime[4]=STIME.minute;
			}
			else	
			{
				GetTime=1;while(GetTime)delay_ms(200);
				Log.StartTime[0]=STIME.year%100;
				Log.StartTime[1]=STIME.month;
				Log.StartTime[2]=STIME.day;
				Log.StartTime[3]=STIME.hour;
				Log.StartTime[4]=STIME.minute;
			}
			
			Log.StartMode=ChoMode;				//��¼��־	������ʽ
			Log.IrrMode=IrrMethod;				//��¼��־	��ȷ�ʽ
			if(ChoPartition1)					//��¼��־	��ȷ���
			{
				if(ChoPartition2)Log.Partition=3;
				else Log.Partition=1;
			}
			else Log.Partition=2;
			Log.Irrtime=IrrTime;				//��¼��־	���ʱ��
			while(Logwait)delay_ms(200);
			Logwait=1;OSTaskSuspend(HDMI_TASK_PRIO);
			FlashWriteLog(&Log.StartTime[0],Logmem.mem32,sizeof(Log));	//��־д��flash
			Logwait=0;OSTaskResume(HDMI_TASK_PRIO);
				//if(IrrMethod)IrrTime+=20;		//�жϹ��ģʽ
			Remaining=IrrTime;					//���µ���ʱ
			Remsign=1;							//��ʾ����ʱʱ��
			Judge_Partition();					//�������ȵķ���
			rema=1;								//�򿪵���ʱ��ʱ��
			if(waitPartition[ATPartition])		//�ж���ʾ��ǰ����
			{
				Current=waitPartition[ATPartition];
				Cursing=1;
			}
			if(waitPartition[ATPartition]==1)Solenoid1Open;	//��һ������ŷ�
			if(waitPartition[ATPartition]==2)Solenoid2Open;	//�򿪶�������ŷ�
			error=0;
			OpenLED;							//��LED
			while(1)							//�쳣���
			{
				IO8Set=1;delay_ms(200);
				if(waitPartition[ATPartition]==1)
				{
					if(SolenoidState1)break;
					if(++error==10)Error=4,Err=1;
				}
				if(waitPartition[ATPartition]==2)
				{
					if(SolenoidState1)break;
					if(++error==10)Error=5,Err=1;
				}
				if(error==10)break;
			}
			if(error==10)goto stop;				//�豸�쳣��������
			up_state=1;
			while(1)							//������  ��Ҫ���ĵ�ǰ�ֹ��� ʣ��ʱ��
			{
				delay_ms(200);
				if(MODEL==ManualModel)goto stop;
				if(IrrMethod)
				{
					if(Remaining<(IrrTime-10))	//�������ʲ��ϴ�״̬
					{
						if(first==0)first=up_state=1;
						FerRun;					//��ʼ����
						Stir1Open;				//����Ͱ1����
						Stir2Open;				//����Ͱ2����
						error=0;
						while(1)				//�쳣���
						{
							IO8Set=1;delay_ms(200);
							if(FerState&StirState1&StirState2)break;
							if(++error==10)
							{
								if(FerState)Error=1;
								if(StirState1)Error=2;
								if(StirState2)Error=3;
								Err=1;
								goto stop;		//�豸�쳣��������
							}	
						}
					}
					if(Remaining<10)			//ֹͣ���ʲ��ϴ�״̬
					{
						if(first==1)first=2,up_state=1;
						FerStop;				//ֹͣ����
						Stir1Close;				//����Ͱ1ֹͣ����
						Stir2Close;				//����Ͱ2ֹͣ����
						error=0;
						while(1)				//�쳣���
						{
							IO8Set=1;delay_ms(200);
							if((FerState|StirState1|StirState2)==0)break;
							if(++error==10)
							{
								if(FerState)Error=1;
								if(StirState1)Error=2;
								if(StirState2)Error=3;
								Err=1;
								goto stop;		//�豸�쳣��������
							}	
						}
					}
				}
				
				if(Remaining==0)				//��ǰ��������������ز���
				{
					if(waitPartition[ATPartition]==1)Solenoid1Close;	//�ر�һ������ŷ�
					if(waitPartition[ATPartition]==2)Solenoid2Close;	//�رն�������ŷ�
					while(IO8Set)delay_ms(100);IO8Set=1;				//ȷ�����ļ̵���״̬
					if(waitPartition[++ATPartition])					//�ж���һ�������
					{
						Remaining=IrrTime;								//���µ���ʱ
						Remsign=1;										//���������µ���ʱ��ʾ
						Current=waitPartition[ATPartition];				//���·�����ʾ
						Cursing=1;										//���������·�����ʾ
						if(waitPartition[ATPartition]==2)Solenoid2Open;	//�򿪶�������ŷ�
						while(1)										//�쳣���
						{
							IO8Set=1;delay_ms(200);
							if(waitPartition[ATPartition]==1)
							{
								if(++error==10)Error=4,Err=1;
								if(SolenoidState1)break;
							}
							if(waitPartition[ATPartition]==2)
							{
								if(++error==10)Error=5,Err=1;
								if(SolenoidState1)break;
							}
							if(error==10)break;
						}
						if(error==10)goto stop;							//�豸�쳣��������
						first=0;										//����Ͱ����Ϊδ����
					}
					else
					{
						rema=0;					//�ص���ʱ��
						break;
					}
					delay_ms(200);				//�ϴ��豸״̬
					up_state=1;
				}

				if(StartButton==0)				//ֹͣ��ť����
				{
stop:				rema=0;
					break;
				}
			}
			while(Err)delay_ms(200);			//�жϴ��󵯴��Ƿ񵯳�
			Current=0;							//��ǰ��ȷ�������
			Solenoid1Close;						//�ر�һ������ŷ�
			Solenoid2Close;						//�رն�������ŷ�
			FerStop;							//ֹͣ����
			Stir1Close;							//����Ͱ1ֹͣ����
			Stir2Close;							//����Ͱ2ֹͣ����
			IO8Set=1;							//����IO8���״̬
			while(IO8Set)delay_ms(100);			//ȷ�����ļ̵���״̬
			CloseLED;							//�ر�LED
			OSTaskSuspend(HDMI_TASK_PRIO);		//ֹͣ������ �������С���־
			HDMI_Set_Button(MCGS_Button&0xD7);
			OSTaskResume(HDMI_TASK_PRIO);
			while(StartButton)delay_ms(200);	//����������ť
			delay_ms(200);
			up_state=1;							//�ϴ��豸״̬
			if(Emw_B)							//��ȡ��ǰʱ��  ���������
			{
				SysTime=1;
				while(SysTime)delay_ms(200);
				Log.StopTime[0]=STIME.year%100;
				Log.StopTime[1]=STIME.month;
				Log.StopTime[2]=STIME.day;
				Log.StopTime[3]=STIME.hour;
				Log.StopTime[4]=STIME.minute;
			}
			else	
			{
				GetTime=1;while(GetTime)delay_ms(200);
				Log.StopTime[0]=STIME.year%100;
				Log.StopTime[1]=STIME.month;
				Log.StopTime[2]=STIME.day;
				Log.StopTime[3]=STIME.hour;
				Log.StopTime[4]=STIME.minute;
			}
			Log.StopMode=ChoMode;
			Log.RemTime=Remaining;
			while(Logwait)delay_ms(200);
			Logwait=1;OSTaskSuspend(HDMI_TASK_PRIO);
			FlashWriteLog(&Log.StopTime[0],Logmem.mem32+10,8);	//��־д��flash
			if(Logmem.mem32==20*100)Logmem.mem32=0;				//��־�洢��ַ����
			Logmem.mem32+=20;FlashWriteMem(Logmem.mem);
			Logwait=0;OSTaskResume(HDMI_TASK_PRIO);
			Planing=0;	//���������־
			ChoMode=1;	//�лر���ģʽ
			TimePopup=0;//��Ϊ����״̬
		}
	}
}
