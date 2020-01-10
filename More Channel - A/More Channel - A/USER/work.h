#ifndef __Work_H
#define __Work_H
#include "sys.h"
#include "main.h"
#include "delay.h"
void	OpenPartition()		//打开分区电磁阀
{
	SetIO8(Run_SetMessage.Terminaladdr,IObuf[Run_SetMessage.TerminalNum]);
}

void	ClosePartition()	//关闭分区电磁阀
{
	SetIO8(Run_SetMessage.Terminaladdr,0);
}

void	OpenFer()			//打开专用桶
{
	SetIO8(1,IObuf[Run_SetMessage.SpecialFer-1]);
}

void	CloseFer()			//关闭专用桶
{
	SetIO8(1,0);
}

void	OpenFer_Public()	//打开公用桶
{
	if(PublicFer)SetIO8(1,IObuf[Run_SetMessage.SpecialFer]|0x80);
}

void	CloseFer_Pubilc()	//关闭公用桶
{
	if(PublicFer)SetIO8(1,0);
}

void	ChangeFlow()		//流量控制
{
	u8	Flowcmd[]={10,06,00,02,0x0b,0xb7,0x6f,0x4c};
	u16	crc;
	Flowcmd[4]=(0x07cf+((int)((float)NeedFlow*1.205)/10*10+10))>>8;
	Flowcmd[5]=(0x07cf+((int)((float)NeedFlow*1.205)/10*10+10))&0xff;
	crc=mc_check_crc16(Flowcmd,6);
	Flowcmd[6]=crc>>8;
	Flowcmd[7]=crc&0xff;
	comSendBuf(COM4,Flowcmd,sizeof(Flowcmd));
	delay_ms(1000);
}

u16		CalculateFlow()		//流量计算
{
	return Run_SetMessage.Mu*Run_SetMessage.AverageMu/(Concentration[Run_SetMessage.SpecialFer]/100.00)/(Remaining/60.00);
}

#define BCDtoINT(value) ((value>>4)*10+(value&0x0f))
u8		ReadWater()			//读水表
{
	u8 Watercmd[]={01,03,00,00,00,04,0x7C,0x0E};
	u8 buf[30];		//专用表
	u16 crc;
	u8 ok;
	ok=0;
	Watercmd[0]=1;
	crc=mc_check_crc16(Watercmd,6);
	Watercmd[6]=crc>>8;
	Watercmd[7]=crc&0xff;
	comClearRxFifo(COM4);
	comSendBuf(COM4,Watercmd,sizeof(Watercmd));
	delay_ms(100);
	COM4GetBuf(buf,13);
	crc=mc_check_crc16(buf,11);
	if((buf[11]==(crc>>8))&(buf[12]==(crc&0xff)))
	{
		water1=(BCDtoINT(buf[4])*1000000+BCDtoINT(buf[4])*10000+BCDtoINT(buf[5])*100+BCDtoINT(buf[6]))/10;
		water1flow=BCDtoINT(buf[8])*100+BCDtoINT(buf[9]);
		ok=1;
	}
	
	Watercmd[0]=2;	//公用表
	crc=mc_check_crc16(Watercmd,6);
	Watercmd[6]=crc>>8;
	Watercmd[7]=crc&0xff;
	comClearRxFifo(COM4);
	comSendBuf(COM4,Watercmd,sizeof(Watercmd));
	delay_ms(100);
	COM4GetBuf(buf,13);
	crc=mc_check_crc16(buf,11);
	if((buf[11]==(crc>>8))&(buf[12]==(crc&0xff)))
	{
		water2=BCDtoINT(buf[4])*10000+BCDtoINT(buf[5])*100+BCDtoINT(buf[6]);
	}
	
	Watercmd[0]=3;	//总水表
	crc=mc_check_crc16(Watercmd,6);
	Watercmd[6]=crc>>8;
	Watercmd[7]=crc&0xff;
	comClearRxFifo(COM4);
	comSendBuf(COM4,Watercmd,sizeof(Watercmd));
	delay_ms(100);
	COM4GetBuf(buf,13);
	crc=mc_check_crc16(buf,11);
	if((buf[11]==(crc>>8))&(buf[12]==(crc&0xff)))
	{
		water0=BCDtoINT(buf[4])*10000+BCDtoINT(buf[5])*100+BCDtoINT(buf[6]);
	}
	
	Watercmd[0]=9;	//压力表
	Watercmd[5]=2;
	crc=mc_check_crc16(Watercmd,6);
	Watercmd[6]=crc>>8;
	Watercmd[7]=crc&0xff;
	comClearRxFifo(COM4);
	comSendBuf(COM4,Watercmd,sizeof(Watercmd));
	delay_ms(100);
	COM4GetBuf(buf,9);
	crc=mc_check_crc16(buf,7);
	if((buf[7]==(crc>>8))&(buf[8]==(crc&0xff)))
	{
		pr.I[0]=(buf[3]<<8)|buf[4];
		pr.I[1]=(buf[5]<<8)|buf[6];
	}
	return ok;
}

void	readthree()						//识别三元组
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

#endif
