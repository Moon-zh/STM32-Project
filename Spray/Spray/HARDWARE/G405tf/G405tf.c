#include "delay.h"
#include "string.h"
#include "rs485.h"
#include "stdio.h"
#include "G405tf.h"

u8 user[]={"AT+CLOUD=00013712000000000007,5qhOKbxu\r"};

void	RTS_4G()
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_11;				 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	GPIO_SetBits(GPIOA,GPIO_Pin_12);						 //PB.5 输出高
//	GPIO_SetBits(GPIOA,GPIO_Pin_11);
	
}

u8		init_4G()			//4G初始化
{
	u8 i;
	u8 buf[250];
	u8 *a=buf;
	RTS_4G();
//	return ;
	memset(buf,0,250);
//	printf("AT+ENTM\r\n");delay_ms(20);
//	printf("AT+ENTM\r\n");delay_ms(20);
//	printf("AT+ENTM\r\n");delay_ms(20);
	printf("+++");delay_ms(20);
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'a')[0]=='a')break;
		if(strchr((const char *)a,'r')[0]=='r')break;
	}
	comClearRxFifo(COM1);
	//comClearRxFifo(COM1);
	if(i==0)return 0;

	delay_ms(20);
	printf("a");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'o')[0]=='o')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;

	delay_ms(20);
	printf("AT+WKMOD=NET\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKAEN=ON\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKA=TCP,data.mingjitech.com,15000\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKASL=LONG\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKATO=5\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKBEN=OFF\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKRSTIM=60\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+HEARTEN=ON\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+HEARTTM=30\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+HEARTDT=4F4B\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+HEARTSND=NET\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+REGEN=ON\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+REGSND=LINK\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;

	delay_ms(20);
	printf("AT+REGTP=CLOUD\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("%s",user);
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+SOCKIND=OFF\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+UART=115200,8,1,NONE,NFC\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+UARTFT=50\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+UARTFL=1024\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	delay_ms(20);
	printf("AT+Z\r");
	for(i=10;i>0;i--)
	{
		delay_ms(50);
		COM1GetBuf(a,200);
		if(strchr((const char *)a,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	if(i==0)return 0;
	
	//delay_ms(1500);delay_ms(1500);
	return 1;
}
