#include "UserHmi.h"
#include "UserCore.h"
#include "Delay.h"
#include "FlashDivide.h"
#include "GM3Dir.h"
#include "hmi_driver.h"
//#include "ObstScan.h"
//#include "AGVMsg.h"

extern u8 g_u8LockScreenCnt;
extern u8 g_u8LockScreenF;
//u16 u16temp; 
extern OS_TMR	CallerTmr;
extern MOCHINEStruct  MOCHINEDATE;//状态机执行所需条件
extern u8 Machine_Ctrl_State; 	//智能灌溉状态机
#define CMD_MAX_BUFFER 128
u8 Hmi_cmd_buffer[CMD_MAX_BUFFER];  //串口命令接收缓冲区
HMIStruct UserHmiDate;
u16 s_screen_id;                  //页面ID编号变量
u16 s_control_id;                 //控件ID编号变量

//u8 fertilizer_Solenoid_valve = 0; //上电默认电磁阀需要都关闭
///*对应吸肥电磁阀
//bit0  吸肥电磁阀1  0代表这个电磁阀要关闭 1代表要开启 开启顺序 1-->5
//bit1  吸肥电磁阀2
//bit2  吸肥电磁阀3
//bit3  吸肥电磁阀4
//bit4  吸肥电磁阀5

//*/
//u8 water_Solenoid_valve =0;//灌水电磁阀 0关闭 1开启

u16 s_TargetID;				//密码正确跳转界面zbz
u16 s_ReturnID;				//密码界面返回跳转界面zbz
StrategyStruct HMI_ProPara;
u8 WaterTimeH = 0,WaterTimeM = 0 , WaterTime = 0;
u8 FertilizerTimeH = 0,FertilizerTimeM = 0,FertilizerTime = 0;

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

