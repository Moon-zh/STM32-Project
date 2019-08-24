#include "sys.h"
#include "rs485.h"
#include "check.h"
#include "delay.h"
#include "main.h"

void	HDMI_Set_Plan(u16 addr)					//设置触摸屏倒计时时间
{
	u8 Plancmd[]={01,06,00,01,00,00,0x7C,0x0E};
	u16 crc;
	Plancmd[2]=addr>>8;
	Plancmd[3]=addr&0xff;
	Plancmd[5]=1;
	crc=mc_check_crc16(Plancmd,6);
	Plancmd[6]=crc>>8;
	Plancmd[7]=crc&0xff;
	comSendBuf(COM5,Plancmd,sizeof(Plancmd));
	delay_ms(100);
}

void	HDMI_Set_Log(u8	Num,u16	addr)			//显示日志
{
	u8 Logcmd[]={01,16,01,0x91,00,17,34,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,0x7C,0x0E};
	u16 crc;
	Logcmd[2]=addr>>8;
	Logcmd[3]=addr&0xff;
	
	Logcmd[8]=Num;
	Logcmd[10]=Log.StartTime[0];
	Logcmd[12]=Log.StartTime[1];
	Logcmd[14]=Log.StartTime[2];
	Logcmd[16]=Log.StartTime[3];
	Logcmd[18]=Log.StartTime[4];
	Logcmd[20]=Log.StartMode;
	Logcmd[22]=Log.IrrMode;
	Logcmd[24]=Log.Partition;
	Logcmd[25]=Log.Irrtime>>8;
	Logcmd[26]=Log.Irrtime&0xff;
	Logcmd[28]=Log.StopTime[0];
	Logcmd[30]=Log.StopTime[1];
	Logcmd[32]=Log.StopTime[2];
	Logcmd[34]=Log.StopTime[3];
	Logcmd[36]=Log.StopTime[4];
	Logcmd[38]=Log.StopMode;
	Logcmd[39]=Log.RemTime>>8;
	Logcmd[40]=Log.RemTime&0xff;

	crc=mc_check_crc16(Logcmd,sizeof(Logcmd)-2);
	Logcmd[sizeof(Logcmd)-2]=crc>>8;
	Logcmd[sizeof(Logcmd)-1]=crc&0xff;
	comSendBuf(COM5,Logcmd,sizeof(Logcmd));
	delay_ms(200);
}

void	HDMI_Set_Error()						//触摸屏显示异常
{
	u8 Errorcmd[]={01,6,01,0x8F,00,00,01,00};
	u16 crc;
	Errorcmd[5]=Error;
	crc=mc_check_crc16(Errorcmd,6);
	Errorcmd[6]=crc>>8;
	Errorcmd[7]=crc&0xff;
	delay_ms(50);
	comSendBuf(COM5,Errorcmd,sizeof(Errorcmd));
	delay_ms(1000);
}

void	HDMI_Set_Button(u8 value)				//设置按键状态
{
	u8 Partitioncmd[]={01,15,00,00,00,8,01,00,0x7C,0x0E};
	u16 crc;
	Partitioncmd[7]=value;
	crc=mc_check_crc16(Partitioncmd,8);
	Partitioncmd[8]=crc>>8;
	Partitioncmd[9]=crc&0xff;
	delay_ms(50);
	comSendBuf(COM5,Partitioncmd,sizeof(Partitioncmd));
	delay_ms(1000);
}

void	HDMI_Set_Remaining_time(u16 time)		//设置触摸屏倒计时时间
{
	u8 Timecmd[]={01,06,00,01,00,00,0x7C,0x0E};
	u16 crc;
	Timecmd[4]=time>>8;
	Timecmd[5]=time&0xff;
	crc=mc_check_crc16(Timecmd,6);
	Timecmd[6]=crc>>8;
	Timecmd[7]=crc&0xff;
	comSendBuf(COM5,Timecmd,sizeof(Timecmd));
	delay_ms(100);
}

void	HDMI_Set_Partition(u8 value)			//设置触摸屏分区选择显示
{
	u8 Partitioncmd[]={01,15,00,8,00,8,01,00,0x7C,0x0E};
	u16 crc;
	Partitioncmd[7]=value;
	crc=mc_check_crc16(Partitioncmd,8);
	Partitioncmd[8]=crc>>8;
	Partitioncmd[9]=crc&0xff;
	comSendBuf(COM5,Partitioncmd,sizeof(Partitioncmd));
	delay_ms(1000);
}

void	HDMI_Set_Current_Partition(u8 value)	//设置当前分区
{
	u8 Currentcmd[]={01,06,00,00,00,00,0x7C,0x0E};
	u16 crc;
	Currentcmd[5]=value;
	crc=mc_check_crc16(Currentcmd,6);
	Currentcmd[6]=crc>>8;
	Currentcmd[7]=crc&0xff;
	comSendBuf(COM5,Currentcmd,sizeof(Currentcmd));
	delay_ms(100);
}

