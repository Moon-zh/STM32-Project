#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
#include "Emw3060.h"
#include "rtc.h"
#include "rs485.h"

#include	"cgq.h"
#include	"FlashDivide.h"

#define CMD_MAX_BUFFER 128
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //串口命令接收缓冲区
u8 hmi_cmp_rtc[7];//触摸屏RTC控件的时间


WorkPara Hmi_Para;


u8 DHCP_sz=1;//设置DPCH的值，配置界面使用  （默认为1  ：1为开启DHCP   0：为关闭DHCP）





void SignedNumberToASCII ( u8 *Arry , s16 Number);
/*******************************************************************************
名称:数字文本输入获取
功能:将报文中的数字解析出来并给目标参数赋值
输入: pDest:目标参数 msg:报文
*******************************************************************************/
void FifureTextInput ( s16 *pDest , PCTRL_MSG msg )
{
	s16 datatxt = 0;
	u16 i = 0;
	u8 u8F = 0;
	while ( ( ( uint8 * ) ( &msg->param ) ) [i] )//ASCII转换为数字
	{
		if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d )//0x2d为负号
		{
			datatxt = datatxt * 10 + ( ( ( uint8 * ) ( &msg->param ) ) [i++] - 0x30 ); //从接收缓冲区取出一个字节数据，再转换成十进制数字
		}
		else
		{
			u8F = 1;
			i++;
		}
	}
	if(u8F == 1)
	{
	   datatxt = datatxt*(-1);
	}
	*pDest = datatxt;
}
/*******************************************************************************
名称:文本输入获取
功能:将报文中的数字解析出来并给目标参数赋值
输入: pDest:目标参数 msg:报文
*******************************************************************************/
void TextInput ( u8 *pDest , PCTRL_MSG msg )
{
	s16 datatxt = 0;
	u16 i = 0;
	while ( ( ( uint8 * ) ( &msg->param ) ) [i] )//ASCII转换为数字
	{
		*(pDest++) = ( ( uint8 * ) ( &msg->param ) ) [i++];
	}
	*pDest = datatxt;
}
/*******************************************************************************
名称:按键状态获取
功能:将报文中的按键状态解析出来，赋值给目的参数
输入: pDest:目标参数 msg:报文
*******************************************************************************/
void ButtonStatueGet ( u8 *pDest , PCTRL_MSG msg )
{
	u8 buttonvalue = msg->param[1];
	*pDest = buttonvalue;
	
}
/*******************************************************************************
名称:列表值获取
功能:将报文中的按键状态解析出来，赋值给目的参数
输入: pDest:目标参数 msg:报文
*******************************************************************************/
void MenuValueGet ( u8 *pDest , PCTRL_MSG msg )
{
	u8 value =  msg->param[0] ; //数值
	*pDest = value;
}
//数字转换成ASCII码
void NumberToASCII ( u8 *Arry , u16 Number)
{
	u8 i = 0 , j = 0;
	u8 temp = 0;
	u8 strtemp[16] = {0,0,0,0,0,0,0,0};
	do					//将数字分开并装换成ASCII码
	{
		temp = Number % 10;
		strtemp[i++] = temp + 0x30;
		Number = Number / 10;
	}
	while(Number);
	while(i)//调整顺序
	{
		Arry[j++] =strtemp[--i];
	}
	Arry[j++] = 0;
	//Arry[j++] = 0x30;
	
}
//16位有符号数转换成ASCII码
void SignedNumberToASCII ( u8 *Arry , s16 Number)//有正负的数
{
	u8 i = 0 , j = 0;
	u8 temp = 0;
	u8 strtemp[16] = {0,0,0,0,0,0,0,0};
	u16 u16Number;
	if(Number >= 0)
	{ 
		u16Number = Number;
		do					//将数字分开并装换成ASCII码
		{
			temp = u16Number % 10;
			strtemp[i++] = temp + 0x30;
			u16Number = u16Number / 10;
		}
		while(u16Number);
		while(i)//调整顺序
		{
			Arry[j++] =strtemp[--i];
		}
		Arry[j++] = 0;
	}
	else
	{
	  u16Number = -Number;
		do					//将数字分开并装换成ASCII码
		{
			temp = u16Number % 10;
			strtemp[i++] = temp + 0x30;
			u16Number = u16Number / 10;
		}
		while(u16Number);
//		strtemp[i++] = 0x2d;//负号
		j = 1;
		while(i)//调整顺序
		{
			Arry[j++] =strtemp[--i];
		}
		Arry[0] = 0x2d;//负号
		Arry[j++] = 0;
	}
}
//显示是倒计时函数
//输入：剩余分钟
//输出：无
//功能：首页界面显示倒计时
void DisCountDown(u16 RemainderMinute)
{
	u16 ReHour = RemainderMinute/60;
	u16 ReMin = RemainderMinute%60;	
	u8 u8Str[16];
	
	NumberToASCII(u8Str,ReHour);
	SetTextValue(LCD_MAIN_PAGE,47,u8Str);
	
	NumberToASCII(u8Str,ReMin);
	SetTextValue(LCD_MAIN_PAGE,53,u8Str);
	
}
const u8 StaStr0[12] = "一号分区";
const u8 StaStr1[12] = "二号分区";
const u8 StaStr2[12] = "3阶段";
const u8 StaStr3[12] = "肥水灌溉";
const u8 StaStr4[12] = "清水灌溉";
const u8 StaStr5[12] = {0,0,0};
const u8* StaText[] = 
{
	StaStr0,
	StaStr1,
	StaStr2,
	StaStr3,
	StaStr4,
	StaStr5,
};



