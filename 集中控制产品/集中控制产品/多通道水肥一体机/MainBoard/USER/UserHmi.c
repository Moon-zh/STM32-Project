#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
#include "Emw3060.h"
#include "hmi_driver.h"
#include "string.h"
#include "rtc.h" 
#include "communication.h"
#include "logic.h"
#include "W25Qxx.h"
#include<stdio.h>
extern OS_TMR	CallerTmr;
extern _calendar_obj calendar;//时钟结构体 
extern u8 com_conect_state;//串口连接状态
u16 amount_fer=0;//每亩施肥量
u8 guest_password[6]={6,6,6,6,6,6}; //访客初始密码为666666 密码长度为6位数
u8 admin_password[6]={8,8,8,8,8,8}; //管理员初始密码为888888 密码长度为6位数
u8 Password_flag=0;//密码验证成功为1 失败为0;
u8 set_Pressbutonn[2]={0,0};//设置压力开关及分区
#define CMD_MAX_BUFFER 128
extern u8 screen_refresh_flag;
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //串口命令接收缓冲区
u16 s_screen_id;                  //页面ID编号变量
u16 s_control_id;                 //控件ID编号变量
u8 task_record_buff[1600]; //用于存储100条任务记录
char u8chardatabuff[100];
char	g_u8UserPassword[9]={0,0,0,0,0,0,0,0,0};//验证完以后要清零
const char	g_u8AdminPassword[9]={0x31,0x39,0x39,0x31,0x30,0x37,0x30,0x31,0};
char g_u8SysTemPassword[9]={0,0,0,0,0,0,0,0,0};
extern u8 Current_task_num;//当前任务号
extern u8 task_start_flag;//任务开始标志
u8 last_display_tasknum=0;//上次界面显示
FlashSet flashset;//设置参数保存
HmiRecord u_HmiRecord;
WorkPara HmiWorkPara;
CtaskRecordunion ctask_record;//记录任务
Targetred ctargetrcord;//当前任务记录
extern fertaskdata fertasktimedate;//水表实时数据读取
extern fercal fercaldate; //计算数据
extern u8 gu8Use_MA_Check_Flag;//肥料开断检测
extern u8 startzone;//开启分区
extern u8 closezone;//关闭分区
u8 g_u8DestPage = 0;//密码正确跳转界面zbz
u8 g_u8SrcPage = 0;//密码界面返回跳转界面zbz
u8 display_count=0;//用于刷新界面
u8 fer_display_count=0;//用于肥水界面
u8 Openboard2=0;//开启板2的施肥合水泵
u8 Closeboard2=0;//关闭板2的施肥合水泵

extern float Use_A_Kg;
extern float Use_B_Kg;
extern float Use_C_Kg;
extern float Use_D_Kg;
const u8 StaStr0[15] = "复合肥灌溉程序";
const u8 StaStr1[15] = "一阶段灌溉程序";
const u8 StaStr2[15] = "二阶段灌溉程序";
const u8 StaStr3[15] = "三阶段灌溉程序";
const u8 StaStr4[15] = "四阶段灌溉程序";
const u8 StaStr5[12] = "清水灌溉";      
const	u8 StaStr6[12] = "肥水灌溉";
const	u8 StaStr7[12] = "完成停止";          //完成1 界面2 远程3
const	u8 StaStr8[12] = "界面停止";
const	u8 StaStr9[12] = "远程停止";
const	u8 StaStr10[12] = {0,0,0};
const u8* StaText[] = 
{
	StaStr0,
	StaStr1,
	StaStr2,
	StaStr3,
	StaStr4,
	StaStr5,
	StaStr6,
	StaStr7,
	StaStr8,
	StaStr9,
	StaStr10	
};
u8 g_u8WifiPara[126];
u8 g_u8DHCP=1;
u8 g_u8WiFiRestart=0;
extern char ssid[31];
extern char password[31];
extern char ipaddr[16];
extern char subnet[16];
extern char gateway[16];
extern char dns[16];