/*******************************************************************************
名称:文本输入获取
功能:将报文中的数字解析出来并给目标参数赋值
输入: Number:目标参数 msg:报文
*******************************************************************************/
u16 ASCIITONUMBER (PCTRL_MSG msg )
{
	u8 i = 0 , j = 0;
	u8 temp = 0;
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

//return Number;

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
//主界面开关选择
//输入：控件ID
//输出：无
//功能：根据控件ID，判断用户选择那个开关开，哪个开关关。
void MainPageSwitchSel(u8 CtrlID, u8 Value)
{
//	u8 i = 0;
//	s16 s16RevVal  = 0;
	if(Value == 1)
	{
		if(CtrlID == 28)
		{	
			HMI_ProPara.PumpFFlag = 1;
		}
		else if(CtrlID == 29)
		{
			HMI_ProPara.PumpWFlag = 1;
		}
		else if((CtrlID>=30) && (CtrlID<=32))
		{
			HMI_ProPara.Passageway[CtrlID-30] = 1;//alfred 因为暂时只有3个通道且没有给另外三个通道留出位置，所以只能为<=32
		}
		else if((CtrlID>=33) && (CtrlID<=34))//alfred 因为暂时只有3个通道且没有给另外三个通道留出位置，所以只能为<=34
		{
			HMI_ProPara.Zone = (CtrlID-32);//分区
			//分区电磁阀互斥
			if(CtrlID == 33)
			{
				SetButtonValue(LCD_MAIN_PAGE,34,0);
			}
			else if(CtrlID == 34)
			{
				SetButtonValue(LCD_MAIN_PAGE,33,0);
			}
		}
	}
	else if(Value == 0)
	{
		if(CtrlID == 28)
		{	
			HMI_ProPara.PumpFFlag = 0;
		}
		else if(CtrlID == 29)
		{
			HMI_ProPara.PumpWFlag = 0;
		}
		else if((CtrlID>=30) && (CtrlID<=32))// alfred 需要确认此参数的用法
		{
			HMI_ProPara.Passageway[CtrlID-30] = 0;//alfred 因为暂时只有3个通道且没有给另外三个通道留出位置，所以只能为<=32
		}
		else if((CtrlID>=33) && (CtrlID<=34))//alfred 因为暂时只有3个通道且没有给另外三个通道留出位置，所以只能为<=34
		{
			HMI_ProPara.Zone = 0;
		}
	}
}
//const u8 StaStr0[10] = "空闲";
//const u8 StaStr1[10] = "浇水中";
//const u8 StaStr2[10] = "施肥中";
//const u8* StaText[] = 
//{
//	StaStr0,
//	StaStr1,
//	StaStr2,
//};
//u8 WaterSta,OldWaterSta ;
////浇水状态显示函数
//void WaterStaShow(u8 State)
//{
//	WaterSta = State;
//	//SetTextValue(LCD_MAIN_PAGE,7,(uchar *)StaText[State]);
//}

//u8 FertilizerSta,OldFertilizerSta;
////施肥状态显示函数
//void FertilizerStaShow(u8 State)
//{
//	FertilizerSta = State;
//	//SetTextValue(LCD_MAIN_PAGE,8,(uchar *)StaText[State]);
//}

////浇水倒计时显示函数
//void WaterTimeShow(u16 Time)
//{
//	u8 TimeH = 0 ;
//	u8 TimeM = 0;
//	u8 u8Str[7] = {0,0};
//	
//	TimeH = Time/60;
//	TimeM = Time%60;
//	
//	NumberToASCII(u8Str,(u16)TimeH);
//	SetTextValue(LCD_MAIN_PAGE,9,u8Str);
//	delay_ms(10);
//	NumberToASCII(u8Str,(u16)TimeM);
//	SetTextValue(LCD_MAIN_PAGE,10,u8Str);
//	
//}
////施肥倒计时显示函数
//void FertilizerTimeShow(u16 Time)
//{
//	u8 TimeH = 0 ;
//	u8 TimeM = 0;
//	u8 u8Str[7] = {0,0};
//	
//	TimeH = Time/60;
//	TimeM = Time%60;
//	
//	NumberToASCII(u8Str,(u16)TimeH);
//	SetTextValue(LCD_MAIN_PAGE,11,u8Str);
//	delay_ms(10);
//	NumberToASCII(u8Str,(u16)TimeM);
//	SetTextValue(LCD_MAIN_PAGE,12,u8Str);
//	
//}
////浇水设置界面
//void WaterSetShow(void)
//{
//	u16 Time = 0;
//	u8 TimeH = 0 ;
//	u8 TimeM = 0;
//	u8 u8Str[7] = {0,0};
//	FlashReadWaterTime((u8*)&Time);
//	TimeH = Time/60;
//	TimeM = Time%60;
//	NumberToASCII(u8Str,(u16)TimeH);
//	SetTextValue(LCD_WARTERSET_PAGE,1,u8Str);
//	NumberToASCII(u8Str,(u16)TimeM);
//	SetTextValue(LCD_WARTERSET_PAGE,2,u8Str);
//	WaterTimeH = TimeH;
//	WaterTimeM = TimeM;
//}
////施肥设置界面
//void FertilizerSetShow(void) 
//{
//	u16 Time = 0;
//	u8 TimeH = 0 ;
//	u8 TimeM = 0;
//	u8 u8Str[7] = {0,0};
//	FlashReadFertilizerTime((u8*)&Time);
//	TimeH = Time/60;
//	TimeM = Time%60;
//	NumberToASCII(u8Str,(u16)TimeH);
//	SetTextValue(LCD_FERTILIZERSET_PAGE,1,u8Str);
//	NumberToASCII(u8Str,(u16)TimeM);
//	SetTextValue(LCD_FERTILIZERSET_PAGE,2,u8Str);
//	FertilizerTimeH = TimeH;
//	FertilizerTimeM = TimeM;
//}
//主界面刷新操作
void Refresh_Main_Page(void)
{
		u8 u8Str[7] = {0,0};
			memset(&UserHmiDate,0,sizeof(UserHmiDate)); 
			NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,2,u8Str);
					NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,3,u8Str);
					NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,4,u8Str);
					NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,5,u8Str);
					NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,6,u8Str);
					NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,7,u8Str);
					NumberToASCII(u8Str,0);
			SetTextValue(LCD_MAIN_PAGE,8,u8Str);
		  SetScreen(0);


}
//执行界面刷新操作
void Refresh_ACTION_Page(void)
{
			SetTexFrontBColor(LCD_ACTION_PAGE,1,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,1,"关闭");
			SetTexFrontBColor(LCD_ACTION_PAGE,2,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,2,"关闭");
			SetTexFrontBColor(LCD_ACTION_PAGE,3,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,3,"关闭");
			SetTexFrontBColor(LCD_ACTION_PAGE,4,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,4,"关闭");
			SetTexFrontBColor(LCD_ACTION_PAGE,5,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,5,"关闭");
			SetTexFrontBColor(LCD_ACTION_PAGE,6,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,6,"关闭");
			SetTexFrontBColor(LCD_ACTION_PAGE,7,RED_COLOR_HMI);//设置文本背景
			SetTextValue(LCD_ACTION_PAGE,7,"关闭");
}
	
	
	