//待机界面接收到的指令
void StandbyPage(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				   //产生消息的控件ID
	
	u8	u8ButtonVaule = msg->param[1]; //只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //产生消息的控件ID

	if((u16Control_ID==1)&&(u8ButtonVaule == 0))
	{
		if(Guangai.zdms==1)//已开启自动模式，显示自动模式界面
		{
				SetScreen(LCD_ZIDONG_MOSHI);	
		}
		else	
		{
				//如果没有工作,显示配置界面
				if(GetSysState()==SYSTEM_IDLE)//空闲状态
				{
					SetScreen(LCD_MODELSEL_PAGE);   
				}
				//如果正在工作,显示状态界面
				else
				{
					SetScreen(LCD_STATESHOW_PAGE);
				}
			
		}
	}
}

//工作状态显示，正在灌溉界面
void WorkingState(PCTRL_MSG msg, qsize size )
{
	MsgStruct Msgtemp;
	u16 u16Control_ID;				   //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1]; //只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //产生消息的控件ID

	if((u16Control_ID==5)&&(u8ButtonVaule == 1))//停止
	{
		//发送停止消息
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdType = MSG_STOP;  //结束
		//Msgtemp.CmdData[0] = 1;//灌溉类型(to_type)    默认-0，清水-1，肥水-2  平台判断的不能为0
		Msgtemp.CmdData[1] = 0;//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
		Msgtemp.CmdData[2] = 0;//灌溉时常（小时）
		Msgtemp.CmdData[3] = 0;//灌溉时常（分钟）	
		PackSendMasterQ(&Msgtemp);
		
	}
}


