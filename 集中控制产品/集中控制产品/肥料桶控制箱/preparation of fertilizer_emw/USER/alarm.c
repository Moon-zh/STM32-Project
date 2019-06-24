#include "sys.h"
#include "includes.h"  
#include "Delay.h"
#include "Alarm.h"
#include "UserCore.h"
#include "FlashDivide.h"
#include "Emw3060.h"
#include "Hmi_driver.h"


OS_EVENT * AlarmQMsg;
void* 	AlarmMsgBlock[4];
OS_MEM* AlarmPartitionPt;
u8 g_u8AlarmMsgMem[5][4];

//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 AlarmQInit(void)
{
	INT8U os_err;
	
	AlarmQMsg = OSQCreate ( AlarmMsgBlock, 4);
	
	if(AlarmQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	AlarmPartitionPt = OSMemCreate (
										g_u8AlarmMsgMem,
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

u8 PackSendAlarmQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(AlarmPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( AlarmQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(AlarmPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveAlarmQ(MsgStruct * MasterQ)
{
	u8 os_err;
	os_err = OSMemPut(AlarmPartitionPt, ( void * )MasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
LevelParam LevelAlarmPara[5];
u8 	CollPerioF[5] = {0,0,0,0,0};//采集标志
u16 LevelData[5] = {0,0,0,0,0};//获取液位数据 单位 mm
u8 	StartLevelColl[5] = {0,0,0,0,0};//开始采集计时标志
u8	_20SecCnt[5] = {0,0,0,0,0};//20秒计数  RTC 使用
u16  SetLevelValue[5] = {0,0,0,0,0};//设置值
u8 LevelErrCnt[5] = {0,0,0,0,0};//多少次以后算坏了//防止液位上升较慢

u8 	StirTimeF[5] = {0,0,0,0,0};//计时器单次计时标志
u8	StirTimeFlag[5] = {0,0,0,0,0};//开始搅拌标志
u8	StirTimeMin[5] = {0,0,0,0,0};//搅拌分钟数
u8  SetStirTime[5] = {0,0,0,0,0};//设置值
u8 	StartStirTime[5] = {0,0,0,0,0};//开始搅拌计数标志 RTC 使用
u8  _60SecCnt[5] = {0,0,0,0,0};//60秒计数

u8 AlarmLow[5]={0,0,0,0,0};
u8 AlarmLowF[5]={0,0,0,0,0};

u8 SysStirTime[5]={0,0,0,0,0};
//清空液位判断的参数
u8 ClearLevelPara(u8 Num)
{
	if(LevelAlarmPara[Num].LevelFlag == 0)
	{
		LevelAlarmPara[Num].LastLevel = 0;
		CollPerioF[Num] = 0;
		StartLevelColl[Num] = 0;
		_20SecCnt[Num] = 0;
		return 1;
	}
	return 0;
}
//清除搅拌时间判断的参数
u8 ClearStirTimePara(u8 Num)
{
	if(StirTimeFlag[Num] == 0)
	{
		StirTimeF[Num] = 0;
		StirTimeMin[Num] = 0;
		StartStirTime[Num]= 0;
		_60SecCnt[Num] = 0;
		return 1;
	}
	return 0;
}

void Alarm_task(void *pdata)
{
	u8 os_err;
	u8 i=0;
	u8 SetLevelBuf[5] = {0,0,0,0,0};
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
	MsgStruct WaterAddMsg;
	AlarmQInit();
  	ParaApplication();
	while(1)
	{
		delay_ms(100);
		pMsgBlk = ( MsgStruct *) OSQPend ( AlarmQMsg,
				10,
				&os_err );
		
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveAlarmQ(pMsgBlk);
			switch(Msgtemp.CmdType)
			{
				case ALARMMSG_LEVEL_OPEN://收到命令时先记录
					//清空参数
					if(ClearLevelPara(Msgtemp.CmdData[0]))
					{
						//FLASH获取设置值
						FlashReadWaterADD(SetLevelBuf);
						SetLevelValue[0] = SetLevelBuf[0]*10;
						SetLevelValue[1] = SetLevelBuf[1]*10;
						SetLevelValue[2] = SetLevelBuf[2]*10;
						SetLevelValue[3] = SetLevelBuf[3]*10;
						SetLevelValue[4] = SetLevelBuf[4]*10;
						//开启判断
						LevelAlarmPara[Msgtemp.CmdData[0]].LevelFlag = 1;
						//开始计时
						StartLevelColl[Msgtemp.CmdData[0]] = 1;
						//记录原始数据
						LevelAlarmPara[Msgtemp.CmdData[0]].LastLevel = LevelData[Msgtemp.CmdData[0]];
					}
					break;
				case ALARMMSG_LEVEL_CLOSE://收到命令时清空
					LevelAlarmPara[Msgtemp.CmdData[0]].LevelFlag = 0;
					StartLevelColl[Msgtemp.CmdData[0]] = 0;
					break;
				case ALARMMSG_STIRTIME_OPEN://收到命令时打开计时器
					if(ClearStirTimePara(Msgtemp.CmdData[0]))
					{
						//FLASH获取设置值
						FlashReadStir(SetStirTime);
						//开启判断
						StirTimeFlag[Msgtemp.CmdData[0]] = 1;
						//开启计时
						StartStirTime[Msgtemp.CmdData[0]] = 1;
						StirTimeMin[Msgtemp.CmdData[0]]=SetStirTime[Msgtemp.CmdData[0]];
						//memcpy(StirTimeMin,SetStirTime,5);
					}
					break;
				case ALARMMSG_STIRTIME_CLOSE://收到命令时清空
					StirTimeFlag[Msgtemp.CmdData[0]] = 0;
					StartStirTime[Msgtemp.CmdData[0]] = 0;
					StirTimeMin[Msgtemp.CmdData[0]] = 0;
					ClearStirTimePara(Msgtemp.CmdData[0]);
					break;
				default:
					break;
			}
		}
		else
		{
			//液位低报警 //
			for(i=0;i<5;i++)
			{
				if((AlarmLow[i]*10>= LevelData[i])&&AlarmLowF[i] == 0)
				{
					//报警
					SetScreen(7);
					AlarmLowF[i] = 1;
				}
				else if(AlarmLow[i]*10< LevelData[i])
				{
					AlarmLowF[i] = 0;
				}
			}
		}
		//液位判断
		for(i=0;i<5;i++)
		{
			if((LevelAlarmPara[i].LevelFlag == 1)&&(CollPerioF[i] == 1))//如果开启并且到了判断周期
			{
				if(SetLevelValue[i] <= LevelData[i])
				{
					//发送液位达到消息
					WaterAddMsg.CmdType = MSG_WATERFULL;
					WaterAddMsg.CmdSrc = ALARM_TASK_CODE;
					WaterAddMsg.CmdData[0] = i+1;
					PackSendWaterAddQ(&WaterAddMsg);
					LevelAlarmPara[i].LevelFlag = 0;
					StartLevelColl[i] = 0;
					LevelErrCnt[i] = 0;
				}
				else if(LevelAlarmPara[i].LastLevel >= LevelData[i])
				{
					//发送液位传感器坏消息.
					if(LevelErrCnt[i]++>30)
					{
						WaterAddMsg.CmdType = MSG_LEVEERR;
						WaterAddMsg.CmdSrc = ALARM_TASK_CODE;
						WaterAddMsg.CmdData[0] = i+1;
						PackSendWaterAddQ(&WaterAddMsg);
						LevelAlarmPara[i].LevelFlag = 0;
						StartLevelColl[i] = 0;
						LevelErrCnt[i] = 0;
					}
				}
				else
				{
					LevelErrCnt[i] = 0;
				}
				CollPerioF[i] = 0;
				//更新历史数据
				LevelAlarmPara[i].LastLevel = LevelData[i];
			}
		}
		//搅拌判断
		for(i=0;i<5;i++)
		{
			if((StirTimeFlag[i] == 1)&&(StirTimeF[i] == 1))//如果开启并且到了判断周期
			{
				if(--StirTimeMin[i] == 0)
				{
					WaterAddMsg.CmdType = MSG_TIMEOVER;
					WaterAddMsg.CmdSrc = ALARM_TASK_CODE;
					WaterAddMsg.CmdData[0] = i+1;
					PackSendStirQ(&WaterAddMsg);
					StirTimeFlag[i] = 0;
					StartStirTime[i] = 0;
					_60SecCnt[i] = 0;
				}
				SetRemoteRead(32+i,StirTimeMin[i]);
				MsgtempBlk.CmdSrc = ALARM_TASK_CODE;
				MsgtempBlk.CmdType= UPLOAD_COUNTDOWN;
				PackSendRemoteQ(&MsgtempBlk);
				StirTimeF[i] = 0;
			}
		}
	}
}
//总水重 246.1kg
u8 BucketPercent[5];//浓度比例 //开机初始化 保存初始化
u16 BucketLimit[5];//液位限制 //开机初始化 保存初始化 单位 mm
u32 BucketS[5];//底面积 //开机初始化 保存初始化
u16 DestFeitilizer[5];//肥料总量 单位KG
u16 BucketFeitilizer[5];//现有肥料量 KG
u16 NeedFertilizer[5];//需要添加的肥料
u16 Limit = 500;
u8 Persent = 50;
 void CaluculateFertilizer(void)
{
	u8 i = 0;
	
	
	/*BucketLimit[0] = Limit;
	BucketLimit[1] = Limit;
	BucketLimit[2] = Limit;
	BucketLimit[3] = Limit;
	BucketLimit[4] = Limit;
	BucketPercent[0] = Persent;
	BucketPercent[1] = Persent;
	BucketPercent[2] = Persent;
	BucketPercent[3] = Persent;
	BucketPercent[4] = Persent;*/
	
	//计算得出总肥量 1M计算
	for(i=0;i<5;i++)
	{
		DestFeitilizer[i] = 2461*BucketPercent[i]/(100-BucketPercent[i]);
	}
	//计算得出还需添加肥量
	for(i=0;i<5;i++)
	{
		NeedFertilizer[i] = (BucketLimit[i]-LevelData[i])*DestFeitilizer[i]/1000;
	}
}
void ParaApplication(void)
{
	u8 AlarmDestLevel[5] = {0,0,0,0,0};
	u8 AlarmStirTime[5] = {0,0,0,0,0};
	u8 AlarmPersent[5] = {0,0,0,0,0};
	u8 AlarmLowLevel[5] = {0,0,0,0,0};
	u16 AlarmSize[5] = {0,0,0,0,0};
	u8 i=0;
	
	FlashReadWaterADD(AlarmDestLevel);
	FlashReadStir(AlarmStirTime);
	FlashReadPersent(AlarmPersent);
	FlashReadLow(AlarmLowLevel);
	FlashReadD((u8*)AlarmSize);
	for(i=0;i<5;i++)
	{
		BucketS[i] = 314*((AlarmSize[i]*10)/2)*((AlarmSize[i]*10)/2)/10000;
		BucketLimit[i] = AlarmDestLevel[i]*10;
		BucketPercent[i] = AlarmPersent[i];
		AlarmLow[i] = AlarmLowLevel[i];
		SysStirTime[i] = AlarmStirTime[i];
	}
}
void FertilizerCalculate(void)
{
	u8 i =0;
	for(i=0;i<5;i++)
	{
		//体积(cm3)=底面积(cm2)*高(cm)
		//体积(ml)=体积(m3)
		//水重(kg) = 体积(ml)*密度（1）
		//肥(kg)=水*浓度/(1-浓度) 等式右边上下同时扩大100倍
		//计算过程先底面积*高度差*(p/1-p)/10(高度差仪mm计算的，需要以CM计算)/1000(g换算成kg)
		if((BucketLimit[i]>LevelData[i]))
		{
			BucketFeitilizer[i] = ((BucketS[i]*(BucketLimit[i]-LevelData[i])*BucketPercent[i])
									/(100-BucketPercent[i]))/10000;
		}
		else
		{
			BucketFeitilizer[i] = 0;
		}
	}
	//总肥量-现有肥量 = 需要添加的肥量
}
//密度计算
u8 BeforeDensity[5];//加肥前计算得出的密度.用此密度计算液位高度
u8 AfterDensity[5];//加肥后计算得出的密度
void CalculateDensity(void)
{
	u8 i =0;
	for(i=0;i<5;i++)
	{
		/*BeforeDensity[i] = (BucketFeitilizer[i]/20
							+BucketS[i]*BucketLimit[i]/100000000)
							/(BucketS[i]*BucketLimit[i]/100000000000);*/
		//由上面的计算变化而来
		BeforeDensity[i]= (BucketFeitilizer[i]*5000000000
							+BucketS[i]*BucketLimit[i]*1000)
							/(BucketS[i]*BucketLimit[i]);
	}
	for(i=0;i<5;i++)
	{
		/*AfterDensity[i] = (BucketS[i]*BucketLimit[i]/100000000)/(100-BucketPercent[i])
							/(BucketS[i]*BucketLimit[i]/100000000000);*/
		AfterDensity[i] = (BucketS[i]*BucketLimit[i]*1000)/(100-BucketPercent[i])
							/(BucketS[i]*BucketLimit[i]);
	}
}

