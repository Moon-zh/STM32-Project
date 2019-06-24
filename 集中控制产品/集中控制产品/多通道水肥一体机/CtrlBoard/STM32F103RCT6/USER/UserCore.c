#include "sys.h"
#include "includes.h"  
#include "UserCore.h"
#include "Delay.h"
#include "ZoneCtrl.h"
#include "GM3Dir.h"
#include "hmi_driver.h"
#include "UserHmi.h"
#include "IO_BSP.h"
#include "FlashDivide.h"
#include "Flowmeter.h"
MsgStruct MasterMsg;
OS_EVENT * MasterQMsg;

void* 	MasterMsgBlock[4];
OS_MEM* MasterPartitionPt;
//u8 g_u8AgvCtrMsgMem[5][4];
u8 g_u8AgvCtrMsgMem[20][4];
MOCHINEStruct  MOCHINEDATE;//状态机执行所需条件
u16 WarterRemainderTime = 0;//浇水剩余工作时间
u16 FertilizerRemainderTime = 0;//施肥剩余工作时间

u8 ControlMode = 0;//水肥一体机控制模式：1：本地 0:网络
u8 SendIoCount=0;//向IO发送指令次数计时
//u8 MasterState = 0;//主状态机
u8 WarterState = 0;//浇水状态
u8 FertilizerState = 0;//浇水状态
u8 Machine_Ctrl_State =0; 	//智能灌溉状态机
StrategyStruct WorkStrategy;
extern FlowmeterStruct FlowmeterCount; //流量计数组
extern HMIStruct UserHmiDate;
//写执行策略
//输入：要设置的值
//返回：0：设置成功 1：设置功能被占用 2：FLASH写失败
//使用时禁止中断
u8 SetStrategy(StrategyStruct *Strategy)
{
//	static u8 Flag = 0;
//	if(Flag == 1)
//		return 1 ;
//	Flag = 1;
//	WorkStrategy.PumpWFlag = Strategy->PumpWFlag;
//	WorkStrategy.PumpFFlag = Strategy->PumpFFlag;
//	WorkStrategy.Passageway[0] = Strategy->Passageway[0];
//	WorkStrategy.Passageway[1] = Strategy->Passageway[1];
//	WorkStrategy.Passageway[2] = Strategy->Passageway[2];
//	WorkStrategy.Passageway[3] = Strategy->Passageway[3];
//	WorkStrategy.Passageway[4] = Strategy->Passageway[4];
//	WorkStrategy.Passageway[5] = Strategy->Passageway[5];
//	WorkStrategy.Zone = Strategy->Zone;
//	WorkStrategy.WorkHour = Strategy->WorkHour;
//	WorkStrategy.WorkMinute = Strategy->WorkMinute;
//	WorkStrategy.WorkDay = Strategy->WorkDay;
//	WorkStrategy.StartHour = Strategy->StartHour;
//	WorkStrategy.StartMinute = Strategy->StartMinute;
//	//写入Flash
//	Flag = 0;
//	return 0;
}
//策略开始
//输入：策略参数
//输出：无
// alfred 还剩MODBUS线圈没有写 水泵和肥泵的控制没有写
void StrategyStart(StrategyStruct *Strategy)
{
//	u8 ConduitF = 0;
//	u8 i = 0;
//	
//	//打开分区电磁阀
//	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YSETSINGLE);
//	SetMDCoil(ConduitF + 2 - 1 , 1);
//	delay_ms(110);
//	
//	if(Strategy->PumpWFlag == 1)
//	{
//		//打开水泵
//		SetMDCoil(0 , 1);
//	}
//	else if(Strategy->PumpWFlag == 0)
//	{
//		//报错或者打开水泵
//	}
//	if(Strategy->PumpFFlag == 1)
//	{
//		//打开肥泵
//		SetMDCoil(1 , 1);
//	}
//	else if(Strategy->PumpFFlag == 0)
//	{
//		//关闭肥泵
//		SetMDCoil(1 , 0);
//	}
//	//打开通道电磁阀 
//	for(i = 0;i<6; i++)
//	{
//		if(Strategy->Passageway[i] == 1)
//		{
//			ConduitF |= i<<i;
//			SetMDCoil(8+i , 1);
//		}
//	}
//	ZoneCtrl_Set(ConduitF,YSETMUL);
//}
////策略停止
////输入：策略参数
////输出：无
//// alfred 还剩MODBUS线圈没有写 水泵和肥泵的控制没有写
//void StrategyStop(StrategyStruct *Strategy)
//{
//	u8 ConduitF = 0;
//	u8 i = 0;
//	
//	//关闭水泵
//	SetMDCoil(0 , 0);
//	//关闭肥泵
//	SetMDCoil(1 , 0);
//	//关闭通道电磁阀 
//	for(i = 0;i<6; i++)
//	{
//		SetMDCoil(8+i , 0);
//	}
//	ZoneCtrl_Set(0,YCLEAR);
//	delay_ms(110);
//	//关闭分区电磁阀
//	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YCLEAR);
//	SetMDCoil(ConduitF + 2 - 1  , 0);
}
//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 MasterQInit(void)
{
	INT8U os_err;
	
	MasterQMsg = OSQCreate ( MasterMsgBlock, 4);
	
	if(MasterQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	MasterPartitionPt = OSMemCreate (
										g_u8AgvCtrMsgMem,
				  						20,
				  						4,
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
//此函数主要给中断使用
u8 InterruptPackSendMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(MasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( MasterQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(MasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//消息打包发走
//输入：消息信息
//输出：0：发送成功
//		1：申请内存失败
//		2：发送消息失败

u8 PackSendMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(MasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( MasterQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(MasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveMasterQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(MasterPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
u8 ZoneAlarm[2];
//检测要开启的施肥电磁阀
u8 Check_FER_ON(u8 IOnumber)
{
  if((IOnumber&0x01)==0x01)	//检测第一个施肥阀是否需要开启
  {
		return 1;
	}		
  else if((IOnumber&0x02)==0x02)	//检测第二个施肥阀是否需要开启
  {
		return 2;
	}	
  else if((IOnumber&0x04)==0x04)	//检测第三个施肥阀是否需要开启
  {
		return 3;
	}	
	  else if((IOnumber&0x068)==0x08)	//检测第四个施肥阀是否需要开启
  {
		return 4;
	}	
	  else if((IOnumber&0x10)==0x10)	//检测第五个施肥阀是否需要开启
  {
		return 5;
	}	
  else
	 return 0;

}
////检测要对应流量计是否有变换
//u8 Check_FLOW_ON(u8 IOnumber)
//{
//  switch(IOnumber)
//	{
//		case 1:
//			if(FlowmeterCount.Flowmeter1>MOCHINEDATE.lastFlowmeter1)
//			{
//			  return 1;
//			}
//		break;
//	
//		case 2:
//			if(FlowmeterCount.Flowmeter2>MOCHINEDATE.lastFlowmeter2)
//			{
//			  return 1;
//			}			
//		break;
//		case 3:
//			if(FlowmeterCount.Flowmeter3>MOCHINEDATE.lastFlowmeter3)
//			{
//			  return 1;
//			}					
//		break;
//		case 4:
//			if(FlowmeterCount.Flowmeter4>MOCHINEDATE.lastFlowmeter4)
//			{
//			  return 1;
//			}					
//		break;
//		case 5:
//			if(FlowmeterCount.Flowmeter5>MOCHINEDATE.lastFlowmeter5)
//			{
//			  return 1;
//			}					
//		break;
//		default:
//			
//		break;		
//	
//	
//	
//	
//	}
// return 0;
//}

//开启灌水电磁阀 直到成功
void Water_on(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//发送消息频率要大于检测频率 防止误发检测  发送函数不需要快速响应
		 if(SendIoCount>IO_SEND_COUNT_MAX) //若需具体分析那个错误需要添加到各个步骤
		 {
			 			 MOCHINEDATE.Io_Step=0;
			MOCHINEDATE.Io_on_Off_Sucess =0;	//跳出循环
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_WATER_ON; //开启浇水失败    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //状态机切换到报警状态
			Msgtemp.CmdType = MSG_ALARM;								//开启报警信息
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 2; //表示任务执行到第几步
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//写IO指令
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.WaterOnID,0,0xff);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.WaterOnID);	
			break;
			case 2:
					
					SendIoCount=0;
				  MOCHINEDATE.WaterOnSucess =1; //浇水电磁阀开启成功
					MOCHINEDATE.Io_Step =0;
					MOCHINEDATE.Io_on_Off_Sucess =0;
					SetTexFrontBColor(LCD_ACTION_PAGE,6,GREEN_COLOR_HMI);//设置文本背景
					SetTextValue(LCD_ACTION_PAGE,6,"开启");
			
			    MOCHINEDATE.FertilizerIoStatrNum = Check_FER_ON(MOCHINEDATE.FertilizerIoNumber);	//检测现在开启第几路施肥阀

				if(MOCHINEDATE.FertilizerIoStatrNum!=0)
				{
					Msgtemp.CmdType = MSG_ACTION_ERTILIZER_ON;								//施肥电磁阀开启
		//								Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1; //表示任务执行到第几步
				OS_ENTER_CRITICAL();
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();	
				}
			break;			
			default :
			break;
		
		}
	  
	
	}


}
void Water_off(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//发送消息频率要大于检测频率 防止误发检测  发送函数不需要快速响应
		 if(SendIoCount>IO_SEND_COUNT_MAX) //若需具体分析那个错误需要添加到各个步骤
		 {
			MOCHINEDATE.Io_on_Off_Sucess =0;	//跳出循环
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_WATER_ON; //开启浇水失败    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //状态机切换到报警状态
			Msgtemp.CmdType = MSG_ALARM;								//开启报警信息
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 1; //表示任务执行到第几步
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//写IO指令
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.WaterOnID,0,0);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.WaterOnID);	
			break;
			case 2:
					
					SendIoCount=0;
					MOCHINEDATE.Io_Step=0;
					MOCHINEDATE.Io_on_Off_Sucess =0;
					Machine_Ctrl_State =MOCHINE_STOP;  //状态机切换到停止(任务完成)
					Msgtemp.CmdType = MSG_STOP;								//施肥电磁阀开启
		//								Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1; //表示任务执行到第几步
				OS_ENTER_CRITICAL();
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();	
			
			break;			
			default :
			break;
		
		}
	  
	
	}


}
//开启施肥电磁阀 直到成功
void Fertilizer_on(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
	  u8 i;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//发送消息频率要大于检测频率 防止误发检测  发送函数不需要快速响应
		 if(SendIoCount>IO_SEND_COUNT_MAX) //若需具体分析那个错误需要添加到各个步骤
		 {
			 MOCHINEDATE.Io_Step=0;
			MOCHINEDATE.Io_on_Off_Sucess =0;	//跳出循环
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //开启施肥失败    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //状态机切换到报警状态
			Msgtemp.CmdType = MSG_ALARM;								//开启报警信息
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 3; //表示任务执行到第几步
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//写IO指令
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.FertilizerOnID,(MOCHINEDATE.FertilizerIoStatrNum-1),0xff);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.FertilizerOnID);	
			break;
			case 2:
						SendIoCount=0;
						IO_OutSet(1, 1);  //开启吸肥电机
						delay_ms(50);
			//为了测试成功暂时不需要开启此部分
//					 if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)==0) //开启成功
//					 {
					 
							MOCHINEDATE.Io_Step =3;//进行流量计数据检测		 
//					 }
//					 else																//开启失败
//					 {
//					 			 MOCHINEDATE.Io_Step=0;
//							MOCHINEDATE.Io_on_Off_Sucess =0;	//跳出循环
//							MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //开启施肥失败    
//							 Machine_Ctrl_State =MOCHINE_ALARM;  //状态机切换到报警状态
//							Msgtemp.CmdType = MSG_ALARM;								//开启报警信息
//					//								Msgtemp.CmdSrc = 0;
//							Msgtemp.CmdData[0] = 3; //表示任务执行到第几步
//							OS_ENTER_CRITICAL();
//							PackSendMasterQ(&Msgtemp);
//							OS_EXIT_CRITICAL();						 
//					 
//					 
//					 }
				

			break;	
		  case 3:
				 if(SendIoCount++<=IO_SEND_COUNT_MAX) //5S内流量对应流量计有变化 具体时间需要测试
				 {
//						if(Check_FLOW_ON(MOCHINEDATE.FertilizerIoStatrNum))
//						{
//						
//							SendIoCount=0;
//							MOCHINEDATE.FertilizerIoSucess =1; //施肥电磁阀开启成功
//							MOCHINEDATE.Io_Step =0;
//							MOCHINEDATE.Io_on_Off_Sucess =0;
//							switch(MOCHINEDATE.FertilizerIoStatrNum)
//						  {
//								case 1:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid1_time;
//									break;
//								case 2:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid2_time;
//									break;
//								case 3:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid3_time;
//									break;
//								case 4:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid4_time;
//									break;
//								case 5:
//									MOCHINEDATE.FertilizerCurrenttime = UserHmiDate.fertilizer_Solenoid5_time;
//									break;
//								default:
//									break;
//								
//								
//								
//							
//							}
//							MOCHINEDATE.FertilizerCurrenttimeing =0;
//							SetTexFrontBColor(LCD_ACTION_PAGE,7,GREEN_COLOR_HMI);//设置文本背景
//							SetTextValue(LCD_ACTION_PAGE,7,"开启");	
//							for(i=1;i<6;i++)
//							{
//							 if(i==MOCHINEDATE.FertilizerIoStatrNum)	//更新文本控件
//							 {
//									SetTexFrontBColor(LCD_ACTION_PAGE,i,GREEN_COLOR_HMI);//设置文本背景
//									SetTextValue(LCD_ACTION_PAGE,i,"开启");
//							 
//							 }
//							 else
//							 {
//									SetTexFrontBColor(LCD_ACTION_PAGE,i,RED_COLOR_HMI);//设置文本背景
//									SetTextValue(LCD_ACTION_PAGE,i,"关闭");							 
//							 
//							 
//							 }
//							
//							}	
//									
//						}
				 
				 
				 }
				 else																//开启失败
				 {
						MOCHINEDATE.Io_Step=0;
						MOCHINEDATE.Io_on_Off_Sucess =0;	//跳出循环
						MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //开启施肥失败    
						 Machine_Ctrl_State =MOCHINE_ALARM;  //状态机切换到报警状态
						Msgtemp.CmdType = MSG_ALARM;								//开启报警信息
				//								Msgtemp.CmdSrc = 0;
						Msgtemp.CmdData[0] = 2; //表示任务执行到第几步
						OS_ENTER_CRITICAL();
						PackSendMasterQ(&Msgtemp);
						OS_EXIT_CRITICAL();						 
				 
				 
				 }				 
			 break;
			default :
			break;
		
		}
	  
	
	}


}
//关闭施肥电磁阀 直到成功
void Fertilizer_off(void)
{
	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
	  u8 i;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);											//发送消息频率要大于检测频率 防止误发检测  发送函数不需要快速响应
		 if(SendIoCount>IO_SEND_COUNT_MAX) //若需具体分析那个错误需要添加到各个步骤
		 {
			 			 MOCHINEDATE.Io_Step=0;
			MOCHINEDATE.Io_on_Off_Sucess =0;	//跳出循环
			MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_FERT_ON; //开启施肥失败    
			 Machine_Ctrl_State =MOCHINE_ALARM;  //状态机切换到报警状态
			Msgtemp.CmdType = MSG_ALARM;								//开启报警信息
	//								Msgtemp.CmdSrc = 0;
			Msgtemp.CmdData[0] = 3; //表示任务执行到第几步
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();	
		 }
		switch(MOCHINEDATE.Io_Step)		
		{
			case 0:										//写IO指令
				  if(SendIoCount++<=IO_SEND_COUNT_MAX)			
						IO_SET_DATA(MOCHINEDATE.FertilizerOnID,(MOCHINEDATE.FertilizerIoStatrNum-1),0);
					
			break;
			case 1:
					if(SendIoCount++<=IO_SEND_COUNT_MAX)
					IO_READ_Input_Data(MOCHINEDATE.FertilizerOnID);	
			break;
			case 2:
					SendIoCount=0;
			  	MOCHINEDATE.Io_Step=0;
					MOCHINEDATE.FertilizerCurrenttimeing =0;
					IO_OutSet(1, 0);  //关闭吸肥电机	
					SetTexFrontBColor(LCD_ACTION_PAGE,7,RED_COLOR_HMI);//设置文本背景
					SetTextValue(LCD_ACTION_PAGE,7,"关闭");					
					for(i=1;i<6;i++)
					{
							SetTexFrontBColor(LCD_ACTION_PAGE,i,RED_COLOR_HMI);//设置文本背景
							SetTextValue(LCD_ACTION_PAGE,i,"关闭");							 
			
					}			
//					MOCHINEDATE.Io_Step =0;
					MOCHINEDATE.Io_on_Off_Sucess =0;
			    MOCHINEDATE.FertilizerIoStatrNum = Check_FER_ON(MOCHINEDATE.FertilizerIoNumber);	//检测现在开启第几路施肥阀
				if(MOCHINEDATE.FertilizerIoStatrNum!=0)
				{
					Msgtemp.CmdType = MSG_ACTION_ERTILIZER_ON;								//施肥电磁阀开启
		//								Msgtemp.CmdSrc = 0;
//				Msgtemp.CmdData[0] = 1; //表示任务执行到第几步
				OS_ENTER_CRITICAL();
				PackSendMasterQ(&Msgtemp);
				OS_EXIT_CRITICAL();	
				}
		
			break;	

			default :
			break;
		
		}
	  
	
	}
}
//任务失败关闭所有电器
void Task_error(void)
{
 	u8 os_err;
//	u8 TestTemp[4];
//	u8 sendcount=0;
	  u8 i;
		MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;
	SendIoCount=0;
	while(MOCHINEDATE.Io_on_Off_Sucess)
	{
		delay_ms(500);
		switch(MOCHINEDATE.Io_Step)
		{
			case 0:					//关闭施肥电磁阀
				if(SendIoCount++<3)
						IO_RESETALL_DATA(MOCHINEDATE.FertilizerOnID);
				else
					MOCHINEDATE.Io_Step=1;
				break;
			case 1:				//关闭施肥电机
							IO_OutSet(1, 0);  //关闭吸肥电机		
					MOCHINEDATE.Io_Step=2;
				break;
			case 2:
				if(SendIoCount++<3) //关闭浇水电磁阀
						IO_RESETALL_DATA(MOCHINEDATE.WaterOnID);
				else
					MOCHINEDATE.Io_Step=3;	
				break;			
			case 3:
					MOCHINEDATE.Io_Step=0;	
					MOCHINEDATE.Io_on_Off_Sucess=0;
			break;	
			default:
			break;
		}
		
	}


}
void SetZoneAlarm(u8 AlarmNum, u8 Flag)
{
	if(Flag == 1)
	{
		if(AlarmNum == 1)
		{
			ZoneAlarm[0] |= 0x0f; 
		}
		else if(AlarmNum == 2)
		{
			ZoneAlarm[0] |= 0xf0; 
		}
		else if(AlarmNum == 3)
		{
			ZoneAlarm[1] |= 0x0f; 
		}
		else if(AlarmNum == 4)
		{
			ZoneAlarm[1] |= 0xf0; 
		}
	}
	else if(Flag == 0)
	{
		if(AlarmNum == 1)
		{
			ZoneAlarm[0] &= 0xf0; 
		}
		else if(AlarmNum == 2)
		{
			ZoneAlarm[0] |= 0x0f; 
		}
		else if(AlarmNum == 3)
		{
			ZoneAlarm[1] |= 0xf0; 
		}
		else if(AlarmNum == 4)
		{
			ZoneAlarm[1] |= 0x0f; 
		}
	}
	
}
void MasterCtrl_task(void *pdata)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
//	u8 TestTemp[4];
	OS_CPU_SR  cpu_sr;
	InitativeStruct InitiativeParaTemp;
	MasterQInit();
	while(1)
	{
		delay_ms(50);
		pMsgBlk = ( MsgStruct *) OSQPend ( MasterQMsg,
				10,
				&os_err );
		
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveMasterQ(pMsgBlk);
			switch (Machine_Ctrl_State)
			{
				case MOCHINE_IDLE:
					if(Msgtemp.CmdType == MSG_START)
					{
						if(MOCHINEDATE.WaterTime>=MOCHINEDATE.Fertilizertime) //浇水时间大于施肥时间
						{
							if((MOCHINEDATE.FertilizerIoNumber!=0)&&(MOCHINEDATE.WaterOnID!=0)) //默认浇水和施肥需要同时又数据若要只浇水情况也可以可修改此条件
							{
								Refresh_ACTION_Page();
								SetScreen(1);
//								MOCHINEDATE.lastFlowmeter1 =FlowmeterCount.Flowmeter1;//需要上传的数据
//								MOCHINEDATE.lastFlowmeter2 =FlowmeterCount.Flowmeter2;//需要上传的数据
//								MOCHINEDATE.lastFlowmeter3 =FlowmeterCount.Flowmeter3;//需要上传的数据
//								MOCHINEDATE.lastFlowmeter4 =FlowmeterCount.Flowmeter4;//需要上传的数据
//								MOCHINEDATE.lastFlowmeter5 =FlowmeterCount.Flowmeter5;//需要上传的数据								
                                Machine_Ctrl_State = MOCHINE_ACTION; //任务开始执行
								Msgtemp.CmdType = MSG_ACTION_WATER_ON;
//								Msgtemp.CmdSrc = 0;
//								Msgtemp.CmdData[0] = MOCHINEDATE.WaterOnID; //要开启灌水电磁阀的ID
								OS_ENTER_CRITICAL();
								PackSendMasterQ(&Msgtemp);
								OS_EXIT_CRITICAL();	
							}
							else  //告警
                            {
								SetScreen(2);
                                MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_START_ON;
                                Machine_Ctrl_State = MOCHINE_ALARM; //任务开始执行
								Msgtemp.CmdType = MSG_ALARM;

//								Msgtemp.CmdSrc = 0;
								Msgtemp.CmdData[0] = 1; //第1步就错误了初始化结构体就可以了
								OS_ENTER_CRITICAL();
								PackSendMasterQ(&Msgtemp);
								OS_EXIT_CRITICAL();									
							 }
						}
						else  //告警
                        {
							SetScreen(2);
							  Machine_Ctrl_State = MOCHINE_ALARM; //任务开始执行
								Msgtemp.CmdType = MSG_ALARM;
							  MOCHINEDATE.u16MOCHINEerrorstate =MOCH_E_STATE_START_ON;
//								Msgtemp.CmdSrc = 0;
								Msgtemp.CmdData[0] = 1; //第1步就错误了初始化结构体就可以了
								OS_ENTER_CRITICAL();
								PackSendMasterQ(&Msgtemp);
								OS_EXIT_CRITICAL();							
						 }
					}
			
				break;
				case MOCHINE_ACTION:
					if(Msgtemp.CmdType == MSG_ACTION_WATER_ON) //开启灌水电磁阀
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Water_on();					
					}
					if(Msgtemp.CmdType == MSG_ACTION_WATER_OFF) //关闭灌水电磁阀
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Water_off();					
					}
					if(Msgtemp.CmdType == MSG_ACTION_ERTILIZER_ON) //开启施肥电磁阀
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Fertilizer_on();					
					}
					if(Msgtemp.CmdType == MSG_ACTION_ERTILIZER_OFF) //关闭电磁阀
					{
						MOCHINEDATE.Io_on_Off_Sucess =1;
//						MOCHINEDATE.Io_Step=0;
							Fertilizer_off();					
					}
				break;
				case MOCHINE_ALARM:
					if(Msgtemp.CmdType == MSG_ALARM) //关闭电磁阀
					{
						Task_error();
						switch(Msgtemp.CmdData[0])
						{
								case 1 :		//条件不满足
								SetScreen(2);	
								break;
							  case 2 :    //开启灌溉电磁阀失败
								SetScreen(5);				
								break;					
							  case 3 :	 //开启施肥电磁阀失败
								SetScreen(3);										
								break;
								case 4 :		//吸肥电机开启失败
								SetScreen(6);										
								break;
								default:
								break;
						}
					
					}				
					
				break;
				case MOCHINE_STOP:
					if(Msgtemp.CmdType == MSG_STOP) //关闭电磁阀
					{
						Machine_Ctrl_State =MOCHINE_IDLE; //切换到空闲状态
						SetScreen(4);	
					
					}
				break;				
			  default:
				
				break;
			
			
			
			
			}