u8 g_u8WifiPara[200];
//模式选择
void ModelSel(PCTRL_MSG msg, qsize size )
{
	u8 sz_hc[20];//缓冲数组
	u16 u16Control_ID;				  //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1]; //只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //产生消息的控件ID
	
	
	if(u16Control_ID == 1)//清水   (在触摸屏程序中有切换界面+批量更新控件数值)
	{
		Hmi_Para.WorkModel = 1;
		Hmi_Para.Zone = 0x03;
	}
	else if(u16Control_ID == 2)//施肥
	{
		Hmi_Para.WorkModel = 2;
		Hmi_Para.Zone = 0x03;//默认开始两个分区
	}
	else if(u16Control_ID == 3)//设备配置
	{
		//从flash中读取参数
		FlashReadWiFi(g_u8WifiPara);
		//展示参数
		SetTextValue(LCD_PARASET_PAGE,1,&g_u8WifiPara[0]);//wifi名称
		SetTextValue(LCD_PARASET_PAGE,2,&g_u8WifiPara[30]);//wifi密码
		SetTextValue(LCD_PARASET_PAGE,3,&g_u8WifiPara[60]);	//IP
		SetTextValue(LCD_PARASET_PAGE,4,&g_u8WifiPara[90]);	//子网掩码
		SetTextValue(LCD_PARASET_PAGE,5,&g_u8WifiPara[120]);//网关	
		SetTextValue(LCD_PARASET_PAGE,6,&g_u8WifiPara[150]);//域名		
		
		FlashReadDHCP(&DHCP);//读取DHCP之前的设置
		SetButtonValue(LCD_PARASET_PAGE,9,DHCP);//DHCP状态
		//跳转设备WIFI配置界面
		SetScreen(LCD_PARASET_PAGE);
	}
	else	if(u16Control_ID == 4&&u8ButtonVaule==1)//自动模式
	{		
		 
		FlashReadZDMS_QDYZ(&Guangai.qdyzbz);//从flash中读取启动阈值
//		Guangai.qdyzbz=15;//自动模式中的 启动阈值标志	
		SignedNumberToASCII(sz_hc,Guangai.qdyzbz);//十进制转成字符串
		SetTextValue(8,3,sz_hc);//触摸屏设置启动阈值
	
		FlashReadZDMS_TZYZ(&Guangai.tzyzbz);//从flash中读取停止阈值		
//		Guangai.tzyzbz=25;//自动模式中的 停止阈值标志	
		SignedNumberToASCII(sz_hc,Guangai.tzyzbz);//十进制转成字符串
		SetTextValue(8,5,sz_hc);//触摸屏设置停止阈值		
		
		Guangai.zdms=1;//开启自动模式
		Guangai.qdbz=0;//自动模式关闭浇水
		Guangai.yichi=0;//标志清除			

		//触摸屏，设置当前灌溉状态
		sprintf((char *)sz_hc,"    清水灌溉");
		SetTextValue(8,4,sz_hc);			
		//跳转自动模式界面
		SetScreen(LCD_ZIDONG_MOSHI);
		
	}
}

//自动模式界面
void ZidongSave(PCTRL_MSG msg,qsize size)
{
	s16 yzbz;//阈值标志
	u8 sz_hc[8]={0,0,0,0,0,0,0,0};//缓冲数组
	MsgStruct Msgtemp;
	
	u16 u16Control_ID;				  //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1];//只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //产生消息的控件ID
		
	if(u16Control_ID==3)//设置启动阈值
	{
		  FifureTextInput(&yzbz,msg);
		
			if(yzbz<Guangai.tzyzbz)//自动模式中，设置的启动阈值小于停止阈值
			{
				Guangai.qdyzbz=yzbz;
				FlashWriteZDMS_QDYZ(&Guangai.qdyzbz);//先flash中写入启动阈值		
			}
			else	//设置的启动阈值大于等于停止阈值，不做处理
			{
					SignedNumberToASCII(sz_hc,Guangai.qdyzbz);//十进制转成字符串
					SetTextValue(8,3,sz_hc);//触摸屏设置启动阈值			
			}

	}
	else	if(u16Control_ID==5)//设置停止阈值
	{

		  FifureTextInput(&yzbz,msg);
		
			if(yzbz>Guangai.qdyzbz)//自动模式中，设置的停止阈值大于启动阈值
			{
				Guangai.tzyzbz=yzbz;
				FlashWriteZDMS_TZYZ(&Guangai.tzyzbz);//先flash中写入停止阈值	
			}
			else	//设置的停止阈值小于等于启动阈值，不做处理
			{
					SignedNumberToASCII(sz_hc,Guangai.tzyzbz);//十进制转成字符串
					SetTextValue(8,5,sz_hc);//触摸屏设置停止阈值			
			}
			
	}	
	else	if(u16Control_ID==6&&u8ButtonVaule==1)//返回按键
	{		
		Guangai.zdms=0;//关闭自动模式
		Guangai.qdbz=0;//自动模式关闭浇水
		Guangai.yichi=0;//标志清除			
		
		//发送停止消息
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdType = MSG_STOP;  //结束
		//Msgtemp.CmdData[0] = 1;//灌溉类型(to_type)    默认-0，清水-1，肥水-2  平台判断的不能为0
		Msgtemp.CmdData[1] = 0;//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
		Msgtemp.CmdData[2] = 0;//灌溉时常（小时）
		Msgtemp.CmdData[3] = 0;//灌溉时常（分钟）	
		PackSendMasterQ(&Msgtemp);	


	}
}


