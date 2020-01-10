/*485��ַ 1ר��Ͱ 2����Ͱ 3��ˮ�� 9ѹ���� 10������*/
#include "main.h"
#include "MCGS.h"

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
//	readflashthree();			//��ȡ��Ԫ��
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
//	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);	//��Ԫ��洢����
	OSTaskCreate(Upyun_task,(void *)0,(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],Upyun_TASK_PRIO);					//WIFI����
	OSTaskCreate(HDMI_task,(void *)0,(OS_STK*)&HDMI_TASK_STK[HDMI_STK_SIZE-1],HDMI_TASK_PRIO);						//����������
	OSTaskCreate(Work_task,(void *)0,(OS_STK*)&Work_TASK_STK[Work_STK_SIZE-1],Work_TASK_PRIO);						//�������
	OSTaskCreate(Water_task,(void *)0,(OS_STK*)&Water_TASK_STK[Water_STK_SIZE-1],Water_TASK_PRIO);					//ˮ������
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
	}
}

void 	Water_task(void *pdata)			//ˮ������
{	 
	u32	w1;
	u8 ok;
	u8 i=0;
	while(1)
	{
		while(!RunState)ReadWater(),delay_ms(200);
		watertime=0;
		w1=thiswater1=water1;
		thiswater2=water2;
		while(RunState)
		{
			delay_ms(200);
			ok=ReadWater();
			Actual_Water=water0-thiswater0;	//ʵ����ˮ��
			Actual_Fer=water1-thiswater1;		//ʵ���÷���
			Actual_FerP=water2-thiswater2;
			if(++i==10)
			{
				i=0;
				if((!ok)|(water1==w1))continue;
				if((water1flow-Mathflow)>20)
				{
					NeedFlow-=10;
					ChangeFlow();
				}
				else if((Mathflow-water1flow)>10)
				{
					NeedFlow+=10;
					ChangeFlow();
				}
				w1=water1;
			}
		}
	}
}

