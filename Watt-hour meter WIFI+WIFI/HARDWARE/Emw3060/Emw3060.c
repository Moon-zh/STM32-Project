#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "ipsec/md5.h"

unsigned char Emwled=0;
u8 sumbz=0;

//鉴权信息
char ProductKey1[20]=	"a1DnsArKWVB";
char DeviceName1[50]=	"mjwlpdx-01";
char DeviceSecret1[50]=	"ZRGfD00hiyjzGZ8rPxdxxyhWFsQt0DQH";

char *ProductKeyw=ProductKey1;
char *DeviceNamew=DeviceName1;
char *DeviceSecretw=DeviceSecret1;

char ssid[20]=		"IoT";
char password[20]=	"ag20190520";
char ip[20]=		"192.168.7.101";
char network[20]=	"255.255.255.0";
char gateway[20]=	"192.168.7.1";
char dns[20]=		"114.114.114.114";

u8 DHCP=1;

extern uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];

void	Emw3060_LED_Init()				//EMWLED初始化
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;	 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
}

void	md5tohex(char *data,char *str)	//分解字节中包含的两个字符串值
{
	u8 i;
	for(i=0;i<16;i++)sprintf(str+i*2,"%02x",data[i]);
}

void	hmacmd5(char *data,u8 size_data,u8 size_ds,char *str)	//hmacmd5计算
{
	char buf[33];
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecretw,size_ds,(unsigned char*)str);
	md5tohex(str,buf);
	sprintf(str,"%s",buf);
}

void	Emw3060_init(void)				//EMW初始化
{
	u8 buf[250];
	Emw3060_LED_Init();
	Emwled=1;
	while(1)	//AT模式
	{
		printf_num=2;
		printf("+++");
		delay_ms(200);
		COM2GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//恢复出厂设置
	{
		printf_num=2;
		printf("AT+FACTORY\r");
		delay_ms(200);
		COM2GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	printf_num=2;
	printf("\r");
	
	if(!DHCP)	//DHCP开关
	{
		while(1)
		{
			printf_num=2;
			printf("AT+WDHCP=OFF\r");	//关闭DHCP
			delay_ms(500);
			COM2GetBuf(buf,30);
			if(strchr((const char *)buf,'O')[0]=='O')break;
		}
		comClearRxFifo(COM2);
		memset(buf,0,sizeof buf);
		
		while(1)	//设置网络参数
		{
			printf_num=2;
			printf("AT+WJAPIP=%s,%s,%s,%s\r",ip,network,gateway,dns);
			delay_ms(500);
			COM2GetBuf(buf,100);
			if(strchr((const char *)buf,'O')[0]=='O')break;
		}
		comClearRxFifo(COM2);
		memset(buf,0,sizeof buf);
	}
		
	printf_num=2;	//设置WIFI
	printf("AT+WJAP=%s,%s\r",ssid,password);
	delay_ms(2000);
	printf_num=2;
	printf("AT+WJAP=%s,%s\r",ssid,password);
	while(1)
	{
		delay_ms(2000);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"STATION_UP")[0]=='S')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//设置北京时间时区
	{
		printf_num=2;
		printf("AT+SNTPCFG=+8\r");
		delay_ms(500);
		COM2GetBuf(buf,30);
		if(strchr((const char *)buf,'O')[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//校准网络时间
	{
		printf_num=2;
		printf("AT+SNTPTIME\r");
		delay_ms(500);
		COM2GetBuf(buf,30);
		if(strchr((const char *)buf,'O')[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
}

void	Emw3060_con(void)				//EMW连接阿里云
{
	u8 buf[250];
	char hc[100];
	char cs[40];
	char mac[33];
	char *m;
	Emwled=2;
	ProductKeyw=ProductKey1;
	DeviceNamew=DeviceName1;
	DeviceSecretw=DeviceSecret1;
	
	while(1)	//读取MAC地址
	{
		printf_num=2;
		printf("AT+WMAC?\r");
		delay_ms(200);
		COM2GetBuf(buf,100);
		if(strstr((const char *)buf,"+WMAC:")[0]=='+')break;
	}
	m=strstr((const char *)buf,"+WMAC:")+6;
	m[12]=0;
	memset(mac,0,sizeof mac);
	sprintf(mac,"%s",m);
	
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	sprintf(hc,"clientId%sdeviceName%sproductKey%stimestamp789",mac,DeviceNamew,ProductKeyw);
	while(1)	//写入设备鉴权信息
	{
		printf_num=2;
		hmacmd5(hc,strlen(hc),strlen(DeviceSecretw),cs);
		printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceNamew,ProductKeyw,cs);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//设置设备CID
	{
		printf_num=2;
		printf("AT+MQTTCID=%s|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r",mac);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//设置目标IP地址
	{
		printf_num=2;
		printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKeyw);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//保活时间设置
	{
		printf_num=2;
		printf("AT+MQTTKEEPALIVE=300\r");
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//MQTT自动重连打开
	{
		printf_num=2;
		printf("AT+MQTTRECONN=ON\r");
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//MQTT连接
	{
		printf_num=2;
		printf("AT+MQTTSTART\r");
		delay_ms(2000);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"MQTTEVENT:CONNECT,SUCCESS")[0]=='M')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	while(1)	//订阅TOPIC
	{
		printf_num=2;
		printf("AT+MQTTSUB=0,/%s/%s/user/get,1\r",ProductKeyw,DeviceNamew);
		delay_ms(200);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	memset(g_RxBuf2,0,UART2_RX_BUF_SIZE);
	Emwled=3;
	sumbz=0;
}

u8	sendEmw(char *data,unsigned char w)	//EMW上报数据
{
	static u8 buf[201];
	char hc[500];
	u16 len;u8 time;
	Emwled=2;
	ProductKeyw=ProductKey1;
	DeviceNamew=DeviceName1;
	DeviceSecretw=DeviceSecret1;
	memset(buf,0,sizeof buf);
	comClearRxFifo(COM2);
	while(1)	//设置发送topic
	{
		printf_num=2;
		if(!w)
			printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKeyw,DeviceNamew);	//切换上报topic
		else printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKeyw,DeviceNamew);
		delay_ms(500);
		COM2GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM2);
	memset(buf,0,sizeof buf);
	
	len=0;time=0;
	while(1)	//发送数据指令
	{
		printf_num=2;
		printf("AT+MQTTSEND=%d\r",strlen(data)+13);
		while(1)
		{
			delay_ms(200);
			COM2GetBuf(buf,20);
			if(strstr((const char *)buf,">")[0]=='>')break;
			if(strstr((const char *)buf,"ERROR")[0]=='E')break;
			if(++time==10)return 0;
		}
		if(strstr((const char *)buf,">")[0]=='>')break;
		comClearRxFifo(COM2);
		if(++len==5){return 0;}
	}
	memset(buf,0,sizeof buf);
	
	sprintf(hc,"{\"params\":{%s}}",data);
	len=strlen(hc);
	if(hc[len-3]==',')hc[len-3]=' ';
	printf_num=2;
	printf("%s",hc);
	
	delay_ms(3000);
	COM2GetBuf(buf,100);
	Emwled=3;
	comClearRxFifo(COM2);
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')sumbz=0;	//判断是否正常发送
	else sumbz++;
	if(sumbz>=3)return 0;
	else return 1;
}
