#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "md5.h"
#include "UserCore.h"
#include "UserHmi.h"
#include "FlashDivide.h"
#include "Rtc.h"
#include "network.h"

extern float Use_A_Kg;
extern float Use_B_Kg;
extern float Use_C_Kg;
extern float Use_D_Kg;
extern u8 com_conect_state;//串口连接状态
extern u8 screen_refresh_flag;
extern u8 Openboard2;//开启板2的施肥合水泵
extern u16 amount_fer;//每亩施肥量
extern FlashSet flashset;//设置参数保存
void Copy_Folwer(void);
extern CtaskRecordunion ctask_record;//记录任务
extern Targetred ctargetrcord;//当前任务记录
extern WorkPara HmiWorkPara;
void task_resolve(u8*databuf,u32 data);

//鉴权信息
 char ProductKey0[20]=	"a1NgUW4yuwB";//"a1q04qjosGa";
 char DeviceName0[50]=	"zbzdtd";//"test_single";
 char DeviceSecret0[50]="uovlKb4qEBQuQvJWAtg1VilKbcksTizr";	//"78eJYG6R0Y9AlPcpA9s4KXAUd3rjcQDn";

 char *ProductKey=ProductKey0;
 char *DeviceName=DeviceName0;
 char *DeviceSecret=DeviceSecret0;
// 
 char ssid[31]="CU_Z4eg";
 char password[31]="z4egdkuj";
 char ipaddr[16]="";
 char subnet[16]="";
 char gateway[16]="";
 char dns[16]="";

Ring NETCircle;

OS_EVENT * RemoteQMsg;
void* 	RemoteMsgBlock[8];
OS_MEM* RemotePartitionPt;
u8 g_u8RemoteMsgMem[20][8];


