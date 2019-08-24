#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
//#include "GM3Dir.h"
#include "hmi_driver.h"
#include "string.h"

extern OS_TMR	CallerTmr;
#define CMD_MAX_BUFFER 128
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //串口命令接收缓冲区
u16 s_screen_id;                  //页面ID编号变量
u16 s_control_id;                 //控件ID编号变量
u8 butonn1_state=0; //按钮1显示/隐藏状态 0隐藏 1显示
u8 butonn2_state=0; //按钮2显示/隐藏状态 0隐藏 1显示
u8 butonn3_state=0;	//按钮3显示/隐藏状态 0隐藏 1显示
u8 butonn4_state=0; //按钮4显示/隐藏状态 0隐藏 1显示
u8 update_count=0;//更新界面计数
char	g_u8UserPassword[9]={0,0,0,0,0,0,0,0,0};//验证完以后要清零
const char	g_u8AdminPassword[9]={0x31,0x39,0x39,0x31,0x30,0x37,0x30,0x31,0};
char g_u8SysTemPassword[9]={0,0,0,0,0,0,0,0,0};

extern char ssid[31];
extern char password[31];
extern char ipaddr[16];
extern char subnet[16];
extern char gateway[16];
extern char dns[16];

HmiRecord u_HmiRecord;
WorkPara HmiWorkPara;

