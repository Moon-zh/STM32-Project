#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "ipsec/md5.h"
#include "hmi_driver.h"

#ifndef TIM
#define TIM
typedef struct
{
	u32	year;
	u8	month;
	u8	day;
	u8	week;
	u8	hour;
	u8	minute;
	u8	second;
}RTC_TIME;
#endif

unsigned char Emwled=0;
u8 sumbz=0;
//��Ȩ��Ϣ
char ProductKey1[20]=	"a1TymkIzezE";
char DeviceName1[50]=	"mjwlcjx-141";
char DeviceSecret1[50]=	"I9ymKFepDHR1SVFaVTemHV54hs5HyP6E";

//char ssid[20]=		"IoT";
//char password[20]=	"ag20190520";
char ssid[20]=		"mingji";
char password[20]=	"mingji2015";
char ip[20]=		"192.168.7.101";
char network[20]=	"255.255.255.0";
char gateway[20]=	"192.168.7.1";
char dns[20]=		"114.114.114.114";

u8 DHCP=1;

extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];
extern RTC_TIME STIME;

void	Emw3060_LED_Init()				//EMWLED��ʼ��
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PB,PE�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;	 //LED0-->PB.5 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
}

void	md5tohex(char *data,char *str)	//�ֽ��ֽ��а����������ַ���ֵ
{
	u8 i;
	for(i=0;i<16;i++)sprintf(str+i*2,"%02x",data[i]);
}

void	hmacmd5(char *data,u8 size_data,u8 size_ds,char *str)	//hmacmd5����
{
	char buf[33];
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret1,size_ds,(unsigned char*)str);
	md5tohex(str,buf);
	sprintf(str,"%s",buf);
}

