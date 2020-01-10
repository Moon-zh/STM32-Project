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

void	HDMI_Set_Log(u16	addr)				//显示日志
{
	u8 Logcmd[35]={01,16,01,0x91,00,13,26,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00};
	u16 crc;
	Logcmd[2]=addr>>8;
	Logcmd[3]=addr&0xff;
	
	Logcmd[8]=Log.HouseNum;
	Logcmd[10]=Log.StartTime[0];
	Logcmd[12]=Log.StartTime[1];
	Logcmd[14]=Log.StartTime[2];
	Logcmd[16]=Log.StartTime[3];
	Logcmd[18]=Log.StartTime[4];
	Logcmd[20]=Log.StartMode;
	Logcmd[22]=Log.StopTime[0];
	Logcmd[24]=Log.StopTime[1];
	Logcmd[26]=Log.StopTime[2];
	Logcmd[28]=Log.StopTime[3];
	Logcmd[30]=Log.StopTime[4];
	Logcmd[32]=Log.StopMode;

	crc=mc_check_crc16(Logcmd,sizeof(Logcmd)-2);
	Logcmd[sizeof(Logcmd)-2]=crc>>8;
	Logcmd[sizeof(Logcmd)-1]=crc&0xff;
	comSendBuf(COM5,Logcmd,sizeof(Logcmd));
	delay_ms(50);
}

void	HDMI_Set_LogDe()						//显示日志详情
{
	u8 Logcmd[21]={01,16,00,39,00,6,12,00,00,00,00,00,00,00,00,00,00,00,00,00,00};
	u16 crc;
	
	Logcmd[8]=Log.IrrMode;
	Logcmd[9]=Log.Irrtime>>8;
	Logcmd[10]=Log.Irrtime&0xff;
	Logcmd[12]=Log.UseWater;
	Logcmd[14]=Log.FerNum;
	Logcmd[16]=Log.SFerusage;
	Logcmd[18]=Log.PFerusage;

	crc=mc_check_crc16(Logcmd,sizeof(Logcmd)-2);
	Logcmd[sizeof(Logcmd)-2]=crc>>8;
	Logcmd[sizeof(Logcmd)-1]=crc&0xff;
	comSendBuf(COM5,Logcmd,sizeof(Logcmd));
	delay_ms(50);
}

