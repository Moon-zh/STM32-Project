#include "Rs485.h"
#include <string.h>
#include "delay.h"
#include "sys.h"
#include "Emw3060.h"
#include "md5.h"
#include "UserCore.h"
#include "UserHmi.h"
#include "FlashDivide.h"

#include	"rtc.h"
#include	"IO_BSP.h"
#include	"cgq.h"
	/*
	三元组
	ProductKey:产品ID     水分一体机
  DeviceName：设备ID    23   24  25
	DeviceSecret：设备的秘钥（密码） 
	*/

//鉴权信息 
 char ProductKey0[20]=	"";//"a1q04qjosGa";
 char DeviceName0[50]=	"";//"test_single";
 char DeviceSecret0[50]="";	//"78eJYG6R0Y9AlPcpA9s4KXAUd3rjcQDn";

 char *ProductKey=ProductKey0;
 char *DeviceName=DeviceName0;
 char *DeviceSecret=DeviceSecret0;
 
 //wifi信息
 char ssid[30]="";  //wifi名称
 char password[30]=""; //wifi密码
 char ipword[30]="";//IP地址  "192.168.7.112";
 char zwword[30]="";//子网掩码  "255.255.255.0";
 char wgword[30]="";//网关地址  	"192.168.7.1";
 char ymword[30]="";//域名  "114.114.114.114";
 u8 DHCP=1;//设置DHCP的开关 (默认开启DHCP)
 u8 DHCP_cq=0;//更改DHCP后，重新启动   (1:为重新启动  0：为启动结束)
 
u8 csh_bz=0;//wifi模组的wifi链接或MQTT连接是否成功。 csh_bz=0，连接成功。csh_bz=1,没有连接成功

Ring NETCircle;//创建平台下发数据的缓冲数组
u8 fsbz_3060=0;//每一次接收启动或停止之后，都先上传一次现在的数据
u8 fsbz_3060_bf=0;//发送数据变量的备份，在开启之后马上停止的时候才使用

OS_EVENT  *RemoteQMsg; //信号量，声明一个OS_EVENT类型的指针指向生成的队列
void  *RemoteMsgBlock[8]; // 消息队列数组， 建立一个指向消息数组的指针和数组的大小，该指针数组必须申明为void类型
OS_MEM  *RemotePartitionPt;/*定义内存控制块指针，创建一个内存分区时，返回值就是它 */
u8 g_u8RemoteMsgMem[20][8]; ///*划分一个具有20个内存块，每个内存块长度是8的内存分区 */


