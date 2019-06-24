#include "sys.h"
#include "includes.h"  
#include "UserCore.h"
#include "Delay.h"
#include "ZoneCtrl.h"
#include "Emw3060.h"
#include "hmi_driver.h"
#include "UserHmi.h"
#include "IO_BSP.h"
#include "FlashDivide.h"
#include "IO_BSP.h"
#include "Alarm.h"

u32 g_FaultCode = 0;//故障码用于处理故障和报警使用

OS_EVENT * WaterAddQMsg;
void* 	WaterAddMsgBlock[4];
OS_MEM* WaterAddPartitionPt;
u8 g_u8WaterAddMsgMem[5][4];

OS_EVENT * StirQMsg;
void* 	StirMsgBlock[4];
OS_MEM* StirPartitionPt;
u8 g_u8StirMsgMem[5][4];

u8 ControlMode = 0;//	控制模式：1：本地 0:网络


//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 MasterQInit(void)
{
	INT8U os_err;
	
	WaterAddQMsg = OSQCreate ( WaterAddMsgBlock, 4);
	
	if(WaterAddQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	WaterAddPartitionPt = OSMemCreate (
										g_u8WaterAddMsgMem,
				  						5,
				  						4,
				  						&os_err 
				  					);
	if(os_err != OS_ERR_NONE)
	{
		return 2;
	}
	//
	StirQMsg = OSQCreate ( StirMsgBlock, 4);
	
	if(StirQMsg == (OS_EVENT *)0)
	{
		return 3;
	}
	
	StirPartitionPt = OSMemCreate (
										g_u8StirMsgMem,
				  						5,
				  						4,
				  						&os_err 
				  					);
	if(os_err != OS_ERR_NONE)
	{
		return 4;
	}
	return 0;
}

//消息打包发走
//输入：消息信息
//输出：0：发送成功
//		1：申请内存失败
//		2：发送消息失败

u8 PackSendWaterAddQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(WaterAddPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( WaterAddQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(WaterAddPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveWaterAddQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(WaterAddPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
u8 WaterAddState1 = 0;
u8 WaterAddState2 = 0;
u8 WaterAddState3 = 0;
u8 WaterAddState4 = 0;
u8 WaterAddState5 = 0;

u8 StirState1 = 0;
u8 StirState2 = 0;
u8 StirState3 = 0;
u8 StirState4 = 0;
u8 StirState5 = 0;

void WaterAdd_task(void *pdata)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	MsgStruct MsgSend;
//	InitativeStruct InitiativeParaTemp;
//	u8 TestTemp[4];
//	OS_CPU_SR  cpu_sr;
	//InitativeStruct InitiativeParaTemp;
	MasterQInit();
	while(1)
	{
		delay_ms(20);
		pMsgBlk = ( MsgStruct *) OSQPend ( WaterAddQMsg,
				10,
				&os_err );
		
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveWaterAddQ(pMsgBlk);
			if(ControlMode == 1)
			{
				if(Msgtemp.CmdSrc == GPRS_TASK_CODE)
				{
					continue;
				}
			}
			switch(Msgtemp.CmdData[0])
			{
				case 1:
					if(Msgtemp.CmdType == MSG_WATERADDCANCEL)
					{
						WaterAddState1 = WATERADD_CANCEL;
					}
					switch(WaterAddState1)
					{
						case WARTERADD_IDLE:
							if(Msgtemp.CmdType == MSG_WATERADDSTART)
							{
								SetWateradd(1);
								//打开电磁阀
								IO_OutSet(Msgtemp.CmdData[0],1);
								//发送打开消息,切换状态机状态
								
								delay_ms(100);
								if(ReadDin(Msgtemp.CmdData[0]) == 1)
								{
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								else
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0];
								PackSendWaterAddQ(&MsgSend);
								WaterAddState1 = WARTERADD_WAITESWITCH;
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITESWITCH:
							if(Msgtemp.CmdType == MSG_SWITCHOPEN)
							{
								//读取液位
								MsgSend.CmdType = ALARMMSG_LEVEL_OPEN;
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								WaterAddState1 = WARTERADD_WAITEFULL;
//								SetMDCoil(0,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(24,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);

							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								g_FaultCode |= FAULT_SW1;
								WaterAddState1 = WARTERADD_IDLE;
								RecoverWateradd(1);
//								SetMDCoil(0,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							SetRemoteRead(24,0);
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdType = UPLOAD_WATERSWITCH;
							PackSendRemoteQ(&MsgSend);
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITEFULL:
							if(Msgtemp.CmdType == MSG_WATERFULL)
							{
								
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								delay_ms(100);
//								SetMDCoil(0,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(24,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
								if(ReadDin(Msgtemp.CmdData[0]) == 0)
								{
									WaterAddState1 = WARTERADD_IDLE;
									RecoverWateradd(1);
								}
								else//如果关闭时出错
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									//MsgSend.CmdSrc = WATERADD_TASK_CODE;
									//MsgSend.CmdData[0] = Msgtemp.CmdData[0];
									//PackSendWaterAddQ(&MsgSend);
									WaterAddState1 = WARTERADD_IDLE;
									RecoverWateradd(1);
								}
								
							}
							else if(Msgtemp.CmdType == MSG_LEVEERR)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								g_FaultCode |= FAULT_LEVEL1;
								WaterAddState1 = WARTERADD_IDLE;
								RecoverWateradd(1);
//								SetMDCoil(0,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(24,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//报警 
								g_FaultCode |= FAULT_SW1;
								WaterAddState1 = WARTERADD_IDLE;
								RecoverWateradd(1);
							}
							else 
							{
								//其他无用消息
							}
							break;
						case WATERADD_CANCEL:
							//关闭电磁阀
							IO_OutSet(Msgtemp.CmdData[0],0);
							delay_ms(100);
							if(ReadDin(Msgtemp.CmdData[0]) == 0)
							{
								WaterAddState1 = WARTERADD_IDLE;
								RecoverWateradd(1);
							}
							else//如果关闭时出错
							{
								//报警 
								g_FaultCode |= FAULT_SW1;
								WaterAddState1 = WARTERADD_IDLE;
								RecoverWateradd(1);
							}
							MsgSend.CmdType = ALARMMSG_LEVEL_CLOSE;
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
							PackSendAlarmQ(&MsgSend);
//							SetMDCoil(0,0);
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 10;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
							SetRemoteRead(24,0);
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdType = UPLOAD_WATERSWITCH;
							PackSendRemoteQ(&MsgSend);
							break;
						default:
							break;
					}
					break;
				case 2:
					if(Msgtemp.CmdType == MSG_WATERADDCANCEL)
					{
						WaterAddState2 = WATERADD_CANCEL;
					}
					switch(WaterAddState2)
					{
						case WARTERADD_IDLE:
							if(Msgtemp.CmdType == MSG_WATERADDSTART)
							{
								SetWateradd(2);
								//打开电磁阀
								IO_OutSet(Msgtemp.CmdData[0],1);
								//发送打开消息,切换状态机状态
								
								delay_ms(100);
								if(ReadDin(Msgtemp.CmdData[0]) == 1)
								{
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								else
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0];
								PackSendWaterAddQ(&MsgSend);
								WaterAddState2 = WARTERADD_WAITESWITCH;
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITESWITCH:
							if(Msgtemp.CmdType == MSG_SWITCHOPEN)
							{
								//读取液位								
								MsgSend.CmdType = ALARMMSG_LEVEL_OPEN;
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								WaterAddState2 = WARTERADD_WAITEFULL;
//								SetMDCoil(1,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(25,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								g_FaultCode |= FAULT_SW2;
								WaterAddState2 = WARTERADD_IDLE;
								RecoverWateradd(2);
//								SetMDCoil(1,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(25,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITEFULL:
							if(Msgtemp.CmdType == MSG_WATERFULL)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								delay_ms(100);
//								SetMDCoil(1,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(25,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
								if(ReadDin(Msgtemp.CmdData[0]) == 0)
								{
									WaterAddState2 = WARTERADD_IDLE;
									RecoverWateradd(2);
								}
								else//如果关闭时出错
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									//MsgSend.CmdSrc = WATERADD_TASK_CODE;
									//MsgSend.CmdData[0] = Msgtemp.CmdData[0];
									//PackSendWaterAddQ(&MsgSend);
									WaterAddState2 = WARTERADD_IDLE;
									RecoverWateradd(2);
								}
								
							}
							else if(Msgtemp.CmdType == MSG_LEVEERR)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								g_FaultCode |= FAULT_LEVEL2;
								WaterAddState2 = WARTERADD_IDLE;
								RecoverWateradd(2);
//								SetMDCoil(1,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(25,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//报警 
								g_FaultCode |= FAULT_SW2;
								WaterAddState2 = WARTERADD_IDLE;
								RecoverWateradd(2);
//								SetMDCoil(1,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(25,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else 
							{
								//其他无用消息
							}
							break;
						case WATERADD_CANCEL:
							//关闭电磁阀
							IO_OutSet(Msgtemp.CmdData[0],0);
							delay_ms(100);
//							SetMDCoil(1,0);
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 10;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
							SetRemoteRead(25,0);
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdType = UPLOAD_WATERSWITCH;
							PackSendRemoteQ(&MsgSend);
							if(ReadDin(Msgtemp.CmdData[0]) == 0)
							{
								WaterAddState2 = WARTERADD_IDLE;
								RecoverWateradd(2);
							}
							else//如果关闭时出错
							{
								//报警 
								g_FaultCode |= FAULT_SW2;
								WaterAddState2 = WARTERADD_IDLE;
								RecoverWateradd(2);
							}
							MsgSend.CmdType = ALARMMSG_LEVEL_CLOSE;
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
							PackSendAlarmQ(&MsgSend);
							break;
						default:
							break;
					}
					break;
				case 3:
					if(Msgtemp.CmdType == MSG_WATERADDCANCEL)
					{
						WaterAddState3 = WATERADD_CANCEL;
					}
					switch(WaterAddState3)
					{
						case WARTERADD_IDLE:
							if(Msgtemp.CmdType == MSG_WATERADDSTART)
							{
								SetWateradd(3);
								//打开电磁阀
								IO_OutSet(Msgtemp.CmdData[0],1);
								//发送打开消息,切换状态机状态
								
								delay_ms(100);
								if(ReadDin(Msgtemp.CmdData[0]) == 1)
								{
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								else
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0];
								PackSendWaterAddQ(&MsgSend);
								WaterAddState3 = WARTERADD_WAITESWITCH;
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITESWITCH:
							if(Msgtemp.CmdType == MSG_SWITCHOPEN)
							{
//								SetMDCoil(2,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(26,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
								//读取液位								
								MsgSend.CmdType = ALARMMSG_LEVEL_OPEN;
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								WaterAddState3 = WARTERADD_WAITEFULL;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								g_FaultCode |= FAULT_SW3;
								WaterAddState3 = WARTERADD_IDLE;
								RecoverWateradd(3);
//								SetMDCoil(2,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(26,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITEFULL:
							if(Msgtemp.CmdType == MSG_WATERFULL)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								delay_ms(100);
//								SetMDCoil(2,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(26,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
								if(ReadDin(Msgtemp.CmdData[0]) == 0)
								{
									WaterAddState3 = WARTERADD_IDLE;
									RecoverWateradd(3);
								}
								else//如果关闭时出错
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									//MsgSend.CmdSrc = WATERADD_TASK_CODE;
									//MsgSend.CmdData[0] = Msgtemp.CmdData[0];
									//PackSendWaterAddQ(&MsgSend);
									WaterAddState3 = WARTERADD_IDLE;
									RecoverWateradd(3);
								}
								
							}
							else if(Msgtemp.CmdType == MSG_LEVEERR)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								g_FaultCode |= FAULT_LEVEL3;
								WaterAddState3 = WARTERADD_IDLE;
								RecoverWateradd(3);
//								SetMDCoil(2,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(26,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//报警 
								g_FaultCode |= FAULT_SW3;
								WaterAddState3 = WARTERADD_IDLE;
								RecoverWateradd(3);
//								SetMDCoil(2,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(26,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else 
							{
								//其他无用消息
							}
							break;
						
						case WATERADD_CANCEL:
							//关闭电磁阀
							IO_OutSet(Msgtemp.CmdData[0],0);
							delay_ms(100);
//							SetMDCoil(2,0);
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 10;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
							SetRemoteRead(26,0);
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdType = UPLOAD_WATERSWITCH;
							PackSendRemoteQ(&MsgSend);
							if(ReadDin(Msgtemp.CmdData[0]) == 0)
							{
								WaterAddState3 = WARTERADD_IDLE;
								RecoverWateradd(3);
							}
							else//如果关闭时出错
							{
								//报警 
								g_FaultCode |= FAULT_SW3;
								WaterAddState3 = WARTERADD_IDLE;
								RecoverWateradd(3);
							}
							MsgSend.CmdType = ALARMMSG_LEVEL_CLOSE;
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
							PackSendAlarmQ(&MsgSend);
							break;
						default:
							break;
					}
					break;
				case 4:
					if(Msgtemp.CmdType == MSG_WATERADDCANCEL)
					{
						WaterAddState4 = WATERADD_CANCEL;
					}
					switch(WaterAddState4)
					{
						case WARTERADD_IDLE:
							if(Msgtemp.CmdType == MSG_WATERADDSTART)
							{
								SetWateradd(4);
								//打开电磁阀
								IO_OutSet(Msgtemp.CmdData[0],1);
								//发送打开消息,切换状态机状态
								
								delay_ms(100);
								if(ReadDin(Msgtemp.CmdData[0]) == 1)
								{
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								else
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									MsgSend.CmdType = MSG_SWITCHOPEN;
								}
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0];
								PackSendWaterAddQ(&MsgSend);
								WaterAddState4 = WARTERADD_WAITESWITCH;
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITESWITCH:
							if(Msgtemp.CmdType == MSG_SWITCHOPEN)
							{
//								SetMDCoil(3,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(27,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
								//读取液位								
								MsgSend.CmdType = ALARMMSG_LEVEL_OPEN;
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								WaterAddState4 = WARTERADD_WAITEFULL;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//关闭电磁阀
								g_FaultCode |= FAULT_SW4;
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								WaterAddState4 = WARTERADD_IDLE;
								RecoverWateradd(4);
//								SetMDCoil(3,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(27,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITEFULL:
							if(Msgtemp.CmdType == MSG_WATERFULL)
							{
								//关闭电磁阀
								IO_OutSet(Msgtemp.CmdData[0],0);
								delay_ms(100);
//								SetMDCoil(3,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(27,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
								if(ReadDin(Msgtemp.CmdData[0]) == 0)
								{
									WaterAddState4 = WARTERADD_IDLE;
									RecoverWateradd(4);
								}
								else//如果关闭时出错
								{
									//MsgSend.CmdType = MSG_SWITCHERR;
									//MsgSend.CmdSrc = WATERADD_TASK_CODE;
									//MsgSend.CmdData[0] = Msgtemp.CmdData[0];
									//PackSendWaterAddQ(&MsgSend);
									WaterAddState4 = WARTERADD_IDLE;
									RecoverWateradd(4);
								}
								
							}
							else if(Msgtemp.CmdType == MSG_LEVEERR)
							{
								//关闭电磁阀
								g_FaultCode |= FAULT_LEVEL4;
								IO_OutSet(Msgtemp.CmdData[0],0);
								//报警 
								WaterAddState4 = WARTERADD_IDLE;
								RecoverWateradd(4);
//								SetMDCoil(3,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(27,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//报警 
								g_FaultCode |= FAULT_SW4;
								WaterAddState4 = WARTERADD_IDLE;
								RecoverWateradd(4);
//								SetMDCoil(3,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(27,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_WATERSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else 
							{
								//其他无用消息
							}
							break;
						
						case WATERADD_CANCEL:
							//关闭电磁阀
							IO_OutSet(Msgtemp.CmdData[0],0);
							delay_ms(100);
//							SetMDCoil(3,0);
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 10;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
							SetRemoteRead(27,0);
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdType = UPLOAD_WATERSWITCH;
							PackSendRemoteQ(&MsgSend);
							if(ReadDin(Msgtemp.CmdData[0]) == 0)
							{
								WaterAddState4 = WARTERADD_IDLE;
								RecoverWateradd(4);
							}
							else//如果关闭时出错
							{
								//报警 
								g_FaultCode |= FAULT_SW4;
								WaterAddState4 = WARTERADD_IDLE;
								RecoverWateradd(4);
							}
							MsgSend.CmdType = ALARMMSG_LEVEL_CLOSE;
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
							PackSendAlarmQ(&MsgSend);
							break;
						default:
							break;
					}
					break;
				case 5:
					if(Msgtemp.CmdType == MSG_WATERADDCANCEL)
					{
						WaterAddState5 = WATERADD_CANCEL;
					}
					switch(WaterAddState5)
					{
						case WARTERADD_IDLE:
							if(Msgtemp.CmdType == MSG_WATERADDSTART)
							{
								SetWateradd(5);
								//打开电磁阀
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_ON;
								MsgSend.CmdData[0] = 5;
								PackSendZoneQ(&MsgSend);
								WaterAddState5 = WARTERADD_WAITESWITCH;
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITESWITCH:
							if(Msgtemp.CmdType == MSG_SWITCHOPEN)
							{
//								SetMDCoil(4,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								//读取液位								
								MsgSend.CmdType = ALARMMSG_LEVEL_OPEN;
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								WaterAddState5 = WARTERADD_WAITEFULL;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHERR)
							{
								//报警
								g_FaultCode |= FAULT_SW5;
								WaterAddState5 = WARTERADD_IDLE;
								RecoverWateradd(5);
//								SetMDCoil(4,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
							else
							{
								//没用的消息
							}
							break;
						case WARTERADD_WAITEFULL:
							if(Msgtemp.CmdType == MSG_WATERFULL)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = 5;
								PackSendZoneQ(&MsgSend);
								WaterAddState5 = WARTERADD_IDLE;
								RecoverWateradd(5);
//								SetMDCoil(4,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								
							}
							else if(Msgtemp.CmdType == MSG_LEVEERR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = 5;
								PackSendZoneQ(&MsgSend);
								//报错
								g_FaultCode |= FAULT_LEVEL5;
								WaterAddState5 = WARTERADD_IDLE;
								RecoverWateradd(5);
//								SetMDCoil(4,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 0;
//								InitiativeParaTemp.DataNum = 10;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
							else 
							{
								//其他无用消息
							}
							break;
						case WATERADD_CANCEL:
							//关闭电磁阀
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdType = YSETSINGLE_OFF;
							MsgSend.CmdData[0] = 5;
							PackSendZoneQ(&MsgSend);
							
							MsgSend.CmdType = ALARMMSG_LEVEL_CLOSE;
							MsgSend.CmdSrc = WATERADD_TASK_CODE;
							MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
							PackSendAlarmQ(&MsgSend);
							WaterAddState5 = WARTERADD_IDLE;
							RecoverWateradd(5);
//							SetMDCoil(4,0);
//							InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//							InitiativeParaTemp.DataAddr = 0;
//							InitiativeParaTemp.DataNum = 10;
//							SetIniactivePara(InitiativeParaTemp);
//							ModbusPara.Initiative = 1;
							break;
						default:
							break;
					}
					break;
				default:
					break;
				}
		}
	}

}
//消息打包发走
//输入：消息信息
//输出：0：发送成功
//		1：申请内存失败
//		2：发送消息失败

u8 PackSendStirQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(StirPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( StirQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(StirPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveStirQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(StirPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void Stir_task(void *pdata)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	MsgStruct MsgSend;
	//InitativeStruct InitiativeParaTemp;
	//	u8 TestTemp[4];
	//	OS_CPU_SR  cpu_sr;
	//InitativeStruct InitiativeParaTemp;
	while(1)
	{
		
		delay_ms(20);
		pMsgBlk = ( MsgStruct *) OSQPend ( StirQMsg,
				10,
				&os_err );
		
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveStirQ(pMsgBlk);
			if(ControlMode == 1)
			{
				if(Msgtemp.CmdSrc == GPRS_TASK_CODE)
				{
					continue;
				}
			}
			switch(Msgtemp.CmdData[0])
			{
				case 1:
					if(Msgtemp.CmdType == MSG_STIRCANCEL)
					{
						//关闭电磁阀
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdType = YSETSINGLE_OFF;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendZoneQ(&MsgSend);
						
						//停止计时
						MsgSend.CmdType = ALARMMSG_STIRTIME_CLOSE;
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendAlarmQ(&MsgSend);
						StirState1 = STIR_WATESWITCH_OFF;
					}
					switch(StirState1)
					{
						case STIR_IDLE:
							if(Msgtemp.CmdType == MSG_STIRSTART)
							{
								SetStir(1);
								//开启电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_ON;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState1 = STIR_WATESWITCH_ON;
							}
							else 
							{
								
							}
							break;
						case STIR_WATESWITCH_ON:
							if(Msgtemp.CmdType == MSG_SWITCHONOK)
							{
							{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;//8路IO第5路输出
								PackSendAlarmQ(&MsgSend);
								StirState1 = STIR_WAITETIME;
//								SetMDCoil(5,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 5;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(28,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_ON;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState1 = STIR_VERIFYSWITCH_ON;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHONERROR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR1;
								StirState1 = STIR_IDLE;
								RecoverStir(1);
							}
							else 
							{
								
							}
							break;
						case STIR_VERIFYSWITCH_ON:
							if(Msgtemp.CmdType == MSG_VERFYONOK)
							{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;//8路IO第5路输出
								PackSendAlarmQ(&MsgSend);
								StirState1 = STIR_WAITETIME;
//								SetMDCoil(5,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 5;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(28,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYONERR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR1;
								StirState1 = STIR_IDLE;
								RecoverStir(1);
							}
							else
							{
								
							}
							break;
						case STIR_WAITETIME:
							if(Msgtemp.CmdType == MSG_TIMEOVER)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState1 = STIR_WATESWITCH_OFF;
							}
							else
							{
								
							}
							break;
						case STIR_WATESWITCH_OFF:
							if(Msgtemp.CmdType == MSG_SWITCHOFFOK)
							{
							{
								StirState1 = STIR_IDLE;
								RecoverStir(1);
//								SetMDCoil(5,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 5;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(28,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_OFF;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState1 = STIR_VERIFYSWITCH_OFF;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHOFFERROR)
							{
								//报警
								g_FaultCode |= FAULT_STIR1;
								StirState1 = STIR_IDLE;
								RecoverStir(1);
							}
							break;
						case STIR_VERIFYSWITCH_OFF:
							if(Msgtemp.CmdType == MSG_VERFYOFFOK)
							{
								StirState1 = STIR_IDLE;
								RecoverStir(1);
//								SetMDCoil(5,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 5;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(28,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYOFFERR)
							{
								//报警
								g_FaultCode |= FAULT_STIR1;
								StirState1 = STIR_IDLE;
								RecoverStir(1);
//								SetMDCoil(5,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 5;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(28,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							break;
						default:
							break;
					}
					break;
				case 2:
					if(Msgtemp.CmdType == MSG_STIRCANCEL)
					{
						//关闭电磁阀
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdType = YSETSINGLE_OFF;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendZoneQ(&MsgSend);
						
						//停止计时
						MsgSend.CmdType = ALARMMSG_STIRTIME_CLOSE;
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendAlarmQ(&MsgSend);
						StirState2 = STIR_WATESWITCH_OFF;
					}
					switch(StirState2)
					{
						case STIR_IDLE:
							if(Msgtemp.CmdType == MSG_STIRSTART)
							{
								SetStir(2);
								//开启电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_ON;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState2 = STIR_WATESWITCH_ON;
							}
							else 
							{
								
							}
							break;
						case STIR_WATESWITCH_ON:
							if(Msgtemp.CmdType == MSG_SWITCHONOK)
							{
							{
//								SetMDCoil(6,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 6;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(29,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);

								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState2 = STIR_WAITETIME;
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_ON;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState2 = STIR_VERIFYSWITCH_ON;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHONERROR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR2;
								StirState2 = STIR_IDLE;
								RecoverStir(2);
							}
							else 
							{
								
							}
							break;
						case STIR_VERIFYSWITCH_ON:
							if(Msgtemp.CmdType == MSG_VERFYONOK)
							{
//								SetMDCoil(6,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 6;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(29,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState2 = STIR_WAITETIME;
							}
							else if(Msgtemp.CmdType == MSG_VERFYONERR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR2;
								StirState2 = STIR_IDLE;
								RecoverStir(2);
							}
							else
							{
								
							}
							break;
						case STIR_WAITETIME:
							if(Msgtemp.CmdType == MSG_TIMEOVER)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState2 = STIR_WATESWITCH_OFF;
							}
							else
							{
								
							}
							break;
						case STIR_WATESWITCH_OFF:
							if(Msgtemp.CmdType == MSG_SWITCHOFFOK)
							{{
								StirState2 = STIR_IDLE;
								RecoverStir(2);
//								SetMDCoil(6,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 6;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(29,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_OFF;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState2 = STIR_VERIFYSWITCH_OFF;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHOFFERROR)
							{
								//报警
								g_FaultCode |= FAULT_STIR2;
							}
							break;
						case STIR_VERIFYSWITCH_OFF:
							if(Msgtemp.CmdType == MSG_VERFYOFFOK)
							{
								StirState2 = STIR_IDLE;
								RecoverStir(2);
//								SetMDCoil(6,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 6;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(29,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYOFFERR)
							{
								//报警
								g_FaultCode |= FAULT_STIR2;
								StirState2 = STIR_IDLE;
								RecoverStir(2);
//								SetMDCoil(6,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 6;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(29,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							break;
						default:
							break;
					}

					break;
				case 3:
					if(Msgtemp.CmdType == MSG_STIRCANCEL)
					{
						//关闭电磁阀
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdType = YSETSINGLE_OFF;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendZoneQ(&MsgSend);
						
						//停止计时
						MsgSend.CmdType = ALARMMSG_STIRTIME_CLOSE;
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendAlarmQ(&MsgSend);
						StirState3 = STIR_WATESWITCH_OFF;
					}
					switch(StirState3)
					{
						case STIR_IDLE:
							if(Msgtemp.CmdType == MSG_STIRSTART)
							{
								SetStir(3);
								//开启电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_ON;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState3 = STIR_WATESWITCH_ON;
							}
							else 
							{
								
							}
							break;
						case STIR_WATESWITCH_ON:
							if(Msgtemp.CmdType == MSG_SWITCHONOK)
							{{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState3 = STIR_WAITETIME;
//								SetMDCoil(7,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 7;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(30,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_ON;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState3 = STIR_VERIFYSWITCH_ON;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHONERROR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR3;
								StirState3 = STIR_IDLE;
								RecoverStir(3);
							}
							else 
							{
								
							}
							break;
						case STIR_VERIFYSWITCH_ON:
							if(Msgtemp.CmdType == MSG_VERFYONOK)
							{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState3 = STIR_WAITETIME;
//								SetMDCoil(7,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 7;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(30,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYONERR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR3;
								StirState3 = STIR_IDLE;
								RecoverStir(3);
							}
							else
							{
								
							}
							break;
						case STIR_WAITETIME:
							if(Msgtemp.CmdType == MSG_TIMEOVER)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState3 = STIR_WATESWITCH_OFF;
							}
							else
							{
								
							}
							break;
						case STIR_WATESWITCH_OFF:
							if(Msgtemp.CmdType == MSG_SWITCHOFFOK)
							{{
								StirState3 = STIR_IDLE;
								RecoverStir(3);
//								SetMDCoil(7,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 7;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(30,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_OFF;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState3 = STIR_VERIFYSWITCH_OFF;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHOFFERROR)
							{
								//报警
								g_FaultCode |= FAULT_STIR3;
							}
							break;
						case STIR_VERIFYSWITCH_OFF:
							if(Msgtemp.CmdType == MSG_VERFYOFFOK)
							{
								StirState3 = STIR_IDLE;
								RecoverStir(3);
//								SetMDCoil(7,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 7;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(30,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYOFFERR)
							{
								//报警
								g_FaultCode |= FAULT_STIR3;
								StirState3 = STIR_IDLE;
								RecoverStir(3);
//								SetMDCoil(7,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 7;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(30,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							break;
						default:
							break;
					}

					break;
				case 4:
					if(Msgtemp.CmdType == MSG_STIRCANCEL)
					{
						//关闭电磁阀
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdType = YSETSINGLE_OFF;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendZoneQ(&MsgSend);
						
						//停止计时
						MsgSend.CmdType = ALARMMSG_STIRTIME_CLOSE;
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendAlarmQ(&MsgSend);
						StirState4 = STIR_WATESWITCH_OFF;
					}
					switch(StirState4)
					{
						case STIR_IDLE:
							if(Msgtemp.CmdType == MSG_STIRSTART)
							{
								SetStir(4);
								//开启电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_ON;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState4 = STIR_WATESWITCH_ON;
							}
							else 
							{
								
							}
							break;
						case STIR_WATESWITCH_ON:
							if(Msgtemp.CmdType == MSG_SWITCHONOK)
							{{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState4 = STIR_WAITETIME;
//								SetMDCoil(8,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 8;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(31,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_ON;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState4 = STIR_VERIFYSWITCH_ON;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHONERROR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR4;
								StirState4 = STIR_IDLE;
								RecoverStir(4);
							}
							else 
							{
								
							}
							break;
						case STIR_VERIFYSWITCH_ON:
							if(Msgtemp.CmdType == MSG_VERFYONOK)
							{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState4 = STIR_WAITETIME;
//								SetMDCoil(8,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 8;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(31,1);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYONERR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR4;
								StirState4 = STIR_IDLE;
								RecoverStir(4);
							}
							else
							{
								
							}
							break;
						case STIR_WAITETIME:
							if(Msgtemp.CmdType == MSG_TIMEOVER)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState4 = STIR_WATESWITCH_OFF;
							}
							else
							{
								
							}
							break;
						case STIR_WATESWITCH_OFF:
							if(Msgtemp.CmdType == MSG_SWITCHOFFOK)
							{{
								StirState4 = STIR_IDLE;
								RecoverStir(4);
//								SetMDCoil(8,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 8;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(31,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_OFF;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState4 = STIR_VERIFYSWITCH_OFF;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHOFFERROR)
							{
								//报警
								g_FaultCode |= FAULT_STIR4;
							}
							break;
						case STIR_VERIFYSWITCH_OFF:
							if(Msgtemp.CmdType == MSG_VERFYOFFOK)
							{
								StirState4 = STIR_IDLE;
								RecoverStir(4);
//								SetMDCoil(8,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 8;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(31,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							else if(Msgtemp.CmdType == MSG_VERFYOFFERR)
							{
								//报警
								g_FaultCode |= FAULT_STIR4;
								StirState4 = STIR_IDLE;
								RecoverStir(4);
//								SetMDCoil(8,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 8;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
								SetRemoteRead(31,0);
								MsgSend.CmdSrc = WATERADD_TASK_CODE;
								MsgSend.CmdType = UPLOAD_STIRSWITCH;
								PackSendRemoteQ(&MsgSend);
							}
							break;
						default:
							break;
					}

					break;
				case 5:
					if(Msgtemp.CmdType == MSG_STIRCANCEL)
					{
						//关闭电磁阀
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdType = YSETSINGLE_OFF;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendZoneQ(&MsgSend);
						
						//停止计时
						MsgSend.CmdType = ALARMMSG_STIRTIME_CLOSE;
						MsgSend.CmdSrc = STIR_TASK_CODE;
						MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
						PackSendAlarmQ(&MsgSend);
						StirState5 = STIR_WATESWITCH_OFF;
					}
					switch(StirState5)
					{
						case STIR_IDLE:
							if(Msgtemp.CmdType == MSG_STIRSTART)
							{
								SetStir(5);
								//开启电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_ON;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState5 = STIR_WATESWITCH_ON;
							}
							else 
							{
								
							}
							break;
						case STIR_WATESWITCH_ON:
							if(Msgtemp.CmdType == MSG_SWITCHONOK)
							{{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState5 = STIR_WAITETIME;
//								SetMDCoil(9,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 9;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_ON;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState5 = STIR_VERIFYSWITCH_ON;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHONERROR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR5;
								StirState5 = STIR_IDLE;
								RecoverStir(5);
							}
							else 
							{
								
							}
							break;
						case STIR_VERIFYSWITCH_ON:
							if(Msgtemp.CmdType == MSG_VERFYONOK)
							{
								//开始计时
								MsgSend.CmdType = ALARMMSG_STIRTIME_OPEN;
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendAlarmQ(&MsgSend);
								StirState5 = STIR_WAITETIME;
//								SetMDCoil(9,1);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 9;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
							else if(Msgtemp.CmdType == MSG_VERFYONERR)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								//报警
								g_FaultCode |= FAULT_STIR5;
								StirState5 = STIR_IDLE;
								RecoverStir(5);
							}
							else
							{
								
							}
							break;
						case STIR_WAITETIME:
							if(Msgtemp.CmdType == MSG_TIMEOVER)
							{
								//关闭电磁阀
								MsgSend.CmdSrc = STIR_TASK_CODE;
								MsgSend.CmdType = YSETSINGLE_OFF;
								MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								PackSendZoneQ(&MsgSend);
								StirState5 = STIR_WATESWITCH_OFF;
							}
							else
							{
								
							}
							break;
						case STIR_WATESWITCH_OFF:
							if(Msgtemp.CmdType == MSG_SWITCHOFFOK)
							{{
								StirState5 = STIR_IDLE;
								RecoverStir(5);
//								SetMDCoil(9,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 9;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
								//验证电磁阀
								//MsgSend.CmdSrc = STIR_TASK_CODE;
								//MsgSend.CmdType = XREAD_OFF;
								//MsgSend.CmdData[0] = Msgtemp.CmdData[0]-1;
								//PackSendZoneQ(&MsgSend);
								//StirState5 = STIR_VERIFYSWITCH_OFF;
							}
							else if(Msgtemp.CmdType == MSG_SWITCHOFFERROR)
							{
								//报警
								g_FaultCode |= FAULT_STIR5;
							}
							break;
						case STIR_VERIFYSWITCH_OFF:
							if(Msgtemp.CmdType == MSG_VERFYOFFOK)
							{
								StirState5 = STIR_IDLE;
								RecoverStir(5);
//								SetMDCoil(9,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 9;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
							else if(Msgtemp.CmdType == MSG_VERFYOFFERR)
							{
								//报警
								g_FaultCode |= FAULT_STIR5;
								StirState5 = STIR_IDLE;
								RecoverStir(5);
//								SetMDCoil(9,0);
//								InitiativeParaTemp.DataType = INITIATIVE_COIL_STA;
//								InitiativeParaTemp.DataAddr = 9;
//								InitiativeParaTemp.DataNum = 1;
//								SetIniactivePara(InitiativeParaTemp);
//								ModbusPara.Initiative = 1;
							}
							break;
						default:
							break;
					}

					break;
				default:
					break;
			}
		}
	}
}

