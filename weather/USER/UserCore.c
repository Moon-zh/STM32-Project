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
MsgStruct MasterMsg;
OS_EVENT * MasterQMsg;

void* 	MasterMsgBlock[4];
OS_MEM* MasterPartitionPt;
u8 g_u8AgvCtrMsgMem[5][4];

u16 WarterRemainderTime = 0;//浇水剩余工作时间
u16 FertilizerRemainderTime = 0;//施肥剩余工作时间

u8 ControlMode = 0;//水肥一体机控制模式：1：本地 0:网络
//u8 MasterState = 0;//主状态机
u8 WarterState = 0;//浇水状态
u8 FertilizerState = 0;//浇水状态

StrategyStruct WorkStrategy;
//写执行策略
//输入：要设置的值
//返回：0：设置成功 1：设置功能被占用 2：FLASH写失败
//使用时禁止中断
u8 SetStrategy(StrategyStruct *Strategy)
{
	static u8 Flag = 0;
	if(Flag == 1)
		return 1 ;
	Flag = 1;
	WorkStrategy.PumpWFlag = Strategy->PumpWFlag;
	WorkStrategy.PumpFFlag = Strategy->PumpFFlag;
	WorkStrategy.Passageway[0] = Strategy->Passageway[0];
	WorkStrategy.Passageway[1] = Strategy->Passageway[1];
	WorkStrategy.Passageway[2] = Strategy->Passageway[2];
	WorkStrategy.Passageway[3] = Strategy->Passageway[3];
	WorkStrategy.Passageway[4] = Strategy->Passageway[4];
	WorkStrategy.Passageway[5] = Strategy->Passageway[5];
	WorkStrategy.Zone = Strategy->Zone;
	WorkStrategy.WorkHour = Strategy->WorkHour;
	WorkStrategy.WorkMinute = Strategy->WorkMinute;
	WorkStrategy.WorkDay = Strategy->WorkDay;
	WorkStrategy.StartHour = Strategy->StartHour;
	WorkStrategy.StartMinute = Strategy->StartMinute;
	//写入Flash
	Flag = 0;
	return 0;
}
//策略开始
//输入：策略参数
//输出：无
// alfred 还剩MODBUS线圈没有写 水泵和肥泵的控制没有写
void StrategyStart(StrategyStruct *Strategy)
{
	u8 ConduitF = 0;
	u8 i = 0;
	
	//打开分区电磁阀
	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YSETSINGLE);
	SetMDCoil(ConduitF + 2 - 1 , 1);
	delay_ms(110);
	
	if(Strategy->PumpWFlag == 1)
	{
		//打开水泵
		SetMDCoil(0 , 1);
	}
	else if(Strategy->PumpWFlag == 0)
	{
		//报错或者打开水泵
	}
	if(Strategy->PumpFFlag == 1)
	{
		//打开肥泵
		SetMDCoil(1 , 1);
	}
	else if(Strategy->PumpFFlag == 0)
	{
		//关闭肥泵
		SetMDCoil(1 , 0);
	}
	//打开通道电磁阀 
	for(i = 0;i<6; i++)
	{
		if(Strategy->Passageway[i] == 1)
		{
			ConduitF |= i<<i;
			SetMDCoil(8+i , 1);
		}
	}
//	ZoneCtrl_Set(ConduitF,YSETMUL);
}
//策略停止
//输入：策略参数
//输出：无
// alfred 还剩MODBUS线圈没有写 水泵和肥泵的控制没有写
void StrategyStop(StrategyStruct *Strategy)
{
	u8 ConduitF = 0;
	u8 i = 0;
	
	//关闭水泵
	SetMDCoil(0 , 0);
	//关闭肥泵
	SetMDCoil(1 , 0);
	//关闭通道电磁阀 
	for(i = 0;i<6; i++)
	{
		SetMDCoil(8+i , 0);
	}
//	ZoneCtrl_Set(0,YCLEAR);
	delay_ms(110);
	//关闭分区电磁阀
	ConduitF = Strategy->Zone;
//	ZoneCtrl_Set(ConduitF,YCLEAR);
	SetMDCoil(ConduitF + 2 - 1  , 0);
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
				  						5,
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
u8 RainFallCntF = 0;
u8 MasterTaskin = 0;
u8 MasterTaskout = 0;
void MasterCtrl_task(void *pdata)
{
	//u8 os_err;
	//MsgStruct * pMsgBlk = NULL;
	//MsgStruct Msgtemp;
//	u8 TestTemp[4];
//	OS_CPU_SR  cpu_sr;
//	InitativeStruct InitiativeParaTemp;
	MasterQInit();
	while(1)
	{
		delay_ms(50);
		MasterTaskin++;
		if(RainFlag == 1)
		{
			RainFallCntF = 1;
		}
		else if((RainFallCntF == 1) && (RainFlag == 0))
		{
			RainfallClearF = 1;
			RainFallCntF = 0;
		}
		MasterTaskout++;
	}

}