void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum);
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
		if((( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d) 
			&&(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2e))//0x2d为负号
		{
			datatxt = datatxt * 10 + ( ( ( uint8 * ) ( &msg->param ) ) [i++] - 0x30 ); //从接收缓冲区取出一个字节数据，再转换成十进制数字
		}
		else if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d)//有小数时
		{
			u8F = 1;
			i++;
		}
		else if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2e)
		{
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
void SignedNumberToASCII ( u8 *Arry , s16 Number,u8 decimalnum)//有正负的数
	{
		u8 i = 0 , j = 0,m=0;
		u8 temp = 0;
		u8 strtemp[16] = {0,0,0,0,0,0,0,0};
		u16 u16Number;
		if(Number >= 0)
		{ 
			u16Number = Number;
			do					//将数字分开并装换成ASCII码
			{
				if(decimalnum != i||decimalnum==0)
				{
					temp = u16Number % 10;
					strtemp[i++] = temp + 0x30;
					u16Number = u16Number / 10;
				}
				else
				{
					if(decimalnum!=0)
					{
						strtemp[i++] = 0x2e;
					}
					else
						i++;
				}
			}
			while(u16Number);
			if(decimalnum>=i)
			{
					for(m=0;m<decimalnum-i;m++)
					{
						strtemp[i++] = 0x30;
					}
					strtemp[i++] = 0x2e;
					strtemp[i++] = 0x30;
			}
			while(i)//调整顺序
			{
				Arry[j++] =strtemp[--i];
			}
			Arry[j++] = 0;
		}
		else
		{
		  	u16Number = -Number;
			do
			{
			  if(decimalnum != i||decimalnum==0)
			  {
				  temp = u16Number % 10;
				  strtemp[i++] = temp + 0x30;
				  u16Number = u16Number / 10;
			  }
			  else
			  {
				  if(decimalnum!=0)
				  {
					  strtemp[i++] = 0x2e;
				  }
				  else
					  i++;
			  }
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


/*******************************************************************************
名称:文本输入获取
功能:将报文中的数字解析出来并给目标参数赋值
输入: Number:目标参数 msg:报文
*******************************************************************************/
u16 ASCIITONUMBER (PCTRL_MSG msg )
{
	u8 i = 0 , j = 0;
//	u8 temp = 0;
	u16 strtemp[4] = {0,0,0,0};
  u16 Number=0;
    for(i=0;i<5;i++)
    {
        if(0==msg->param[i])
        {   j=i;
            break;
        }
    }
    switch(j)
    {
        case 1:
        strtemp[0]=msg->param[0]-0x30;
        Number+=strtemp[0];           
         return Number;
        case 2:
     strtemp[0]=msg->param[0]-0x30;
    Number+=strtemp[0]*10;
    strtemp[1]=msg->param[1]-0x30;
    Number+=strtemp[1]*1;	
        return Number;
        case 3:
     strtemp[0]=msg->param[0]-0x30;
    Number+=strtemp[0]*100;
    strtemp[1]=msg->param[1]-0x30;
    Number+=strtemp[1]*10;	
    strtemp[2]=msg->param[2]-0x30;
    Number+=strtemp[2];            
        return Number;
        case 4:
     strtemp[0]=msg->param[0]-0x30;
    Number+=strtemp[0]*1000;
    strtemp[1]=msg->param[1]-0x30;
    Number+=strtemp[1]*100;	
    strtemp[2]=msg->param[2]-0x30;
    Number+=strtemp[2]*10;
    strtemp[3]=msg->param[3]-0x30;
    Number+=strtemp[3];           
        return Number;
        default:
        break;
    }
return Number;
}
//时间赋值函数
void time_copy(void)
{
	ctargetrcord.trecord.curtargetyear=ctask_record.record.start_year=calendar.w_year;
	ctargetrcord.trecord.curtargetmon=ctask_record.record.start_month=calendar.w_month;	
	ctargetrcord.trecord.curtargetday=ctask_record.record.start_day=calendar.w_date;
	ctargetrcord.trecord.curtargethour =ctask_record.record.start_hour=calendar.hour;
	ctargetrcord.trecord.curtargetmin=ctask_record.record.start_minitue=calendar.min;
	ctargetrcord.trecord.curtargetsec=ctask_record.record.start_second=calendar.sec;
}
//任务进行的时间
u16 Tasking_time(u8 hour,u8 min,u8 sec,u8 hour1,u8 min1,u8 sec1)
{
 	u16 thour,tmin;
 	if(hour<hour1)
 	{
 		hour = hour+24;
 	}
	thour=(hour-hour1)*3600;
	tmin=thour+min*60+sec-min1*60-sec1;
	tmin=tmin/60;
	return tmin;
}
//任务分解
//输入选择分区把任务分解成小任务
//然后分别进行完成
void task_resolve(u8*databuf,u32 data)
{
	 u8 i;
	 u8 j=0;
	 for(i=0;i<32;i++)
	{
		if((data>>i)&0x01)//若该分区被选中则记录
		  databuf[j++]=i+1; //分区没有从0开始的
		 ctargetrcord.trecord.curtarget_max_num=j;

	}


}
//待机界面
void StangbyPage(PCTRL_MSG msg, qsize size)
{
//	u8 Temp=0;
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID

	if(u16Control_ID == 1 &&u8ButtonVaule == 1)
	{
		//根据状态显示界面
	}
}
//u16 FlashPara[76];
//灌溉OR设置
void IrrOrSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if(u16Control_ID == 2&&u8ButtonVaule == 1)
	{
//		FlashReadPara((u8*)flashset.FlashPara,0,76);
		for(i=0;i<4;i++)
		{
			NumberToASCII(u8Str,flashset.FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP1_PAGE,1,4,u32ShowBuf);
	}
}
//灌溉选择界面
void IRRSTEP(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
//	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
  	switch(u16Control_ID)
	{
		case 2:
			SetButtonValue1(LCD_WATERSTEP2_PAGE,1,16,u8Str); //更新清水分区选择
			HmiWorkPara.WaterPartition=0;
			SetRemotePara(16,1);
		break;
		case 3:
			SetButtonValue1(LCD_FERSTEP2_PAGE,1,16,u8Str); //更新肥水分区选择	
			HmiWorkPara.FerPartition=0;
			SetRemotePara(16,2);
		break;		
	}
}
//清水选择分区界面
void WaterZoneSel(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8 u8str[2]={0,0};
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		if(u8ButtonVaule==0)
		{
			HmiWorkPara.WaterPartition^=(1<<(u16Control_ID-1));
		}
		else if(u8ButtonVaule==1)
		{
			HmiWorkPara.WaterPartition|=(1<<(u16Control_ID-1));
		}
	}
	else if(u16Control_ID == 18)
	{
		if(HmiWorkPara.WaterPartition != 0)
		{
			NumberToASCII(u8str,0);
			SetTextValue(LCD_WATERSTEP3_PAGE,1,u8str);
			SetScreen(LCD_WATERSTEP3_PAGE);
		}
	}
}

//肥水选择分区界面
void FerZoneSel(PCTRL_MSG msg, qsize size)
{
 u8 u8Str[5]={0,0,0,0,0};
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		if(u8ButtonVaule==0)
		{
			HmiWorkPara.FerPartition^=(1<<(u16Control_ID-1));
		}
		else if(u8ButtonVaule==1)
		{
			HmiWorkPara.FerPartition|=(1<<(u16Control_ID-1));
		}
	}
	else if(u16Control_ID == 18)
	{
		if(HmiWorkPara.FerPartition != 0)
		{
			SetScreen(LCD_FERSTEP3_PAGE);
			SetButtonValue1(LCD_FERSTEP3_PAGE,3,5,u8Str); //更新清水分区选择			
		}
	}
}
//清水操作第三步 设置浇水时长
void WaterTimeSet(PCTRL_MSG msg, qsize size)
{
	u8 i=0;
	u8 u8Str[7] = {0,0,0,0,0,0,0}; 
//	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	
	s16 s16RevVal  = 0;
//	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if(u16Control_ID >=4 && u16Control_ID<=9)
	{
		HmiWorkPara.Para.WaterTime = (u16Control_ID-4)*30+60;
	}
	else if(u16Control_ID == 1)
	{
		FifureTextInput ( &s16RevVal, msg );
		HmiWorkPara.Para.WaterTime = s16RevVal;
	}
	else if((u16Control_ID ==3)&&(u8ButtonVaule==1)) //启动按钮需判断按钮状态不然容易重复进入
	{
		SetRemotePara(33,HmiWorkPara.Para.WaterTime);
		ctask_record.record.task_num++;
		if(ctask_record.record.task_num>100) //最多记录100条任务
		{
			ctask_record.record.task_num=1;
		}
//		 ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;
		ctargetrcord.trecord.lasttarget_id=ctargetrcord.trecord.currenttarget_id=0;//初始化任务ID
		time_copy();//拷贝最新时间
        ctargetrcord.trecord.curtarget_mode=ctask_record.record.irrigated_mode=1; //清水灌溉		 
		ctargetrcord.trecord.curtarget_time=ctask_record.record.irrigated_time=HmiWorkPara.Para.WaterTime;//灌溉时长
		ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //保存分区
		ctask_record.record.fertilizing_amount=0;//清水灌溉不需要施肥
		ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time;
		task_resolve(ctargetrcord.trecord.targetnum,HmiWorkPara.WaterPartition);
		ctargetrcord.trecord.Ctrl_update_state=1;//有需要更新的控件
		
		for(i=0;i<16;i++)
		{
			if((HmiWorkPara.WaterPartition>>i)&0x01)
		 	{
				if((i+1)==ctargetrcord.trecord.targetnum[0])
				{
					SetTexFrontBColor(LCD_WATERSHOW_PAGE,i+1,GREEN_COLOR_HMI);//设置文本背景	
					SetRemoteRead(i,2);
					SetRemotePara(17+i,1);
				}
				else		
				{
					SetTexFrontBColor(LCD_WATERSHOW_PAGE,i+1,ORANGE_COLOR_HMI);//设置文本背景
					SetRemoteRead(i,1);
					SetRemotePara(17+i,1);
				}
			}
			else
			{
				SetTexFrontBColor(LCD_WATERSHOW_PAGE,i+1,GRAY_COLOR_HMI);//设置文本背景
				SetRemoteRead(i,0);
				SetRemotePara(17+i,0);
			}
		}
		SetRemotePara(35,1);
		SetRemoteRead(16,1);
		//上传灌溉参数
		MsgtempBlk.CmdType = UPLOAD_IRRPARA;
		MsgtempBlk.CmdSrc = SCREEN_TASK_CODE;
		PackSendRemoteQ(&MsgtempBlk);
		
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
		MsgtempBlk.CmdType = MSG_START; //发送消息告诉状态机启动任务
      	MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
      	MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
		PackSendMasterQ(&MsgtempBlk);
	}
}
void WaterShowPage(PCTRL_MSG msg, qsize size)
{
//	u8 i=0;
//	u8 u8Str[7] = {0,0,0,0,0,0,0}; 
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	
//	s16 s16RevVal  = 0;
//	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	switch(u16Control_ID)
	{
	  case 22:
		if(u8ButtonVaule==1)
		{ 
			    
						ctargetrcord.trecord.pause_flag=0;
						ctargetrcord.trecord.pausehour=0;
						ctargetrcord.trecord.pausemin=0;
						ctargetrcord.trecord.pausesec=0;	
						ctargetrcord.trecord.pausetime=0;		
					 com_conect_state=0;
				 	ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;			
            MsgtempBlk.CmdType = MSG_PAGEEND; //发送消息告诉状态机启动任务
            MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
            MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要关闭的电磁阀
            PackSendMasterQ(&MsgtempBlk);  
            if(ctask_record.record.stop_mode==0) //非正常停止才进此函数
			{
				ctask_record.record.irrigated_time=ctargetrcord.trecord.curtarget_time-ctargetrcord.trecord.currenttargetlefttime;
//		  for(i=1;i<57;i++)
//		  {
//				ctargetrcord.target[i]=0;//初始化结构体 不初始化任务号 需要保存
//			
//			}
//			ctargetrcord.trecord.currenttarget_id=ctask_record.record.task_num;	
			FlashWritetargetrecord(ctargetrcord.target,67);
			
			ctask_record.record.stop_mode=2;//界面停止
			Save_TaskRecord(ctask_record.record.task_num,0,1);
			ctask_record.record.stop_mode=0;
		}
		
		
		}
		break;
		case 23:        //暂停回复按钮
		if(u8ButtonVaule==1)
		{		
				ctargetrcord.trecord.pause_flag=1;
			  ctargetrcord.trecord.pausehour=calendar.hour;
				ctargetrcord.trecord.pausemin=calendar.min;
				ctargetrcord.trecord.pausesec=calendar.sec;
				SetTexFrontBColor(LCD_WATERSHOW_PAGE,ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id],ORANGE_COLOR_HMI);//设置文本背景
				MsgtempBlk.CmdType = MSG_STOP; //发送消息告诉状态机启动任务
				MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
				MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要关闭的电磁阀
				PackSendMasterQ(&MsgtempBlk);  		
		}
		else
		{
				ctargetrcord.trecord.pause_flag=0;
				SetTexFrontBColor(LCD_WATERSHOW_PAGE,ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id],GREEN_COLOR_HMI);//设置文本背景
				MsgtempBlk.CmdType = MSG_CONTIUE; //发送消息告诉状态机启动任务
				MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
				MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要关闭的电磁阀
				PackSendMasterQ(&MsgtempBlk); 		
		
		
		}
		break;
	
	
	}


}
//肥水第三步 选择灌溉程序
void FerPjSel(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if((u16Control_ID >=3)&&(u16Control_ID<=7))
	{
		ctargetrcord.trecord.wateringprogram=u16Control_ID-2;
		HmiWorkPara.Para.IrrPjNum = u16Control_ID-2;
		SetRemotePara(34,HmiWorkPara.Para.IrrPjNum);
	}
	else if((u16Control_ID== 2)&&(u8ButtonVaule==1))
	{
		if(HmiWorkPara.Para.IrrPjNum!=0)
		{
			//根据所选灌溉程序号显示需确认的内容
			FlashReadPara((u8*)flashset.FlashPara,0,76);
			NumberToASCII(u8Str,HmiWorkPara.Para.IrrPjNum);
			ctask_record.record.formula =u8Str[0];//配方名称
			SetTextValue(LCD_FERSTEP4_PAGE,5,u8Str);
			SetTextValue(LCD_FERSTEP4_PAGE,3,(uchar *)StaText[HmiWorkPara.Para.IrrPjNum-1]);
			NumberToASCII(u8Str,(flashset.FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+36]
							+flashset.FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+37]
							+flashset.FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+38]));
			SetTextValue(LCD_FERSTEP4_PAGE,4,u8Str);
			NumberToASCII(u8Str,flashset.FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+43]);
			SetTextValue(LCD_FERSTEP4_PAGE,6,u8Str);
			SetScreen(LCD_FERSTEP4_PAGE);
		}

	}
}