u8 g_u8DestPage = 0;//密码正确跳转界面zbz
u8 g_u8SrcPage = 0;//密码界面返回跳转界面zbz
const u8 StaStr0[15] = "复合肥灌溉程序";
const u8 StaStr1[15] = "一阶段灌溉程序";
const u8 StaStr2[15] = "二阶段灌溉程序";
const u8 StaStr3[15] = "三阶段灌溉程序";
const u8 StaStr4[15] = "四阶段灌溉程序";
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
		else if(( ( uint8 * ) ( &msg->param ) ) [i] != 0x2d)
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
u8 g_u8WifiPara[110];
u8 g_u8DeviceID;
//待机界面
void StangbyPage(PCTRL_MSG msg, qsize size)
{
	u8 Temp=0;
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID

	if(u16Control_ID == 5 &&u8ButtonVaule == 1)
	{
		//读出WIFI信息，并显示
		FlashReadWiFi(g_u8WifiPara);
		//WIFI界面屏幕显示设置内容
		SetTextValue(1,1,&g_u8WifiPara[0]);
		SetTextValue(1,2,&g_u8WifiPara[31]);
		SetTextValue(1,3,&g_u8WifiPara[62]);
		SetTextValue(1,4,&g_u8WifiPara[78]);
		SetTextValue(1,5,&g_u8WifiPara[94]);
		FlashReadID(&g_u8DeviceID);
		NumberToASCII(u8Str,g_u8DeviceID);
		SetTextValue(1,8,u8Str);
	}
}
u16 FlashPara[76];
u8 g_u8DHCP=1;
//灌溉OR设置
void IrrOrSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	s16 s16RevVal  = 0;
	u8	u8ButtonVaule = msg->param[1];
	uint32 u32ShowBuf[5]={0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	s16 value=0;
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
    else if(u16Control_ID == 11)
    {
        FifureTextInput ( &s16RevVal, msg );
        g_u8DHCP = s16RevVal;
    }
    else if(u16Control_ID == 9)
    {
        memcpy(&g_u8WifiPara[110],msg->param,(size-12));
        memcpy(dns,msg->param,(size-12));
    }
	else if(u16Control_ID == 8)
	{
		FifureTextInput(&value,msg);
		g_u8DeviceID = (u8)value;
	}
	
}
//清水选择分区界面
void WaterZoneSel(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
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
			SetScreen(LCD_WATERSTEP3_PAGE);
		}
	}
}
//肥水选择分区界面
void FerZoneSel(PCTRL_MSG msg, qsize size)
{
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
		}
	}
}
//清水操作第三步 设置浇水时长
void WaterTimeSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	MsgStruct Msgtemp;
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
		HmiWorkPara.Para.IrrPjNum = u16Control_ID-2;
	}
	else if(u16Control_ID== 2)
	{
		if(HmiWorkPara.Para.IrrPjNum!=0)
		{
			//根据所选灌溉程序号显示需确认的内容
			FlashReadPara((u8*)FlashPara,0,76);
			NumberToASCII(u8Str,HmiWorkPara.Para.IrrPjNum);
			SetTextValue(LCD_FERSTEP4_PAGE,5,u8Str);
			SetTextValue(LCD_FERSTEP4_PAGE,3,(uchar *)StaText[HmiWorkPara.Para.IrrPjNum-1]);
			NumberToASCII(u8Str,(FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+36]
							+FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+37]
							+FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+38]));
			SetTextValue(LCD_FERSTEP4_PAGE,4,u8Str);
			NumberToASCII(u8Str,FlashPara[(HmiWorkPara.Para.IrrPjNum-1)*8+43]);
			SetTextValue(LCD_FERSTEP4_PAGE,6,u8Str);
			SetScreen(LCD_FERSTEP4_PAGE);
		}
	}
}
//设置第一步肥料罐浓度设置
void PercentSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if((u16Control_ID>=1)&&(u16Control_ID<=4))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1] = s16RevVal;
	}
	else if(u16Control_ID == 7)
	{
		for(i=4;i<=19;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
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
		SetMulSerialText2(LCD_SETSTEP2_PAGE,1,16,u32ShowBuf);
	}
}
//设置第二步 种植面积设置
void AreaSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[17]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=16))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+4] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=36;i<=43;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
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
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+36] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=44;i<=51;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
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
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+44] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=52;i<=59;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
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
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+52] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=60;i<=67;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
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
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	uint32 u32ShowBuf[9]={0,0,0,0,0,0,0,0,0};
	u8 u8Str[7] = {0,0,0,0,0,0,0};
	u8 i=0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+60] = s16RevVal;
	}
	else if(u16Control_ID == 19)
	{
		for(i=68;i<=75;i++)
		{
			NumberToASCII(u8Str,FlashPara[i]);
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
	u8	u8ButtonVaule = msg->param[1];
	s16 s16RevVal  = 0;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	
	if((u16Control_ID>=1)&&(u16Control_ID<=8))
	{
		FifureTextInput ( &s16RevVal, msg );
		FlashPara[u16Control_ID-1+68] = s16RevVal;
	}
}
extern u8 tagendid;//终端ID

//保存
void SaveConfim(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	u8	u8ButtonVaule = msg->param[1];
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID

	if(u16Control_ID == 5)
	{
		FlashWriteWiFi((u8*)g_u8WifiPara);
		FlashWriteID(&g_u8DeviceID);
		tagendid = g_u8DeviceID;
		FlashWriteDHCP(&g_u8DHCP);
		SetScreen(1);
	}
	else if(u16Control_ID==1)
	{
		SetScreen(1);
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
							SaveConfim(msg,size);
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
							
							break;	
						case LCD_WATERSHOW_PAGE://清水显示
							break;
						case LCD_FERSHOW_PAGE://肥水显示
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
u8 Task_percent=0;//任务执行进度百分比
/*接受AGV的触摸屏显示器交互信息*/
void Task_HMIMonitor ( void * parg )
{
	parg = parg;
	SetScreen(0);
	FlashReadWiFi(g_u8WifiPara);
    FlashReadDHCP(&g_u8DHCP);
	while ( 1 ) 
	{
		delay_ms(100);
		update_count++;
		ReceiveFromMonitor();
	  if(update_count>5) //500ms更新一次界面
		{
			update_count=0;
			if(butonn1_state==0)//灰色代表不工作
			{
				SetTexFrontBColor(0,1,GRAY_COLOR_HMI);//设置文本背景			
			}
			else
				SetTexFrontBColor(0,1,GREEN_COLOR_HMI);//设置文本背景			
			if(butonn2_state==0)//灰色代表不工作
			{
				SetTexFrontBColor(0,2,GRAY_COLOR_HMI);//设置文本背景			
			}
			else
				SetTexFrontBColor(0,2,GREEN_COLOR_HMI);//设置文本背景			
			if(butonn3_state==0)//灰色代表不工作
			{
				SetTexFrontBColor(0,3,GRAY_COLOR_HMI);//设置文本背景			
			}
			else
				SetTexFrontBColor(0,3,GREEN_COLOR_HMI);//设置文本背景			
			if(butonn4_state==0)//灰色代表不工作
			{
				SetTexFrontBColor(0,4,GRAY_COLOR_HMI);//设置文本背景			
			}
			else
				SetTexFrontBColor(0,4,GREEN_COLOR_HMI);//设置文本背景						
		}
	}
}