//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
//    QSQPost为先进先出函数(FIFO)，QSQPostFornt为后进先出函数(LIFO)
u8 RemoteQInit(void)
{
	INT8U os_err;
	
	RemoteQMsg = OSQCreate ( RemoteMsgBlock, 8);//调用OSQcreate()函数创建消息队列
	
	if(RemoteQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	RemotePartitionPt = OSMemCreate (   ///*创建动态内存区  *//*划分一个具有20个内存块，每个内存块长度是8的内存分区 */
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
//   MsgStruct是8个变量的结构体
u8 PackSendRemoteQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct  *MsgTemp = NULL;//定义结构体指针
	MsgTemp = (MsgStruct *)OSMemGet(RemotePartitionPt,&os_err);//获取存储块，内存申请
	if(MsgTemp == NULL)
	{
		return 1;
	}
	
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	os_err = OSQPost ( RemoteQMsg,(void*)MsgTemp ); //向消息队列发送一则消息(FIFO)
	
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(RemotePartitionPt, ( void * )MsgTemp);//释放内存
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		 1：释放失败
u8 DepackReceiveRemoteQ(MsgStruct * RemoteQ)
{
	u8 os_err;
	os_err = OSMemPut(RemotePartitionPt, ( void * )RemoteQ);//释放内存
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//从消息队列读取数据
//输入：收到消息队列中的数据，存储的地方
//输出： 0：读取成功
//       1：读取失败
u8 PendReceiveRemoteQ(MsgStruct *Pend1RemoteQ)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;

	pMsgBlk = ( MsgStruct *) OSQPend ( RemoteQMsg,
		10,
		&os_err );
	// //请求消息队列，参数分别是：RemoteQMsg为所请求消息队列的指针  第二个参数为等待时间
		 //0表示无限等待，&err为错误信息，返回值为队列控制块RemoteQMsg成员OSQOut指向的消息(如果队列中有消息可用的话)，如果
		//没有消息可用，在使调用OSQPend的任务挂起，使之处于等待状态，并引发一次任务调度
		
	if(os_err == OS_ERR_NONE)
	{
		memcpy(Pend1RemoteQ,(u8*)pMsgBlk,sizeof(MsgStruct) );//从源pMsgBlk拷贝n个字节到目Msgtemp中
		DepackReceiveRemoteQ(pMsgBlk);//队列消息解析，消息内存释放
		
		return 0;
	}	
	return 1;
}



s16 Remote_Para[4];
s16 Remote_Read[4];
void SetRemoteRead(u8 addr,s16 value)
{
	Remote_Read[addr] = value;
}
void SetRemotePara(u8 addr,s16 value)
{
	Remote_Para[addr] = value;
}

//向平台发送数据的格式，格式开头的字符串
const u8 TypeStr[8]="to_type\0";
const u8 TimeStr[8]="to_time\0";
const u8 PartitionStr[15]="need_partition\0";
const u8 SwitchStr[7]="switch\0";
const u8 WorkStateStr[11]="work_state\0";
const u8 CountDownStr[8]="in_time\0";
const u8 At_Partition[13]="at_partition\0";
u8 CCIDstr[21];
const u8* SetText[] = 
{
	TypeStr,
	TimeStr,
	PartitionStr,
	SwitchStr
};
const u8* ReadText[] = 
{
	TypeStr,
	TimeStr,
	PartitionStr,
	SwitchStr,
	WorkStateStr,
	CountDownStr,
	At_Partition,
	CCIDstr
};

/*
把字符串中的数字，取出并组合成数值
*/
s16 RevFigure(u8*src)
{
	u8 u8F = 0;
	s16 revalue=0;

	if(*src==0x2d)//负号
	{
		u8F = 1;
		src++;
	}
	
	while((*src>=0x30)&&(*src<=0x39))//数字0-9
	{
		revalue = revalue*10+ (*src-0x30);
		src++;
	}
	
	if(u8F == 1)//把数据改成负数
	{
		revalue = revalue*(-1);
	}
	return revalue;
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
/*
解析平台下发的数据
{"to_type":1,"to_time":25,"need_partition":3,"switch":1}

SetText[0]→const u8 TypeStr[8]="to_type\0";
SetText[1]→const u8 TimeStr[8]="to_time\0";
SetText[2]→const u8 PartitionStr[15]="need_partition\0";
SetText[3]→const u8 SwitchStr[7]="switch\0";
工作状态（work_state） 空闲-0，清水-1，肥水-2 
灌溉类型(to_type)    默认-0，清水-1，肥水-2
灌溉时常（to_time）  时间常数
需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
倒计时（in_time）
当前灌溉分区（at_partition）  1分区-1,2分区-2
启停(switch)    停止-0，启动-1
*/
s16 Remote_Para_hc[4];//Remote_Para_hc数组是Remote_Para的缓冲数组
void DepackRevData(void)
{
	u8* p=NULL;
	u8 u8Cnt = 0;
	MsgStruct Msgtemp;
	
	p=EMW3060_check_cmd("MQTTRECV:");
	if(p)
	{     //   EmwLED1=!EmwLED1;//调试
		for(u8Cnt=0;u8Cnt<ParaNum;u8Cnt++)
		{
			p=EMW3060_check_cmd((u8*)SetText[u8Cnt]);
			
			if(p)
			{
				Remote_Para_hc[u8Cnt] = RevFigure(p+strlen((const char*)SetText[u8Cnt])+2);
			}
		}
		
		if(Remote_Para_hc[3]==1&&MasterState==0)//启动且主程序运行状态为空闲
		{
			delay_ms(1000);
			if(MasterState==0)//延时1秒之后再判断一下是不是出于空闲状态
			{
				//先停止之前的灌溉类型和分区，在进行新的灌溉类型和分区		
				if(Guangai.zdms==1)//假如现在在自动模式下，清除自动模式的参数
				{
						Guangai.zdms=0;//关闭自动模式
						Guangai.qdbz=0;//自动模式关闭浇水
						Guangai.yichi=0;//标志清除						
				}
				//发送任务停止消息
				Msgtemp.CmdSrc = SCREEN_TASK_CODE;//触摸屏任务
				Msgtemp.CmdType = MSG_STOP;  //结束	
				PackSendMasterQ(&Msgtemp);//消息队列，消息打包发走
				while(MasterState!=0)delay_ms(10);//主任务运行状态为空闲跳出
				IO_OutSet(1,0);//关闭电磁阀-肥水
				IO_OutSet(2,0);//关闭电磁阀-第一分区清水
				IO_OutSet(3,0);//关闭电磁阀-第二分区清水

				Remote_Para[0]=Remote_Para_hc[0];//灌溉类型(to_type)    默认-0，清水-1，肥水-2
				Remote_Para[1]=Remote_Para_hc[1];//灌溉时常（to_time）  时间常数			
				Remote_Para[2]=Remote_Para_hc[2];//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3			
				Remote_Para[3]=Remote_Para_hc[3];	//启动-1 停止-0		
						
				if(Remote_Para[0]==1)//灌溉类型-清水
				{
					if((Remote_Para[1]==0)||(Remote_Para[1]>1440))//灌溉时常
					{
						return;
					}
				}
				else	//if(Remote_Para[0]==2)//灌溉类型-肥水
				{
					if((Remote_Para[1]<=20)||(Remote_Para[1]>1440))//灌溉时常
					{
						return;
					}			
				}

				Msgtemp.CmdSrc = SCREEN_TASK_CODE;   //触摸屏任务
				Msgtemp.CmdType = MSG_START;   //启动
				Msgtemp.CmdData[0] = Remote_Para[0];//灌溉类型(to_type)    默认-0，清水-1，肥水-2
				Msgtemp.CmdData[1] = Remote_Para[2];//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
				Msgtemp.CmdData[2] = Remote_Para[1]/60;//灌溉时常（小时）
				Msgtemp.CmdData[3] = Remote_Para[1]%60;//灌溉时常（分钟）
				PackSendMasterQ(&Msgtemp);//消息队列，消息打包发走
				SetScreen(LCD_STATESHOW_PAGE);//切换界面
			
				Remote_Read[3]+=2;//调试
				
			}
		}
		else if(Remote_Para_hc[3]==0)//停止
		{
			if(Guangai.zdms==1)//假如现在在自动模式下，清除自动模式的参数
			{
					Guangai.zdms=0;//关闭自动模式
					Guangai.qdbz=0;//自动模式关闭浇水
					Guangai.yichi=0;//标志清除						
			}
			
			//发送任务停止消息
			Msgtemp.CmdSrc = SCREEN_TASK_CODE;//触摸屏任务
			Msgtemp.CmdType = MSG_STOP;  //结束
			//Msgtemp.CmdData[0] = 1;//灌溉类型(to_type)    默认-0，清水-1，肥水-2  平台判断的不能为0
			Msgtemp.CmdData[1] = 0;//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
			Msgtemp.CmdData[2] = 0;//灌溉时常（小时）
			Msgtemp.CmdData[3] = 0;//灌溉时常（分钟）		
			PackSendMasterQ(&Msgtemp);//消息队列，消息打包发走
			
	
			Remote_Read[3]+=1;//调试
			
		}
		
		
	}
		comClearRxFifo(COM1);	
//		memset(g_RxBuf1,0,UART1_RX_BUF_SIZE);//清空缓冲数组g_RxBuf1
	  memset(NETCircle.buf,0,510);//清空缓冲数组NETCircle.buf
}



char xxbuf[34];

char	*hmacmd5_2(char *data,u8 size_data,u8 size_ds)
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

void	md5tohex(char *data,char *str)	//分解字节中包含的两个字符串值
{
	u8 i;
	for(i=0;i<16;i++)sprintf(str+i*2,"%02x",data[i]);
}

void	hmacmd5(char *data,u8 size_data,u8 size_ds,char *str)	//hmacmd5计算
{
	char buf[33];
	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
	md5tohex(str,buf);
	sprintf(str,"%s",buf);
}

char	*hmacmd5_5(char *data,u8 size_data,u8 size_ds)
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
//char	*hmacmd5(char *data,u8 size_data,u8 size_ds)
//{
//	char buf[20];
//	char i=0,L=0;
//	char *str=buf;
//	memset(buf,0,20);
//	hmac_md5((unsigned char*)data,size_data,(unsigned char*)DeviceSecret,size_ds,(unsigned char*)str);
//	for(i=0;i<16;i++)
//	{
//		L = strlen(xxbuf);
//		sprintf(xxbuf+L,"%02x",buf[i]);
//	}
//	str = xxbuf;
//	return str;
//}

void	Emw3060_LED_Init(void)				//EMWLED初始化
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;	 //LED0-->PB.5 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	
	
}

//8421码（BCD码）转十进制数
unsigned char bcd_to_hex(unsigned char data)
{
	unsigned	char temp;
	temp=((data>>4)*10+(data&0x0f));
	return temp;
}

//十进制数转8421码(BCD码)
unsigned	char hex_to_bcd(unsigned	char data)
{
	unsigned	char temp;
	temp=(((data/10)<<4)+(data%10));
	return temp;
}


u8 tiaoshi_cs_1[50];
//EMW3060初始化时使用，读取NTP时间
void dq_ntp_cx()
{
	
	u8 buf[250];
	char *cs_1;
	u8 cs[50];
	u8 i,a=0;
	u8 error=0;//超时处理标志
	_calendar_obj calendar2;	//日历结构体
	
	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);

	error=0;
	while(1)	//获取NTP时间
	{
		u1_printf("AT+SNTPTIME\r");
		delay_ms(5000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"SNTPTIME:20")[0]=='S')break;
		delay_ms(500);
		comClearRxFifo(COM1);
		delay_ms(500);
		if(++error==10)//超时处理
		{
			a=1;
			break;
		}
	}
	
	if(a==1)//读取网络时间超时
	{
		error=0;
		while(1)//获取触摸屏RTC时钟
		{
			hmi_cmp_rtc[0]=0;
			ReadRTC();//读取屏幕RTC时间
			delay_ms(2000);
			if(hmi_cmp_rtc[0]!=0)break;
			if(++error==5)//超时处理
			{
				csh_bz=1;
				return;
			}
		}		
	}

	if(a==0)//读取网络时间成功
	{
			cs_1=(char *)strchr((const char *)buf,'2'); 
			//2019-06-12  T  16:09:09.030464
			//0123456789  10 11 12  
			
			for(i=0;i<=18;i++)
			{
				cs[i]=cs_1[i]-0x30;
				
				tiaoshi_cs_1[i]=cs[i];//调试
			}
			
			//年
			calendar2.w_year=(cs[0]*1000)+(cs[1]*100)+(cs[2]*10)+cs[3];       
			//月
			calendar2.w_month=(cs[5]*10)+cs[6];  			
			//日期
			calendar2.w_date=(cs[8]*10)+cs[9];        
			 //小时
			calendar2.hour=(cs[11]*10)+cs[12];  
			 //分钟
			calendar2.min=(cs[14]*10)+cs[15];        
			 //秒
			calendar2.sec=(cs[17]*10)+cs[18];	

      //sec:秒 min：分 hour：时 day：日 week：星期 mon：月 year:年
			//设置触摸屏RTC时钟
			SetRTC(
			hex_to_bcd(calendar2.sec),	//秒
			hex_to_bcd(calendar2.min),  //分
			hex_to_bcd(calendar2.hour),  //时
			hex_to_bcd(calendar2.w_date),   //日
			hex_to_bcd(RTC_Get_Week((cs[2]*10)+cs[3],calendar2.w_month,calendar2.w_date)),		//星期
			hex_to_bcd(calendar2.w_month),   //月
			hex_to_bcd((cs[2]*10)+cs[3])	//年			
			);
			
	}
	else	if(a==1)//读取触摸屏时间成功
	{
//					hmi_cmp_rtc[0]=Hmi_cmd_buffer[2];//年
//					hmi_cmp_rtc[1]=Hmi_cmd_buffer[3];//月
//					hmi_cmp_rtc[2]=Hmi_cmd_buffer[4];//星期
//					hmi_cmp_rtc[3]=Hmi_cmd_buffer[5];//日
//					hmi_cmp_rtc[4]=Hmi_cmd_buffer[6];//小时
//					hmi_cmp_rtc[5]=Hmi_cmd_buffer[7];//分
//					hmi_cmp_rtc[6]=Hmi_cmd_buffer[8];//秒

			//年
			calendar2.w_year=2000+bcd_to_hex(hmi_cmp_rtc[0]);       
			//月
			calendar2.w_month=bcd_to_hex(hmi_cmp_rtc[1]);  			
			//日期
			calendar2.w_date=bcd_to_hex(hmi_cmp_rtc[3]);         
			 //小时
			calendar2.hour=bcd_to_hex(hmi_cmp_rtc[4]); 
			 //分钟
			calendar2.min=bcd_to_hex(hmi_cmp_rtc[5]);        
			 //秒
			calendar2.sec=bcd_to_hex(hmi_cmp_rtc[6]);				
	}



	RTC_Set(calendar2.w_year,
	calendar2.w_month,
	calendar2.w_date,
	calendar2.hour,
	calendar2.min,
	calendar2.sec);  //设置时间
	
	i=calendar2.w_date;
	FlashWriteSJ(&i);//把日期写入写入flash备份区域
}


//extern _calendar_obj calendar;	//日历结构体
void rct_dueishi_cx()//RCT对时程序  查询网络时间，并将时间同步至RTC，每24小时同步一次
{
	u8 sj;//之前日期
	u8 dqsj;//当前日期
	
	u8 buf[250];
	char *cs_1;
	u8 cs[50];
	u8 i;
	u8 error=0;//超时处理标志
	_calendar_obj calendar2;	//日历结构体
	
	
	FlashReadSJ(&sj);//读取flash存储的之前日期
	
	tiaoshi_cs_1[30]=RTC_Get();//更新时间	calendar.w_date
	
	dqsj=calendar.w_date;//当前日期
	tiaoshi_cs_1[31]=dqsj;
//		dqsj=calendar.min;
	tiaoshi_cs_1[28]=sj;
	
	//现在实时的RTC时钟
	tiaoshi_cs_1[32]=(calendar.w_year/1000);
	tiaoshi_cs_1[33]=(calendar.w_year/100%10);
	tiaoshi_cs_1[34]=(calendar.w_year/10%10);
	tiaoshi_cs_1[35]=(calendar.w_year%10);
	tiaoshi_cs_1[36]=calendar.w_month;
	tiaoshi_cs_1[37]=calendar.w_date;	
	tiaoshi_cs_1[38]=calendar.hour;
	tiaoshi_cs_1[39]=calendar.min;
	tiaoshi_cs_1[40]=calendar.sec;

	
	delay_ms(500);
	if(dqsj!=sj)//日期改变，查询网络时间，并将时间同步至RTC，每24小时同步一次
	{	
			comClearRxFifo(COM1);
			memset(buf,0,sizeof buf);
		
			error=0;
			while(1)	//获取NTP时间
			{
				u1_printf("AT+SNTPTIME\r");
				delay_ms(5000);
				COM1GetBuf(buf,200);
				if(strstr((const char *)buf,"SNTPTIME:20")[0]=='S')break;
				delay_ms(500);
				comClearRxFifo(COM1);
	    	delay_ms(500);
				if(++error==10)
				{
					return;
				}
				
			}
			
			cs_1=(char *)strchr((const char *)buf,'2');   
			//2019-06-12  T  16:09:09.030464
			//0123456789  10 11 12                                 
			
			for(i=0;i<=18;i++)
			{
				cs[i]=cs_1[i]-0x30;
				
				tiaoshi_cs_1[i]=cs[i];//调试
			}
	
			//年
			calendar2.w_year=(cs[0]*1000)+(cs[1]*100)+(cs[2]*10)+cs[3];       
			//月
			calendar2.w_month=(cs[5]*10)+cs[6];  			
			//日期
			calendar2.w_date=(cs[8]*10)+cs[9];        
			 //小时
			calendar2.hour=(cs[11]*10)+cs[12];  
			 //分钟
			calendar2.min=(cs[14]*10)+cs[15];        
			 //秒
			calendar2.sec=(cs[17]*10)+cs[18];
 			
      //设置时间
			RTC_Set(calendar2.w_year,
			calendar2.w_month,
			calendar2.w_date,
			calendar2.hour,
			calendar2.min,
			calendar2.sec);  //设置时间


      //sec:秒 min：分 hour：时 day：日 week：星期 mon：月 year:年
			//设置触摸屏RTC时钟
			SetRTC(
			hex_to_bcd(calendar2.sec),	//秒
			hex_to_bcd(calendar2.min),  //分
			hex_to_bcd(calendar2.hour),  //时
			hex_to_bcd(calendar2.w_date),   //日
			hex_to_bcd(RTC_Get_Week((cs[2]*10)+cs[3],calendar2.w_month,calendar2.w_date)),		//星期
			hex_to_bcd(calendar2.w_month),   //月
			hex_to_bcd((cs[2]*10)+cs[3])	//年			
			);
			
			
			i=calendar2.w_date;
			tiaoshi_cs_1[29]=i;
			FlashWriteSJ(&i);//把日期写入写入flash备份区域
			
	}
}


u8 tiaoshi_3060=0;//调试时使用的变量，查看当前运行到哪个阶级
void	Emw3060_init(void)  	//EMW初始化
{
	u8 buf[250];
//	u8 error=0;//超时处理标志
	
	csh_bz=0;//假设初始化成功
	
	Emw3060_LED_Init();//模板3060板载LED初始化驱动
	
	
	EmwLED2=0;//调试使用（点亮D4，1为点亮LED灯）
	
	while(1)//AT模式
	{
		u1_printf("+++"); //AT指令模式
		delay_ms(200);
		COM1GetBuf(buf,10);
		if(strchr((const char *)buf,'+')[0]=='+')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);//将buf中当前位置后面的buf个字节 用 0 替换并返回 s 	
tiaoshi_3060=1;/*调试*/
	
	while(1)//恢复模块出厂设置
	{
		u1_printf("AT+FACTORY\r");//恢复模块出厂设置
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"OK")[0]=='O')break;	
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=2;/*调试*/
	
	if(DHCP==0)//关闭DHCP
	{
			while(1)//关闭 DHCP 
			{
				u1_printf("AT+WDHCP=OFF\r");//关闭 DHCP 功能
				delay_ms(500);
				COM1GetBuf(buf,30);
				if(strchr((const char *)buf,'O')[0]=='O')break;
			}
			comClearRxFifo(COM1);
			memset(buf,0,sizeof buf);
tiaoshi_3060=3;/*调试*/
			
			while(1)//设置网络参数
			{
				u1_printf("AT+WJAPIP=%s,%s,%s,%s\r",ipword,zwword,wgword,ymword);  //设置 Station 模式时的 IP 地址。（IP,子网掩码，网关地址，域名dns）
				delay_ms(500);
				COM1GetBuf(buf,100);
				if(strchr((const char *)buf,'O')[0]=='O')break;
			}
			comClearRxFifo(COM1);
			memset(buf,0,sizeof buf);		
tiaoshi_3060=4;/*调试*/
	}

//	u1_printf("AT+WJAP=%s,%s\r",ssid,password);//wifi名称+wifi密码   设置在Station 模式，要接入的 AP 的名称和密码。
//	delay_ms(2000);
//	u1_printf("AT+WJAP=%s,%s\r",ssid,password);//wifi名称+wifi密码   设置在Station 模式，要接入的 AP 的名称和密码。	
	while(1)//设置WIFI
	{
		u1_printf("AT+WJAP=%s,%s\r",ssid,password);//wifi名称+wifi密码   设置在Station 模式，要接入的 AP 的名称和密码。
		delay_ms(2000);delay_ms(2000);delay_ms(2000);delay_ms(2000);delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"STATION_UP")[0]=='S')break;
		comClearRxFifo(COM1);
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	
tiaoshi_3060=5;/*调试*/
	
	while(1)	//设置北京时间时区
	{
		u1_printf("AT+SNTPCFG=+8\r");
		delay_ms(500);
		COM1GetBuf(buf,30);
		if(strchr((const char *)buf,'O')[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=6;/*调试*/

//	error=0;
//	while(1)	//校准网络时间    +SNTPTIME:2019-06-18T09:00:29.001392
//	{
//		u1_printf("AT+SNTPTIME\r");	
//		delay_ms(3000);
//		COM1GetBuf(buf,100);
//		if(strstr((const char *)buf,"SNTPTIME:20")[0]=='S')break;
//		delay_ms(500);
//		comClearRxFifo(COM1);
//		delay_ms(500);
//		if(++error==10)//超时处理
//		{
//			csh_bz=1;
//			return;
//		}
//	}
//	comClearRxFifo(COM1);
//	memset(buf,0,sizeof buf);	
//tiaoshi_3060=7;/*调试*/
	
	//读取NTP时间，保存日期。
	dq_ntp_cx();
tiaoshi_3060=8;/*调试*/
	
	delay_ms(500);
}

//MQTT协议：遗嘱  订阅  发布  鉴权
void	Emw3060_con(void)  	//EMW连接阿里云  （后期修改zq）
{
	u8 buf[250];
	char hc[100];
	char cs[40];
	char mac[33];
	char *m;
	u8 error=0;
	
	memset(hc,0,100);
	
	while(1)	//读取MAC地址
	{
		u1_printf("AT+WMAC?\r");
		delay_ms(200);
		COM1GetBuf(buf,100);
		if(strstr((const char *)buf,"+WMAC:")[0]=='+')break;
	}
	m=strstr((const char *)buf,"+WMAC:")+6;  //+6:表示指针地址加六
	m[12]=0;//0表示字符串为空NULL,所有的字符串操作都是判断NULL（空，0）为字符串截止。
	memset(mac,0,sizeof mac);
	sprintf(mac,"%s",m);
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=9;/*调试*/
	
	
	/*
	应用层：MQTT支持客户标识、用户名和密码认证；
	客户标识（Client Identifier）一般来说可以使用嵌入式芯片的MAC地址或者芯片序列号。
	MQTT协议支持通过CONNECT消息的username和password字段发送用户名和密码。
	
	*/
	sprintf(hc,"clientId%sdeviceName%sproductKey%stimestamp789",mac,DeviceName,ProductKey); //格式化输入
	while(1)	//写入设备鉴权信息    设置MQTT用户授权信息
	{
		hmacmd5(hc,strlen(hc),strlen(DeviceSecret),cs);//进行MD5签名算法
		u1_printf("AT+MQTTAUTH=%s&%s,%s\r",DeviceName,ProductKey,cs);
		delay_ms(200);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);	
tiaoshi_3060=10;/*调试*/	
	
	/*
	设置客户端ID
	securemode=3：模式3
	signmethod=hmacmd5：校验方式
	timestamp=789：时间戳
	*/
	while(1)//设置设备CID  设置MQTT客户端标识符。
	{
		u1_printf("AT+MQTTCID=%s|securemode=3\\,signmethod=hmacmd5\\,timestamp=789|\r",mac);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=11;/*调试*/		
	
	/*
	连接TCP服务器，   ProductKey:产品ID    
	1883：服务器端口号
	连接域名
	*/
	while(1)//设置TCP目标IP地址   设置MQTT socket信息  
	{
		u1_printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,1883\r",ProductKey);
//		u1_printf("AT+MQTTSOCK=%s.iot-as-mqtt.cn-shanghai.aliyuncs.com,443\r",ProductKey);
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=12;/*调试*/
	
	/*
	每300S向服务器发送心跳（简短的指令）
	*/
	while(1) //保活时间设置   设置MQTT心跳周期。 
	{
		u1_printf("AT+MQTTKEEPALIVE=300\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=13;/*调试*/	
	
	/*
	客户端MQTT自动重连打开。
	*/
	while(1)//MQTT自动重连打开   设置MQTT自动重连使能状态
	{
		u1_printf("AT+MQTTRECONN=ON\r");
		delay_ms(500);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=14;/*调试*/
	
	error=0;
	while(1)//MQTT连接   启动MQTT服务
	{
		u1_printf("AT+MQTTSTART\r");
		delay_ms(5000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"MQTTEVENT:CONNECT,SUCCESS")[0]=='M')break;
		if(++error==10)//MQTT链接超时处理
		{
			csh_bz=1;
			return;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
tiaoshi_3060=15;/*调试*/	
	

//	while(1)//订阅TOPIC  设置MQTT订阅信息
//	{
//		u3_printf("AT+MQTTSUB=0,/%s/%s/user/get,1\r",ProductKey,DeviceName);
//		delay_ms(2000);
//		COM1GetBuf(buf,200);
//		if(strstr((const char *)buf,"OK")[0]=='O')break;
//	}
//	comClearRxFifo(COM1);
//	memset(buf,0,sizeof buf);
	
	error=0;
	while(1)//订阅主题  订阅TOPIC  设置MQTT订阅信息
	{
		u1_printf("AT+MQTTSUB=0,/%s/%s/user/irr_set,1\r",ProductKey,DeviceName);
		delay_ms(2000);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"+MQTTEVENT:0,SUBSCRIBE,SUCCESS")[0]=='+')break;
		if(++error==10)//MQTT订阅超时处理
		{
			csh_bz=1;
			return;
		}
	}
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
		
tiaoshi_3060=16;/*调试*/
		
	EmwLED2=1;//调试使用（点亮D4）

}


//u8	sendEmw(char *data,unsigned char w)//EMW上报数据  （zq）
//{
//	u16 len;u8 time;
//	u8 buf[250];
//	char hc[500];
//  u8 sumbz=0;
	/*
	ProductKey:产品ID     水分一体机
  DeviceName：设备ID    23   24  25
	DeviceSecret：设备的秘钥（密码） 
  MQTT协议只负责通信部分，用户协议可以自己选择(MQTT+JSON是目前最优方案)
	*/
//	
//	while(1)//设置发送topic  //MQTT发布信息
//	{
//		if(!w)
//			u3_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName); //切换上报topic
//		else u3_printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
//		delay_ms(100);
//		COM1GetBuf(buf,200);
//		if(strstr((const char *)buf,"OK")[0]=='O')break;
//	}
//	comClearRxFifo(COM1);
//	memset(buf,0,sizeof buf);

//	len=0;time=0;	
//	while(1)	//发送数据指令  MQTT发布数据
//	{
//		u3_printf("AT+MQTTSEND=%d\r",strlen(data)+13);   //发布数据的字节长度
//		while(1)
//		{
//			delay_ms(200);
//			COM1GetBuf(buf,20);
//			if(strstr((const char *)buf,">")[0]=='>')break;
//			if(strstr((const char *)buf,"ERROR")[0]=='E')break;//模组正在忙
//			if(++time==10)return 0;//判断模组是不是死机了
//		}
//		if(strstr((const char *)buf,">")[0]=='>')break;//可以发送数据了
//		comClearRxFifo(COM1);
//		if(++len==5){return 0;}
//	}
//	memset(buf,0,sizeof buf);
//	
		/*
		使用C语言中的sprintf生成JSON字符串,可以生成一个{“String”:”Hello World!”, “Value”:12345}JSON字符串了。
		
		*/
//	sprintf(hc,"{\"params\":{%s}}",data);   //格式要符合Alink协议
//	len=strlen(hc);
//	if(hc[len-3]==',')hc[len-3]=' ';
//	u3_printf("%s",hc);//发送真正的数据
//	
//	delay_ms(3000);
//	COM1GetBuf(buf,100);

//	comClearRxFifo(COM1);
//	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')sumbz=0;//（数据发送成功）	//判断是否正常发送
//	else sumbz++;
//	if(sumbz>=3)return 0;
//	else return 1;
//}



////EMW上报数据
u8	sendEmw(char *data,unsigned char w)
{
	u8 buf[250];
	u16 len;
	/*
	ProductKey:产品ID     水分一体机
  DeviceName：设备ID    23   24  25
	*/
	while(1)//MQTT发布信息
	{
		if(!w)
			u1_printf("AT+MQTTPUB=/sys/%s/%s/thing/event/property/post,1\r",ProductKey,DeviceName);
		else u1_printf("AT+MQTTPUB=/%s/%s/user/war,1\r",ProductKey,DeviceName);
		delay_ms(100);
		COM1GetBuf(buf,200);
		if(strstr((const char *)buf,"OK")[0]=='O')break;
	}
	if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//解析收到的数据，阿里云下发的数据	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	len=0;
	while(1)
	{
		u1_printf("AT+MQTTSEND=%d\r",strlen(data)); //发布数据的字节长度

		delay_ms(50);
		COM1GetBuf(buf,50);

		delay_ms(20);
	  if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//解析收到的数据，阿里云下发的数据			
		if(strstr((const char *)buf,">")[0]=='>')break;//可以发送数据了
		comClearRxFifo(COM1);
		delay_ms(500);
		if(++len==10){return 0;} //超时处理
	}
	if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//解析收到的数据，阿里云下发的数据	
	comClearRxFifo(COM1);
	memset(buf,0,sizeof buf);
	
	u1_printf("%s",data);

	delay_ms(5000);//多延时一会，等读取到+MQTTEVENT:PUBLISH,SUCCESS，发送成功的标志
	COM1GetBuf(buf,200);

	if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//解析收到的数据，阿里云下发的数据	
	
	if(strstr((const char *)buf,"+MQTTEVENT:PUBLISH,SUCCESS")[0]=='+')return 1;//上传数据成功
	else return 0; //上传数据没有成功
}
/*
发送数据的格式
{\"params\":{\"to_type\":1,\"to_time\":2,\"need_partition\":3,\"switch\":4,\"work_state\":5,\"in_time\":6,\"at_partition\":7}}
*/
void SplicingPubHead(u8* Dest)
{
	strcpy((char*)Dest,"{\"params\":{");  //把从src地址开始且含有NULL结束符的字符串复制到以dest开始的地址空间
}
void SignedNumberToASCII ( u8 *Arry , s16 Number);//有正负的数
void SplicingPubData(u8*Dest,const u8*Text,s16 Value,u8 FirstF)
{
	u8 u8Str[16];
	if(FirstF >0)
	{
		strcat((char*)Dest,",");///将两个char类型连接,并存放在Dest数组中
	}
	strcat((char*)Dest,"\"");//加上\"
	strcat((char*)Dest,(char*)Text);
	strcat((char*)Dest,"\":");//加上\":
	SignedNumberToASCII(u8Str,Value);//16位有符号数转换成ASCII码
	strcat((char*)Dest,(char*)u8Str);
}
void SplicingPubTail(u8* Dest)
{
	strcat((char*)Dest,"}}");
}
u8 NetSendBuf[256];
void  SplicingPubStr(void)//向平台发送数据的整合，（清水，肥水）
{
	memset(NetSendBuf,0,256);//把NetSendBuf数组全部清零
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[0],Remote_Para[0],0);//to_type 灌溉类型(to_type)    默认-0，清水-1，肥水-2
	SplicingPubData(NetSendBuf,ReadText[1],Remote_Para[1],1);//to_time 灌溉时常（to_time）  时间常数
	SplicingPubData(NetSendBuf,ReadText[2],Remote_Para[2],2);//need_partition 需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
	SplicingPubData(NetSendBuf,ReadText[3],Remote_Para[3],3);//switch 启停(switch)    停止-0，启动-1
	SplicingPubData(NetSendBuf,ReadText[4],Remote_Read[0],4);//work_state 工作状态（work_state） 空闲-0，清水-1，肥水-2
	SplicingPubData(NetSendBuf,ReadText[5],Remote_Read[1],5);//in_time 倒计时（in_time)
	SplicingPubData(NetSendBuf,ReadText[6],Remote_Read[2],6);//at_partition 当前灌溉分区（at_partition）  1分区-1,2分区-2  应该补充一个空闲-0
	//SplicingPubData(NetSendBuf,ReadText[7],Remote_Read[3],7);
	SplicingPubTail(NetSendBuf);
}

void  SplicingPubStr_zdms(void)//自动模式向平台发送数据
{
	memset(NetSendBuf,0,256);//把NetSendBuf数组全部清零
	SplicingPubHead(NetSendBuf);
	SplicingPubData(NetSendBuf,ReadText[0],Remote_Para[0],0);//to_type 灌溉类型(to_type)    默认-0，清水-1，肥水-2
	SplicingPubData(NetSendBuf,ReadText[2],Remote_Para[2],1);//need_partition 需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
	SplicingPubData(NetSendBuf,ReadText[3],Remote_Para[3],2);//switch 启停(switch)    停止-0，启动-1
	SplicingPubData(NetSendBuf,ReadText[4],Remote_Read[0],3);//work_state 工作状态（work_state） 空闲-0，清水-1，肥水-2
	SplicingPubData(NetSendBuf,ReadText[6],Remote_Read[2],4);//at_partition 当前灌溉分区（at_partition）  1分区-1,2分区-2  应该补充一个空闲-0
	SplicingPubTail(NetSendBuf);
}

/*
查询wifi状态
*/
void CX_wifi_zt(void)
{
	u8 buf[250];
	static u8 error=0;
	static u8 error1=0;
	static u8 error2=0;
	
	if(++error==5)
	{
		
			if(csh_bz==1)//之前模组的初始化没有完成，重新执行一次初始化
			{
					Emw3060_init();
					Emw3060_con();				
			}
			else	//维护网络程序
			{
					//WIFI断线重连，每次重连以后将数据上报。	
					u1_printf("AT+WJAPS\r");//查看当前 Station 模式的连接状态
					delay_ms(200);						
					COM1GetBuf(buf,100);
					if(strstr((const char *)buf,"STATION_UP")[0]!='S')
					{	
							if(++error1==3)
							{ 	
								EmwLED1=1;//调试使用（点亮D5）wifi断线重连开始
								
								Emw3060_init();
								Emw3060_con();
								
								EmwLED1=0;//调试使用，wifi断线重连完成
								error1=0;
							}
					}
					else error1=0;				
					if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//解析收到的数据，阿里云下发的数据								
					comClearRxFifo(COM1);
					
			
					//查询MQTT状态	   
					//MQTT连接成功 +MQTTSTATUS:CONNECT,SUCCESS
					//MQTT连接失败 （现在要等10分钟才会掉线）
					if(csh_bz==0)//之前模组的初始化成功了
					{
							u1_printf("AT+MQTTSTATUS=connect\r");
							delay_ms(200);
							COM1GetBuf(buf,200);
							if(strstr((const char *)buf,"+MQTTSTATUS:CONNECT,SUCCESS")[0]!='+')
							{
									if(++error2==3)
									{
											EmwLED1=1;//调试使用（点亮D5）wifi断线重连开始
										
											Emw3060_init();
											Emw3060_con();//MQTT掉线之后，需要重新连接MQTT
										
											EmwLED1=0;//调试使用，wifi断线重连完成
											error2=0;
									}					
							}
							else	error2=0;
							if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)DepackRevData();//解析收到的数据，阿里云下发的数据				
							comClearRxFifo(COM1);				
					}
				
			}
		
			error=0;
	}	
}

/*主程序-调用上传数据程序*/
void fsbz_3060_cx(void)
{
		if(fsbz_3060==1)//现在正在发生数据中
		{
			fsbz_3060_bf=1; //把发生数据的指令存储到备份标志中
		}
		else	if(fsbz_3060==0)//现在不是发送状态
		{
			//组合发送数据的格式
			if(Guangai.zdms==1)
			{
				SplicingPubStr_zdms();//自动模式状态，发送数据
			}
			else	  //普通模式下，发送数据
			{
				SplicingPubStr();	
			}
			fsbz_3060=1;//进行一次上传数据的程序 
		}
}


u8 WifiPara[180];
//EMW任务
void network_task(void *pdata)
{
	u16 i=0,i1=0,i2=0;
	
	Remote_Para[0]=1;//灌溉类型(to_type)    默认-0，清水-1，肥水-2
	Remote_Para[1]=1;//灌溉时常（to_time）  时间常数
	Remote_Para[2]=1;//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
	Remote_Para[3]=0;//启动-1 停止-0
	SplicingPubStr();//组合发送数据的格式
	fsbz_3060=1;	
	
	
	FlashReadWiFi(WifiPara);//读取WIFI相关的数据
	memcpy(ssid,&WifiPara[0],30);//wifi名称       //将WifiPara中的31位数据复制到SSID变量中
	memcpy(password,&WifiPara[30],30);//wifi密码
	memcpy(ipword,&WifiPara[60],30);//IP地址
	memcpy(zwword,&WifiPara[90],30);//子网掩码
  memcpy(wgword,&WifiPara[120],30);//网关地址
  memcpy(ymword,&WifiPara[150],30);//域名	
	
	FlashReadDHCP(&DHCP);//读取DHCP之前的设置
	if(DHCP==0XFF)//表示DHCP之前没有设置过参数，修改成DHCP开
	{
		DHCP=1;//开DHCP
		FlashWriteDHCP(&DHCP);//保存DHCP的参数到FLASH地址
	}
	
	delay_ms(1000);
	
	Emw3060_init();	//EMW初始化
	Emw3060_con();	//EMW连接阿里云	

	rct_dueishi_cx();//RCT对时程序  查询网络时间，并将时间同步至RTC，每24小时同步一次
	
	while(1)
	{
		
		if(DHCP_cq==1)//DHCP设置之后，重新启动
		{
				FlashReadWiFi(WifiPara);//读取WIFI相关的数据
				memcpy(ssid,&WifiPara[0],30);//wifi名称       //将WifiPara中的31位数据复制到SSID变量中
				memcpy(password,&WifiPara[30],30);//wifi密码
				memcpy(ipword,&WifiPara[60],30);//IP地址
				memcpy(zwword,&WifiPara[90],30);//子网掩码
				memcpy(wgword,&WifiPara[120],30);//网关地址
				memcpy(ymword,&WifiPara[150],30);//域名	
					
				Emw3060_init();	//EMW初始化
				Emw3060_con();	//EMW连接阿里云		
				DHCP_cq=0;//完成重启
		
		}
				
		delay_ms(1000);
		
		
		if(COMGetBuf(COM1 ,NETCircle.buf, 500) > 10)
		{
			//解析收到的数据，阿里云下发的数据
			DepackRevData();
		}

				
		CX_wifi_zt();//查询wifi模组的状态
	

		if(++i1==300)//60是一分钟  
		{
			 if(csh_bz==0)rct_dueishi_cx();//RCT对时程序  查询网络时间，并将时间同步至RTC，每24小时同步一次
			 i1=0;
		}
							
		

				
		if((fsbz_3060==1)&&(csh_bz==0))
		{
				delay_ms(1000);			
				
				//发送函数,向平台发送数据
				i=sendEmw((char *)NetSendBuf,0);
				if(i==1)//发送成功
				{
					fsbz_3060=0;
					i2=0;
				}		
				else	//发送失败
				{
					delay_ms(8000);
					if(++i2==5)//发送失败5次，重新执行连接MQTT程序
					{
							if(csh_bz==0)//之前wifi模组初始化成功过，现在MQTT断开连接了
							{	
								  EmwLED1=1;//调试使用（点亮D5） 红灯+绿灯亮
								
									Emw3060_con();	//EMW连接阿里云

								  EmwLED1=0;//调试使用 
							}
							i2=0;
					}
				}
		}
		else	if((fsbz_3060==0)&&(csh_bz==0)&&(fsbz_3060_bf==1))//发送标志清空且之前初始化结束且发送备份标志为1
		{

				//组合发送数据的格式
				if(Guangai.zdms==1)
				{
					SplicingPubStr_zdms();//自动模式状态，发送数据
				}
				else	
				{
					SplicingPubStr();	
				}	
			  fsbz_3060=1;//需要上传一次数据
			  fsbz_3060_bf=0;//清除	
				
		}

		
	}
}