void 	HDMI_task(void *pdata)			//�������������
{	  
	u16 LogAddr;
	u8 i,so1;
	u8 HouseNum=0;
	GetTime=1;
	while(1)
	{
		delay_ms(200);
		HDMI_Check_Button();		//��ȡ��Ļ����״̬0
		if(HDMI_READ_START)			//�������¶�ȡ�û����в���
		{
			if(!NET)
			HDMI_Read_Greenhouse();
			HDMI_Read_Fer();
			HDMI_READ_START=0;
		}
		if(HDMI_SET_STOP)			//ϵͳֹͣ���������
		{
			HDMI_Set_Button(MCGS_Button&(~RunState));
			HDMI_Check_Button();
			HDMI_SET_STOP=0;
		}
		if(RunState)				//ϵͳ������ ��ʾ�׶� ��ˮ�� �÷��� ����ʱ  ѹ��
		{
			HDMI_Set_Stage();
			HDMI_Set_Water_Fer();
		}
		if(FerSButton)HDMI_Set_Button(MCGS_Button&(~FerSButton)),HDMI_Read_Fer();	//����Ͱ���ñ���
		if(HouseRButton)			//�������ö�ȡ
		{
			HDMI_Set_Button(MCGS_Button&(~HouseRButton));
			HDMI_Read_SetHouseNum(&HouseNum);
			FlashReadFer(&Run_SetMessage.Mu,HouseNum*10);
			HDMI_Set_HouseSet();	//��ʾ������������
		}
		if(HouseSButton)HDMI_Set_Button(MCGS_Button&(~HouseSButton)),HDMI_Read_HouseSet();	//�������ñ���
		while(CechkSelf)
		{
			HDMI_Check_Button();		//��ȡ��Ļ����״̬
			HDMI_Check_Check();			//��ȡ�Լ찴��
			HDMI_Check_Checkvalue();	//��ȡ�Լ�����ֵ
			if(CheckFerM){FerRun;}else{FerStop;}
			if(CheckSolenoid){so1=1;FlashReadFer(&Run_SetMessage.Mu,checknum*10);OpenPartition();}else if(so1){so1=0;FlashReadFer(&Run_SetMessage.Mu,checknum*10);ClosePartition();}
			if(CheckFlow){NeedFlow=checkflow;ChangeFlow();}
			Run_SetMessage.SpecialFer=checkfer;OpenFer();
			if(!CechkSelf)CloseFer(),FerStop,ClosePartition();
		}
		if(GetTime)HDMI_Check_SysTime(),GetTime=0;						//��ȡ������ʱ��
		if(LogButton)					//��־��ѯ��ť����
		{
			HDMI_Check_LogPage();		//��ѯ��ǰҳ��
			LogAddr=LogPage*200;		//ָ���Ӧ�洢��ַ
			for(i=0;i<10;i++)			//��ʾ10����־
			{
				HDMI_Check_Button();
				if(Details)goto De;
				FlashReadLog(&Log.HouseNum,LogAddr+i*20);				//��ȡ��־
				HDMI_Set_Log(99+(i*13));								//��ʾ��־
			}
		}
De:		if(Details)						//��ʾ��־����
		{
			HDMI_Check_LogDe();
			FlashReadLog(&Log.HouseNum,LogAddr+(LogDe-1)*20);
			HDMI_Set_LogDe();			
			while(Details){HDMI_Check_Button();delay_ms(200);}
		}
		if(SuspendButton)				//��ͣ
		{
			ClosePartition();
			FerStop;
			while(SuspendButton)
			{
				ttm=0;
				HDMI_Check_Button();
			}
			OpenPartition();
			if(MODEL)FerRun;
		}
//		if(TimeButton)SysTime=1,HDMI_Set_Partition(MCGS_Partition&0x7F);//����ʱ��ͬ��
		if(SysTime==2)HDMI_SetSysTime(),SysTime=0;			//����ʱ��ͬ��
//		if(Err)HDMI_Set_Error(),Err=0;									//��������
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

void 	Work_task(void *pdata)			//�������
{
	while(1)
	{
	 	delay_ms(200);	
		if(RunState)
		{
			HDMI_READ_START=1;									//��������ȡ������������
			while(HDMI_READ_START)delay_ms(200);				//��ȡ���
			if(n21_set){GetTime=1;while(GetTime)delay_ms(200);}	//��ȡ������ʱ��
			else{SysTime=2;while(SysTime)delay_ms(200);}
			Log.StartTime[0]=STIME.year%100;
			Log.StartTime[1]=STIME.month;
			Log.StartTime[2]=STIME.day;
			Log.StartTime[3]=STIME.hour;
			Log.StartTime[4]=STIME.minute;
			Log.StartMode=NET;
			
			FlashReadFer(&Run_SetMessage.Mu,HouseNum*10);
			OpenPartition();									//�򿪷�����ŷ�
			ttm=0;htime=0;										//����ʱ�������
			PreTime=Run_SetMessage.Pre;
			BehindTime=Run_SetMessage.Behind;
			nbup=1;
			if(MODEL==0){stage=3;while((Remaining>0)&RunState)delay_ms(200);}	//��ˮ��ȵ���ʱ
			else						//��ˮ���
			{
				Mathflow=NeedFlow=CalculateFlow();//װ�ؼ���õ�����
				FerRun;					//�����ʱ�
				stage=0;while((htime!=PreTime)&RunState)delay_ms(200);			//ǰ�ý���ʱ��
				if(!RunState)goto end;
				OpenFer();				//��ר��Ͱ
				OpenFer_Public();		//�򿪹���Ͱ
				CalculateFlow();		//��������
				ChangeFlow();			//��������
				stage=1;while((Remaining!=BehindTime)&RunState)delay_ms(200);	//��ˮ��ȵ���ʱ
				if(!RunState)goto end;
				CloseFer();				//�ر�ר��Ͱ
				CloseFer_Pubilc();		//�رչ���Ͱ
				stage=2;while((Remaining>0)&RunState)delay_ms(200);				//���ó�ϴʱ��
				if(!RunState)goto end;
				FerStop;				//�ر����ʱ�
			}
end:		CloseFer();											//�رշ���Ͱ
			ClosePartition();									//�رշ�����ŷ�
			FerStop;											//�ر����ʱ�
			{													//��¼��־
				Log.HouseNum=HouseNum;							//��¼��־  ���ұ��
				if(n21_set){GetTime=1;while(GetTime)delay_ms(200);}	//��ȡ������ʱ��
				else{SysTime=2;while(SysTime)delay_ms(200);}
				Log.StopTime[0]=STIME.year%100;
				Log.StopTime[1]=STIME.month;
				Log.StopTime[2]=STIME.day;
				Log.StopTime[3]=STIME.hour;
				Log.StopTime[4]=STIME.minute;
				Log.StopMode=NET;									//��¼��־	������ʽ
				Log.IrrMode=MODEL;									//��¼��־	��ȷ�ʽ
				Log.Irrtime=htime+1;								//��¼��־	���ʱ��
				Log.UseWater=Actual_Water;							//������ˮ��
				Log.FerNum=Run_SetMessage.SpecialFer;				//�����÷���Ͱ��
				Log.SFerusage=Actual_Fer;							//����ר�÷���
				Log.PFerusage=Actual_Fer;							//���ι��÷���
				FlashWriteLog(&Log.HouseNum,Logmem.mem32,20);		//��־д��flash
				if(Logmem.mem32==20*100)Logmem.mem32=0;				//��־�洢��ַ����
				Logmem.mem32+=20;FlashWriteMem(Logmem.mem);			//��־��ַ����			
			}
			HDMI_SET_STOP=1;									//����������������
			while(HDMI_SET_STOP)delay_ms(200);
			stage=0;
			nbup=1;
			NET=0;
		}
	}
}

void	Uptoaliyun()					//�ϴ��豸״̬��������
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	sprintf(b,"\\\"State\\\":%d,",RunState);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"House\\\":%d,",HouseNum);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Water\\\":%d,",Actual_Water);	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Fer\\\":%d,",Actual_Fer);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Pressure\\\":%.1f,",(float)pr.f);for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"IrrMode\\\":%d,",MODEL);			for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Time\\\":%d,",Remaining);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Sbarrel\\\":%d,",Run_SetMessage.SpecialFer);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\\\"Pbarrel\\\":%d,",PublicFer);		for(;*b;b++)a[i++]=*b;
	Nsendok=sendN21(a,0);
}