//灌溉程序读取数据
void Water_Program(u8 program)
{
//可以联合的数组直接对应 懒得找了
	u8 i=0;
	task_start_flag=1;
	ctargetrcord.trecord.fer_chanle=0;//清通道
	task_resolve(ctargetrcord.trecord.targetnum,HmiWorkPara.FerPartition);

		fercaldate.Th_A=flashset.FlashPara[0];
		fercaldate.Th_B=flashset.FlashPara[1];
		fercaldate.Th_C=flashset.FlashPara[2];
		fercaldate.Th_D=flashset.FlashPara[3];
		fercaldate.PH_A=PH_Cal(fercaldate.Th_A);
		fercaldate.PH_B=PH_Cal(fercaldate.Th_B);
		fercaldate.PH_C=PH_Cal(fercaldate.Th_C);
		fercaldate.PH_D=PH_Cal(fercaldate.Th_D);
		fercaldate.formula_A=flashset.FlashPara[39+(program-1)*8];
		fercaldate.formula_B=flashset.FlashPara[40+(program-1)*8];		
		fercaldate.formula_C=flashset.FlashPara[41+(program-1)*8];
		fercaldate.formula_D=flashset.FlashPara[42+(program-1)*8];
		amount_fer=fercaldate.amount=flashset.FlashPara[43+(program-1)*8];//记录亩均施肥量
		ctask_record.record.formula=program;	
		Chanel_Fer_Weight(flashset.FlashPara[ctargetrcord.trecord.targetnum[0]+3]);
  switch(program)
	{
		case 1:
		ctargetrcord.trecord.curtarget_front_time=flashset.set.Complex_Front_time;	//前置浇水
		ctargetrcord.trecord.curtarget_fertilizing_time=flashset.set.Complex_Fer_time;//施肥时长
		ctargetrcord.trecord.curtarget_back_time=flashset.set.Complex_Back_time;//后置清洗时长
//			fercaldate.M_A=Weight_cal(u8 pot,u16 acre,u8 amount,u8 formula);

		
		break;
		case 2:
		ctargetrcord.trecord.curtarget_front_time=flashset.set.First_Front_time;	//前置浇水
		ctargetrcord.trecord.curtarget_fertilizing_time=flashset.set.First_Fer_time;//施肥时长
		ctargetrcord.trecord.curtarget_back_time=flashset.set.First_Back_time;//后置清洗时长

		break;
		case 3:
		ctargetrcord.trecord.curtarget_front_time=flashset.set.Second_Front_time;	//前置浇水
		ctargetrcord.trecord.curtarget_fertilizing_time=flashset.set.Second_Fer_time;//施肥时长
		ctargetrcord.trecord.curtarget_back_time=flashset.set.Second_Back_time;//后置清洗时长			
		break;
		case 4:
		ctargetrcord.trecord.curtarget_front_time=flashset.set.Third_Front_time;	//前置浇水
		ctargetrcord.trecord.curtarget_fertilizing_time=flashset.set.Third_Fer_time;//施肥时长
		ctargetrcord.trecord.curtarget_back_time=flashset.set.Third_Back_time;//后置清洗时长						
		break;
		case 5:
		ctargetrcord.trecord.curtarget_front_time=flashset.set.Four_Front_time;	//前置浇水
		ctargetrcord.trecord.curtarget_fertilizing_time=flashset.set.Four_Fer_time;//施肥时长
		ctargetrcord.trecord.curtarget_back_time=flashset.set.Four_Back_time;//后置清洗时长				
		break;
		default:
		break;
	
	}
	
		for(i=0;i<4;i++)
		{
		 if(flashset.FlashPara[i+program*8+31]>0)
		 {
			ctargetrcord.trecord.fer_chanle|=(1<<i);
		 }
//		 else
//		 {
//			ctargetrcord.trecord.fer_chanle^=(1<<i);		 
//		 }
	 }
		ctargetrcord.trecord.fer_onoff=1;

//		Copy_Folwer();//拷贝水表数据
	 		 
		ctargetrcord.trecord.curtarget_time=ctargetrcord.trecord.curtarget_front_time+ctargetrcord.trecord.curtarget_fertilizing_time
		+ctargetrcord.trecord.curtarget_back_time;//灌溉总时长
}
//肥水第四步程序
void FerStart(PCTRL_MSG msg, qsize size)
{
	MsgStruct Msgtemp;
	u8 i;
	u16 u16Control_ID;				   //控件ID编号变量
//	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	u8	u8ButtonVaule = msg->param[1];
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if((u16Control_ID== 2)&&(u8ButtonVaule==1)) //启动按钮
	{
		
		Water_Program(ctargetrcord.trecord.wateringprogram);
		ctask_record.record.task_num++;
		if(ctask_record.record.task_num>100) //最多记录100条任务
		{
			ctask_record.record.task_num=1;
		}
//		ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;
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
		 	if((HmiWorkPara.FerPartition>>i)&0x01)
		  	{
				if((i+1)==ctargetrcord.trecord.targetnum[0])
				{
					SetTexFrontBColor(LCD_FERSHOW_PAGE,i+1,GREEN_COLOR_HMI);//设置文本背景	
					SetRemoteRead(i,2);
					SetRemotePara(17+i,1);
				}
				else	
				{
					SetTexFrontBColor(LCD_FERSHOW_PAGE,i+1,ORANGE_COLOR_HMI);//设置文本背景
					SetRemoteRead(i,1);
					SetRemotePara(17+i,1);
		 		}
			}
			else
			{
				SetTexFrontBColor(LCD_FERSHOW_PAGE,i+1,GRAY_COLOR_HMI);//设置文本背景
				SetRemoteRead(i,0);
				SetRemotePara(17+i,0);
			}
		}
		SetRemotePara(35,1);
		SetRemoteRead(16,2);
		SetRemoteRead(18,ctargetrcord.trecord.wateringprogram);
		//上传灌溉参数
		Msgtemp.CmdType = UPLOAD_IRRPARA;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		PackSendRemoteQ(&Msgtemp);
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
		MsgtempBlk.CmdType = MSG_START; //发送消息告诉状态机启动任务
      	MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
      	MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
		PackSendMasterQ(&MsgtempBlk);	
		Use_A_Kg=0;
		Use_B_Kg=0;
		Use_C_Kg=0;
		Use_D_Kg=0;

	}
}