void	Emw3060_init(void)				//EMW��ʼ��
{
	u8 buf[250];
	char *msg;
	Emw3060_LED_Init();
	Emwled=1;
	while(1)	//ATģʽ
	{
		printf_num=1;
		printf("+++");			
		delay_ms(200);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//�ָ���������
	{
		printf_num=1;
		printf("AT+FACTORY\r");
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	printf_num=1;
	printf("\r");
	
	if(!DHCP)	//DHCP����
	{
		while(1)
		{
			printf_num=1;
			printf("AT+WDHCP=OFF\r");	//�ر�DHCP
			delay_ms(500);
			COM1GetBuf(buf,30);
			if(strchr((const char *)buf,'O')[0]=='O')break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
		
		while(1)	//�����������
		{
			printf_num=1;
			printf("AT+WJAPIP=%s,%s,%s,%s\r",ip,network,gateway,dns);
			delay_ms(500);
			COM1GetBuf(buf,100);
			if(strchr((const char *)buf,'O')[0]=='O')break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
	}
		
	printf_num=1;	//����WIFI
	printf("AT+WJAP=%s,%s\r",ssid,password);
	delay_ms(2000);
	printf_num=1;
	printf("AT+WJAP=%s,%s\r",ssid,password);
	while(1)
	{
		delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"STATION_UP")[0]=='S')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//���ñ���ʱ��ʱ��
	{
		printf_num=1;
		printf("AT+SNTPCFG=+8\r");
		delay_ms(500);
		COM1GetBuf(buf,30);
		if(strchr((const char *)buf,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//У׼����ʱ��
	{
		printf_num=1;
		printf("AT+SNTPTIME\r");
		delay_ms(1000);
		COM1GetBuf(buf,100);
		if(strchr((const char *)buf,'2')[0]=='2')break;
	}
	msg=strchr((const char *)buf,'2');
	STIME.year=(msg[0]-0x30)*1000+(msg[1]-0x30)*100+(msg[2]-0x30)*10+(msg[3]-0x30);
	STIME.month=(msg[5]-0x30)*10+(msg[6]-0x30);
	STIME.day=(msg[8]-0x30)*10+(msg[9]-0x30);
	STIME.hour=(msg[11]-0x30)*10+(msg[12]-0x30);
	STIME.minute=(msg[14]-0x30)*10+(msg[15]-0x30);
	STIME.second=(msg[17]-0x30)*10+(msg[18]-0x30);
	STIME.week=1;
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
}

unsigned char Emw3060_con(void)				//EMW���Ӱ�����
{
	u8 buf[250];
	char hc[100];
	char cs[40];
	char mac[33];
	char *m;
	u8 to=0;
	Emwled=2;to=0;	
	memset(cs,0,sizeof cs);
	
	while(1)	//��ȡMAC��ַ
	{
		printf_num=1;
		printf("AT+WMAC?\r");
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"+WMAC:")[0]=='+')break;
	}
	m=strstr((const char *)buf,"+WMAC:")+6;
	m[12]=0;
	memset(mac,0,sizeof mac);
	sprintf(mac,"%s",m);
	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	
	sprintf(hc,"clientId%sdeviceName%sproductKey%stimestamp789",mac,DeviceName1,ProductKey1);
	while(1)	//д���豸��Ȩ��Ϣ
	{
		printf_num=1;
		hmacmd5(hc,strlen(hc),strlen(DeviceSecret1),cs);
		printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName1,ProductKey1,cs);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	
	while(1)	//�����豸CID
	{
		printf_num=1;
		printf("AT+MQTTCID=%s|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r",mac);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	
	while(1)	//����Ŀ��IP��ַ
	{
		printf_num=1;
		printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKey1);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//����ʱ������
	{
		printf_num=1;
		printf("AT+MQTTKEEPALIVE=300\r");
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//MQTT�Զ�������
	{
		printf_num=1;
		printf("AT+MQTTRECONN=ON\r");
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//MQTT����
	{
		printf_num=1;
		printf("AT+MQTTSTART\r");
		delay_ms(4000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"SUCCESS")[0]=='S')break;
		if(++to==20){Emwled=3;return 0;}
		comClearRxFifo(COM1);
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)	//����TOPIC
	{
		printf_num=1;
		printf("AT+MQTTSUB=0,/%s/%s/user/set,1\r",ProductKey1,DeviceName1);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);
	Emwled=3;
	sumbz=0;
	return 1;
}

u8	sendEmw(char *data,unsigned char w)	//EMW�ϱ�����
{
	static u8 buf[201];
	char hc[500];
	u16 len;u8 time;
	Emwled=2;
	memset(buf,0,sizeof buf);
	comClearRxFifo(COM1);
	time=0;
	while(1)	//���÷���topic
	{
		printf_num=1;
		if(!w)
			printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey1,DeviceName1);	//�л��ϱ�topic
		else printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey1,DeviceName1);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
		if(++time==10){Emwled=3;return 0;}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);

	len=0;time=0;
	while(1)	//��������ָ��
	{
		printf_num=1;
		printf("AT+MQTTSEND=%d\r",strlen(data)+13);
		while(1)
		{
			delay_ms(100);
			COM1GetBuf(buf,20);
			if(strstr((const char *)buf,">")[0]=='>')break;
			if(strstr((const char *)buf,"ERROR")[0]=='E')break;
			if(++time==10){Emwled=3;return 0;}
		}
		if(strstr((const char *)buf,">")[0]=='>')break;
		comClearRxFifo(COM1);
		if(++len==5){Emwled=3;return 0;}
	}
	memset(buf,0,sizeof buf);
	comClearRxFifo(COM1);
	
	sprintf(hc,"{\"params\":{%s}}",data);
	len=strlen(hc);
	if(hc[len-3]==',')hc[len-3]=' ';
	printf_num=1;
	printf("%s",hc);
	delay_ms(3000);
	COM1GetBuf(buf,100);
	Emwled=3;
	comClearRxFifo(COM1);
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')sumbz=0;	//�ж��Ƿ���������
	else sumbz++;
	if(sumbz>=3)return 0;
	else return 1;
}