void	readset()						//��ȡ�����·�������
{
	u8 len;cJSON *json;
	u8 buf[250];
	if(n21_set)return;
	len=COM1GetBuf(buf,200);
	if(len>64)
	{
		if(strstr((const char *)buf,"RCVPUB")[0]!='R'){memset(buf,0,200);comClearRxFifo(COM1);return;}
		json=cJSON_Parse(strstr((const char *)buf,"{"));
		NET=1;
		HouseNum=atoi(cJSON_Print(cJSON_GetObjectItem(json,"House")));
		MODEL=atoi(cJSON_Print(cJSON_GetObjectItem(json,"IrrMode")));
		PublicFer=atoi(cJSON_Print(cJSON_GetObjectItem(json,"Pbarrel")));
		Remaining=atoi(cJSON_Print(cJSON_GetObjectItem(json,"Time")));
		OSTaskSuspend(HDMI_TASK_PRIO);
		HDMI_Set_IrrMode(MODEL);
		len=atoi(cJSON_Print(cJSON_GetObjectItem(json,"State")));
		if(len)HDMI_Set_Button(MCGS_Button|1);
		else HDMI_Set_Button(MCGS_Button&0xfe);
		OSTaskResume(HDMI_TASK_PRIO);
		memset(buf,0,200);
	}
}

void	Upccid()						//�ϱ�CCID
{
	char c[50];
	char *b=c;
	sprintf(b,"\\\"CCID\\\":\\\"%s\\\"",CCID);
	Nsendok=sendN21(b,0);
}

void 	Upyun_task(void *pdata)			//�ϴ������� N21
{
	u8 i;
	u8 buf[20];
	NeoWayN21_init();					//N21��ʼ�� ע��
	conN21();							//N21���ӵ�������
	for(i=0;i<3;i++)					//�ȴ�N21��ȫ��ʼ����
	delay_ms(1000);
	Upccid();							//�ϱ�CCID
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	while(1)
	{
		delay_ms(200);
		readset();						//��ȡ�������·�����
		if(nbup)
		{
			nbup=0;
			n21_set=1;
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
				NeoWayN21_init();					//N21��ʼ�� ע��
				conN21();							//N21���ӵ�������
				memset(buf,0,10);
				n21_set=0;
				memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			}
			Uptoaliyun();
			n21_set=0;
		}
		if(!Nsendok)
		{
			n21_set=1;
			NeoWayN21_init();
			conN21();
			comClearRxFifo(COM1);
			memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
			n21_set=0;
		}	
	}
}
