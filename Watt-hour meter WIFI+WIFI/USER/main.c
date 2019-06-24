#include "main.h"

typedef struct			//����������ṹ��
{
	float	power;
	u32	water;
}Meter;
Meter	PAW;

//����181214018391
//char number[]   ="190315009687";
char number[]   ="190313008477";
u8 tab_read[]	={0xFE ,0xFE ,0xFE, 0xFE ,0x68 ,0x91 ,0x83 ,0x01 ,0x14 ,0x12 ,0x18 ,0x68 ,0x91 ,0x08 ,0x33 ,0x33 ,0x34 ,0x33 ,0x35 ,0x33 ,0x33 ,0x33 ,0x91 ,0x16 };
u8 tab_send[]   ={0xFE ,0xFE ,0xFE ,0xFE ,0x68 ,0x91 ,0x83 ,0x01 ,0x14 ,0x12 ,0x18 ,0x68 ,0x11 ,0x04 ,0x33 ,0x34 ,0x34 ,0x33 ,0x06 ,0x16};
u8 tab_water[]  ={0x01 ,0x03 ,0x00 ,0x00 ,0x00 ,0x04 ,0x44 ,0x09};
u8 tab_water_b[]={0x01 ,0x03 ,0x08 ,0x00 ,0x00 ,0x00 ,0x14 ,0x00 ,0x00 ,0x00 ,0x00 ,0xA5 ,0xD4};

void	strtobcd(u8 *a)
{
	u8 i;
	for(i=0;i<6;i++)
	{
		a[5+i]=((number[10-i*2]-0x30)<<4)|number[11-i*2]-0x30;
	}
}

u8		bcdtoint(u8 a)
{
	return (a>>4)*10+(a&0x0f);
}

void	send(u8 *a)	//485���ͺ���
{
	u16 crc;
	comClearRxFifo(COM3);
	crc=mc_check_sum(a+4,14);
	a[18]=crc;
	comSendBuf(COM3,a,20);
}

void	send1(u8 *a)//485���ͺ���
{
	comClearRxFifo(COM5);
	comSendBuf(COM5,a,8);
	delay_ms(100);
	delay_ms(10);
}

u8		hextoint(u8 data)
{
	data-=0x33;
	return (data>>4)*10+(data&0xf);
}

void	savekWh(u8 *a)  //18 19 20 21  С����ʮλ��ǧλ��ʮ��
{
	PAW.power=hextoint(a[18])/100.00+hextoint(a[19])+hextoint(a[20])*100+hextoint(a[21])*10000;
}

void	save3m3h(u8 *a)
{
	PAW.water=bcdtoint(a[4])*10000+bcdtoint(a[5])*100+bcdtoint(a[6]);//*10+bcdtoint(a[10])+bcdtoint(a[3])/10.0+bcdtoint(a[4])/100.0+bcdtoint(a[5])/1000.0+bcdtoint(a[6])/10000.0;
}

u8		read(u8 *a)		//485��ȡ����
{
	u8 len;
	len=COM3GetBuf(a,30);
	if(len>=11)
	{
		delay_ms(100);
		if(len>50)comClearRxFifo(COM3);
		return 1;
	}
	len=COM5GetBuf(a,30);
	if(len>=11)
	{
		delay_ms(100);
		if(len>50)comClearRxFifo(COM5);
		return 1;
	}
	return 0;
}