//配置界面
//由msg->param所指内存区域复制msg->param个字节到g_u8WifiPara所指内存区域。
//+1 是为了将字符串后面的'\0'字符结尾符放进来，去掉+1可能出现乱码
void ParaSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				 //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1]; //只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //产生消息的控件ID
	
	if(u16Control_ID == 1)//wifi名称
	{
//		memcpy(&g_u8WifiPara[0],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+0,(const char*)msg->param);
	}
	else if(u16Control_ID == 2)//wifi密码
	{
//		memcpy(&g_u8WifiPara[30],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+30,(const char*)msg->param);
	}
	else if(u16Control_ID == 3)//IP
	{
//		memcpy(&g_u8WifiPara[60],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+60,(const char*)msg->param);
	}
	else if(u16Control_ID == 4)//子网掩码
	{
//		memcpy(&g_u8WifiPara[90],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+90,(const char*)msg->param);
	}
	else if(u16Control_ID == 5)//网关
	{
//		memcpy(&g_u8WifiPara[120],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+120,(const char*)msg->param);
	}
	else if(u16Control_ID == 6)//域名
	{
//		memcpy(&g_u8WifiPara[150],msg->param,strlen((char *)msg->param)+1);
		strcpy((char *)g_u8WifiPara+150,(const char*)msg->param);
	}
	else if(u16Control_ID == 8)//保存
	{
		SetScreen(LCD_PARASAVE_PAGE);//跳转保存确认界面
	}
	else	if(u16Control_ID==9)//设置DHCP
	{
			if(u8ButtonVaule==0)//关DHCP
			{
				DHCP_sz=0;
			}
			else	if(u8ButtonVaule==1)//开DHCP （默认）
			{
				DHCP_sz=1;
			}
	}
	
}


//保存确认界面
void ParaSave(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				 //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1]; //只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id );  //产生消息的控件ID
	if(u16Control_ID == 5 && u8ButtonVaule ==1)//保存按键  5是按键编号，1是按下
	{
		FlashWriteWiFi(g_u8WifiPara);//保存FLASH数据
		//保存设置之后更新设置
//		memcpy(ssid,g_u8WifiPara+0,30);//wifi名称       //将WifiPara中的30位数据复制到SSID变量中  (内存复制)
//		memcpy(password,g_u8WifiPara+30,30);//wifi密码
//		memcpy(ipword,g_u8WifiPara+60,30);//IP地址
//		memcpy(zwword,g_u8WifiPara+90,30);//子网掩码
//		memcpy(wgword,g_u8WifiPara+120,30);//网关地址
//		memcpy(ymword,g_u8WifiPara+150,30);//域名			
		strncpy(ssid,(const char *)g_u8WifiPara+0,30);//wifi名称       //将WifiPara中的30位数据复制到SSID变量中  （字符串复制）
		strncpy(password,(const char *)g_u8WifiPara+30,30);//wifi密码
		strncpy(ipword,(const char *)g_u8WifiPara+60,30);//IP地址
		strncpy(zwword,(const char *)g_u8WifiPara+90,30);//子网掩码
		strncpy(wgword,(const char *)g_u8WifiPara+120,30);//网关地址
		strncpy(ymword,(const char *)g_u8WifiPara+150,30);//域名			
		
//		if(DHCP!=DHCP_sz)//DHCP设置发生变化，需要重启模块 (只要点击保存按键，就要重新设置)
		{
				DHCP=DHCP_sz;//把设计的DHCP，送给DHCP保存。
				FlashWriteDHCP(&DHCP);//保存DHCP的参数到FLASH地址
				DHCP_cq=1;//重新启动,进行EMW初始化，EMW连接阿里云			
		}
		
		SetScreen(LCD_PARASET_PAGE);//跳转设备配置界面
	}
	else if(u16Control_ID == 1 && u8ButtonVaule ==1)//返回按键
	{
		SetScreen(LCD_PARASET_PAGE);//跳转设备配置界面
	}
}