//浇水启停按钮动作接收
//输入：键值
//输出：无
//根据键值做出响应
void WaterButtonRev(u8 ButtonVal)
{
	MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	//alfred //添加向主状态机发送消息
	if(ButtonVal == 0)
	{
		Msgtemp.CmdType = MSG_WARTERING;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdData[0] = 0;
		OS_ENTER_CRITICAL();
		PackSendMasterQ(&Msgtemp);
		//SetStrategy(&HMI_ProPara);
		OS_EXIT_CRITICAL();
	}
	else if(ButtonVal == 1)
	{
		
		Msgtemp.CmdType = MSG_WARTERING;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdData[0] = 1;
		OS_ENTER_CRITICAL();
		PackSendMasterQ(&Msgtemp);
		OS_EXIT_CRITICAL();
	}
}
//施肥启停按钮动作接收
//输入：键值
//输出：无
//根据键值做出响应
void FertilizerButtonRev(u8 ButtonVal)
{
	MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	//alfred //添加向主状态机发送消息
	if(ButtonVal == 0)
	{
		Msgtemp.CmdType = MSG_FERTILIZER;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdData[0] = 0;
		OS_ENTER_CRITICAL();
		PackSendMasterQ(&Msgtemp);
		SetStrategy(&HMI_ProPara);
		OS_EXIT_CRITICAL();
	}
	else if(ButtonVal == 1)
	{
		
		Msgtemp.CmdType = MSG_FERTILIZER;
		Msgtemp.CmdSrc = SCREEN_TASK_CODE;
		Msgtemp.CmdData[0] = 1;
		OS_ENTER_CRITICAL();
		PackSendMasterQ(&Msgtemp);
		OS_EXIT_CRITICAL();
	}
}
void SetRadiotube(u8 Num, u8 Value)
{
	SetButtonValue(LCD_MAIN_PAGE,Num+16,Value);
}
//接收解析主界面发来的报文
void MainPage(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				   //控件ID编号变量
//	s16 s16RevVal  = 0;
	u8	u8ButtonVaule = msg->param[1];
	MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
//	InitativeStruct InitiativeParaTemp;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	switch(u16Control_ID)
	{
	  case 1 :
			OS_ENTER_CRITICAL();
		MOCHINEDATE.WaterTime = UserHmiDate.water_Solenoid_time;
		MOCHINEDATE.Fertilizertime =UserHmiDate.fertilizer_Solenoid1_time+UserHmiDate.fertilizer_Solenoid2_time+
		UserHmiDate.fertilizer_Solenoid3_time+UserHmiDate.fertilizer_Solenoid4_time+UserHmiDate.fertilizer_Solenoid5_time;
		MOCHINEDATE.FertilizerIoNumber = UserHmiDate.fertilizer_Solenoid_valve;
		MOCHINEDATE.WaterOnID = UserHmiDate.water_select;
//				SetScreen(1);
				Msgtemp.CmdType = MSG_START;
//				Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1;
				
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();						
		break;
	  case 2 :
    		UserHmiDate.fertilizer_Solenoid1_time=ASCIITONUMBER(msg);
				if(UserHmiDate.fertilizer_Solenoid1_time>0) //设置时间认为开启
					UserHmiDate.fertilizer_Solenoid_valve |= (1<<0);
		break;
	  case 3 :
    		UserHmiDate.fertilizer_Solenoid2_time=ASCIITONUMBER(msg);
				if(UserHmiDate.fertilizer_Solenoid2_time>0) //设置时间认为开启
					UserHmiDate.fertilizer_Solenoid_valve |= (1<<1);
		break;
	  case 4 :
    		UserHmiDate.fertilizer_Solenoid3_time=ASCIITONUMBER(msg);
				if(UserHmiDate.fertilizer_Solenoid3_time>0) //设置时间认为开启
					UserHmiDate.fertilizer_Solenoid_valve |= (1<<2);
		break;
	  case 5 :
    		UserHmiDate.fertilizer_Solenoid4_time=ASCIITONUMBER(msg);
				if(UserHmiDate.fertilizer_Solenoid4_time>0) //设置时间认为开启
					UserHmiDate.fertilizer_Solenoid_valve |= (1<<3);
		break;
	  case 6 :
    		UserHmiDate.fertilizer_Solenoid5_time=ASCIITONUMBER(msg);
				if(UserHmiDate.fertilizer_Solenoid5_time>0) //设置时间认为开启
					UserHmiDate.fertilizer_Solenoid_valve |= (1<<4);
		break;
	  case 7 :
				UserHmiDate.water_select=ASCIITONUMBER(msg);
//				if(UserHmiDate.fertilizer_Solenoid1_time>0) //设置时间认为开启
//					UserHmiDate.fertilizer_Solenoid_valve |= (1<<0);
		break;
	  case 8 :
    		UserHmiDate.water_Solenoid_time=ASCIITONUMBER(msg);
				if(UserHmiDate.water_Solenoid_time>0) //设置时间认为开启
					UserHmiDate.water_Solenoid_valve |= (1<<0);
		break;		
		default :
		break;
		
	
	}

	
//	if((u16Control_ID>=28) && (u16Control_ID<=34))//上方的按键
//	{
//		MainPageSwitchSel(u16Control_ID , u8ButtonVaule);
//	}
//	switch(u16Control_ID)
//	{
//		case 1://浇水启动
//			WaterButtonRev(1);
//			break;
//		case 2://浇水停止
//			WaterButtonRev(0);
//			break;
//		case 3://施肥启动
//			FertilizerButtonRev(1);
//			break;
//		case 4://施肥停止
//			FertilizerButtonRev(0);
//			break;
//		case 5:
//			WaterSetShow();
//			break;
//		case 6:
//			FertilizerSetShow();
//			break;
//		case 15://网络开关
//			if(u8ButtonVaule == 1)
//			{
//				Msgtemp.CmdType = MSG_MODECHANGE;
//				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
//				Msgtemp.CmdData[0] = 1;
//				OS_ENTER_CRITICAL();
//				PackSendMasterQ(&Msgtemp);
//				OS_EXIT_CRITICAL();
//			}
//			else if(u8ButtonVaule == 0)
//			{
//				Msgtemp.CmdType = MSG_MODECHANGE;
//				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
//				Msgtemp.CmdData[0] = 0;
//				OS_ENTER_CRITICAL();
//				PackSendMasterQ(&Msgtemp);
//				OS_EXIT_CRITICAL();
//			}
//			break;
//		case 16:
//		case 17:
//		case 18:
//		case 19:
//		case 20:
//		case 21:
//		case 22:
//		case 23:
//		case 24:
//		case 25:
//		case 26:
//		case 27:
//		case 28:
//		case 29:
//		case 30:
//		case 31:
//			if(u8ButtonVaule == 0)
//			{
//				Msgtemp.CmdType = MSG_RADIOTUBE;
//				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
//				Msgtemp.CmdData[0] = 0;//电磁阀号
//				Msgtemp.CmdData[1] = 0;//动作(清零不看动作号)
//				OS_ENTER_CRITICAL();
//				PackSendMasterQ(&Msgtemp);
//				OS_EXIT_CRITICAL();
//			}
//			else if(u8ButtonVaule == 1)
//			{
//				Msgtemp.CmdType = MSG_RADIOTUBE;
//				Msgtemp.CmdSrc = SCREEN_TASK_CODE;
//				Msgtemp.CmdData[0] = u16Control_ID-16;//电磁阀号
//				Msgtemp.CmdData[1] = 1;//打开相应电磁阀,关闭其他电磁阀
//				OS_ENTER_CRITICAL();
//				PackSendMasterQ(&Msgtemp);
//				OS_EXIT_CRITICAL();
//			}
//			break;
//		default:
//			break;
//		
//	}
}
//任务执行界面
void ActionPage(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				   //控件ID编号变量
//	s16 s16RevVal  = 0;
	u8 u8Str[7] = {0,0};
	u8	u8ButtonVaule = msg->param[1];
	MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
//	InitativeStruct InitiativeParaTemp;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
  if(u16Control_ID==8)
	{
		Refresh_Main_Page();//刷新主界面文本信息
	}
}