void	HDMI_SetSysTime()						//设置触摸屏时间
{
	u8 SysTimecmd[21]={01,16,00,03,00,06,12,0x7C,0x0E};
	u8 Stimecmd[]={01,05,00,16,0xFF,00,0x8D,0XFF};
	u16 crc;
	SysTimecmd[7]=STIME.year>>8;
	SysTimecmd[8]=STIME.year&0xff;
	SysTimecmd[9]=STIME.month>>8;
	SysTimecmd[10]=STIME.month&0xff;
	SysTimecmd[11]=STIME.day>>8;
	SysTimecmd[12]=STIME.day&0xff;
	SysTimecmd[13]=STIME.hour>>8;
	SysTimecmd[14]=STIME.hour&0xff;
	SysTimecmd[15]=STIME.minute>>8;
	SysTimecmd[16]=STIME.minute&0xff;
	SysTimecmd[17]=STIME.second>>8;
	SysTimecmd[18]=STIME.second&0xff;
	crc=mc_check_crc16(SysTimecmd,19);
	SysTimecmd[19]=crc>>8;
	SysTimecmd[20]=crc&0xff;
	comSendBuf(COM5,SysTimecmd,sizeof(SysTimecmd));
	delay_ms(100);
	
	if(TimePopup)return;

	comSendBuf(COM5,Stimecmd,sizeof(Stimecmd));
	delay_ms(100);
}

void	HDMI_Check_Acquisition_Frequency()		//读取传感器采集频率
{
	u8 Acquisitioncmd[]={01,03,00,02,00,01,0x25,0xCA};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Acquisitioncmd,sizeof(Acquisitioncmd));
	delay_ms(100);
	COM5GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		up_time=buf[3]<<8|buf[4];
}

void	HDMI_Check_Net(u16 CMD,char *data)		//读取网络参数
{
	u8 Netcmd[]={01,03,00,02,00,01,0x25,0xCA};
	u8 buf[100];
	u8 len;
	u16 crc;
	char hc[30];
	comClearRxFifo(COM5);
	Netcmd[2]=CMD>>8;
	Netcmd[3]=CMD&0xff;
	crc=mc_check_crc16(Netcmd,6);
	Netcmd[6]=crc>>8;
	Netcmd[7]=crc&0xff;
	comSendBuf(COM5,Netcmd,sizeof(Netcmd));
	delay_ms(100);
	len=COM5GetBuf(buf,100);
	crc=mc_check_crc16(buf,len-2);
	if((buf[len-2]==(crc>>8))&(buf[len-1]==(crc&0xff)))
	{
		buf[len-2]=0;
		sprintf(hc,(char *)buf+9);
		strcpy(data,hc);
	}
}

void	HDMI_Check_Button()						//读取按钮开关按下状态
{
	u8 Buttoncmd[]={01,01,00,00,00,16,0x3D,0xC6};
	u8 buf[100];
	u8 len;
	u16 crc;
	len=0;
	comClearRxFifo(COM5);
	do
	{
		comSendBuf(COM5,Buttoncmd,sizeof(Buttoncmd));
		delay_ms(200);
		len=COM5GetBuf(buf,7);
	}while(len<5);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		MCGS_Button=buf[3],MCGS_Partition=buf[4];
}

void	HDMI_Check_Irrigation_time()			//读取设置的灌溉时常
{
	u8 Irrigationcmd[]={01,04,00,00,00,01,0x31,0xCA};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Irrigationcmd,sizeof(Irrigationcmd));
	delay_ms(100);
	COM5GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		IrrTime=(buf[3]<<8)|buf[4];
}

void	HDMI_Check_SysTime()					//读取触摸屏系统时间
{
	u8 Syscmd[]={01,04,00,04,00,06,0x31,0xC9};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Syscmd,sizeof(Syscmd));
	delay_ms(100);
	COM5GetBuf(buf,18);
	crc=mc_check_crc16(buf,15);
	if((buf[15]==(crc>>8))&(buf[16]==(crc&0xff)))
	{
		STIME.year=buf[3]<<8|buf[4];
		STIME.month=buf[6];
		STIME.day=buf[8];
		STIME.hour=buf[10];
		STIME.minute=buf[12];
		STIME.second=buf[14];
	}
}

void	HDMI_Check_LogPage()					//读取日志页码
{
	u8 LogPagecmd[]={01,04,00,0X0A,00,01,0x11,0xC8};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,LogPagecmd,sizeof(LogPagecmd));
	delay_ms(100);
	COM5GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		LogPage=(buf[3]<<8)|buf[4];
}

void	HDMI_Check_Plan(u16 addr)				//读取触摸屏计划
{
	u8 Plancmd[]={01,03,00,04,00,9,0x31,0xC9};
	u8 buf[100];
	u16 crc;
	Plancmd[2]=addr>>8;
	Plancmd[3]=addr&0xff;
	crc=mc_check_crc16(Plancmd,6);
	Plancmd[6]=crc>>8;
	Plancmd[7]=crc&0xff;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Plancmd,sizeof(Plancmd));
	delay_ms(100);
	COM5GetBuf(buf,25);
	crc=mc_check_crc16(buf,21);
	if((buf[21]==(crc>>8))&(buf[22]==(crc&0xff)))
	{
		Plan.month=buf[3]<<8|buf[4];
		Plan.day=buf[6];
		Plan.hour=buf[8];
		Plan.minute=buf[10];
		Plan.IrrMode=buf[12];
		Plan.Partition=buf[14];
		Plan.Irrtime=(buf[15]<<8)|(buf[16]&0xff);
		Plan.State=buf[18];
	}
}