//			if(ControlMode == 1)
//			{
//				if(Msgtemp.CmdSrc == GPRS_TASK_CODE)
//				{
//					continue;
//				}
//			}
//			switch(WarterState)
//			{
//				case WARTER_IDLE://
//					if(Msgtemp.CmdType == MSG_WARTERING && Msgtemp.CmdData[0] == 1)
//					{
//						//开启浇水倒计时
//						FlashReadWaterTime((u8*)&WarterRemainderTime); 
//						if(WarterRemainderTime > 0)
//						{
//							IO_OutSet(2,1);
//							SetMDCoil(0,1);
//							//界面显示浇水工作状态
//							
//							//OS_ENTER_CRITICAL();
//							WaterStaShow(1);
//							//OS_EXIT_CRITICAL();
//							
//							if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//							{
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
//							}
//							WarterState = WARTER_WORK;
//						}
//					}
//					break;
//				case WARTER_WORK:
//					if(Msgtemp.CmdType == MSG_WARTERING && Msgtemp.CmdData[0] == 0)
//					{
//						IO_OutSet(2,0);
//						SetMDCoil(0,0);
//						//停止浇水倒计时
//						//OS_ENTER_CRITICAL();
//						WaterStaShow(0);
//						//OS_EXIT_CRITICAL();
//						WarterRemainderTime = 0;
//						//界面显示浇水工作状态
//						if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//						{
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 1;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
//						}
//						WarterState = WARTER_IDLE;
//					}
//					break;
//				default:
//					break;
//			}
//			switch(FertilizerState)
//			{
//				case FERTILIZER_IDLE:
//					if(Msgtemp.CmdType == MSG_FERTILIZER && Msgtemp.CmdData[0] == 1)
//					{
//						FlashReadFertilizerTime((u8*)&FertilizerRemainderTime); 
//						if(FertilizerRemainderTime > 0)
//						{
//							IO_OutSet(3,1);
//							SetMDCoil(1,1);
//							//开启施肥倒计时
//							//界面显示施肥工作状态
//							//OS_ENTER_CRITICAL();
//							FertilizerStaShow(2);
//							//OS_EXIT_CRITICAL();
//							if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//							{
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 1;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
//							}
//							FertilizerState = FERTILIZER_WORK;
//						}
//					}
//					break;
//				case FERTILIZER_WORK:
//					if(Msgtemp.CmdType == MSG_FERTILIZER && Msgtemp.CmdData[0] == 0)
//					{
//						IO_OutSet(3,0);
//						SetMDCoil(1,0);
//						//停止施肥倒计时
//						FertilizerRemainderTime = 0;
//						//界面显示施肥工作状态
//						//OS_ENTER_CRITICAL();
//						FertilizerStaShow(0);
//						//OS_EXIT_CRITICAL();
//						if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//						{
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 1;
//							InitiativeParaTemp.DataNum = 1;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
//						}
//						FertilizerState = FERTILIZER_IDLE;
//					}
//					break;
//				default:
//					break;
//			}
//			if(Msgtemp.CmdType == MSG_MODECHANGE)
//			{
//				if(Msgtemp.CmdData[0] == 0)
//				{
//					ControlMode = 0;
//					//改变网络信号寄存器
//					SetInSta(0,0);
//				}
//				else
//				{
//					ControlMode = 1;
//					//改变网络信号寄存器
//					SetInSta(0,1);
//				}
//				if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//				{
//					InitiativeParaTemp.DataType = INITIATIVE_INPUT_STA;
//					InitiativeParaTemp.DataAddr = 0;
//					InitiativeParaTemp.DataNum = 1;
//					SetIniactivePara(InitiativeParaTemp);
//					ModbusPara.Initiative = 1;
//				}
//			}
//			else if(Msgtemp.CmdType == MSG_RADIOTUBE)
//			{
//				if(Msgtemp.CmdData[1] == 0)
//				{
//					
//					//全部清零
//					SetMDCoil(2,0);
//					SetMDCoil(3,0);
//					SetMDCoil(4,0);
//					SetMDCoil(5,0);
//					SetMDCoil(6,0);
//					SetMDCoil(7,0);
//					SetMDCoil(8,0);
//					SetMDCoil(9,0);
//					SetMDCoil(10,0);
//					SetMDCoil(11,0);
//					SetMDCoil(12,0);
//					SetMDCoil(13,0);
//					SetMDCoil(14,0);
//					SetMDCoil(15,0);
//					SetMDCoil(16,0);
//					SetMDCoil(17,0);
//					//向状态机发送命令
//					//if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//					{
//						InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//						InitiativeParaTemp.DataAddr = 2;
//						InitiativeParaTemp.DataNum = 16;
//						SetIniactivePara(InitiativeParaTemp);
//						ModbusPara.Initiative = 1;
//					}
//					//else
//					{
//						SetRadiotube(Msgtemp.CmdData[0],0);
//					}
//				}
//				else if(Msgtemp.CmdData[1] == 1)
//				{
//					//先把原来置位的清除掉
//					//先清零
//					u8 i ;
//					for(i=0;i<6;i++)
//					{
//						if((ModbusCoil[0]&(1<<(2+i)))>0)
//						{
//							SetRadiotube(i,0);
//						}
//					}
//					for(i=0;i<8;i++)
//					{
//						if((ModbusCoil[1]&(1<<i))>0)
//						{
//							SetRadiotube(i+2,0);
//						}
//					}
//					for(i=0;i<2;i++)
//					{
//						if((ModbusCoil[1]&(1<<i))>0)
//						{
//							SetRadiotube(i+10,0);
//						}
//					}
//					SetMDCoil(2,0);
//					SetMDCoil(3,0);
//					SetMDCoil(4,0);
//					SetMDCoil(5,0);
//					SetMDCoil(6,0);
//					SetMDCoil(7,0);
//					SetMDCoil(8,0);
//					SetMDCoil(9,0);
//					SetMDCoil(10,0);
//					SetMDCoil(11,0);
//					SetMDCoil(12,0);
//					SetMDCoil(13,0);
//					SetMDCoil(14,0);
//					SetMDCoil(15,0);
//					SetMDCoil(16,0);
//					SetMDCoil(17,0);
//					SetMDCoil(Msgtemp.CmdData[0]+2,1);
//					//if(Msgtemp.CmdSrc != GPRS_TASK_CODE)
//					{
//						InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//						InitiativeParaTemp.DataAddr = 2;
//						InitiativeParaTemp.DataNum = 16;
//						SetIniactivePara(InitiativeParaTemp);
//						ModbusPara.Initiative = 1;
//					}
//					//else
//					{
//						SetRadiotube(Msgtemp.CmdData[0],1);
//					}
//				}
//			}
//			else if(Msgtemp.CmdType == MSG_ZONEALARM)
//			{
//				SetZoneAlarm(Msgtemp.CmdData[0],Msgtemp.CmdData[1]);
//				if((ZoneAlarm[0] != ModbusInSta[1]) || (ZoneAlarm[1] != ModbusInSta[2]))
//				{
//					InitiativeParaTemp.DataType = INITIATIVE_INPUT_STA;
//					InitiativeParaTemp.DataAddr = 0;
//					InitiativeParaTemp.DataNum = 24;
//					SetIniactivePara(InitiativeParaTemp);
//					ModbusPara.Initiative = 1;
//					SetSwitchAlarm();
//				}
//			}
		}
	}

}