//灌溉分区选择
void ZoneSel(PCTRL_MSG msg, qsize size )
{ 
	char *i;
	u16 u16Control_ID;				   //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1];//只取按钮状态的部分 按下：1  弹起：0
	u16Control_ID = PTR2U16 ( &msg->control_id ); //产生消息的控件ID
	
	
	//一号分区，二号分区选择
	if(u16Control_ID == 3 ||u16Control_ID == 4)//1:为选中   0：为不选中
	{
		if(u8ButtonVaule == 0)
		{
			Hmi_Para.Zone = Hmi_Para.Zone^(1<<(u16Control_ID-3));
		}
		else
		{
			Hmi_Para.Zone = Hmi_Para.Zone|(1<<(u16Control_ID-3));
		}
	}
	else if(u16Control_ID == 2&&u8ButtonVaule==1)//下一步
	{
		if(Hmi_Para.Zone != 0)//有需要灌溉的分区
		{
				Hmi_Para.TimeH =0;//灌溉时长清零
	     	Hmi_Para.TimeM =0;
			
			  if(Hmi_Para.WorkModel==1)//清水
				{
						i="20";
//					  strcpy((char *)i, "20"); 
						SetTextValue(3,3,(uchar *)i);//灌溉时长的文本框，初始化为20分钟
				}
				else	if(Hmi_Para.WorkModel==2)//肥水
				{
						i="25";
			    	SetTextValue(3,3,(uchar *)i);//灌溉时长的文本框，初始化为25分钟
				} 	
			  SetScreen(LCD_TIMESET_PAGE);//跳转到灌溉时长界面
		}
		else   //灌溉分区为空	
		{
				SetScreen(LCD_ZONESEL_PAGE_KONG);//跳转到灌溉分区为空界面
				delay_ms(2000);
			  SetScreen(LCD_ZONESEL_PAGE);//跳转到灌溉分区选择界面
		}
	}
}