//肥水显示程序
void FerShowPage(PCTRL_MSG msg, qsize size)
{
//	u8 i=0;
//	u8 u8Str[7] = {0,0,0,0,0,0,0}; 
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	
//	s16 s16RevVal  = 0;
//	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	switch(u16Control_ID)
	{
	  case 22:
		if(u8ButtonVaule==1)        
		{ 
			ctargetrcord.trecord.Ctrl_update_state=0;
			gu8Use_MA_Check_Flag=0;
			Openboard2=0;
            MsgtempBlk.CmdType = MSG_PAGEEND; //发送消息告诉状态机启动任务
            MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
            MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀  
            PackSendMasterQ(&MsgtempBlk);
			Use_A_Kg=0;
			Use_B_Kg=0;
			Use_C_Kg=0;
			Use_D_Kg=0;
        	if(ctask_record.record.stop_mode==0) //非正常停止才进此函数
			{
				ctask_record.record.irrigated_time=ctargetrcord.trecord.curtarget_time-ctargetrcord.trecord.currenttargetlefttime;
				ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;		
  				FlashWritetargetrecord(ctargetrcord.target,67);
			
				ctask_record.record.stop_mode=2;//界面停止
				Save_TaskRecord(ctask_record.record.task_num,0,1);
				ctask_record.record.stop_mode=0;
			}
		
		
		}
		break;
		case 23:        //暂停回复按钮
		break;
	
	
	}

}
//设置第一步肥料罐浓度设置
void PercentSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8 u8ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if((u16Control_ID>=1)&&(u16Control_ID<=4))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1] = s16RevVal;
	}
	else if(u16Control_ID == 7)
	{
		for(i=4;i<=19;i++)
		{
				if(flashset.FlashPara[i]<0xf000)
				{
					NumberToASCII(u8Str,flashset.FlashPara[i]);
				if(u8Str[3]>0)
				{
					u32ShowBuf[i-4] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
				}
				else if(u8Str[2]>0)
				{
					u32ShowBuf[i-4] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
				}
				else if(u8Str[1]>0)
				{
					u32ShowBuf[i-4] = u8Str[0]<<8|u8Str[1];
				}
				else if(u8Str[0]>0)
				{
					u32ShowBuf[i-4] = u8Str[0];
				}
			}
			 else
			 {
				 
				u8ShowBuf[i-4]=1;
				u32ShowBuf[i-4] =65536-flashset.FlashPara[i]; 
				if(u32ShowBuf[i-4]<10)
				{	
					u8Str[0] =0x2e;
					u8Str[1] =(65536-flashset.FlashPara[i])%10+0X30;	
					u32ShowBuf[i-4]=u8Str[0]<<8|u8Str[1];
   			}	
				else if((u32ShowBuf[i-4]>=10)&&(u32ShowBuf[i-4]<100))
				{	
					u8Str[0] =((65536-flashset.FlashPara[i])/10)%10+0X30;	
					u8Str[1] =0x2e;
					u8Str[2] =(65536-flashset.FlashPara[i])%10+0X30;	
					u32ShowBuf[i-4]=u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
   			}
				else if((u32ShowBuf[i-4]>=100)&&(u32ShowBuf[i-4]<1000))
				{	
					u8Str[0] =((65536-flashset.FlashPara[i])/100)%10+0X30;	
					u8Str[1] =((65536-flashset.FlashPara[i]))%100/10+0X30;	
					u8Str[2] =0x2e;
					u8Str[3] =(65536-flashset.FlashPara[i])%10+0X30;	
					u32ShowBuf[i-4]=u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
   			}					
			 }
		}
		SetMulSerialText3(LCD_SETSTEP2_PAGE,1,16,u32ShowBuf,u8ShowBuf);
	}
}
//设置第二步 种植面积设置
void AreaSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1+4] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=36;i<=43;i++)
		{
			NumberToASCII(u8Str,flashset.FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-36] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-36] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-36] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-36] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP3_PAGE,1,8,u32ShowBuf);
	}
	
}
//设置第三步 复合肥灌溉程序
void ComFerSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1+36] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=44;i<=51;i++)
		{
			NumberToASCII(u8Str,flashset.FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-44] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-44] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-44] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-44] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP4_PAGE,1,8,u32ShowBuf);
	}
	
}
//设置第四步 1阶段灌溉程序
void Stage1Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1+44] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=52;i<=59;i++)
		{
			NumberToASCII(u8Str,flashset.FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-52] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-52] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-52] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-52] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP5_PAGE,1,8,u32ShowBuf);
	}
	
}
//设置第五步 2阶段灌溉程序
void Stage2Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1+52] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=60;i<=67;i++)
		{
			NumberToASCII(u8Str,flashset.FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-60] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-60] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-60] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-60] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP6_PAGE,1,8,u32ShowBuf);
	}
	
}
//设置第六步 3阶段灌溉程序
void Stage3Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1+60] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=68;i<=75;i++)
		{
			NumberToASCII(u8Str,flashset.FlashPara[i]);
			if(u8Str[3]>0)
			{
				u32ShowBuf[i-68] = u8Str[0]<<24|u8Str[1]<<16|u8Str[2]<<8|u8Str[3];
			}
			else if(u8Str[2]>0)
			{
				u32ShowBuf[i-68] = u8Str[0]<<16|u8Str[1]<<8|u8Str[2];
			}
			else if(u8Str[1]>0)
			{
				u32ShowBuf[i-68] = u8Str[0]<<8|u8Str[1];
			}
			else if(u8Str[0]>0)
			{
				u32ShowBuf[i-68] = u8Str[0];
			}
		}
		SetMulSerialText2(LCD_SETSTEP7_PAGE,1,8,u32ShowBuf);
	}
}
//设置第七步 4阶段灌溉程序
void Stage4Set(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		flashset.FlashPara[u16Control_ID-1+68] = s16RevVal;
	}
}
//保存
void SaveConfim(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	MsgStruct Msgtemp;
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID

	if(u16Control_ID == 1&&u8ButtonVaule == 1)
	{
		//FlashWriteWiFi((u8*)g_u8WifiPara);
		FlashWritePara((u8*)flashset.FlashPara,76);
		Msgtemp.CmdType = UPLOAD_PARTITIONARE;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
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
	}
}
//任务记录保存函数读取函数 
//tasknum要查看的任务号
//read 1读
//write 1写
void Save_TaskRecord(u8 tasknum,u8 read,u8 write)
{
 u8 templ=0;

if(write==1)
{
	  for(templ=0;templ<16;templ++)
		task_record_buff[(tasknum-1)*16+templ]=ctask_record.task[templ]; //上电需要先读
		FlashWritetaskrecord(task_record_buff,ADDR_TASKRECORD ,1600);
}
else if(write==2)
{
	
	memset(task_record_buff,0xff,1600);
	FlashWritetaskrecord(task_record_buff,ADDR_TASKRECORD ,1600);
	memset(ctargetrcord.target,0x00,67);
	FlashWritetargetrecord(ctargetrcord.target,67);
	
}
else 
{
	if(read==1)
		FlashReadtaskrecord(task_record_buff,ADDR_TASKRECORD ,1600);
	if(tasknum>0)
	{
		for(templ=0;templ<16;templ++)
		ctask_record.task[templ]=task_record_buff[(tasknum-1)*16+templ];
	}	

}

}