void	HDMI_Set_HouseSet()						//显示温室设置
{
	u8 Fercmd[23]={01,16,00,10,00,07,14,00,00};
	u16 crc;
	
	Fercmd[8]=Run_SetMessage.Mu;
	Fercmd[10]=Run_SetMessage.AverageMu;
	Fercmd[12]=Run_SetMessage.SpecialFer;
	Fercmd[14]=Run_SetMessage.Pre;
	Fercmd[16]=Run_SetMessage.Behind;
	Fercmd[18]=Run_SetMessage.Terminaladdr;
	Fercmd[20]=Run_SetMessage.TerminalNum;

	crc=mc_check_crc16(Fercmd,sizeof(Fercmd)-2);
	Fercmd[sizeof(Fercmd)-2]=crc>>8;
	Fercmd[sizeof(Fercmd)-1]=crc&0xff;
	comSendBuf(COM5,Fercmd,sizeof(Fercmd));
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

void	HDMI_Set_IrrMode(u8 mode)				//设置触摸屏倒计时时间
{
	u8 Timecmd[]={01,06,00,01,00,00,0x7C,0x0E};
	u16 crc;
	Timecmd[5]=mode;
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
	u8 SysTimecmd[21]={01,16,00,50,00,06,12,0x1B,0x93};
	u8 Stimecmd[]={01,05,00,8,0xFF,00,0x0D,0XF8};
	u16 crc;
	comClearRxFifo(COM5);
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
	comClearRxFifo(COM5);
//	if(TimePopup)return;

	comSendBuf(COM5,Stimecmd,sizeof(Stimecmd));
	delay_ms(100);
}

void	HDMI_Set_Stage()						//显示触摸屏阶段
{
	u8 Stagecmd[]={01,06,00,05,00,00,0x7C,0x0E};
	u16 crc;
	Stagecmd[5]=stage;
	crc=mc_check_crc16(Stagecmd,6);
	Stagecmd[6]=crc>>8;
	Stagecmd[7]=crc&0xff;
	comSendBuf(COM5,Stagecmd,sizeof(Stagecmd));
	delay_ms(200);
	
	Stagecmd[3]=4;
	Stagecmd[5]=(int)(pr.f*10);
	crc=mc_check_crc16(Stagecmd,6);
	Stagecmd[6]=crc>>8;
	Stagecmd[7]=crc&0xff;
	comSendBuf(COM5,Stagecmd,sizeof(Stagecmd));
	delay_ms(200);
}

void	HDMI_Set_Water_Fer()					//显示触摸屏实际用水量，用肥量，倒计时
{
	u8 WF[15]={01,16,00,06,00,03,06,0x7C,0x0E};
	u16 crc;
	comClearTxFifo(COM5);
	WF[7]=Actual_Water>>8;
	WF[8]=Actual_Water&0xff;
	WF[9]=Actual_Fer>>8;
	WF[10]=Actual_Fer&0xff;
	WF[11]=Remaining>>8;
	WF[12]=Remaining&0xff;
	crc=mc_check_crc16(WF,sizeof(WF)-2);
	WF[sizeof(WF)-2]=crc>>8;
	WF[sizeof(WF)-1]=crc&0xff;
	comSendBuf(COM5,WF,sizeof(WF));
	delay_ms(1000);
}

void	HDMI_Set_Fer_Look()						//显示肥料桶设置
{
	u8 Look[23]={01,16,00,29,00,07,14,0,0x0E};
	u16 crc;
	for(crc=0;crc<7;crc++)
	{
		Look[8+crc*2]=Concentration[crc];
	}
	crc=mc_check_crc16(Look,21);
	Look[sizeof(Look)-2]=crc>>8;
	Look[sizeof(Look)-1]=crc&0xff;
	comSendBuf(COM5,Look,sizeof(Look));
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

void	HDMI_Read_SetHouseNum(u8 *Num)			//读取设置棚号
{
	u8 SetHouseNum[]={01,03,00,9,00,01,0x54,0x08};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,SetHouseNum,sizeof(SetHouseNum));
	delay_ms(100);
	COM5GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		*Num=buf[4];
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
	u8 Buttoncmd[]={01,01,00,00,00,0x08,0x3D,0xCC};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Buttoncmd,sizeof(Buttoncmd));
	delay_ms(50);
	COM5GetBuf(buf,6);
	crc=mc_check_crc16(buf,4);
	if((buf[4]==(crc>>8))&(buf[5]==(crc&0xff)))
		MCGS_Button=buf[3];
}

void	HDMI_Check_Check()						//读取自检按钮开关按下状态
{
	u8 Checkcmd[]={01,01,00,16,00,16,0x3C,0x03};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Checkcmd,sizeof(Checkcmd));
	delay_ms(100);
	COM5GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		//checkfer=buf[3];
		MCGS_Check2=buf[4];
}

void	HDMI_Check_Checkvalue()					//读取设置的自检值
{
	u8 Checkvalue[]={01,03,00,22,00,03,0xE4,0x0F};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Checkvalue,sizeof(Checkvalue));
	delay_ms(100);
	COM5GetBuf(buf,11);
	crc=mc_check_crc16(buf,9);
	if((buf[9]==(crc>>8))&(buf[10]==(crc&0xff)))
		checknum=(buf[3]<<8)|buf[4];
		checkflow=(buf[5]<<8)|buf[6];
		checkfer=(buf[7]<<8)|buf[8];
}

void	HDMI_Check_Irrigation_time()			//读取设置的灌溉时长
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