//灌溉时长设置
void TimeSet(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				  	   //产生消息的控件ID
	u8	u8ButtonVaule = msg->param[1];//只取按钮状态的部分 按下：1  弹起：0
	s16 s16RevVal  = 0;//存放文本框输入的灌溉时长
	MsgStruct Msgtemp;
	u16Control_ID = PTR2U16 ( &msg->control_id ); 	   //产生消息的控件ID
	
	if(u16Control_ID >=4 && u16Control_ID<=9)//时长选择显示界面（60到210分钟）
	{
		u16Control_ID =u16Control_ID-3;
		Hmi_Para.TimeH = (u16Control_ID+1)/2 ;
		Hmi_Para.TimeM = ((u16Control_ID+1)%2)*30;
	}
	else if(u16Control_ID == 2&& u8ButtonVaule ==1 &&MasterState==0)//启动按键按下且运行为空闲状态
	{
		delay_ms(1000);
		if(MasterState==0)//延时1秒之后再判断一下是不是出于空闲状态
		{
			if(Hmi_Para.WorkModel==1)//清水
			{
				if((Hmi_Para.TimeH==0)&&(Hmi_Para.TimeM<20))//方法1    方法2：读取屏幕中文本框的灌溉时长
				{
					Hmi_Para.TimeM=20;
			//		Hmi_Para.TimeM=3;//调试
				}		
			}
			else	if(Hmi_Para.WorkModel==2)//肥水
			{
				if((Hmi_Para.TimeH==0)&&(Hmi_Para.TimeM<=20))//方法1    方法2：读取屏幕中文本框的灌溉时长
				{
					Hmi_Para.TimeM=25;
				}		
			}
		
			Msgtemp.CmdSrc = SCREEN_TASK_CODE;
			Msgtemp.CmdType = MSG_START;//启动
			Msgtemp.CmdData[0] = Hmi_Para.WorkModel;//区分：是清水还是施肥
			Msgtemp.CmdData[1] = Hmi_Para.Zone;//要执行几个分区
			Msgtemp.CmdData[2] = Hmi_Para.TimeH;//时间：小时
			Msgtemp.CmdData[3] = Hmi_Para.TimeM;//时间：分钟
			PackSendMasterQ(&Msgtemp);//消息打包发走
			SetScreen(LCD_STATESHOW_PAGE); //正在灌溉，状态显示界面
		
		}
			
	}
	else if(u16Control_ID == 3)//读取文本框中的数据，触摸屏中处理了范围
	{
		FifureTextInput ( &s16RevVal, msg );
		Hmi_Para.TimeH = s16RevVal/60;  //灌溉小时
		Hmi_Para.TimeM = s16RevVal%60;  //灌溉分钟
	}
	
}


/*在正在灌溉界面 
更新 当前灌溉模式和当前灌溉分区

*/
//0:施肥 1:浇水   
//更新触摸屏显示数据
void WorkModelShow(u8 Value,u8 ZoneNum)
{
	SetTextValue(LCD_STATESHOW_PAGE,10,(uchar *)StaText[Value+3]);   //当前灌溉模式
	SetTextValue(LCD_STATESHOW_PAGE,3,(uchar *)StaText[ZoneNum-2]);  //当前灌溉分区
}


//0:1分区 1:2分区
void WorkZoneShow(u8 Value)
{
	SetButtonValue(LCD_STATESHOW_PAGE,1,Value);
}


extern  WorkPara IrrPara;
extern CountDownStruct WorkTime;
//触摸屏显示倒计时时间
void WorkCountShow(u8 hour,u8 minute)
{
	u8 TimeH = hour ;
	u8 TimeM = minute;
	u8 u8Str[7] = {0,0};

	//应对新界面 根据阶段显示倒计时时间
	if(GetSysState()==2)//主程序的运行状态  施肥阶段1
	{
		if(IrrPara.TimeM>=FERTILIZER1TIME)//分钟大于10
		{
			TimeM = IrrPara.TimeM-FERTILIZER1TIME+minute;
			TimeH =  IrrPara.TimeH;
		}
		else
		{
			TimeM = IrrPara.TimeM+60-FERTILIZER1TIME+minute;
			TimeH = IrrPara.TimeH-1;
		}
	}
	else if(GetSysState()==3)// 施肥阶段2
	{
		TimeM = minute+FERTILIZER3TIME;
	}
	
	NumberToASCII(u8Str,(u16)((u16)(TimeH*60)+(u16)TimeM));////数字转换成ASCII码
	SetTextValue(LCD_STATESHOW_PAGE,2,u8Str);//在触摸屏上更新倒计时时间（分钟）	
	
}


void FerStageShow(u8 Stage)
{
	if(Stage>2)
		return;
	SetTextValue(LCD_STATESHOW_PAGE,4,(uchar *)StaText[Stage]);
}
void FerStateShow(u8 State)
{
	if(State>4)
		return;
	SetTextValue(LCD_STATESHOW_PAGE,8,(uchar *)StaText[State]);
}



