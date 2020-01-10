#include "sim800c.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 
#include "ZoneCtrl.h"
//#include "lcd.h" 
#include "w25qxx.h" 	 
//#include "touch.h" 	 
#include "malloc.h"
#include "string.h"    
//#include "text.h"		
#include "Rs485.h" 
//#include "ff.h"
//#include "timer.h"
#include "Includes.h"
//////////////////////////////////////////////////////////////////////////////////	   
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103开发板 
//ATK-SIM800C GSM/GPRS模块驱动	  
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/4/1
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//******************************************************************************** 
//无
 	
Ring GPRSCircle;
u8 ConnectFlag = 0;//gprs链接标志
u8 SimImei[15] = {0};
StateMonitor  Sim800cMonitor;
//设置SIM800C模块当前状态并清空之前状态错误计数
void SetSim800cCurrSta(u8 Sta)
{
	Sim800cMonitor.CurrSta = Sta;
	if(Sta > Sim800cMonitor.ErrSta)
		Sim800cMonitor.ErrTime = 0;
}
//设置SIM800C模块出错误的状态并累加错误次数
void SetSim800cErrSta(u8 Sta)
{
	Sim800cMonitor.ErrSta = Sta;
	Sim800cMonitor.ErrTime++;
}
//初始化GPRSCircle
void InitGPRSRing(void)
{
	memset(&GPRSCircle, 0 , sizeof(GPRSCircle));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//SIM800C发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)GPRSCircle.buf,(const char*)str);
	return (u8*)strx;
}
//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
u32 alfredtestCnt = 0;
u8	g_AnswerType = 0;
//通讯协议解析
//输入：收到的报文
u8 GPRS_Unpack(u8*Buf,u8 Len)
{
	u8 uCnt = 0;
	//判断帧头
	if(Buf[0] != 0xAA)
	{
		return 0;
	}
	//判断IMEI号
	for(uCnt = 0; uCnt<15; uCnt++)
	{
		if(Buf[uCnt+1] != SimImei[uCnt])
		{
			return 0;
		}
	}
	switch(Buf[16])
	{
		case 0x01://查询
			g_AnswerType = 1;
			break;
	}
	return Len;
}
//SIM800C发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u8*p = NULL;
	u8 uCnt = 0;
	if((u32)cmd<=0XFF)//
	{
		while((USART3->SR&0X40)==0);//等待上一次数据发送完成  
		USART3->DR=(u32)cmd;
	}
	else 
	{
		u3_printf("%s\r\n",cmd);//发送命令
	}
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{ 
			delay_ms(10);
			//串口收到新的数据，并判断是否有为有效数据
			COMGetBuf(COM3 ,&GPRSCircle, 256);
			{
				p = sim800c_check_cmd(ack);
				if(p)
				{
					if(strcmp((const char *)ack , "AT+CGSN") == 0)
					{
						for(uCnt = 0; uCnt<15; uCnt++)
						{
							SimImei[uCnt] = *(p+uCnt+10);
						}
					}
					*p = 1;//破坏字符串
					break;//得到有效数据 
				}
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//解析链接完成后平台发送的数据
//返回 0：链路正常 1：链接已断开
u8 UnpackPlatform(void)
{
	u8 u8Cnt = 0;
//	u8 straddr = 0;
	u8 *straddrp = NULL;
	u8 uLen = 0;
	u8 u8BaseAddr = 7;
	u8 ReceiveData[256] = {0};
	//只管收，不管收没收到
	COMGetBuf(COM3 ,&GPRSCircle, sizeof(GPRSCircle.buf));
	{
		straddrp = sim800c_check_cmd("+IPD,");
		if(straddrp != NULL)//查找收到的数据
		{
			*straddrp = 1;//破坏字符串防止再次找到
			if(*(straddrp+6) == 0x3A)
			{
				uLen = sim800c_chr2hex(*(straddrp+5));
			}
			else if(*(straddrp+7) == 0x3A)
			{
				uLen = sim800c_chr2hex(*(straddrp+5))*10 
						+ sim800c_chr2hex(*(straddrp+6));
				u8BaseAddr = 8;
			}
			else if(*(straddrp+8) == 0x3A)
			{
				uLen = sim800c_chr2hex(*(straddrp+5))*100
						+ sim800c_chr2hex(*(straddrp+6))*10
						+ sim800c_chr2hex(*(straddrp+7));
				u8BaseAddr = 9;
			}
			else if(*(straddrp+9) == 0x3A)
			{
				//异常，协议中定义长度不超过256
			}
			for(u8Cnt = 0; u8Cnt<uLen; u8Cnt++)
			{
				ReceiveData[u8Cnt] = *(straddrp++ +u8BaseAddr);
				//破坏字符串中0的字节
				if(*(straddrp+u8BaseAddr) == 0)
				{
					*(straddrp+u8BaseAddr) = 1;
				}
			}
			GPRS_Unpack(ReceiveData, u8Cnt);
		}
		else if(sim800c_check_cmd("CLOSED"))
		{
			ConnectFlag = 0;//断开链接
			return 1;
		}
	}
		
	return 0;
}
u16 Check_sun(u8* Buf, u8 Len)
{
	u8 i ;
	u16 ReValue = 0;
	for(i = 0; i<Len; i++)
	{
		ReValue = ReValue + Buf[i];
	}
	return (ReValue&0x00ff);
}
u8 g_GPRSTimerFlag = 0;
u8 g_HeartErrorCnt = 0;
u8 Initiative_Send(void)
{
	u8 SendBuf[256];
	u8 uCnt = 0;
	u8 i = 0,j = 0;
	u8 sendFlag = 0;
	if((g_AnswerType|g_GPRSTimerFlag)!=0)
	{
		SendBuf[uCnt++] = 0xAA;
		for(i=0; i<15; i++)
		{
			SendBuf[uCnt++] = SimImei[i];
		}
	}
	switch(g_AnswerType)
	{
		case 1:
			//TemperatureGet((u8*)&TemperatureData);
			SendBuf[uCnt++] = 0x01;
			SendBuf[uCnt++] = 170;
			SendBuf[uCnt++] = 0x00;
			SendBuf[uCnt++] = 0x01;
			for(i=0;i<28; i++)
			{
				for(j=0;j<3;j++)
				{
				}
			}
			SendBuf[uCnt++] = Check_sun(&SendBuf[18] , 170);
			sendFlag = 1;
			g_AnswerType = 0;
			break;
		default:
			break;
	}
	if((g_AnswerType == 0) && (g_GPRSTimerFlag == 1))//心跳
	{
		uCnt = 0;
		SendBuf[uCnt++] = 0x4F;
		SendBuf[uCnt++] = 0x4B;
		g_GPRSTimerFlag = 0;
		sendFlag = 1;
	}
	if(sendFlag == 1)
	{
		if(sim800c_send_cmd("AT+CIPSEND",">",500)==0)		//发送数据
		{
			usart3send(SendBuf,(u16) uCnt);			
		}
		else
		{
			if((++g_HeartErrorCnt) > 100)
			{
				g_HeartErrorCnt = 0;
				ConnectFlag = 0;
				return 1;
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//GPRS测试部分代码

//tcp/udp测试
//带心跳功能,以维持连接
//ip port全部固定写死
void sim800c_tcpudp_test(void)
{ 
	u16 timex=0;
	SetSim800cCurrSta(AT_CONNECT);
	if(sim800c_send_cmd("AT+CIPSTART=\"TCP\",\"wlw.mingjitech.com\",\"9266\"","CONNECT OK",500))
	{
		SetSim800cErrSta(AT_CONNECT);
		return;		//发起连接
	}
	InitGPRSRing();
	while(1)//alfred 有数据才发送
	{ 
		if(UnpackPlatform())
		{
			return ;
		}
		delay_ms(1000); 
		if(Initiative_Send())
		{
			return;
		}
		if(timex++%60 == 0)
		{
			g_GPRSTimerFlag = 1;
		}
	}
}
//SIM800C GPRS测试
//用于测试TCP/UDP连接
//返回值:0,正常
//其他,错误代码
u8 sim800c_gprs_test(void)
{
	while(1)
	{
		switch(ConnectFlag)
		{
			case 0:
				delay_ms(50);
				SetSim800cCurrSta(AT_SHUT);
				if(sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100)) //关闭移动场景
				{
					SetSim800cErrSta(AT_SHUT);
					return 10;		
				}
				SetSim800cCurrSta(AT_CLOSED);
				if(sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100))//关闭连接
				{
					SetSim800cErrSta(AT_CLOSED);
					return 11;
				}
				SetSim800cCurrSta(AT_CLASS);
				if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",500))			//设置GPRS移动台类别为B,支持包交换和数据交换 
				{
					SetSim800cErrSta(AT_CLASS);
					return 12;	
				}
				SetSim800cCurrSta(AT_PDP);
				if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",500))//设置PDP上下文,互联网接协议,接入点等信息
				{
					SetSim800cErrSta(AT_PDP);
					return 13;
				}
				SetSim800cCurrSta(AT_ATTACHMENT);
				if(sim800c_send_cmd("AT+CGATT=1","OK",500))			//附着GPRS业务
				{
					SetSim800cErrSta(AT_ATTACHMENT);
					return 14;		
				}
				SetSim800cCurrSta(AT_CMNET);
				if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))	//设置为GPRS连接模式
				{
					SetSim800cErrSta(AT_CMNET);
					return 15;	
				}
				SetSim800cCurrSta(AT_DATADIS);
				if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500))					//设置接收数据显示IP头(方便判断数据来源)
				{
					SetSim800cErrSta(AT_DATADIS);
					return 16;
				}
				SetSim800cCurrSta(AT_SENDMODE);
				if(sim800c_send_cmd("AT+CIPATS=1,1","OK",500))//设置1s定时发送
				{
					SetSim800cErrSta(AT_SENDMODE);
					return 17;
				}
				ConnectFlag = 1;
				break;
			case 1:
				sim800c_tcpudp_test();
			break;
			default:
				break;
		}
	}
	return 0;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//GSM信息显示(信号质量,电池电量,日期时间)
//返回值:0,正常
//其他,错误代码
u8 sim800c_gsminfo_show(void)
{
//	u8 *p,*p1,*p2;
	u8 res=0;

	SetSim800cCurrSta(AT_SIMCHECK);
	
	if(sim800c_send_cmd("AT+CPIN?","OK",200))//查询SIM卡是否在位 
	{
		SetSim800cErrSta(AT_SIMCHECK);
		res = 7;	
	}
	SetSim800cCurrSta(AT_CSQ);
	if(sim800c_send_cmd("AT+CSQ","+CSQ:",200)==0)		//查询信号质量
	{ 
	}
	else
	{
		SetSim800cErrSta(AT_CSQ);
		res = 8;
	}
	SetSim800cCurrSta(AT_POW);
	if(sim800c_send_cmd("AT+CBC","+CBC:",200)==0)		//查询电池电量
	{ 
	}
	else 
	{
		SetSim800cErrSta(AT_POW);
		res = 9; 
	}
	return res;
}
//NTP更新时间
void ntp_update(void)
{  
	 sim800c_send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",200);//配置承载场景1
	 sim800c_send_cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",200);
	 sim800c_send_cmd("AT+SAPBR=1,1",0,200);//激活一个GPRS上下文
	 delay_ms(5);
	 sim800c_send_cmd("AT+CNTPCID=1","OK",200);//设置CNTP使用的CID
	 sim800c_send_cmd("AT+CNTP=\"202.120.2.101\",32","OK",200);//设置NTP服务器和本地时区(32时区 时间最准确)
	 sim800c_send_cmd("AT+CNTP","+CNTP: 1",600);//同步网络时间
	
}
//GPRD配置
//u8 g_u8GPRS_Cfg_Step = 0;//alfred 测试使用
void GPRS_Config(void)
{
	SetSim800cCurrSta(AT_TEST);
	while(sim800c_send_cmd("AT","OK",100))//检测是否应答AT指令 
	{
		delay_ms(100);
		SetSim800cErrSta(AT_TEST);
	} 	 
	SetSim800cCurrSta(AT_AE1);
	while(sim800c_send_cmd("ATE1","OK",100))//打开回显
	{
		delay_ms(20);
		SetSim800cErrSta(AT_AE1);
	}
	SetSim800cCurrSta(AT_IMEI);
	while(sim800c_send_cmd("AT+CGSN","AT+CGSN",100))//查询IMEI号
	{
		delay_ms(20);
		SetSim800cErrSta(AT_IMEI);
	}
	SetSim800cCurrSta(AT_AE0);
	while(sim800c_send_cmd("ATE0","OK",100))//关闭回显
	{
		delay_ms(20);
		SetSim800cErrSta(AT_AE0);
	}
	SetSim800cCurrSta(AT_ATCOMMAND);
	while(sim800c_send_cmd("AT_CIPMODE=0","OK",100))//设置命令模式回显
	{
		delay_ms(20);
		SetSim800cErrSta(AT_ATCOMMAND);
	}
//	while(sim800c_send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",100))//配置承载场景1
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 3;
//	}
//	while(sim800c_send_cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",100))//配置承载场景1
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 4;
//	}
//	while(sim800c_send_cmd("AT+SAPBR=1,1",0,100))//激活一个GPRS上下文
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 5;
//	}
//	while(sim800c_send_cmd("AT+CNTPCID=1","OK",100))//设置CNTP使用的CID
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 6;
//	}
//	while(sim800c_send_cmd("AT+CNTP=\"202.120.2.101\",32","OK",100))//设置NTP服务器和本地时区(32时区 时间最准确)
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 7;
//	}
//	//while(sim800c_send_cmd("AT+CNTP","+CNTP: 1",600))//同步网络时间
//	while(sim800c_send_cmd("AT+CNTP","+CNTP: 1",600))//同步网络时间
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 8;
//	}
	
}
//网络通信任务
void gprs_task(void *pdata)
{
	u8 timex=0;
	u8 sim_ready=0;
	GPRS_Config();
	while(1)
	{
		delay_ms(50); 
		if(sim_ready)		//SIM卡就绪.
		{
			sim800c_gprs_test();//GPRS测试
			timex=0;						
		}
		if(timex==0)		//2.5秒左右更新一次
		{
			
			if(sim800c_gsminfo_show()==0)
			{
				sim_ready = 1;
			}
			else 
			{
				sim_ready = 0;
			}
		}	
		if((timex%20)==0)
		{
			LED0=!LED0;//1s闪烁 
		}
		timex++;	 
	} 	
}