//任务记录显示调用子函数
void task_display(u8* u8Str)
{
//		uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			NumberToASCII(u8Str,ctask_record.record.start_year);
			SetTextValue(LCD_TASKRECORD_PAGE,6,u8Str);
			NumberToASCII(u8Str,ctask_record.record.start_month);
			SetTextValue(LCD_TASKRECORD_PAGE,7,u8Str);
			NumberToASCII(u8Str,ctask_record.record.start_day);
			SetTextValue(LCD_TASKRECORD_PAGE,8,u8Str);
			NumberToASCII(u8Str,ctask_record.record.start_hour);
			SetTextValue(LCD_TASKRECORD_PAGE,9,u8Str);
			NumberToASCII(u8Str,ctask_record.record.start_minitue);
			SetTextValue(LCD_TASKRECORD_PAGE,10,u8Str);
			NumberToASCII(u8Str,ctask_record.record.start_second);
			SetTextValue(LCD_TASKRECORD_PAGE,11,u8Str);
            switch(ctask_record.record.irrigated_mode)
			{
			  case 1:
		
//				SetTextValue(LCD_TASKRECORD_PAGE,12,(uchar *)StaText[5]);
				NumberToASCII(u8Str,ctask_record.record.irrigated_time);
				SetTextValue(LCD_TASKRECORD_PAGE,13,u8Str);
				NumberToASCII(u8Str,0);
				SetTextValue(LCD_TASKRECORD_PAGE,14,u8Str);	
				NumberToASCII(u8Str,0);
				SetTextValue(LCD_TASKRECORD_PAGE,15,u8Str);
		
				break;
				case 2:
				NumberToASCII(u8Str,ctask_record.record.irrigated_time);
				SetTextValue(LCD_TASKRECORD_PAGE,13,u8Str);
				NumberToASCII(u8Str,ctask_record.record.formula);
				SetTextValue(LCD_TASKRECORD_PAGE,14,u8Str);	
				if(ctask_record.record.fertilizing_amount<10)
				{	
					u8Str[0] =0x2e;
					u8Str[1] =(ctask_record.record.fertilizing_amount)%10+0X30;	
//					u32ShowBuf[0]=u8Str[0]<<8|u8Str[1];
   			}	
				else if((ctask_record.record.fertilizing_amount>=10)&&(ctask_record.record.fertilizing_amount<100))
				{	
					u8Str[0] =((ctask_record.record.fertilizing_amount)/10)%10+0X30;	
					u8Str[1] =0x2e;
					u8Str[2] =(ctask_record.record.fertilizing_amount)%10+0X30;	

   			}
				else if((ctask_record.record.fertilizing_amount>=100)&&(ctask_record.record.fertilizing_amount<1000))
				{	
					u8Str[0] =((ctask_record.record.fertilizing_amount)/100)%10+0X30;	
					u8Str[1] =((ctask_record.record.fertilizing_amount))%100/10+0X30;	
					u8Str[2] =0x2e;
					u8Str[3] =(ctask_record.record.fertilizing_amount)%10+0X30;	

   			}	
				SetTextValue(LCD_TASKRECORD_PAGE,15,u8Str);  
               
				break;
				default:
				break;
			
			}
							NumberToASCII(u8Str,ctask_record.record.select_zone);
				SetTextValue(LCD_TASKRECORD_PAGE,17,u8Str);     
            SetTextValue(LCD_TASKRECORD_PAGE,12,(uchar *)StaText[ctask_record.record.irrigated_mode+4]);
			SetTextValue(LCD_TASKRECORD_PAGE,16,(uchar *)StaText[ctask_record.record.stop_mode+6]);					



}
u8 TaskNumTemp;
//任务记录显示页面
void Display_taskrecord(PCTRL_MSG msg, qsize size)
{
//	u8 i;
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
 if((u16Control_ID==1)&&(u8ButtonVaule==1))
  {    
  		if(last_display_tasknum>0)
	    	last_display_tasknum--;
		  if(last_display_tasknum==0)
			{
				last_display_tasknum=100; //此处需要改成100若需要可以扩展
			
			}
			Save_TaskRecord(last_display_tasknum,0,0); //读取更新任务记录
			if(ctask_record.record.task_num>0&&ctask_record.record.task_num<100)
			task_display(u8Str);	 
			else
				last_display_tasknum = 1;
 
 }
 else if((u16Control_ID==2)&&(u8ButtonVaule==1))
 {
	    last_display_tasknum++;
		  if(last_display_tasknum>100)
			{
				last_display_tasknum=1;
			
			}
			Save_TaskRecord(last_display_tasknum,0,0); //读取更新任务记录
			if(ctask_record.record.task_num>0&&ctask_record.record.task_num<100)
				task_display(u8Str);	 
			else
				last_display_tasknum--;

 }
 else if((u16Control_ID==3)&&(u8ButtonVaule==1))
 {

			Save_TaskRecord(last_display_tasknum,0,0); //读取更新任务记录
			if(ctask_record.record.task_num>0&&ctask_record.record.task_num<100)
				task_display(u8Str);	 
			else
				last_display_tasknum=1;

 } 
  else if(u16Control_ID==5)
 {
	    last_display_tasknum=(u8)ASCIITONUMBER(msg);
 }
else if(u16Control_ID == 4)
{
	ctask_record.record.task_num = TaskNumTemp;
}


}
//用户选择界面
void User_Select(PCTRL_MSG msg, qsize size)
{
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID

	if((u16Control_ID==3)&&(u8ButtonVaule==0))
  {    
	    last_display_tasknum=ctask_record.record.task_num;
			Save_TaskRecord(last_display_tasknum,1,0); //读取更新任务记录
			task_display(u8Str);	
			TaskNumTemp = last_display_tasknum;


 
 }


}
/******************************************
	名称:密码输入
	输入:msg
	密码正确 跳转相应界面 
	密码错误返回上一界面
*******************************************/
void Guest_PasswordVerify (  PCTRL_MSG msg )
{
	if ( 

	   ( strcmp ((( ( char*)msg->param  )), ( char*)guest_password)!= 0 )
		)
		{
			Password_flag=1;			
		}
	else
		{
//			Password_flag=0;
		}
}
/******************************************
	名称:密码输入
	输入:msg
	密码正确 跳转相应界面 
	密码错误返回上一界面
*******************************************/
void Admin_PasswordVerify (  PCTRL_MSG msg )
{
	if ( 

	   ( strcmp ((( ( char*)msg->param  )), ( char*)admin_password)!= 0 )
		)
		{
			Password_flag=1;			
		}
	else
		{
//			Password_flag=0;
		}
}
//密码验证 根据不同的画面给的信息进行密码验证
void PassWordInput ( PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				   //控件ID编号变量
	u16 u16Screen_ID;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	u16Screen_ID = PTR2U16 ( &msg->screen_id); //控件ID
	if(u16Screen_ID==25)
	{	switch( u16Control_ID )
	{
		case 4://密码输入框
			Guest_PasswordVerify( msg );
			break;
    case 2: //验证密码
		 if(msg->param[1]==0x01) //确定按钮按下
		 { if(Password_flag==1)  

			 { 
				 SetScreen(36);
				Display_SetButonn();
			 }
		 else SetScreen(26);
				 Password_flag=0; //清验证标志		
		 }
		 break;
		default:
			break;
	}
}
	else if(u16Screen_ID==27)
	{	switch( u16Control_ID )
	{
		case 3://密码输入框
			Admin_PasswordVerify( msg );
			break;
    case 2: //验证密码
		 if(msg->param[1]==0x01) //确定按钮按下
		 {
	   if(Password_flag==1)
			 { 
				 SetScreen(36);
				Display_SetButonn();
			 }
		 else SetScreen(28);
		 Password_flag=0; //清验证标志			 
		 }
		 break;
		default:
			break;
	}
}
	else if(u16Screen_ID==30)
	{	switch( u16Control_ID )
	{
		case 3://密码输入框
			Guest_PasswordVerify( msg );
			break;
    case 2: //验证密码
		 if(msg->param[1]==0x01) //确定按钮按下
		 {			
	   if(Password_flag==1)
		 {
			 SetScreen(31);

		 }
		 else SetScreen(34);
			 Password_flag=0;		 
		}
		 break;
		default:
			break;
	}
}
}
void Check_TaskRecord(PCTRL_MSG msg, qsize size)
{



}
//显示设置界面
void Display_SetButonn(void)
{
		u8 u8Str[7];
 		NumberToASCII(u8Str,set_Pressbutonn[0]);
 		SetTextValue ( LCD_SETBUTONN_PAGE, 1, u8Str );
		SetButtonValue ( LCD_SETBUTONN_PAGE, 2, set_Pressbutonn[1] );
}
//设置界面
void Setbutonn_Page(PCTRL_MSG msg, qsize size)
{
	s16 s16RevVal=0;
 	u16 u16Control_ID;				   //控件ID编号变量
//	u16 u16Screen_ID;
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
//	u16Screen_ID = PTR2U16 ( &msg->screen_id); //控件ID
	
	if(u16Control_ID==1)
	{
	 	FifureTextInput ( &s16RevVal, msg );
		set_Pressbutonn[0] = (u8)s16RevVal;
	
	}
	if(u16Control_ID==2)
	{
		set_Pressbutonn[1]=msg->param[1];
	
	}
	 if((u16Control_ID==3)&&(u8ButtonVaule==1))
	 {
	 	W25QXX_Write ( set_Pressbutonn, ADDR_SETBUTONN, 2 );
		FlashReadWiFi(g_u8WifiPara);
		FlashReadDHCP(&g_u8DHCP);
		//屏幕显示设置内容
		SetTextValue(LCD_WIFI_PAGE,1,&g_u8WifiPara[0]);
		SetTextValue(LCD_WIFI_PAGE,2,&g_u8WifiPara[31]);
		SetTextValue(LCD_WIFI_PAGE,3,&g_u8WifiPara[62]);
		SetTextValue(LCD_WIFI_PAGE,4,&g_u8WifiPara[78]);
		SetTextValue(LCD_WIFI_PAGE,5,&g_u8WifiPara[94]);
		if(g_u8DHCP == 1)
		{
			NumberToASCII(u8Str,g_u8DHCP);
		}
		else 
		{
			u8Str[0]=0x30;
		}
		SetTextValue(LCD_WIFI_PAGE,6,u8Str);
	 }


}