/*
//接受触摸屏消息
EE B1 11 00 01 00 01  11          31 32 33        00         FF FC FF FF
         画面ID控件ID 文本控件    ASCLL码的123     结束符
*/
void ReceiveFromMonitor( void )
{
	uint8 cmd_type = 0;//指令类型
	uint8 ctrl_msg = 0;//消息的类型
	uint16 screen_id = 0;//画面ID

	PCTRL_MSG msg = 0;
	qsize size = 0;
	
	size = queue_find_cmd ( Hmi_cmd_buffer, CMD_MAX_BUFFER ); //从缓冲区中获取一条指令	
	
	if(size>0)//接收到数据
	{	
			msg=(PCTRL_MSG)Hmi_cmd_buffer;//把接收的缓冲区数组转换为结构体模式
			cmd_type = msg->cmd_type;//指令类型
			ctrl_msg = msg->ctrl_msg;//消息的类型
			screen_id = PTR2U16(&msg->screen_id);//画面ID
			
			switch (cmd_type) //指令类型
			{
				case NOTIFY_CONTROL://控件更新通知  0XB1
				{
						if(ctrl_msg==MSG_GET_CURRENT_SCREEN) //画面ID变化通知
						{
	
						}
						else     //主机请求获取控件的数值  0X11 MSG_GET_DATA	
						{
					
							switch(screen_id)//画面ID
							{							
								case LCD_STANDBY_PAGE://待机界面
									StandbyPage(msg, size);
									break;
								
								case LCD_STATESHOW_PAGE: //正在灌溉，状态显示界面
									WorkingState(msg, size);
									break;
								
								case LCD_MODELSEL_PAGE: //模式选择
									ModelSel(msg, size);
									break;
								
								case LCD_ZONESEL_PAGE: //分区选择
									ZoneSel(msg, size);
									break;
								
								case LCD_TIMESET_PAGE: //时长设置
									TimeSet(msg, size);
									break;
								
								case LCD_PARASET_PAGE: //wifi设置
									ParaSet(msg, size);
									break;
								
								case LCD_PARASAVE_PAGE: //保存界面
									ParaSave(msg, size);
									break;
								
								case LCD_ZIDONG_MOSHI:  //自动模式界面
									ZidongSave(msg,size);
									break;
									
								default:
									break;
							}					 
					 }			
				}
				break;
			
				case 0xf7 : //读取触摸屏RTC时钟
				{
					hmi_cmp_rtc[0]=Hmi_cmd_buffer[2];//年
					hmi_cmp_rtc[1]=Hmi_cmd_buffer[3];//月
					hmi_cmp_rtc[2]=Hmi_cmd_buffer[4];//星期
					hmi_cmp_rtc[3]=Hmi_cmd_buffer[5];//日
					hmi_cmp_rtc[4]=Hmi_cmd_buffer[6];//小时
					hmi_cmp_rtc[5]=Hmi_cmd_buffer[7];//分
					hmi_cmp_rtc[6]=Hmi_cmd_buffer[8];//秒				
				}
				break;

				default:
					break;
			}

			memset(g_RxBuf5,0,UART5_RX_BUF_SIZE);
			comClearRxFifo(COM5);			
		
	}

	

}


void	HDMI_Init(void)	//触摸屏初始化
{
	queue_reset();			//清空缓存区
	delay_ms(300);			//必须等待300ms
}



u8 RefreshF=0;
extern CountDownStruct WorkTime;
/*接受AGV的触摸屏显示器交互信息*/
void Task_HMIMonitor ( void * parg )
{
	parg = parg;
	
	while ( 1 )
	{
		delay_ms(50);
		ReceiveFromMonitor();//接受触摸屏消息
		
		if(RefreshF == 1)
		{		
			delay_ms(50);
			WorkCountShow(WorkTime.hour,WorkTime.min);//触摸屏显示倒计时时间
			RefreshF = 0;
		}
		
	}
}

