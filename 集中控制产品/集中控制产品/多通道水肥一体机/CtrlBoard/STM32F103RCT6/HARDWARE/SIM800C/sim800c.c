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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������ 
//ATK-SIM800C GSM/GPRSģ������	  
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/4/1
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//******************************************************************************** 
//��
 	
Ring GPRSCircle;
u8 ConnectFlag = 0;//gprs���ӱ�־
u8 SimImei[15] = {0};
StateMonitor  Sim800cMonitor;
//����SIM800Cģ�鵱ǰ״̬�����֮ǰ״̬�������
void SetSim800cCurrSta(u8 Sta)
{
	Sim800cMonitor.CurrSta = Sta;
	if(Sta > Sim800cMonitor.ErrSta)
		Sim800cMonitor.ErrTime = 0;
}
//����SIM800Cģ��������״̬���ۼӴ������
void SetSim800cErrSta(u8 Sta)
{
	Sim800cMonitor.ErrSta = Sta;
	Sim800cMonitor.ErrTime++;
}
//��ʼ��GPRSCircle
void InitGPRSRing(void)
{
	memset(&GPRSCircle, 0 , sizeof(GPRSCircle));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//SIM800C���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* sim800c_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)GPRSCircle.buf,(const char*)str);
	return (u8*)strx;
}
//��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim800c_chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10); 
	return 0;
}
//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim800c_hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A'); 
	return '0';
}
u32 alfredtestCnt = 0;
u8	g_AnswerType = 0;
//ͨѶЭ�����
//���룺�յ��ı���
u8 GPRS_Unpack(u8*Buf,u8 Len)
{
	u8 uCnt = 0;
	//�ж�֡ͷ
	if(Buf[0] != 0xAA)
	{
		return 0;
	}
	//�ж�IMEI��
	for(uCnt = 0; uCnt<15; uCnt++)
	{
		if(Buf[uCnt+1] != SimImei[uCnt])
		{
			return 0;
		}
	}
	switch(Buf[16])
	{
		case 0x01://��ѯ
			g_AnswerType = 1;
			break;
	}
	return Len;
}
//SIM800C��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	u8*p = NULL;
	u8 uCnt = 0;
	if((u32)cmd<=0XFF)//
	{
		while((USART3->SR&0X40)==0);//�ȴ���һ�����ݷ������  
		USART3->DR=(u32)cmd;
	}
	else 
	{
		u3_printf("%s\r\n",cmd);//��������
	}
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{ 
			delay_ms(10);
			//�����յ��µ����ݣ����ж��Ƿ���Ϊ��Ч����
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
					*p = 1;//�ƻ��ַ���
					break;//�õ���Ч���� 
				}
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//����������ɺ�ƽ̨���͵�����
//���� 0����·���� 1�������ѶϿ�
u8 UnpackPlatform(void)
{
	u8 u8Cnt = 0;
//	u8 straddr = 0;
	u8 *straddrp = NULL;
	u8 uLen = 0;
	u8 u8BaseAddr = 7;
	u8 ReceiveData[256] = {0};
	//ֻ���գ�������û�յ�
	COMGetBuf(COM3 ,&GPRSCircle, sizeof(GPRSCircle.buf));
	{
		straddrp = sim800c_check_cmd("+IPD,");
		if(straddrp != NULL)//�����յ�������
		{
			*straddrp = 1;//�ƻ��ַ�����ֹ�ٴ��ҵ�
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
				//�쳣��Э���ж��峤�Ȳ�����256
			}
			for(u8Cnt = 0; u8Cnt<uLen; u8Cnt++)
			{
				ReceiveData[u8Cnt] = *(straddrp++ +u8BaseAddr);
				//�ƻ��ַ�����0���ֽ�
				if(*(straddrp+u8BaseAddr) == 0)
				{
					*(straddrp+u8BaseAddr) = 1;
				}
			}
			GPRS_Unpack(ReceiveData, u8Cnt);
		}
		else if(sim800c_check_cmd("CLOSED"))
		{
			ConnectFlag = 0;//�Ͽ�����
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
	if((g_AnswerType == 0) && (g_GPRSTimerFlag == 1))//����
	{
		uCnt = 0;
		SendBuf[uCnt++] = 0x4F;
		SendBuf[uCnt++] = 0x4B;
		g_GPRSTimerFlag = 0;
		sendFlag = 1;
	}
	if(sendFlag == 1)
	{
		if(sim800c_send_cmd("AT+CIPSEND",">",500)==0)		//��������
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
//GPRS���Բ��ִ���

//tcp/udp����
//����������,��ά������
//ip portȫ���̶�д��
void sim800c_tcpudp_test(void)
{ 
	u16 timex=0;
	SetSim800cCurrSta(AT_CONNECT);
	if(sim800c_send_cmd("AT+CIPSTART=\"TCP\",\"wlw.mingjitech.com\",\"9266\"","CONNECT OK",500))
	{
		SetSim800cErrSta(AT_CONNECT);
		return;		//��������
	}
	InitGPRSRing();
	while(1)//alfred �����ݲŷ���
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
//SIM800C GPRS����
//���ڲ���TCP/UDP����
//����ֵ:0,����
//����,�������
u8 sim800c_gprs_test(void)
{
	while(1)
	{
		switch(ConnectFlag)
		{
			case 0:
				delay_ms(50);
				SetSim800cCurrSta(AT_SHUT);
				if(sim800c_send_cmd("AT+CIPSHUT","SHUT OK",100)) //�ر��ƶ�����
				{
					SetSim800cErrSta(AT_SHUT);
					return 10;		
				}
				SetSim800cCurrSta(AT_CLOSED);
				if(sim800c_send_cmd("AT+CIPCLOSE=1","CLOSE OK",100))//�ر�����
				{
					SetSim800cErrSta(AT_CLOSED);
					return 11;
				}
				SetSim800cCurrSta(AT_CLASS);
				if(sim800c_send_cmd("AT+CGCLASS=\"B\"","OK",500))			//����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
				{
					SetSim800cErrSta(AT_CLASS);
					return 12;	
				}
				SetSim800cCurrSta(AT_PDP);
				if(sim800c_send_cmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",500))//����PDP������,��������Э��,��������Ϣ
				{
					SetSim800cErrSta(AT_PDP);
					return 13;
				}
				SetSim800cCurrSta(AT_ATTACHMENT);
				if(sim800c_send_cmd("AT+CGATT=1","OK",500))			//����GPRSҵ��
				{
					SetSim800cErrSta(AT_ATTACHMENT);
					return 14;		
				}
				SetSim800cCurrSta(AT_CMNET);
				if(sim800c_send_cmd("AT+CIPCSGP=1,\"CMNET\"","OK",500))	//����ΪGPRS����ģʽ
				{
					SetSim800cErrSta(AT_CMNET);
					return 15;	
				}
				SetSim800cCurrSta(AT_DATADIS);
				if(sim800c_send_cmd("AT+CIPHEAD=1","OK",500))					//���ý���������ʾIPͷ(�����ж�������Դ)
				{
					SetSim800cErrSta(AT_DATADIS);
					return 16;
				}
				SetSim800cCurrSta(AT_SENDMODE);
				if(sim800c_send_cmd("AT+CIPATS=1,1","OK",500))//����1s��ʱ����
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
//GSM��Ϣ��ʾ(�ź�����,��ص���,����ʱ��)
//����ֵ:0,����
//����,�������
u8 sim800c_gsminfo_show(void)
{
//	u8 *p,*p1,*p2;
	u8 res=0;

	SetSim800cCurrSta(AT_SIMCHECK);
	
	if(sim800c_send_cmd("AT+CPIN?","OK",200))//��ѯSIM���Ƿ���λ 
	{
		SetSim800cErrSta(AT_SIMCHECK);
		res = 7;	
	}
	SetSim800cCurrSta(AT_CSQ);
	if(sim800c_send_cmd("AT+CSQ","+CSQ:",200)==0)		//��ѯ�ź�����
	{ 
	}
	else
	{
		SetSim800cErrSta(AT_CSQ);
		res = 8;
	}
	SetSim800cCurrSta(AT_POW);
	if(sim800c_send_cmd("AT+CBC","+CBC:",200)==0)		//��ѯ��ص���
	{ 
	}
	else 
	{
		SetSim800cErrSta(AT_POW);
		res = 9; 
	}
	return res;
}
//NTP����ʱ��
void ntp_update(void)
{  
	 sim800c_send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",200);//���ó��س���1
	 sim800c_send_cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",200);
	 sim800c_send_cmd("AT+SAPBR=1,1",0,200);//����һ��GPRS������
	 delay_ms(5);
	 sim800c_send_cmd("AT+CNTPCID=1","OK",200);//����CNTPʹ�õ�CID
	 sim800c_send_cmd("AT+CNTP=\"202.120.2.101\",32","OK",200);//����NTP�������ͱ���ʱ��(32ʱ�� ʱ����׼ȷ)
	 sim800c_send_cmd("AT+CNTP","+CNTP: 1",600);//ͬ������ʱ��
	
}
//GPRD����
//u8 g_u8GPRS_Cfg_Step = 0;//alfred ����ʹ��
void GPRS_Config(void)
{
	SetSim800cCurrSta(AT_TEST);
	while(sim800c_send_cmd("AT","OK",100))//����Ƿ�Ӧ��ATָ�� 
	{
		delay_ms(100);
		SetSim800cErrSta(AT_TEST);
	} 	 
	SetSim800cCurrSta(AT_AE1);
	while(sim800c_send_cmd("ATE1","OK",100))//�򿪻���
	{
		delay_ms(20);
		SetSim800cErrSta(AT_AE1);
	}
	SetSim800cCurrSta(AT_IMEI);
	while(sim800c_send_cmd("AT+CGSN","AT+CGSN",100))//��ѯIMEI��
	{
		delay_ms(20);
		SetSim800cErrSta(AT_IMEI);
	}
	SetSim800cCurrSta(AT_AE0);
	while(sim800c_send_cmd("ATE0","OK",100))//�رջ���
	{
		delay_ms(20);
		SetSim800cErrSta(AT_AE0);
	}
	SetSim800cCurrSta(AT_ATCOMMAND);
	while(sim800c_send_cmd("AT_CIPMODE=0","OK",100))//��������ģʽ����
	{
		delay_ms(20);
		SetSim800cErrSta(AT_ATCOMMAND);
	}
//	while(sim800c_send_cmd("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK",100))//���ó��س���1
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 3;
//	}
//	while(sim800c_send_cmd("AT+SAPBR=3,1,\"APN\",\"CMNET\"","OK",100))//���ó��س���1
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 4;
//	}
//	while(sim800c_send_cmd("AT+SAPBR=1,1",0,100))//����һ��GPRS������
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 5;
//	}
//	while(sim800c_send_cmd("AT+CNTPCID=1","OK",100))//����CNTPʹ�õ�CID
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 6;
//	}
//	while(sim800c_send_cmd("AT+CNTP=\"202.120.2.101\",32","OK",100))//����NTP�������ͱ���ʱ��(32ʱ�� ʱ����׼ȷ)
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 7;
//	}
//	//while(sim800c_send_cmd("AT+CNTP","+CNTP: 1",600))//ͬ������ʱ��
//	while(sim800c_send_cmd("AT+CNTP","+CNTP: 1",600))//ͬ������ʱ��
//	{
//		delay_ms(20);
//		g_u8GPRS_Cfg_Step = 8;
//	}
	
}
//����ͨ������
void gprs_task(void *pdata)
{
	u8 timex=0;
	u8 sim_ready=0;
	GPRS_Config();
	while(1)
	{
		delay_ms(50); 
		if(sim_ready)		//SIM������.
		{
			sim800c_gprs_test();//GPRS����
			timex=0;						
		}
		if(timex==0)		//2.5�����Ҹ���һ��
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
			LED0=!LED0;//1s��˸ 
		}
		timex++;	 
	} 	
}