void Alarm1Page(PCTRL_MSG msg, qsize size )
{
	u16 u16Control_ID;				   //控件ID编号变量
//	s16 s16RevVal  = 0;
	u8 u8Str[7] = {0,0};
	u8	u8ButtonVaule = msg->param[1];
	MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
//	InitativeStruct InitiativeParaTemp;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
  if(u16Control_ID==1)
	{
		Machine_Ctrl_State=0;
		Refresh_Main_Page();//刷新主界面文本信息
		memset(&MOCHINEDATE,0,sizeof(MOCHINEDATE));
		MOCHINEDATE.FertilizerOnID=1;
	}
}
//void Alarm2Page(PCTRL_MSG msg, qsize size )
//{
//	u16 u16Control_ID;				   //控件ID编号变量
////	s16 s16RevVal  = 0;
//	u8 u8Str[7] = {0,0};
//	u8	u8ButtonVaule = msg->param[1];
//	MsgStruct Msgtemp;
//	OS_CPU_SR  cpu_sr;
////	InitativeStruct InitiativeParaTemp;
//	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
//  if(u16Control_ID==1)
//	{
//		Refresh_Main_Page();//刷新主界面文本信息
//	}
//}
void WaterSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	s16 s16RevVal  = 0;
//	MsgStruct Msgtemp;
//	OS_CPU_SR  cpu_sr;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if(u16Control_ID == 1)
	{
		FifureTextInput ( &s16RevVal, msg );
		WaterTimeH =(u8) s16RevVal;
	}
	else if(u16Control_ID == 2)
	{
		FifureTextInput ( &s16RevVal, msg );
		WaterTimeM =(u8) s16RevVal;
	}
	else if(u16Control_ID == 3)
	{
		WaterTime = WaterTimeH*60+WaterTimeM;
		FlashWriteWaterTime((u8*)&WaterTime);
	}
	
}
void FertilizerSet(PCTRL_MSG msg, qsize size)
{
	u16 u16Control_ID;				   //控件ID编号变量
	s16 s16RevVal  = 0;
//	MsgStruct Msgtemp;
//	OS_CPU_SR  cpu_sr;
	u16Control_ID = PTR2U16 ( &msg->control_id ); //控件ID
	if(u16Control_ID == 1)
	{
		FifureTextInput ( &s16RevVal, msg );
		FertilizerTimeH =(u8) s16RevVal;
	}
	else if(u16Control_ID == 2)
	{
		FifureTextInput ( &s16RevVal, msg );
		FertilizerTimeM =(u8) s16RevVal;
	}
	else if(u16Control_ID == 3)
	{
		FertilizerTime = FertilizerTimeH*60+FertilizerTimeM;
		FlashWriteFertilizerTime((u8*)&FertilizerTime);
	}
}
//接受触摸屏消息
void ReceiveFromMonitor( void )
{
	uint8 cmd_type = 0;
	uint8 msg_type = 0;
//	uint8 control_type = 0;
	static u16 IdleCnt = 0;
	PCTRL_MSG msg = 0;
	qsize size = 0;
	//if(GetScreen()!= LCD_STANDBY_PAGE)
	{
		if(++IdleCnt > IDLECOUNTNUM)
		{
//			SetScreen(LCD_STANDBY_PAGE);
			IdleCnt = 0;
		}
	}
	size = queue_find_cmd ( Hmi_cmd_buffer, CMD_MAX_BUFFER ); //从缓冲区中获取一条指令
	if ( size <= 10 ) //没有接收到指令
		return;
//		continue;
	IdleCnt = 0;//收到数据清空计数
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
						case LCD_MAIN_PAGE:
							MainPage(msg, size);
							break;
						case LCD_ACTION_PAGE :
							ActionPage(msg, size);
						break;
						case LCD_STARTERR_PAGE:
							  
							Alarm1Page(msg, size);
							break;
						case LCD_FRETERR_PAGE:
							Alarm1Page(msg, size);
							break;
						case LCD_WORKOVERTIP_PAGE:
							Alarm1Page(msg, size);
							break;
						case LCD_WATERERR_PAGE:
							Alarm1Page(msg, size);
							break;		
						case LCD_MOTORERR_PAGE:
							Alarm1Page(msg, size);
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
void SetSwitchAlarm(void)
{
//	u8 Cnt =0 ;
//	u8 Cnt1 = 0;
//	for(Cnt1 = 0; Cnt1<2 ;Cnt1++)
//	{
//		for(Cnt = 0; Cnt<8 ; Cnt++)
//		{
//			if((ZoneAlarm[Cnt1]&(1<<Cnt)) == 0)
//			{
//				SetButtonValue(LCD_SWITCHALARM_PAGE,Cnt1*8+Cnt+1,0);
//			}
//			else 
//			{
//				SetButtonValue(LCD_SWITCHALARM_PAGE,Cnt1*8+Cnt+1,1);
//			}
//		}
//	}
}
void TimerUpdate(void)
{
//	WaterTimeShow(WarterRemainderTime);
//	delay_ms(10);
//	FertilizerTimeShow(FertilizerRemainderTime);
}
u8 TimeUpdateCnt = 0;
u8 Task_percent=0;//任务执行进度百分比
/*接受AGV的触摸屏显示器交互信息*/
void Task_HMIMonitor ( void * parg )
{
	//uint8 cmd_type = 0;
	//uint8 msg_type = 0;
//	uint8 control_type = 0;
	//qsize size = 0;
//	uint8 update_en = 1;
	//u8 os_err;
	float water_all_time=0;
	float water_Cruent_time=0;
	float percent=0;
	parg = parg;
	memset(&UserHmiDate,0,15); //初始化结构体 不要用sizeof
	SetSliderValue ( 1, 8, Task_percent );
	SetScreen(0);
	while ( 1 ) 
	{
		delay_ms(100);
		ReceiveFromMonitor();
//		if(MOCHINEDATE.WaterOnSucess==1)
//		{
			if(TimeUpdateCnt++>20)	//2S刷新一次任务进度
			{
			if(MOCHINEDATE.WaterTimeing>0)
			{
				water_all_time=(float)MOCHINEDATE.WaterTime;
				water_Cruent_time=(float)MOCHINEDATE.WaterTimeing;
				percent= water_Cruent_time*100/water_all_time;
				Task_percent=(u8)percent;
				SetSliderValue ( 1, 9, Task_percent );
			}
			}
		
		
//		}
//		if(TimeUpdateCnt++>10)
//		{
//			TimerUpdate();
//			TimeUpdateCnt = 0;
//		}
//		if(WaterSta!=OldWaterSta)
//		{
//			SetTextValue(LCD_MAIN_PAGE,7,(uchar *)StaText[WaterSta]);
//			OldWaterSta = WaterSta;
//		}
//		if(FertilizerSta!=OldFertilizerSta)
//		{
//			SetTextValue(LCD_MAIN_PAGE,8,(uchar *)StaText[FertilizerSta]);
//			OldFertilizerSta = FertilizerSta;
//		}
		
	}
}

