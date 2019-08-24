#ifndef __inter_H
#define __inter_H
#include "sys.h"
#include "main.h"
void	Uptoaliyun_wifi_Env(Environmental data,u8 group)	//�ϴ���������ֵ��������
{
	char a[250];
	char c[250];
	char *b=c;
	u8 i;i=0;b=c;
	memset(a,0,sizeof a);
	sprintf(b,"\"airtemp_%d\":%2.1f,",group+1,(float)(data.airtemp/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"airhumi_%d\":%2.1f,",group+1,(float)(data.airhumi/10.0));		for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"CO2_%d\":%d,",group+1,data.CO2);								for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"light_%d\":%d,",group+1,data.light);							for(;*b;b++)a[i++]=*b;
	
	sprintf(b,"\"soiltemp_%d\":%2.1f,",group+1,(float)(data.soiltemp/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"soilhumi_%d\":%2.1f,",group+1,(float)(data.soilhumi/10.0));	for(;*b;b++)a[i++]=*b;
	sprintf(b,"\"EC_%d\":%d",group+1,data.EC);									for(;*b;b++)a[i++]=*b;
	
	sendEmw(a,0);
}

void	Uptoaliyun_wifi_State()			//�ϴ���ǰ״̬��������
{
	char a[250];
	char c[250];
	char *b=c;
	u8 data;
	u8 i;i=0;b=c;
	memset(a,0,sizeof a);
	sprintf(b,"\"FerState\":%d,",(int)FerState);			for(;*b;b++)a[i++]=*b;	//����״̬
	if(StirState1)data=1;else data=0;
	sprintf(b,"\"StirState1\":%d,",data);					for(;*b;b++)a[i++]=*b;	//����1
	if(StirState2)data=1;else data=0;	
	sprintf(b,"\"StirState2\":%d,",data);					for(;*b;b++)a[i++]=*b;	//����2
	if(SolenoidState1)data=1;else data=0;
	sprintf(b,"\"Partition1\":%d,",data);					for(;*b;b++)a[i++]=*b;	//��ŷ�1
	if(SolenoidState2)data=1;else data=0;
	sprintf(b,"\"Partition2\":%d,",data);					for(;*b;b++)a[i++]=*b;	//��ŷ�2
	sendEmw(a,0);
	
	if(MODEL==ManualModel)return;

	i=0;b=c;
	memset(a,0,sizeof a);
	if(IrrMethod)data=1;else data=0;
	sprintf(b,"\"IrrMethod\":%d,",data);					for(;*b;b++)a[i++]=*b;	//��ȷ�ʽ
	sprintf(b,"\"Current\":%d,",Current);					for(;*b;b++)a[i++]=*b;	//��ǰ��ȷ���
	sprintf(b,"\"IrrTime\":%d,",IrrTime);					for(;*b;b++)a[i++]=*b;	//���ʱ��
	if(RunState)data=1;else data=0;
	sprintf(b,"\"State\":%d,",data);						for(;*b;b++)a[i++]=*b;	//����״̬
	sendEmw(a,0);
}

void	readset()						//����ƽ̨�·���ָ��
{
	u8 len;u8 Method=0,Partition=0;
	u8 buf[250];
	char c[100];
	char *msg=c;
	if(emw_set)return;
	Method=MCGS_Button;
	Partition=MCGS_Partition;
	len=COM1GetBuf(buf,200);
	if(len>40)
	{
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"RECV")[0]!='R'){memset(buf,0,sizeof buf);memset(c,0,sizeof c);comClearRxFifo(COM1);return;}
		msg=strstr((const char *)buf,"IrrMethod:");	if(msg[0]=='I'){if(msg[10]-0x30)Method|=0x02;else Method&=0xFD;}
		msg=strstr((const char *)buf,"Partition1:");if(msg[0]=='P'){if(msg[11]-0x30)Partition|=0x01;else Partition&=0xFE;}
		msg=strstr((const char *)buf,"Partition2:");if(msg[0]=='P'){if(msg[11]-0x30)Partition|=0x02;else Partition&=0xFD;}
		msg=strstr((const char *)buf,"IrrTime:");	if(msg[0]=='I'){IrrTime=(msg[8]-0x30)*10000+(msg[9]-0x30)*1000+(msg[10]-0x30)*100+(msg[11]-0x30)*10+(msg[12]-0x30);}
		msg=strstr((const char *)buf,"State:");		if(msg[0]=='S'){if(msg[6]-0x30)Method|=(0x20|0x08);else Method&=(0xDF&0xF7);}
		
		if(PlanButton)goto cc;						//�ƻ������в�����ָ��
		if(RunState){if((Method&0x08))goto cc;}		//��ǰ�������в������¼ƻ�ָ��
		if(IrrTime<21)goto cc;						//����ʱ��С��21���Ӳ�ִ��
		if(Partition<1)goto cc;						//���ٿ���һ������
		HC_Partition=Partition;
		HC_IrrMode=Method;
		SetRun=1;									//����������в���
		while(SetRun)delay_ms(200);
		Net=1;
		ChoMode=2;
cc:		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
		memset(c,0,sizeof c);
	}
}
#endif