void	pdcmd(u8 *a)	//�ж�ָ��
{
	u8 k,sum;
	u16 crc;
	sum=0;
	for(k=0;k<14;k++)	//�ж��Ƿ�����ͬ������
	{
		if(a[k]==tab_read[k])sum++;		
	}
	if(sum>=13)			//�ж��Ƿ���������ָ��
	{
		savekWh(a);
		comClearRxFifo(COM3);
	}
	for(k=0;k<14;k++)	//�ж��Ƿ�����ͬ������
	{
		if(a[k]==tab_water_b[k])sum++;		
	}
	if(sum>=3)			//�ж��Ƿ���������ָ��
	{
		crc=mc_check_crc16(a,11);
		if((a[11]==crc>>8)&(a[12]==(crc&0xff)))
		save3m3h(a);
		comClearRxFifo(COM5);
	}
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

void	init()							//ϵͳ��ʼ��
{
	SystemInit();
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�	
	bsp_InitUart();
	readflashthree();				//��ȡ�ڴ���д�����Ԫ������
//	while(!init_4G());
}

int 	main(void)						//ϵͳ��ʼ
{	 
	init();
	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}

void 	start_task(void *pdata)			//����ʼ
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    					   
 	OSTaskCreate(LED_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);
	OSTaskCreate(ReadMeter_task,(void *)0,(OS_STK*)&ReadMeter_TASK_STK[ReadMeter_STK_SIZE-1],ReadMeter_TASK_PRIO);
	OSTaskCreate(SaveThree_task,(void *)0,(OS_STK*)&SaveThree_TASK_STK[SaveThree_STK_SIZE-1],SaveThree_TASK_PRIO);
	OSTaskCreate(Upyun_task,(void *)0,(OS_STK*)&Upyun_TASK_STK[Upyun_STK_SIZE-1],Upyun_TASK_PRIO);
	OSTaskCreate(UpyunWF_task,(void *)0,(OS_STK*)&UpyunWF_TASK_STK[UpyunWF_STK_SIZE-1],UpyunWF_TASK_PRIO);
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

void 	LED_task(void *pdata)			//LED����״ָ̬ʾ������
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
		switch(Emwled2)
		{
			case 0:
				Emw2LED1=0;
				Emw2LED2=0;
				break;
			case 1:
				Emw2LED1=!Emw2LED1;
				break;
			case 2:
				Emw2LED1=1;
				Emw2LED2=!Emw2LED2;
				break;
			case 3:
				Emw2LED2=0;
				break;
		}
	}
}

void	Uptoaliyun_wifi()				//�ϴ����������ݵ������� WIFI
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);

	sprintf(b,"\"power\":%05.2f,",PAW.power);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"water\":%d,",PAW.water);			for(;*b;b++)a[i++]=*b;

	sendEmw(a,0);

}

void	Uptoaliyun_wifi2()			//�ϴ����������ݵ�������
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;
	memset(a,0,sizeof a);
	
	sprintf(b,"\"power\":%05.1f,",PAW.power);		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"water\":%d,",PAW.water);			for(;*b;b++)a[i++]=*b;
	
	sendEmw2(a,0);
}

void	readthree()										//��COM3 485���ڶ�ȡ�Ƿ���������Ԫ������
{
	u8 len,i;
	char hc[250];
	char *msg=hc;
	u8 buf[250];
	memset(buf,0,sizeof buf);
	memset(hc,0,sizeof hc);
	len=0;i=0;
	len=COM4GetBuf(buf,200);
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
			
		sendflashthree(len);
		comSendBuf(COM4,buf,strlen((char *)buf));
		comClearRxFifo(COM4);
		memset(buf,0,sizeof buf);
		memset(hc,0,sizeof buf);
	}
}

void	readset(u8 t)									//��ȡ�����·�������
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
		msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=	(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
		
	}
	else
	{
qw:		if(emw_set)return;
		len=COM2GetBuf(buf,100);
		if(len>10)
		{
			delay_ms(200);COM2GetBuf(buf,100);		//��ȡ���豸�·�������
			if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,200);memset(c,0,200);comClearRxFifo(COM2);return;}
			msg=strstr((const char *)buf,"up_time:");		if(msg[0]=='u')up_time=	(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);
		}
	}
	memset(buf,0,200);
	memset(c,0,200);
}

extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
void 	Upyun_task(void *pdata)							//�ϴ������� N21
{
	u16 i;
	Emw3060_init2();
	Emw3060_con2();
	N21_B=1;n21_set=0;
	while(1)
	{
		delay_ms(200);
		readset(0);						//��ȡ�������·�����
		if(((++i)/270)>=up_time){i=0;Uptoaliyun_wifi2();}
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
		readset(1);						//��ȡ�������·�����
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

void 	ReadMeter_task(void *pdata)						//��Ԫ��洢����
{
	u8 *rs485buf;
	u8 buf[200];
	u8 bz=0;
	rs485buf=buf;
	strtobcd(tab_read);
	strtobcd(tab_send); 
	while(1)
	{
		delay_ms(600);
		if(read(rs485buf))	//�ȴ�RS485��������
		{
			pdcmd(rs485buf);
		}
		memset(buf,0,200);
		bz=!bz;
		if(bz)send(tab_send);	//�����
		else send1(tab_water);	//��ˮ��
	} 
}