void	HDMI_Read_Greenhouse()					//读取设置的运行参数
{
	u8 Greenhousecmd[]={01,03,00,00,00,04,0x44,0x09};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Greenhousecmd,sizeof(Greenhousecmd));
	delay_ms(300);
	COM5GetBuf(buf,13);
	crc=mc_check_crc16(buf,11);
	if((buf[11]==(crc>>8))&(buf[12]==(crc&0xff)))
		{HouseNum=buf[4];MODEL=buf[6];Remaining=(buf[7]<<8)|buf[8];PublicFer=buf[10];}
}

void	HDMI_Read_HouseSet()					//读取设置的温室参数
{
	u8 HouseSetcmd[]={01,03,00,9,00,0x08,0x94,0x0E};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,HouseSetcmd,sizeof(HouseSetcmd));
	delay_ms(100);
	COM5GetBuf(buf,21);
	crc=mc_check_crc16(buf,19);
	if((buf[19]==(crc>>8))&(buf[20]==(crc&0xff)))
	{
		Run_SetMessage.Mu=buf[6];
		Run_SetMessage.AverageMu=buf[8];
		Run_SetMessage.SpecialFer=buf[10];
		Run_SetMessage.Pre=buf[12];
		Run_SetMessage.Behind=buf[14];
		Run_SetMessage.Terminaladdr=buf[16];
		Run_SetMessage.TerminalNum=buf[18];
		FlashWriteFer(&Run_SetMessage.Mu,buf[4]*10,10);
	}	
}

void	HDMI_Read_Fer()							//读取肥料桶的设置
{
	u8 Fercmd[]={01,01,00,48,00,0x08,0x3D,0xC3};
	u8 Fercmd1[]={01,03,00,29,00,07,0x94,0x0E};
	u8 buf[100];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Fercmd,sizeof(Fercmd));
	delay_ms(100);
	COM5GetBuf(buf,6);
	crc=mc_check_crc16(buf,4);
	if((buf[4]==(crc>>8))&(buf[5]==(crc&0xff)))
		MCGS_Fer=buf[3];
	
	comClearRxFifo(COM5);
	comSendBuf(COM5,Fercmd1,sizeof(Fercmd1));
	delay_ms(200);
	COM5GetBuf(buf,19);
	crc=mc_check_crc16(buf,17);
	if((buf[17]==(crc>>8))&(buf[18]==(crc&0xff)))
	{
		for(crc=0;crc<7;crc++)Concentration[crc]=buf[4+crc*2];
		FlashWriteFer0(Concentration);
	}
}

void	HDMI_Check_SysTime()					//读取触摸屏系统时间
{
	u8 Syscmd[]={01,04,00,04,00,05,0x71,0xC8};
	u8 buf[20];
	u16 crc;
	comClearRxFifo(COM5);
	comSendBuf(COM5,Syscmd,sizeof(Syscmd));
	delay_ms(100);
	COM5GetBuf(buf,15);
	crc=mc_check_crc16(buf,13);
	if((buf[13]==(crc>>8))&(buf[14]==(crc&0xff)))
	{
		STIME.year=buf[3]<<8|buf[4];
		STIME.month=buf[6];
		STIME.day=buf[8];
		STIME.hour=buf[10];
		STIME.minute=buf[12];
		//STIME.second=buf[14];
	}
}

void	HDMI_Check_LogPage()					//读取日志页码
{
	u8 LogPagecmd[]={01,04,00,00,00,01,0x31,0xCA};
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

void	HDMI_Check_LogDe()						//读取日志页码
{
	u8 LogPagecmd[]={01,04,00,01,00,01,0x60,0x0A};
	u8 buf[100];
	u16 crc;
	delay_ms(50);
	comClearRxFifo(COM5);
	comSendBuf(COM5,LogPagecmd,sizeof(LogPagecmd));
	delay_ms(100);
	COM5GetBuf(buf,7);
	crc=mc_check_crc16(buf,5);
	if((buf[5]==(crc>>8))&(buf[6]==(crc&0xff)))
		LogDe=(buf[3]<<8)|buf[4];
}