//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 RemoteQInit(void)
{
	INT8U os_err;
	
	RemoteQMsg = OSQCreate ( RemoteMsgBlock, 8);
	
	if(RemoteQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	RemotePartitionPt = OSMemCreate (
										g_u8RemoteMsgMem,
				  						20,
				  						8,
				  						&os_err 
				  					);
	if(os_err != OS_ERR_NONE)
	{
		return 2;
	}
	return 0;
}
//消息打包发走
//输入：消息信息
//输出：0：发送成功
//		1：申请内存失败
//		2：发送消息失败

u8 PackSendRemoteQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(RemotePartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
//MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
//	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	os_err = OSQPost ( RemoteQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(RemotePartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveRemoteQ(MsgStruct * RemoteQ)
{
	u8 os_err;
	os_err = OSMemPut(RemotePartitionPt, ( void * )RemoteQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//void GM3_check_cmd(u8*str)
//GM3发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* EMW3060_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)NETCircle.buf,(const char*)str);
	return (u8*)strx;
}
//gm3发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 EMW3060_send_cmd(u8 *cmd,u8 *ack,u16 waittime, u8 flag)
{
	u8 res=0; 
	u8*p = NULL;
//	u8 uCnt = 0;

	if(flag == 1)//
	{
		u3_printf("%s\r\n",cmd);//发送命令
	}
	else if(flag == 0)
	{
		u3_printf("%s",cmd);//发送命令
	}
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{ 
			delay_ms(10);
			//串口收到新的数据，并判断是否有为有效数据
			COMGetBuf(COM1 ,NETCircle.buf, 256);
			{
				p = EMW3060_check_cmd(ack);
				if(p)
				{
					memset(&NETCircle, 0 , sizeof(NETCircle));
					//*p = 1;//破坏字符串
					break;//得到有效数据 
				}
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
s16 RevFigure(u8*src)
{
	u8 u8F = 0;
	s16 revalue=0;

	if(*src==0x2d)
	{
		u8F = 1;
		src++;
	}
	while((*src>=0x30)&&(*src<=0x39))
	{
		revalue = revalue*10+ (*src-0x30);
		src++;
	}
	if(u8F == 1)
	{
		revalue = revalue*(-1);
	}
	return revalue;
}
void DepackRevData(void)
{
	u8* p=NULL;
	u8 u8Cnt = 0,i=0;
	MsgStruct Msgtemp;
	u8 u8Str[7] = {0,0,0,0,0,0,0}; 
	U16Union WaterPar;
	_calendar_obj NetTime;
	//u16 RevNum=0;
	
	p=EMW3060_check_cmd("+MQTTRECV");
	if(p)
	{
		for(u8Cnt=0;u8Cnt<ParaNum;u8Cnt++)
		{
			p=EMW3060_check_cmd((u8*)SetText[u8Cnt]);
			
			if(p)
			{
				*p = 1;
				ReSet.Remote_Para[u8Cnt] = RevFigure(p+strlen((const char*)SetText[u8Cnt])+2);
			}
			if(u8Cnt == 15&&(p>0))
			{
				memcpy((u8*)&flashset.FlashPara[76],(u8*)ReSet.NetSet.PartitionEnable,32);
				FlashWritePara((u8*)flashset.FlashPara,92);
			}
			else if(u8Cnt == 35&&(p>0))
			{
				if(ReSet.NetSet.Switch==1)
				{
					if((ReSet.NetSet.IrrType == 1)&&(ReSet.NetSet.IrrTime>0))//清水灌溉
					{
						WaterPar.WordBit.Bit1 = ReSet.NetSet.partitionSel[0];
						WaterPar.WordBit.Bit2 = ReSet.NetSet.partitionSel[1];
						WaterPar.WordBit.Bit3 = ReSet.NetSet.partitionSel[2];
						WaterPar.WordBit.Bit4 = ReSet.NetSet.partitionSel[3];
						WaterPar.WordBit.Bit5 = ReSet.NetSet.partitionSel[4];
						WaterPar.WordBit.Bit6 = ReSet.NetSet.partitionSel[5];
						WaterPar.WordBit.Bit7 = ReSet.NetSet.partitionSel[6];
						WaterPar.WordBit.Bit8 = ReSet.NetSet.partitionSel[7];
						WaterPar.WordBit.Bit9 = ReSet.NetSet.partitionSel[8];
						WaterPar.WordBit.Bit10 = ReSet.NetSet.partitionSel[9];
						WaterPar.WordBit.Bit11 = ReSet.NetSet.partitionSel[10];
						WaterPar.WordBit.Bit12 = ReSet.NetSet.partitionSel[11];
						WaterPar.WordBit.Bit13 = ReSet.NetSet.partitionSel[12];
						WaterPar.WordBit.Bit14 = ReSet.NetSet.partitionSel[13];
						WaterPar.WordBit.Bit15 = ReSet.NetSet.partitionSel[14];
						WaterPar.WordBit.Bit16 = ReSet.NetSet.partitionSel[15];
						ctask_record.record.task_num++;
						if(ctask_record.record.task_num>100) //最多记录100条任务
						{
							ctask_record.record.task_num=1;
						}
						//		 ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;
						ctargetrcord.trecord.lasttarget_id=ctargetrcord.trecord.currenttarget_id=0;//初始化任务ID
						time_copy();//拷贝最新时间
						ctargetrcord.trecord.curtarget_mode=ctask_record.record.irrigated_mode=1; //清水灌溉		 
						ctargetrcord.trecord.curtarget_time=ctask_record.record.irrigated_time=ReSet.NetSet.IrrTime;//灌溉时长
						ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //保存分区
						ctask_record.record.fertilizing_amount=0;//清水灌溉不需要施肥
						ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time;
						task_resolve(ctargetrcord.trecord.targetnum,(u32)WaterPar.Word);
						ctargetrcord.trecord.Ctrl_update_state=1;//有需要更新的控件
						for(i=0;i<16;i++)
						{
							if((ReSet.NetSet.partitionSel[i])&0x01)
							{
								if((i+1)==ctargetrcord.trecord.targetnum[0])
								{
									SetTexFrontBColor(LCD_WATERSHOW_PAGE,i+1,GREEN_COLOR_HMI);//设置文本背景
									SetRemoteRead(i,2);
								}
								else					
								{
									SetTexFrontBColor(LCD_WATERSHOW_PAGE,i+1,ORANGE_COLOR_HMI);//设置文本背景
									SetRemoteRead(i,1);
								}
							}
							else
							{
								SetTexFrontBColor(LCD_WATERSHOW_PAGE,i+1,GRAY_COLOR_HMI);//设置文本背景
								SetRemoteRead(i,0);
							}
								
						}
						SetRemoteRead(16,1);
						NumberToASCII(u8Str,ctargetrcord.trecord.currenttargetlefttime);
						SetTextValue(LCD_WATERSHOW_PAGE,17,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_WATERSHOW_PAGE,18,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_WATERSHOW_PAGE,19,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_WATERSHOW_PAGE,20,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_WATERSHOW_PAGE,21,u8Str);	
						screen_refresh_flag=1;	 
						com_conect_state=1; 	 
						Msgtemp.CmdType = MSG_START; //发送消息告诉状态机启动任务
						Msgtemp.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
						Msgtemp.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
						PackSendMasterQ(&Msgtemp);
						SetScreen(LCD_WATERSHOW_PAGE);
					}
					else if((ReSet.NetSet.IrrType == 2)&&((ReSet.NetSet.IrrPioj>0)&&(ReSet.NetSet.IrrPioj<6)))
					{
						WaterPar.WordBit.Bit1 = ReSet.NetSet.partitionSel[0];
						WaterPar.WordBit.Bit2 = ReSet.NetSet.partitionSel[1];
						WaterPar.WordBit.Bit3 = ReSet.NetSet.partitionSel[2];
						WaterPar.WordBit.Bit4 = ReSet.NetSet.partitionSel[3];
						WaterPar.WordBit.Bit5 = ReSet.NetSet.partitionSel[4];
						WaterPar.WordBit.Bit6 = ReSet.NetSet.partitionSel[5];
						WaterPar.WordBit.Bit7 = ReSet.NetSet.partitionSel[6];
						WaterPar.WordBit.Bit8 = ReSet.NetSet.partitionSel[7];
						WaterPar.WordBit.Bit9 = ReSet.NetSet.partitionSel[8];
						WaterPar.WordBit.Bit10 = ReSet.NetSet.partitionSel[9];
						WaterPar.WordBit.Bit11 = ReSet.NetSet.partitionSel[10];
						WaterPar.WordBit.Bit12 = ReSet.NetSet.partitionSel[11];
						WaterPar.WordBit.Bit13 = ReSet.NetSet.partitionSel[12];
						WaterPar.WordBit.Bit14 = ReSet.NetSet.partitionSel[13];
						WaterPar.WordBit.Bit15 = ReSet.NetSet.partitionSel[14];
						WaterPar.WordBit.Bit16 = ReSet.NetSet.partitionSel[15];
						ctargetrcord.trecord.wateringprogram=ReSet.NetSet.IrrPioj;
						HmiWorkPara.Para.IrrPjNum = ReSet.NetSet.IrrPioj;
						HmiWorkPara.FerPartition = (u32)WaterPar.Word;
						Water_Program(ctargetrcord.trecord.wateringprogram);
						ctask_record.record.task_num++;
						if(ctask_record.record.task_num>100) //最多记录100条任务
						{
							ctask_record.record.task_num=1;
						}
				//		 ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;
						ctargetrcord.trecord.lasttarget_id=ctargetrcord.trecord.currenttarget_id=0;//初始化任务ID
						Copy_Folwer();//拷贝水流数据
						time_copy();//拷贝最新时间
						ctargetrcord.trecord.curtarget_mode=ctask_record.record.irrigated_mode=2; //肥水灌溉		 
						ctask_record.record.irrigated_time=ctargetrcord.trecord.curtarget_time;//灌溉时长
				//		 ctask_record.record.fertilizing_amount=0;//清水灌溉不需要施肥
						ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //保存分区
						if(flashset.FlashPara[ctask_record.record.select_zone+3]>0xf000)
							ctask_record.record.fertilizing_amount=65536- flashset.FlashPara[ctask_record.record.select_zone+3];
						else
							ctask_record.record.fertilizing_amount=flashset.FlashPara[ctask_record.record.select_zone+3]; 
						ctask_record.record.fertilizing_amount=amount_fer*ctask_record.record.fertilizing_amount;//总施肥量
						ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time;
						Openboard2=0;
						ctargetrcord.trecord.Ctrl_update_state=1;//有需要更新的控件
						for(i=0;i<16;i++)
						{
							if((ReSet.NetSet.partitionSel[i])&0x01)
							{
								if((i+1)==ctargetrcord.trecord.targetnum[0])
								{
									SetTexFrontBColor(LCD_FERSHOW_PAGE,i+1,GREEN_COLOR_HMI);//设置文本背景	
									SetRemoteRead(i,2);
								}
								else	
								{
									SetTexFrontBColor(LCD_FERSHOW_PAGE,i+1,ORANGE_COLOR_HMI);//设置文本背景
									SetRemoteRead(i,1);
								}
							}
							else
							{
								SetTexFrontBColor(LCD_FERSHOW_PAGE,i+1,GRAY_COLOR_HMI);//设置文本背景
								SetRemoteRead(i,0);
							}
						}
						SetRemoteRead(16,2);
						SetRemoteRead(18,ctargetrcord.trecord.wateringprogram);
						NumberToASCII(u8Str,ctargetrcord.trecord.currenttargetlefttime);
						SetTextValue(LCD_FERSHOW_PAGE,17,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_FERSHOW_PAGE,18,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_FERSHOW_PAGE,19,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_FERSHOW_PAGE,20,u8Str);
						NumberToASCII(u8Str,0);
						SetTextValue(LCD_FERSHOW_PAGE,21,u8Str);
						screen_refresh_flag=1;	
						com_conect_state=1; 			 
						Msgtemp.CmdType = MSG_START; //发送消息告诉状态机启动任务
						Msgtemp.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
						Msgtemp.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
						PackSendMasterQ(&Msgtemp);	
						Use_A_Kg=0;
						Use_B_Kg=0;
						Use_C_Kg=0;
						Use_D_Kg=0;
				
						SetScreen(LCD_FERSHOW_PAGE);
					}
				}
				else if(ReSet.NetSet.Switch==0)
				{
					//发送任务停止消息
					Msgtemp.CmdSrc = GPRS_TASK_CODE;
					Msgtemp.CmdType = MSG_PAGEEND;
					PackSendMasterQ(&Msgtemp);
				}
			}
		}
	}
	else
	{
		p==EMW3060_check_cmd("+SNTPTIME:");
		if(p)
		{
			NetTime.w_year= RevFigure(p+10);
			NetTime.w_month= RevFigure(p+15);
			NetTime.w_date= RevFigure(p+18);
			NetTime.hour= RevFigure(p+21);
			NetTime.min= RevFigure(p+24);
			NetTime.sec= RevFigure(p+27);
			RtcTimeupdate(NetTime);
		}
	}
}
char xxbuf[34];

char	*hmacmd5(char *data,u8 size_data,u8 size_ds)
{
	char buf[20];
	char i=0,L=0;
	char *str=buf;
	memset(buf,0,20);
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
	for(i=0;i<16;i++)
	{
		L = strlen(xxbuf);
		sprintf(xxbuf+L,"%02x",buf[i]);
	}
	str = xxbuf;
	return str;
}

void	Emw3060_init(void)
{
	u8 buf[250];
	delay_ms(1000);
	u8 DnchF=0;
	u8 *p;
	_calendar_obj NetTime;
	while(1)
	{
		u3_printf("AT+FACTORY\r");
		delay_ms(1000);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
		
	}
	while(1)
	{
		u3_printf("+++");
		delay_ms(500);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	FlashReadDHCP(&DnchF);
	if(DnchF==0)
	{
		while(1)
		{
			u3_printf("AT+WDHCP=OFF\r");
			delay_ms(50);
			COM1GetBuf(buf,200);
			if(strstr((const char *)buf,"OK")[0]=='+')
			break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
		
		while(1)
		{
			u3_printf("AT+WJAPIP=%s,%s,%s,%s\r",ipaddr,subnet,gateway,dns);
			delay_ms(50);
			COM1GetBuf(buf,200);
			if(strstr((const char *)buf,"OK")[0]=='+')
			break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
	}
	while(1)
	{
		u3_printf("AT+WJAP=%s,%s\r",ssid,password);
		delay_ms(2000);delay_ms(2000);delay_ms(2000);delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+WEVENT:STATION_UP")[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+SNTPCFG=+8\r");
		delay_ms(100);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')
			break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	while(1)
	{
		u3_printf("AT+SNTPTIME\r");
		delay_ms(100);
		COM1GetBuf(buf,200);
		p = (unsigned char *)strstr((const char *)buf,"+SNTPTIME:");
		if(p)
		{
			NetTime.w_year= RevFigure(p+10);
			NetTime.w_month= RevFigure(p+15);
			NetTime.w_date= RevFigure(p+18);
			NetTime.hour= RevFigure(p+21);
			NetTime.min= RevFigure(p+24);
			NetTime.sec= RevFigure(p+27);
			RTC_Set(NetTime.w_year,NetTime.w_month,NetTime.w_date,
					NetTime.hour,NetTime.min,NetTime.sec);
			break;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
}

void	Emw3060_con(void)
{
	u8 buf[250];
	char hc[100];
	u8 Flag=0;
	memset(hc,0,100);
	sprintf(hc,"clientId123deviceName%sproductKey%stimestamp789",DeviceName,ProductKey);
	while(1)
	{
//		printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,DeviceSecret);
		u3_printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,hmacmd5(hc,strlen(hc),strlen(DeviceSecret)));
		//u3_printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,"ff0facf561b96759dc44eea0c444100b");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTCID=123|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKey);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTKEEPALIVE=300\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTRECONN=ON\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTSTART\r");
		delay_ms(5000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+MQTTEVENT:CONNECT,SUCCESS")[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	while(1)
	{
		u3_printf("AT+MQTTSUB=?\r");
		delay_ms(1000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+MQTTSUB:0")[0]=='+')
		{
			Flag = 1;
			break;
		}
		else if(strstr((const char *)buf,"+MQTTSUB:1")[0]=='+')
		{
			//break;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	if(Flag == 1)
	{
		while(1)
		{
			//u3_printf("AT+MQTTSUB=1,/%s/%s/user/irr_set,1\r",ProductKey,DeviceName);
			u3_printf("AT+MQTTSUB=1,/sys/%s/%s/thing/service/property/set,1\r",ProductKey,DeviceName);
			delay_ms(2000);
			COM1GetBuf(buf,200);
			if(strstr((const char *)buf,"+MQTTEVENT:1,SUBSCRIBE,SUCCESS")[0]=='+')break;
		}
		comClearRxFifo(COM1);
		memset(buf,0,sizeof buf);
	}
	
	while(1)///a1q04qjosGa/test_single/user/update
	{
		u3_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		//u3_printf("AT+MQTTPUB=/a1q04qjosGa/test_single/user/update,1\r");
		delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
}

u8	sendEmw(char *data,unsigned char w)
{
	u8 buf[250];
	while(1)
	{
		if(!w)
			u3_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		else u3_printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
		delay_ms(100);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	while(1)
	{
		u3_printf("AT+MQTTSEND=%d\r",strlen(data));
		delay_ms(50);
		COM1GetBuf(buf,20);
		if(strstr((const char *)buf,">")[0]=='>')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	u3_printf("%s",data);
	
	delay_ms(2000);
	COM1GetBuf(buf,100);
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')return 1;
	else return 0;
}
void SplicingPubHead(u8* Dest)
{
	strcpy((char*)Dest,"{\"params\":{");
}
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum);
void SplicingPubData(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");
	}
	strcat((char*)Dest,"\"");
	strcat((char*)Dest,(const char*)Text);
	strcat((char*)Dest,"\":");
	SignedNumberToASCII(u8Str,Value,0);
	strcat((char*)Dest,(const char*)u8Str);
}
void SplicingPubData2(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");
	}
	strcat((char*)Dest,"\"");
	strcat((char*)Dest,(const char*)Text);
	strcat((char*)Dest,"\":");
	SignedNumberToASCII(u8Str,Value,1);
	strcat((char*)Dest,(const char*)u8Str);
}
void SplicingPubTail(u8* Dest)
{
	strcat((char*)Dest,"}}");
}

u8 NetSendBuf[512];
void  SplicingPubStr_PartitionEnable(void)//
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)ReSet.NetSet.PartitionEnable,(u8*)&flashset.FlashPara[76],32);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	for(u8Cnt=0;u8Cnt<16;u8Cnt++)
	{
		SplicingPubData(NetSendBuf,SetText[u8Cnt],ReSet.NetSet.PartitionEnable[u8Cnt],u8Cnt);
	}
	SplicingPubTail(NetSendBuf);
}
/*void  SplicingPubStr_PartitionEnable(void)//
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)ReSet.NetSet.PartitionEnable,(u8*)&flashset.FlashPara[76],32);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	for(u8Cnt=0;u8Cnt<16;u8Cnt++)
	{
		SplicingPubData(NetSendBuf,SetText[u8Cnt],ReSet.NetSet.PartitionEnable[u8Cnt],u8Cnt);
	}
	SplicingPubTail(NetSendBuf);
}*/
void  SplicingPubStr_IrrPara(void)//
{
	u8 u8Cnt=0;
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,SetText[16],ReSet.NetSet.IrrType,0);
	for(u8Cnt=17;u8Cnt<33;u8Cnt++)
	{
		SplicingPubData(NetSendBuf,SetText[u8Cnt],ReSet.NetSet.partitionSel[u8Cnt],u8Cnt);
	}
	SplicingPubData(NetSendBuf,SetText[33],ReSet.NetSet.IrrTime,0);
	SplicingPubData(NetSendBuf,SetText[34],ReSet.NetSet.IrrPioj,0);
	SplicingPubData(NetSendBuf,SetText[35],ReSet.NetSet.Switch,0);
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_Percent(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)ReRead.NetRead.BucketFerPer,(u8*)flashset.FlashPara,8);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	for(u8Cnt=0;u8Cnt<4;u8Cnt++)
	{
		SplicingPubData(NetSendBuf,ReadText[19+u8Cnt],ReRead.NetRead.BucketFerPer[u8Cnt],u8Cnt);
	}
	SplicingPubTail(NetSendBuf);
}

void SplicingPubStr_IrrState(void)
{
	u8 u8Cnt=0;
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	for(u8Cnt=0;u8Cnt<16;u8Cnt++)
	{
		SplicingPubData(NetSendBuf,ReadText[u8Cnt],ReRead.NetRead.PartitionState[u8Cnt],u8Cnt);
	}
	SplicingPubData(NetSendBuf,ReadText[16],ReRead.NetRead.DeviceState,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[17],ReRead.NetRead.TimeCountDown,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[18],ReRead.NetRead.IrrProj,u8Cnt);
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_PartitionAre(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)ReRead.NetRead.PartitionAre,(u8*)&flashset.FlashPara[4],32);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	for(u8Cnt=0;u8Cnt<16;u8Cnt++)
	{
		if(ReRead.NetRead.PartitionAre[u8Cnt]>0xf000)
		{
			ReRead.NetRead.PartitionAre[u8Cnt] = 65536-ReRead.NetRead.PartitionAre[u8Cnt];
		}
		SplicingPubData2(NetSendBuf,ReadText[u8Cnt+23],ReRead.NetRead.PartitionAre[u8Cnt],u8Cnt);
	}
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_CFIP(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)&ReRead.NetRead.CFIP,(u8*)&flashset.FlashPara[36],16);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[39],ReRead.NetRead.CFIP.pre_flushtime,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[40],ReRead.NetRead.CFIP.fer_irrtimr,1);
	SplicingPubData(NetSendBuf,ReadText[41],ReRead.NetRead.CFIP.post_flushtimr,1);
	SplicingPubData(NetSendBuf,ReadText[42],ReRead.NetRead.CFIP.AferPer,1);
	SplicingPubData(NetSendBuf,ReadText[43],ReRead.NetRead.CFIP.BferPer,1);
	SplicingPubData(NetSendBuf,ReadText[44],ReRead.NetRead.CFIP.CferPer,1);
	SplicingPubData(NetSendBuf,ReadText[45],ReRead.NetRead.CFIP.DferPer,1);
	SplicingPubData(NetSendBuf,ReadText[46],ReRead.NetRead.CFIP.are_fer,1);
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_S1IP(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)&ReRead.NetRead.S1IP,(u8*)&flashset.FlashPara[44],16);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[47],ReRead.NetRead.S1IP.pre_flushtime,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[48],ReRead.NetRead.S1IP.fer_irrtimr,1);
	SplicingPubData(NetSendBuf,ReadText[49],ReRead.NetRead.S1IP.post_flushtimr,1);
	SplicingPubData(NetSendBuf,ReadText[50],ReRead.NetRead.S1IP.AferPer,1);
	SplicingPubData(NetSendBuf,ReadText[51],ReRead.NetRead.S1IP.BferPer,1);
	SplicingPubData(NetSendBuf,ReadText[52],ReRead.NetRead.S1IP.CferPer,1);
	SplicingPubData(NetSendBuf,ReadText[53],ReRead.NetRead.S1IP.DferPer,1);
	SplicingPubData(NetSendBuf,ReadText[54],ReRead.NetRead.S1IP.are_fer,1);
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_S2IP(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)&ReRead.NetRead.S2IP,(u8*)&flashset.FlashPara[52],16);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[55],ReRead.NetRead.S2IP.pre_flushtime,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[56],ReRead.NetRead.S2IP.fer_irrtimr,1);
	SplicingPubData(NetSendBuf,ReadText[57],ReRead.NetRead.S2IP.post_flushtimr,1);
	SplicingPubData(NetSendBuf,ReadText[58],ReRead.NetRead.S2IP.AferPer,1);
	SplicingPubData(NetSendBuf,ReadText[59],ReRead.NetRead.S2IP.BferPer,1);
	SplicingPubData(NetSendBuf,ReadText[60],ReRead.NetRead.S2IP.CferPer,1);
	SplicingPubData(NetSendBuf,ReadText[61],ReRead.NetRead.S2IP.DferPer,1);
	SplicingPubData(NetSendBuf,ReadText[62],ReRead.NetRead.S2IP.are_fer,1);
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_S3IP(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)&ReRead.NetRead.S3IP,(u8*)&flashset.FlashPara[60],16);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[63],ReRead.NetRead.S3IP.pre_flushtime,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[64],ReRead.NetRead.S3IP.fer_irrtimr,1);
	SplicingPubData(NetSendBuf,ReadText[65],ReRead.NetRead.S3IP.post_flushtimr,1);
	SplicingPubData(NetSendBuf,ReadText[66],ReRead.NetRead.S3IP.AferPer,1);
	SplicingPubData(NetSendBuf,ReadText[67],ReRead.NetRead.S3IP.BferPer,1);
	SplicingPubData(NetSendBuf,ReadText[68],ReRead.NetRead.S3IP.CferPer,1);
	SplicingPubData(NetSendBuf,ReadText[69],ReRead.NetRead.S3IP.DferPer,1);
	SplicingPubData(NetSendBuf,ReadText[70],ReRead.NetRead.S3IP.are_fer,1);
	SplicingPubTail(NetSendBuf);
}
void SplicingPubStr_S4IP(void)
{
	u8 u8Cnt=0;
	FlashReadPara((u8*)flashset.FlashPara,0,92);
	memcpy((u8*)&ReRead.NetRead.S4IP,(u8*)&flashset.FlashPara[68],16);
	memset(NetSendBuf,0,512);
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[71],ReRead.NetRead.S4IP.pre_flushtime,u8Cnt);
	SplicingPubData(NetSendBuf,ReadText[72],ReRead.NetRead.S4IP.fer_irrtimr,1);
	SplicingPubData(NetSendBuf,ReadText[73],ReRead.NetRead.S4IP.post_flushtimr,1);
	SplicingPubData(NetSendBuf,ReadText[74],ReRead.NetRead.S4IP.AferPer,1);
	SplicingPubData(NetSendBuf,ReadText[75],ReRead.NetRead.S4IP.BferPer,1);
	SplicingPubData(NetSendBuf,ReadText[76],ReRead.NetRead.S4IP.CferPer,1);
	SplicingPubData(NetSendBuf,ReadText[77],ReRead.NetRead.S4IP.DferPer,1);
	SplicingPubData(NetSendBuf,ReadText[78],ReRead.NetRead.S4IP.are_fer,1);
	SplicingPubTail(NetSendBuf);
}