void WifiSetPage(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	s16 s16RevVal  = 0;
//	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if(u16Control_ID == 1)
	{
		memcpy(g_u8WifiPara,msg->param,(size-12));
		memcpy(ssid,msg->param,(size-12));
	}
	else if(u16Control_ID == 2)
	{
		memcpy(&g_u8WifiPara[31],msg->param,(size-12));
		memcpy(password,msg->param,(size-12));
	}
	else if(u16Control_ID == 3)
	{
		memcpy(&g_u8WifiPara[62],msg->param,(size-12));
		memcpy(ipaddr,msg->param,(size-12));
	}
	else if(u16Control_ID == 4)
	{
		memcpy(&g_u8WifiPara[78],msg->param,(size-12));
		memcpy(subnet,msg->param,(size-12));
	}
	else if(u16Control_ID == 5)
	{
		memcpy(&g_u8WifiPara[94],msg->param,(size-12));
		memcpy(gateway,msg->param,(size-12));
	}
	else if(u16Control_ID == 6)
	{
		FifureTextInput ( &s16RevVal, msg );
		g_u8DHCP = s16RevVal;
	}
	else if(u16Control_ID == 8)
	{
		memcpy(&g_u8WifiPara[110],msg->param,(size-12));
		memcpy(dns,msg->param,(size-12));
	}
	
}
//告警界面
void Error_Page(PCTRL_MSG msg, qsize size)
{



}
void WiFiSaveConfim(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
//	MsgStruct Msgtemp;
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if(u16Control_ID == 1&&u8ButtonVaule == 1)
	{
		FlashWriteWiFi((u8*)g_u8WifiPara);
		FlashWriteDHCP(&g_u8DHCP);
		g_u8WiFiRestart = 1 ;
	}
}
//接受触摸屏消息
void ReceiveFromMonitor( void )
{
	uint8 cmd_type = 0;
	uint8 msg_type = 0;
	PCTRL_MSG msg = 0;
	qsize size = 0;
	size = queue_find_cmd ( Hmi_cmd_buffer, CMD_MAX_BUFFER ); //从缓冲区中获取一条指令
	if ( size <= 10 ) //没有接收到指令
		return;
	msg = ( PCTRL_MSG ) Hmi_cmd_buffer;
	cmd_type = msg->cmd_type;
	switch ( cmd_type ) //指令类型
	{
		case CMD_GIFMOVE_END:
			break;
		case CMD_READ_TIME:
//		 ctask_record.task_num++;
//		 ctask_record.start_year=2000+msg->param[0];
//     ctask_record.start_month=msg->param[1];	
//		 ctask_record.start_day=msg->param[2];
//		 ctask_record.start_hour=msg->param[3];
//     ctask_record.start_minitue=msg->param[4];
//     ctask_record.start_second=	msg->param[5];	
		break;
		case CMD_HAND_SHAKE:
			break;
		case CMD_TYPE_UPDATE_CONTROL://控件更新消息
			msg_type = msg->ctrl_msg;
			switch ( msg_type ) //消息类型
			{
				case kCtrlMsgAnimationEnd://动画结束
					break;
				case kCtrlMsgCountdownEnd://倒计时控件到
					break;
				case kCtrlMsgGetCurrentScreen:
					break;
				case kCtrlMsgGetMenuData:
				case kCtrlMsgGetData:
					
					s_screen_id = PTR2U16 ( &msg->screen_id );
					switch( s_screen_id )
					{
						case LCD_STANDBY_PAGE://待机界面
							StangbyPage(msg,size);
							break;
						case LCD_START_PAGE ://启动界面
							IrrOrSet(msg,size);
							break;
						case LCD_IRRSTEP1_PAGE://灌溉操作第一步
							IRRSTEP(msg,size);
							break;
						case LCD_WATERSTEP2_PAGE://清水灌溉第二步
							WaterZoneSel(msg,size);
							break;
						case LCD_FERSTEP2_PAGE://肥水操作第二步
							FerZoneSel(msg,size);
							break;
						case LCD_WATERSTEP3_PAGE://清水操作第三步
							WaterTimeSet(msg,size);
							break;		
						case LCD_FERSTEP3_PAGE://肥水操作第三步
							FerPjSel(msg,size);
							break;							
						case LCD_FERSTEP4_PAGE://肥水操作第四步
							FerStart(msg, size);//启动肥水灌溉
							break;	
						case LCD_WATERSHOW_PAGE://清水显示
							WaterShowPage(msg,size);
							break;
						case LCD_FERSHOW_PAGE://肥水显示
							FerShowPage(msg,size);
							break;
						case LCD_SETSTEP1_PAGE://设置第一步
							PercentSet(msg,size);
							break;
						case LCD_SETSTEP2_PAGE://设置第二步
							AreaSet(msg,size);
							break;
						case LCD_SETSTEP3_PAGE://设置第三步
							ComFerSet(msg,size);
							break;
						case LCD_SETSTEP4_PAGE://设置第四步
							Stage1Set(msg,size);
							break;
						case LCD_SETSTEP5_PAGE://设置第五步
							Stage2Set(msg,size);
							break;
						case LCD_SETSTEP6_PAGE://设置第六步
							Stage3Set(msg,size);
							break;
						case LCD_SETSTEP7_PAGE://设置第七步
							Stage4Set(msg,size);
							break;
						case LCD_SAVE1_PAGE://浓度保存
						case LCD_SAVE2_PAGE://分区面积保存
						case LCD_SAVE3_PAGE://复合肥保存
						case LCD_SAVE4_PAGE://一阶段保存
						case LCD_SAVE5_PAGE://二阶段保存
						case LCD_SAVE6_PAGE://三阶段保存
						case LCD_SAVE7_PAGE://四阶段保存
							SaveConfim(msg,size);
							break;
						case LCD_USESELECT_PAGE:  //用户选择					
								User_Select(msg,size);
							break;
						case LCD_GUESTLOAD_PAGE:  //访客登录界面
							PassWordInput ( msg, size );
							break;
							case LCD_GUESTLOADERROR_PAGE: //访客登录密码错误
							break;
						case LCD_ADMINLOAD_PAGE: //管理员登录界面
							PassWordInput ( msg, size );
							break;
							case LCD_ADMINLOADERROR_PAGE: //管理员登录界面错误
							break;
						case LCD_TASKRECORD_PAGE: //任务记录
							Display_taskrecord( msg, size );
							break;	
							case LCD_GUESTVERIYFY_PAGE: //旧密码校验
								PassWordInput ( msg, size );
							break;
						case LCD_NEWPASSWORD_PAGE: //新密码输入
							break;
						case LCD_PASSWORDSAVE_PAGE: //保存成功提示
							break;
						case LCD_UNCOMPLETWORK_PAGE: //未完成任务提示
							break;	
							case LCD_PASSWORDERROR_PAGE: //密码错误提示
							break;
						case LCD_NOTSAVE_PAGE:  //未保存提示 LCD_ERROR_PAGE
							break;		
							case LCD_SETBUTONN_PAGE: //密码错误提示
								Setbutonn_Page( msg,size);
							break;
						case LCD_ERROR_PAGE:  //未保存提示 LCD_ERROR_PAGE
							break;	
						case LCD_WIFI_PAGE:
							WifiSetPage( msg,size);
							break;
						case LCD_WIFISAVE_PAGE:
							WiFiSaveConfim( msg,size);
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		default:
			break;
	}
}
u8 TimeUpdateCnt = 0;
u8 Task_percent = 0;//任务执行进度百分比
u8 g_u8LastMin  = 0;
u8 g_u8UpCycle = 1;
/*接受AGV的触摸屏显示器交互信息*/
void Task_HMIMonitor ( void * parg )
{
	u8 u8Str[7] = {0,0,0,0,0,0,0}; 
	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
//	MsgComStruct Msgtemp1;
	MsgComStruct MsgtempBlk1;		
	parg = parg;
	//SetScreen(1);
	FlashWriteDHCP(&g_u8DHCP);
	while ( 1 ) 
	{
		delay_ms(100);
		display_count++;
		fer_display_count++;
		ReceiveFromMonitor();
		//由于keil不支持直接将浮点数装换成字符串只能先转换成整数两个%d之间有个点就是小数点
//		sprintf(&u8chardatabuff[0],"%d.%d",(unsigned int)i,(unsigned int)((i-(unsigned int)i)*10));
//		SetTextValue(36,1,u8chardatabuff);

	 if(ctargetrcord.trecord.curtarget_mode==1)//若是清水灌溉需刷新的界面
	 {
		 if(ctargetrcord.trecord.Ctrl_update_state==1)//倒计时计算
		 {
			ctargetrcord.trecord.tasktimeing=Tasking_time(calendar.hour,calendar.min,calendar.sec,ctargetrcord.trecord.curtargethour,ctargetrcord.trecord.curtargetmin,ctargetrcord.trecord.curtargetsec);
			ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time+ctargetrcord.trecord.pausetime-ctargetrcord.trecord.tasktimeing;
			if(g_u8LastMin == 0)
			{
				g_u8LastMin=ctargetrcord.trecord.currenttargetlefttime;
				SetRemoteRead(17,g_u8LastMin);
				Msgtemp.CmdType = UPLOAD_IRRSTATE;
				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
				PackSendRemoteQ(&Msgtemp);
			}
			else if(g_u8LastMin-ctargetrcord.trecord.currenttargetlefttime>=g_u8UpCycle)
			{
				g_u8LastMin=ctargetrcord.trecord.currenttargetlefttime;
				SetRemoteRead(17,g_u8LastMin);
				Msgtemp.CmdType = UPLOAD_IRRSTATE;
				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
				PackSendRemoteQ(&Msgtemp);
			}
			
		 }
		 if((ctargetrcord.trecord.Ctrl_update_state==0)&&(ctargetrcord.trecord.pause_flag==1))
		 {
			ctargetrcord.trecord.pausetime=Tasking_time(calendar.hour,calendar.min,calendar.sec,ctargetrcord.trecord.pausehour,ctargetrcord.trecord.pausemin,ctargetrcord.trecord.pausesec);
		 	//每个任务只允许暂停一次若是重复暂停需要重复记录暂停时间
			
		 }
     	if(display_count>10)	//1S刷新一次	 
		 //此函数可能存在一定问题若是跨月跨天可能会出现bug
		{
			display_count=0;
		 	if((ctargetrcord.trecord.Ctrl_update_state==1)&&(ctargetrcord.trecord.currenttargetlefttime==0))
		 	{
			 
			 	if(ctargetrcord.trecord.currenttarget_id+1<=ctargetrcord.trecord.curtarget_max_num)
				{  
					ctargetrcord.trecord.pause_flag=0;
					ctargetrcord.trecord.pausehour=0;
					ctargetrcord.trecord.pausemin=0;
					ctargetrcord.trecord.pausesec=0;	
					ctargetrcord.trecord.pausetime=0;			
					MsgtempBlk.CmdType = MSG_END; //发送消息告诉状态机启动任务
					MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
					MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
					PackSendMasterQ(&MsgtempBlk);
					ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;
					FlashWritetargetrecord(ctargetrcord.target,67);
	//				ctask_record.record.task_num++;
					ctask_record.record.stop_mode=1;//完成停止
					ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //保存分区					 
					Save_TaskRecord(ctask_record.record.task_num,0,1);//更新任务记录
	          		ctargetrcord.trecord.currenttarget_id+=1;
					ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //保存分区					 
					ctask_record.record.stop_mode=0;//更新停止模式
					time_copy();//拷贝最新时间
					 
					SetTexFrontBColor(LCD_WATERSHOW_PAGE,ctargetrcord.trecord.targetnum[ctargetrcord.trecord.lasttarget_id],RED_COLOR_HMI);//设置文本背景	
					SetRemoteRead(ctargetrcord.trecord.targetnum[ctargetrcord.trecord.lasttarget_id]-1,3);
	                if(ctargetrcord.trecord.currenttarget_id<ctargetrcord.trecord.curtarget_max_num) //任务已经结束不需要更新此控件
	                {
						SetTexFrontBColor(LCD_WATERSHOW_PAGE,ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id],GREEN_COLOR_HMI);//设置文本背景
						SetRemoteRead(ctargetrcord.trecord.targetnum[ctargetrcord.trecord.lasttarget_id]-1,2);
	                }
					ctargetrcord.trecord.Ctrl_update_state=0;//控件不需要更新
					Msgtemp.CmdType = UPLOAD_IRRSTATE;
					Msgtemp.CmdSrc = SCREEN_TASK_CODE;
					PackSendRemoteQ(&Msgtemp);
				}
			}
			NumberToASCII(u8Str,ctargetrcord.trecord.currenttargetlefttime);
			SetTextValue(LCD_WATERSHOW_PAGE,17,u8Str); //更新剩余时间
		}
	 }
	 else if(ctargetrcord.trecord.curtarget_mode==2)//若是肥水灌溉需要刷新的界面
	 {
	 	if(ctargetrcord.trecord.Ctrl_update_state==1)
		{	
			ctargetrcord.trecord.tasktimeing=Tasking_time(calendar.hour,calendar.min,calendar.sec,ctargetrcord.trecord.curtargethour,ctargetrcord.trecord.curtargetmin,ctargetrcord.trecord.curtargetsec);
			ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time-ctargetrcord.trecord.tasktimeing;	
			
			if(g_u8LastMin == 0)
			{
				g_u8LastMin=ctargetrcord.trecord.currenttargetlefttime;
				SetRemoteRead(17,g_u8LastMin);
				Msgtemp.CmdType = UPLOAD_IRRSTATE;
				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
				PackSendRemoteQ(&Msgtemp);
			}
			else if(g_u8LastMin-ctargetrcord.trecord.currenttargetlefttime>=g_u8UpCycle)
			{
				g_u8LastMin=ctargetrcord.trecord.currenttargetlefttime;
				SetRemoteRead(17,g_u8LastMin);
				Msgtemp.CmdType = UPLOAD_IRRSTATE;
				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
				PackSendRemoteQ(&Msgtemp);
			}
			if((ctargetrcord.trecord.curtarget_front_time<=(ctargetrcord.trecord.curtarget_time-ctargetrcord.trecord.currenttargetlefttime))&&(Openboard2==0))
			{
				gu8Use_MA_Check_Flag=1;
				ctargetrcord.trecord.fer_onoff=1;//更新水泵状态
				Copy_Folwer();
				Openboard2=1;//开启板卡2
				MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
				MsgtempBlk1.CmdData[0]= 2;//串口4发送
				MsgtempBlk1.CmdData[1]= 1;//开启施肥
				PackSendComQ(&MsgtempBlk1); 
			}
		}
		if((ctargetrcord.trecord.curtarget_fertilizing_time>=(ctargetrcord.trecord.curtarget_time-ctargetrcord.trecord.currenttargetlefttime-ctargetrcord.trecord.curtarget_front_time))&&(Closeboard2==0))
		{
			
		 //
		
		}		

     if(display_count>10)	//1S刷新一次	 
		 //此函数可能存在一定问题若是跨月跨天可能会出现bug
		{
			display_count=0;
		 if((ctargetrcord.trecord.Ctrl_update_state==1)&&(ctargetrcord.trecord.currenttargetlefttime==0))
		 {
			 
			 if(ctargetrcord.trecord.currenttarget_id+1<=ctargetrcord.trecord.curtarget_max_num)
			 {  
				 	MsgtempBlk.CmdType = MSG_END; //发送消息告诉状态机启动任务
					MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
					MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
				 	PackSendMasterQ(&MsgtempBlk);
				 	Water_Program(ctargetrcord.trecord.wateringprogram);

				 if(flashset.FlashPara[ctask_record.record.select_zone+3]>0xf000)
					ctask_record.record.fertilizing_amount=65536- flashset.FlashPara[ctask_record.record.select_zone+3];
				 else
					ctask_record.record.fertilizing_amount=flashset.FlashPara[ctask_record.record.select_zone+3]; 
					ctask_record.record.fertilizing_amount=amount_fer*ctask_record.record.fertilizing_amount;//总施肥量
				 	ctask_record.record.stop_mode=1;//完成停止
				 	ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;
					FlashWritetargetrecord(ctargetrcord.target,67);
//				 ctask_record.record.task_num++;
					ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //先存后变更
					Save_TaskRecord(ctask_record.record.task_num,0,1);//更新任务记录
					ctargetrcord.trecord.currenttarget_id+=1;
					ctask_record.record.select_zone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id]; //保存分区
					ctask_record.record.stop_mode=0;//更新停止模式
					Use_A_Kg=0;
					Use_B_Kg=0;
					Use_C_Kg=0;
					Use_D_Kg=0;
				  	Copy_Folwer();//拷贝水流数据
					Openboard2=0;
					time_copy();//拷贝最新时间
					SetTexFrontBColor(LCD_FERSHOW_PAGE,ctargetrcord.trecord.targetnum[ctargetrcord.trecord.lasttarget_id],RED_COLOR_HMI);//设置文本背景	
					SetRemoteRead(ctargetrcord.trecord.targetnum[ctargetrcord.trecord.lasttarget_id]-1,3);
                    if(ctargetrcord.trecord.currenttarget_id<ctargetrcord.trecord.curtarget_max_num) //任务已经结束不需要更新此控件
                    {
						SetTexFrontBColor(LCD_FERSHOW_PAGE,ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id],GREEN_COLOR_HMI);//设置文本背景
						SetRemoteRead(ctargetrcord.trecord.targetnum[ctargetrcord.trecord.lasttarget_id]-1,2);
                    }
					ctargetrcord.trecord.Ctrl_update_state=0;//控件不需要更新
					Msgtemp.CmdType = UPLOAD_IRRSTATE;
					Msgtemp.CmdSrc = SCREEN_TASK_CODE;
					PackSendRemoteQ(&Msgtemp);
			 }
		}
		NumberToASCII(u8Str,ctargetrcord.trecord.currenttargetlefttime);
		SetTextValue(LCD_FERSHOW_PAGE,17,u8Str); //更新剩余时间
		//由于keil不支持直接将浮点数装换成字符串只能先转换成整数两个%d之间有个点就是小数点
		sprintf(&u8Str[0],"%d.%d",(unsigned int)fercaldate.USE_M_A,(unsigned int)((fercaldate.USE_M_A-(unsigned int)fercaldate.USE_M_A)*10));
		SetTextValue(LCD_FERSHOW_PAGE,18,u8Str);
		sprintf(&u8Str[0],"%d.%d",(unsigned int)fercaldate.USE_M_B,(unsigned int)((fercaldate.USE_M_B-(unsigned int)fercaldate.USE_M_B)*10));
		SetTextValue(LCD_FERSHOW_PAGE,19,u8Str);
		sprintf(&u8Str[0],"%d.%d",(unsigned int)fercaldate.USE_M_C,(unsigned int)((fercaldate.USE_M_C-(unsigned int)fercaldate.USE_M_C)*10));
		SetTextValue(LCD_FERSHOW_PAGE,20,u8Str);
		sprintf(&u8Str[0],"%d.%d",(unsigned int)fercaldate.USE_M_D,(unsigned int)((fercaldate.USE_M_D-(unsigned int)fercaldate.USE_M_D)*10));
		SetTextValue(LCD_FERSHOW_PAGE,21,u8Str);


		}	 
	 
	 }
	}
}

