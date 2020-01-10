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
#include "rtc.h" 		
#include "communication.h"
extern void Chanel_Fer_Weight(u16 data);
MsgStruct MasterMsg;
OS_EVENT * MasterQMsg;
extern CtaskRecordunion ctask_record;//记录任务
extern Targetred ctargetrcord;//当前任务记录
extern u8 com_conect_state;//串口连接状态
void* 	MasterMsgBlock[4];
OS_MEM* MasterPartitionPt;
u8 g_u8AgvCtrMsgMem[20][4];
u8 screen_refresh_flag=0;//任务开始屏幕刷新
u16 Hmi_Buf[10];
extern u8 startzone;//开启分区
extern u8 closezone;//关闭分区	
extern u8 com3state;//串口3工作状态
u8 Solenoidvalve_Close_flag=0;//结束后需要关闭的电磁阀分区
extern u8 gu8Use_MA_Check_Flag;//肥料开断检测
extern u8 task_start_flag;
ProgramStruct ProgramData;
MasterStruct MasterCtrl;
PercentStruct Percent ;
extern FlashSet flashset;//设置参数保存

WaterProgStruct UserWaterProg;

void CalculateProgPara(u8 Partition ,u8 IrrPro)
{
	u8 DataTemp[3];
	u16 WaterData[3];
	u8 DosingData[5];
	u8 AmountData;
	
	FlashReadIrrigationProg(DataTemp,IrrPro-1,3);
	FlashReadWaterProg((u8*)WaterData,DataTemp[0]-1,6);
	FlashReadDosingProg(DosingData,DataTemp[1]-1,5);
	FlashReadFertilizerAmount(&AmountData,DataTemp[2]-1,1);
	Hmi_Buf[0] = WaterData[0];
	Hmi_Buf[1] = WaterData[1];
	Hmi_Buf[2] = WaterData[2];
	Hmi_Buf[3] = 0;
	Hmi_Buf[4] = 0;
	Hmi_Buf[5] = 0;
	Hmi_Buf[6] = 0;
	Hmi_Buf[7] = 0;
	Hmi_Buf[8] = 736;
	Hmi_Buf[9] = 2;
	//WorkStateShow();
	
	SlavePara.WaterBefore = WaterData[0];
	SlavePara.WaterAfter = WaterData[2];
	SlavePara.WaterTime = WaterData[1];
	SlavePara.DosingQty[0] = DosingData[0];
	SlavePara.DosingQty[1] = DosingData[1];
	SlavePara.DosingQty[2] = DosingData[2];
	SlavePara.DosingQty[3] = DosingData[3];
	SlavePara.DosingQty[4] = DosingData[4];
	if(AmountData!=0)
		SlavePara.DosingSwitch = 1;
	SlavePara.ValveGroup = 1;
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
u8 alfred_test = 0;
void TestProg(u8 Test_Type)
{
	MsgStruct MsgtempBlk;
	switch(Test_Type)
	{
		case 1:
			MsgtempBlk.CmdType = MSG_START;
			PackSendMasterQ(&MsgtempBlk);
			break;
		case 2:
			break;
	}
}
//任务更新函数
void target_updat(void)
{
//	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
 if(ctargetrcord.trecord.currenttargetlefttime==0)
 {
	 //判断新任务触发条件
	if((ctargetrcord.trecord.curtarget_mode==1)||(ctargetrcord.trecord.curtarget_mode==2)) //清水灌溉模式
	{	
        if((ctargetrcord.trecord.currenttarget_id!=0)&&(ctargetrcord.trecord.currenttarget_id!=ctargetrcord.trecord.lasttarget_id))
		{
			if(ctargetrcord.trecord.currenttarget_id<ctargetrcord.trecord.curtarget_max_num)
			{
					ctask_record.record.task_num++;
					if(ctask_record.record.task_num>100) //最多记录100条任务
					{
						ctask_record.record.task_num=1;
					}
//                    ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;//更新任务号
                    ctargetrcord.trecord.Ctrl_update_state=1;//控件需要更新
                    ctargetrcord.trecord.lasttarget_id=ctargetrcord.trecord.currenttarget_id;
                    ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time;
					
					MsgtempBlk.CmdType = MSG_START; //发送消息告诉状态机启动任务
					MsgtempBlk.CmdData[0]= ctargetrcord.trecord.curtarget_mode;//灌溉模式
					MsgtempBlk.CmdData[1]= ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];//需要开启的电磁阀
					PackSendMasterQ(&MsgtempBlk);
			}
		}
        if((ctargetrcord.trecord.currenttarget_id==ctargetrcord.trecord.curtarget_max_num)&&(screen_refresh_flag==1))//最后一个任务已经完成
        {
            screen_refresh_flag=0;
            ctask_record.record.stop_mode=1;//更新停止模式 此处防止按停止按钮更改任务记录
						SetScreen(1);
						task_start_flag=0;//检测压力开关
        }
	 //此处发起新任务 需要更新任务记录
	}
//	else if(ctargetrcord.trecord.curtarget_mode==2) //肥水灌溉模式
//	{	
//        if((ctargetrcord.trecord.currenttarget_id!=0)&&(ctargetrcord.trecord.currenttarget_id!=ctargetrcord.trecord.lasttarget_id))
//		{
//			if(ctargetrcord.trecord.currenttarget_id<ctargetrcord.trecord.curtarget_max_num)
//			{
//					ctask_record.record.task_num++;
//					if(ctask_record.record.task_num>100) //最多记录100条任务
//					{
//						ctask_record.record.task_num=1;
//					}
//				ctargetrcord.trecord.curtargetnum=ctask_record.record.task_num;//更新任务号
//				ctargetrcord.trecord.Ctrl_update_state=1;//控件需要更新
//				ctargetrcord.trecord.lasttarget_id=ctargetrcord.trecord.currenttarget_id;
//				ctargetrcord.trecord.currenttargetlefttime=ctargetrcord.trecord.curtarget_time;
//			}
//		}
//        if(ctargetrcord.trecord.currenttarget_id==ctargetrcord.trecord.curtarget_max_num)//最后一个任务已经完成
//        {
//            ctask_record.record.stop_mode=1;//更新停止模式 此处防止按停止按钮更改任务记录
//    
//        }
//	 //此处发起新任务 需要更新任务记录
//	}    
 }


}
u8 Master_State;
void MasterCtrl_task(void *pdata)
{
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	MsgStruct MsgtempBlk;
//	MsgComStruct Msgtemp1;
	MsgComStruct MsgtempBlk1;	
	//OS_CPU_SR  cpu_sr;
	MasterQInit();
	while(1)
	{
		delay_ms(100);
		TestProg(alfred_test);
		pMsgBlk = ( MsgStruct *) OSQPend ( MasterQMsg,
				10,
				&os_err );

	
		if(os_err == OS_ERR_NONE)
		{
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
			DepackReceiveMasterQ(pMsgBlk);
			switch(Master_State)
			{
				case MASTER_IDLE:
                if(Msgtemp.CmdType == MSG_START)
                {
					task_start_flag=1;
                    startzone=Msgtemp.CmdData[1];//需要开启的电磁阀
                    MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                    MsgtempBlk1.CmdData[0]= 1;//串口3发送心跳
                    MsgtempBlk1.CmdData[1]= 2;//开启分区
                    PackSendComQ(&MsgtempBlk1); 
                    Master_State=MASTER_WORK;
					//SetRemoteRead(i,2);
                }
                break;
				case MASTER_WORK:
                if(Msgtemp.CmdType == MSG_END)//停止
                {
                        Solenoidvalve_Close_flag=1;//计时3S关闭电磁阀
                        closezone=Msgtemp.CmdData[1];//需要关闭的
                        if(Msgtemp.CmdData[0]==2)
                        {
                            
                            gu8Use_MA_Check_Flag=0;
                            ctargetrcord.trecord.fer_onoff=2;//关闭水泵
                            MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                            MsgtempBlk1.CmdData[0]= 2;//串口4
                            MsgtempBlk1.CmdData[1]= 3;//关闭水泵
                            PackSendComQ(&MsgtempBlk1); 
                        }
                        
                }
                else if(Msgtemp.CmdType == MSG_STOP) //暂停
                {
                	if(Msgtemp.CmdData[0]==1)   
					{
						ctargetrcord.trecord.Ctrl_update_state=0;
						closezone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];
                        MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                        MsgtempBlk1.CmdData[0]= 1;//串口3发送心跳
                        MsgtempBlk1.CmdData[1]= 1;//关闭分区
                        PackSendComQ(&MsgtempBlk1); 
					}
                    if(Msgtemp.CmdData[0]==2)
                    {
                            
                        gu8Use_MA_Check_Flag=0;
                        ctargetrcord.trecord.fer_onoff=2;//关闭水泵
                        MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                        MsgtempBlk1.CmdData[0]= 2;//串口4
                        MsgtempBlk1.CmdData[1]= 3;//关闭水泵
                        PackSendComQ(&MsgtempBlk1); 
                    }
                        
                }
                else if(Msgtemp.CmdType == MSG_CONTIUE) //恢复
                {
												 if(Msgtemp.CmdData[0]==1)   
												 {
													ctargetrcord.trecord.Ctrl_update_state=1;
													startzone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];
													MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
													MsgtempBlk1.CmdData[0]= 1;//串口3发送心跳
													MsgtempBlk1.CmdData[1]= 2;//关闭分区
													PackSendComQ(&MsgtempBlk1); 
												 }
												else if(Msgtemp.CmdData[0]==2)
                        {
                            
                            gu8Use_MA_Check_Flag=0;
                            ctargetrcord.trecord.fer_onoff=2;//关闭水泵
                            MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                            MsgtempBlk1.CmdData[0]= 2;//串口4
                            MsgtempBlk1.CmdData[1]= 3;//关闭水泵
                            PackSendComQ(&MsgtempBlk1); 
                        }
                        
                }                           
                else if(Msgtemp.CmdType == MSG_ALARM)//收到的异常默认为从机已经停止所有工作
                {
                        ctargetrcord.trecord.Ctrl_update_state=0;
                        closezone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];
                            MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                            MsgtempBlk1.CmdData[0]= 1;//串口3发送心跳
                            MsgtempBlk1.CmdData[1]= 1;//关闭分区
                            PackSendComQ(&MsgtempBlk1); 
                            delay_ms(10);
                            ctargetrcord.trecord.fer_chanle=0;
                            MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                            MsgtempBlk1.CmdData[0]= 2;//串口4
                            MsgtempBlk1.CmdData[1]= 3;//关闭水泵
                            PackSendComQ(&MsgtempBlk1); 
                            SetScreen(37);
							ctask_record.record.task_num--;
                }
                else if(Msgtemp.CmdType == MSG_START) //继续下一个分区工作
                {
												com_conect_state=1;
                        startzone=Msgtemp.CmdData[1];//需要开启的电磁阀
                        Chanel_Fer_Weight(flashset.FlashPara[startzone+3]);//更新各通道施肥量
                        MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                        MsgtempBlk1.CmdData[0]= 1;//串口3发送心跳
                        MsgtempBlk1.CmdData[1]= 2;//开启分区
                        PackSendComQ(&MsgtempBlk1); 

                }
                else if(Msgtemp.CmdType == MSG_PAGEEND)
                {
					SetRemotePara(35,0);
					Msgtemp.CmdType = UPLOAD_IRRPARA;
					Msgtemp.CmdSrc = USERCORE_TASK_CODE;
					PackSendRemoteQ(&Msgtemp);
					SetRemoteRead(16,0);
					Msgtemp.CmdType = UPLOAD_IRRSTATE;
					Msgtemp.CmdSrc = USERCORE_TASK_CODE;
					PackSendRemoteQ(&Msgtemp);
					SetScreen(LCD_START_PAGE);
                        com_conect_state=0;
                        screen_refresh_flag=0;
												closezone=ctargetrcord.trecord.targetnum[ctargetrcord.trecord.currenttarget_id];
                        MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                        MsgtempBlk1.CmdData[0]= 1;//串口3发送心跳
                        MsgtempBlk1.CmdData[1]= 1;//关闭分区
                        PackSendComQ(&MsgtempBlk1); 
                        delay_ms(10);
                        ctargetrcord.trecord.fer_chanle=0;
                        MsgtempBlk1.CmdType = COM_WRITE; //发送消息告诉状态机启动任务
                        MsgtempBlk1.CmdData[0]= 2;//串口4
                        MsgtempBlk1.CmdData[1]= 3;//关闭水泵
                        PackSendComQ(&MsgtempBlk1);                            
            
                
                }
					break;
				case MASTER_ERROR:
					{
						if(Msgtemp.CmdType == MSG_ERROR)
						{
							SetRemotePara(35,0);
							Msgtemp.CmdType = UPLOAD_IRRPARA;
							Msgtemp.CmdSrc = USERCORE_TASK_CODE;
							PackSendRemoteQ(&Msgtemp);
							SetRemoteRead(16,0);
							Msgtemp.CmdType = UPLOAD_IRRSTATE;
							Msgtemp.CmdSrc = USERCORE_TASK_CODE;
							PackSendRemoteQ(&Msgtemp);
							//报警
							//弹异常界面 上报云平台
							//主动上报流量数据
//							MsgtempBlk.CmdType = INITIATIVE_INPUT_REG;
							MsgtempBlk.CmdData[0] = 00;
							MsgtempBlk.CmdData[1] = 12;
							PackSendRemoteQ(&MsgtempBlk);
							Master_State = MASTER_IDLE;
						}
					}
					break;
				case MASTER_CANCLE:
					if(Msgtemp.CmdType == MSG_CANCLEOK)
					{
						Master_State = MASTER_END;
						MsgtempBlk.CmdType = MSG_END;
						PackSendMasterQ(&MsgtempBlk);
					}
					else if(Msgtemp.CmdType == MSG_ALARM)
					{
						//状态转换
						Master_State = MASTER_ERROR;
						//发送错误信息
						MsgtempBlk.CmdType = MSG_ERROR;
						PackSendMasterQ(&MsgtempBlk);
						
						ctask_record.record.task_num--;
					}
					break;
				case MASTER_END:
					{
						if(Msgtemp.CmdType == MSG_END)
						{
							SetRemotePara(35,0);
							Msgtemp.CmdType = UPLOAD_IRRPARA;
							Msgtemp.CmdSrc = USERCORE_TASK_CODE;
							PackSendRemoteQ(&Msgtemp);
							SetRemoteRead(16,0);
							Msgtemp.CmdType = UPLOAD_IRRSTATE;
							Msgtemp.CmdSrc = USERCORE_TASK_CODE;
							PackSendRemoteQ(&Msgtemp);
							//更换界面
							//主动上报流量数据
//							MsgtempBlk.CmdType = INITIATIVE_INPUT_REG;
							MsgtempBlk.CmdData[0] = 00;
							MsgtempBlk.CmdData[1] = 12;
							PackSendRemoteQ(&MsgtempBlk);
							Master_State =MASTER_IDLE;
						}
					}
					break;
				default:
					{
						Master_State = MASTER_CANCLE;
						//下发取消命令
						MsgtempBlk.CmdType = STOP_CMD;
						PackSendZoneQ(&MsgtempBlk);
					}
					break;
			}
		}
	}

}