u8 g_u8SendStep=0;
void InitUp(void)
	{
		MsgStruct Msgtemp;		
		Msgtemp.CmdType = UPLOAD_PARTITIONARE;
		Msgtemp.CmdSrc = GPRS_TASK_CODE;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_PERCENT;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_CFIP;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_S1IP;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_S2IP;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_S3IP;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_S4IP;
		PackSendRemoteQ(&Msgtemp);
		
		Msgtemp.CmdType = UPLOAD_PARTITIONENABLE;
		PackSendRemoteQ(&Msgtemp);
	}

u8 WifiPara[126];//前31ssid 31 密码 16本地IP 16子网掩码 16网关 16DNS
void network_task(void *pdata)
{
	//u8 Com3RxBuf[10]={0};
//	u8 SendNum = 0;
	//u8 buf[250];
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	RemoteQInit();
	FlashReadWiFi(WifiPara);
	if((WifiPara[0]!=0xff)&(WifiPara[0]!=0))//如果flash中未写入，使用程序中固化的
	{
		memcpy(ssid,WifiPara,31);
		memcpy(password,&WifiPara[31],31);
		memcpy(ipaddr,&WifiPara[62],16);
		memcpy(subnet,&WifiPara[78],16);
		memcpy(gateway,&WifiPara[94],16);
		memcpy(dns,&WifiPara[110],16);
	}
	else
	{
		memcpy(WifiPara,ssid,31);
		memcpy(&WifiPara[31],password,31);
		memcpy(&WifiPara[62],ipaddr,16);
		memcpy(&WifiPara[78],subnet,16);
		memcpy(&WifiPara[94],gateway,16);
		memcpy(&WifiPara[110],dns,16);
		FlashWriteWiFi(WifiPara);
	}
	
	//IO_OutSet(6,1);
	delay_ms(1000);
	InitUp();
	Emw3060_init();
	Emw3060_con();
	while(1)
	{
		
		delay_ms(100);
		{
			if(g_u8WiFiRestart == 1)
			{
				InitUp();
				Emw3060_init();
				Emw3060_con();
				g_u8WiFiRestart = 0;
			}
			if(COMGetBuf(COM1 ,NETCircle.buf, 512) > 0)
			{
				//解析收到的数据
				DepackRevData();
			}
			else
			{
				if(g_u8SendStep == 0)//解析
				{
					pMsgBlk = ( MsgStruct *) OSQPend ( RemoteQMsg,
						10,
						&os_err );
						
					if(os_err == OS_ERR_NONE)
					{
						memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
						DepackReceiveRemoteQ(pMsgBlk);
						switch(Msgtemp.CmdType)
						{
							case UPLOAD_PARTITIONENABLE:
								SplicingPubStr_PartitionEnable();
								break;
							case UPLOAD_IRRPARA:
								SplicingPubStr_IrrPara();
								break;
							case UPLOAD_PERCENT:
								SplicingPubStr_Percent();
								break;
							case UPLOAD_IRRSTATE:
								SplicingPubStr_IrrState();
								break;
							case UPLOAD_PARTITIONARE:
								SplicingPubStr_PartitionAre();
								break;
							case UPLOAD_CFIP:
								SplicingPubStr_CFIP();
								break;
							case UPLOAD_S1IP:
								SplicingPubStr_S1IP();
								break;
							case UPLOAD_S2IP:
								SplicingPubStr_S2IP();
								break;
							case UPLOAD_S3IP:
								SplicingPubStr_S3IP();
								break;
							case UPLOAD_S4IP:
								SplicingPubStr_S4IP();
								break;
							case READTIME:
								u3_printf("AT+SNTPTIME\r");
								break;
							default:
								break;
						}
						//发送函数
						//SplicingPubStr();
						sendEmw((char*)NetSendBuf,0);
					}
				}
			}
		}
	}
}

