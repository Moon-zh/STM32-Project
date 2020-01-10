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

#include	"cgq.h"


extern CountDownStruct WorkTime;

MsgStruct MasterMsg;
OS_EVENT *MasterQMsg; //信号量，声明一个OS_EVENT类型的指针指向生成的队列
void* 	MasterMsgBlock[8]; //消息队列数组， 建立一个指向消息数组的指针和数组的大小，该指针数组必须申明为void类型
OS_MEM* MasterPartitionPt;/*定义内存控制块指针，创建一个内存分区时，返回值就是它 */
u8 g_u8AgvCtrMsgMem[20][8];///*划分一个具有20个内存块，每个内存块长度是8的内存分区 */

WorkPara IrrPara;//主程序运行时使用的变量
u8 MasterState = 0;//主程序状态变量


//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 MasterQInit(void)
{
	INT8U os_err;
	
	MasterQMsg = OSQCreate ( MasterMsgBlock, 8);
	
	if(MasterQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	MasterPartitionPt = OSMemCreate (
										g_u8AgvCtrMsgMem,
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
	MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	MsgTemp ->CmdData[4] = MsgBlk->CmdData[4];
	MsgTemp ->CmdData[5] = MsgBlk->CmdData[5];
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
	MsgTemp ->CmdData[2] = MsgBlk->CmdData[2];
	MsgTemp ->CmdData[3] = MsgBlk->CmdData[3];
	MsgTemp ->CmdData[4] = MsgBlk->CmdData[4];
	MsgTemp ->CmdData[5] = MsgBlk->CmdData[5];
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

//输入:8位的数字
//输出:最低的为1的位号
//功能:将输入数的最低的为1的位数返回,并将该位置零
u8 Seek1LowBit(u8*Figure)
{
	u8 Ret = 0;
	u8 i = 0;
	for(i=0;i<8;i++)
	{
		if((*Figure&(1<<i))>0)
		{
			Ret = i+1;
			*Figure = *Figure^(1<<i);
			break;
		}
	}
	return Ret;
}

//读取主程序的现在的运行状态
u8 GetSysState(void)
{
	return MasterState;
}

/*
//主任务程序
IO_OutSet(1,1或0); 肥水的电磁阀 开-1，关-0
IO_OutSet(2,1或0); 清水的电磁阀-第一个分区 开-1，关-0
IO_OutSet(3,1或0); 清水的电磁阀-第二个分区 开-1，关-0
*/
void MasterCtrl_task(void *pdata)
{
	u8 os_err;
	MsgStruct *pMsgBlk = NULL;
  MsgStruct Msgtemp;
	

	MasterQInit();//创建消息队列和内存块
	
	while(1)
	{
		delay_ms(100);
		
		pMsgBlk = ( MsgStruct *) OSQPend ( MasterQMsg,
				10,
				&os_err );
	   //请求消息队列，参数分别是：RemoteQMsg为所请求消息队列的指针  第二个参数为等待时间
		 //0表示无限等待，&err为错误信息，返回值为队列控制块RemoteQMsg成员OSQOut指向的消息(如果队列中有消息可用的话)，如果
		//没有消息可用，在使调用OSQPend的任务挂起，使之处于等待状态，并引发一次任务调度
		
		if(os_err == OS_ERR_NONE)//通话成功，任务收到信息
		{ 
			memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );//从源pMsgBlk拷贝n个字节到目Msgtemp中			
			DepackReceiveMasterQ(pMsgBlk);//队列消息解析，消息内存释放
			
			switch(MasterState)//主任务运行状态
			{
				case SYSTEM_IDLE://空闲状态 0
					if(Msgtemp.CmdType == MSG_START)//启动
					{
						SetRemotePara(3,1);//设置停止-0，启动-1
						
						IrrPara.WorkModel  = Msgtemp.CmdData[0];//灌溉类型(to_type)    默认-0，清水-1，肥水-2
						IrrPara.Zone = Msgtemp.CmdData[1];//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
						IrrPara.TimeH = Msgtemp.CmdData[2];//灌溉时常（小时）
						IrrPara.TimeM = Msgtemp.CmdData[3];//灌溉时常（分钟）
						
						SetRemoteRead(0,IrrPara.WorkModel);//设置当前工作状态，灌溉类型(to_type)    默认-0，清水-1，肥水-2					
						SetRemotePara(0,IrrPara.WorkModel);//灌溉类型(to_type)    默认-0，清水-1，肥水-2
						SetRemotePara(2,IrrPara.Zone);//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
						SetRemotePara(1,(IrrPara.TimeH*60+IrrPara.TimeM));//灌溉时常（to_time）  时间常数

											
						//灌溉类型：清水
						if(IrrPara.WorkModel == 1)
						{
							//查找是哪个分区需要灌溉
							IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
							if(IrrPara.ImplementZone != 1)//第一个分区需要灌溉，IrrPara.ImplementZone就是2，第二个分区需要灌溉，就是3
							{
								//打开分区电磁阀  清水电磁阀开启
								IO_OutSet(IrrPara.ImplementZone,1);
								//启动倒计时
								StartCount(&WorkTime,IrrPara.TimeH,IrrPara.TimeM);
								MasterState = SYSTEM_WATER;//工作状态：清水
								//刷新界面 
								WorkModelShow(1,IrrPara.ImplementZone);//更新触摸屏显示数据（当前灌溉模式+当前灌溉分区）
								
								SetRemoteRead(2,IrrPara.ImplementZone-1);//设置当前灌溉分区1分区-1,2分区-2
	
								if(Guangai.zdms==1)fsbz_3060_cx();//进行一次上传数据的程序（只在自动模式下发送数据)
							}
						}
						//灌溉类型：肥水
						else if(IrrPara.WorkModel == 2)
						{
							//查找是哪个分区需要灌溉
							IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
							
							if(IrrPara.ImplementZone != 1)//第一个分区需要灌溉，IrrPara.ImplementZone就是2，第二个分区需要灌溉，就是3
							{
								//打开分区电磁阀，先进行浇清水  清水电磁阀开启
								IO_OutSet(IrrPara.ImplementZone,1);
								//启动倒计时
								StartCount(&WorkTime,0,FERTILIZER1TIME);//先倒计时10分钟
								MasterState = SYSTEM_FERTILIZER1;//施肥1阶段
								//刷新界面 
								WorkModelShow(0,IrrPara.ImplementZone);//更新触摸屏显示数据（当前灌溉模式+当前灌溉分区）
								
								SetRemoteRead(2,IrrPara.ImplementZone-1);//设置当前灌溉分区1分区-1,2分区-2							
				
							}
						}
					}
					break;
					
				case SYSTEM_WATER://工作状态：清水 1
					if(Msgtemp.CmdType == MSG_STOP) //停止
					{
						//关闭电磁阀   清水电磁阀关闭
						IO_OutSet(IrrPara.ImplementZone,0);
						//停止计时
						ClearCount(&WorkTime);
						//改变工作状态
						MasterState = SYSTEM_IDLE;  //空闲状态
						//刷新界面
						if(Guangai.zdms==0)SetScreen(LCD_MODELSEL_PAGE);//切换界面，回到功能选择界面(在开启自动模式时，不切换界面)
						//清空控制参数
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//设置停止-0，启动-1
						SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）    1分区-1,2分区-2，空闲-0
						SetRemoteRead(0,0);//工作状态（work_state） 空闲-0，清水-1，肥水-2 
						SetRemoteRead(1,0);//清除倒计时（in_time）0
						
            fsbz_3060_cx();//进行一次上传数据的程序 
			
							
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER) //倒计时结束
					{
						//关闭已经完成的分区  清水电磁阀关闭
						IO_OutSet(IrrPara.ImplementZone,0);
						//改变平台状态
						SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）  1分区-1,2分区-2
						
						//查找还有没有需要灌溉的分区
						IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
						//还有分区没有浇完
						if(IrrPara.ImplementZone!=1)//第二个分区需要灌溉，IrrPara.ImplementZone就是3
						{

							//打开分区电磁阀  清水电磁阀开启
							IO_OutSet(IrrPara.ImplementZone,1);
							//开始倒计时
							StartCount(&WorkTime,IrrPara.TimeH,IrrPara.TimeM);
							//刷新界面
							WorkModelShow(1,IrrPara.ImplementZone);//更新触摸屏显示数据（当前灌溉模式+当前灌溉分区）
							//改变平台状态
							SetRemoteRead(2,IrrPara.ImplementZone-1);//设置当前灌溉分区1分区-1,2分区-2			

							if(Guangai.zdms==1)fsbz_3060_cx();//进行一次上传数据的程序（只在自动模式下发送数据)
//							Msgtemp.CmdType = MSG_START;//启动
//		          PackSendMasterQ(&Msgtemp);//消息打包发走
						}
						else//所有分区灌溉完成
						{
							//改变平台状态
							
							//停止计时
						  ClearCount(&WorkTime);
							//改变工作状态
							MasterState = SYSTEM_IDLE;//空闲状态
							//刷新界面
							if(Guangai.zdms==0)SetScreen(LCD_MODELSEL_PAGE);//切换界面，回到功能选择界面 (在开启自动模式时，不切换界面)
							//清空控制参数
							memset(&IrrPara,0,sizeof(IrrPara));
							SetRemotePara(3,0);//设置停止-0，启动-1
							SetRemoteRead(0,0);//工作状态（work_state） 空闲-0，清水-1，肥水-2 
							SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）  1分区-1,2分区-2，空闲-0
							SetRemoteRead(1,0);//清除倒计时（in_time）0
							
							fsbz_3060_cx();//进行一次上传数据的程序
						}
						
					}				
					break;
					
				case SYSTEM_FERTILIZER1://施肥1阶段 (此阶段先进行浇清水)
					if(Msgtemp.CmdType == MSG_STOP)//停止
					{
						//关闭电磁阀，关闭浇清水 清水电磁阀关闭
						IO_OutSet(IrrPara.ImplementZone,0);
						//停止计时
						ClearCount(&WorkTime);
						//改变工作状态
						MasterState = SYSTEM_IDLE;//空闲状态
						//刷新界面
						SetScreen(LCD_MODELSEL_PAGE);//切换界面
						//清空控制参数
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//设置停止-0，启动-1
						SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）    1分区-1,2分区-2，空闲-0
						SetRemoteRead(0,0);//工作状态（work_state） 空闲-0，清水-1，肥水-2 
						SetRemoteRead(1,0);//清除倒计时（in_time）0

            fsbz_3060_cx();//进行一次上传数据的程序
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER)//倒计时结束
					{
						//启动施肥 肥水电磁阀（1,1）开启
						IO_OutSet(1,1);
						//启动倒计时，设置倒计时时间
						IrrPara.FerTimeM=0;
						IrrPara.FerTimeH=0;
						if(IrrPara.TimeM>=(FERTILIZER1TIME+FERTILIZER3TIME))//灌溉分钟
						{
							IrrPara.FerTimeM = IrrPara.TimeM-(FERTILIZER1TIME+FERTILIZER3TIME);
							IrrPara.FerTimeH = IrrPara.TimeH;
						}
						else   if(IrrPara.TimeH>0)//灌溉小时大于0
						{
							IrrPara.FerTimeM = IrrPara.TimeM+60-(FERTILIZER1TIME+FERTILIZER3TIME);
							IrrPara.FerTimeH = IrrPara.TimeH-1;
						}
						StartCount(&WorkTime,IrrPara.FerTimeH,IrrPara.FerTimeM);//设置倒计时
									
						MasterState = SYSTEM_FERTILIZER2;//进行施肥2阶段						
					}
					break;
					
				case SYSTEM_FERTILIZER2://施肥2阶段
					if(Msgtemp.CmdType == MSG_STOP)//停止
					{
						//停止施肥   肥水电磁阀（1,0）关闭 
						IO_OutSet(1,0);
						//关闭电磁阀，关闭浇清水  
						IO_OutSet(IrrPara.ImplementZone,0);
						//停止计时
						ClearCount(&WorkTime);
						
						//改变工作状态
						MasterState = SYSTEM_IDLE;//空闲状态
						//刷新界面
						SetScreen(LCD_MODELSEL_PAGE);//切换界面
						//清空控制参数
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//设置停止-0，启动-1
						SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）    1分区-1,2分区-2，空闲-0
						SetRemoteRead(0,0);//工作状态（work_state） 空闲-0，清水-1，肥水-2 
						SetRemoteRead(1,0);//清除倒计时（in_time）0
						
            fsbz_3060_cx();//进行一次上传数据的程序
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER)//倒计时结束
					{
						//停止施肥   肥水电磁阀（1,0）关闭 
						IO_OutSet(1,0);
						//打开分区电磁阀，先进行浇清水  清水电磁阀开启
			  		IO_OutSet(IrrPara.ImplementZone,1);					
						
						//启动倒计时
						StartCount(&WorkTime,0,FERTILIZER3TIME);//进行倒计时10分钟
												
						MasterState = SYSTEM_FERTILIZER3;//施肥3阶段

						WorkCountShow(WorkTime.hour,WorkTime.min);//触摸屏更新显示，//倒计时	
					}
					break;
					
				case SYSTEM_FERTILIZER3://施肥3阶段(再次浇清水)
					if(Msgtemp.CmdType == MSG_STOP)//倒计时结束
					{
						//关闭电磁阀，关闭浇清水   
						IO_OutSet(IrrPara.ImplementZone,0);
						//停止计时
						ClearCount(&WorkTime);
						//改变工作状态
						MasterState = SYSTEM_IDLE;//空闲状态
						//刷新界面
						SetScreen(LCD_MODELSEL_PAGE);//切换界面
						//清空控制参数
						memset(&IrrPara,0,sizeof(IrrPara));
						SetRemotePara(3,0);//设置停止-0，启动-1
						SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）    1分区-1,2分区-2，空闲-0
						SetRemoteRead(0,0);//工作状态（work_state） 空闲-0，清水-1，肥水-2 
						SetRemoteRead(1,0);//清除倒计时（in_time）0
						
            fsbz_3060_cx();//进行一次上传数据的程序
					}
					else if(Msgtemp.CmdType == MSG_TIMEOVER)//倒计时结束
					{
						//关闭电磁阀，关闭浇清水
						IO_OutSet(IrrPara.ImplementZone,0);
						//改变平台状态
						SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）  1分区-1,2分区-2
						
						//查找还有没有需要灌溉的分区
						IrrPara.ImplementZone = Seek1LowBit(&IrrPara.Zone)+1;
						//还有分区没有浇完
						if(IrrPara.ImplementZone!=1)//第二个分区需要灌溉，IrrPara.ImplementZone就是3
						{
							//打开分区电磁阀
							IO_OutSet(IrrPara.ImplementZone,1);
							//启动倒计时
							StartCount(&WorkTime,0,FERTILIZER1TIME);//先倒计时10分钟
							MasterState = SYSTEM_FERTILIZER1;//施肥1阶段
							//刷新界面 
							WorkModelShow(0,IrrPara.ImplementZone);
							//改变平台状态
							SetRemoteRead(2,IrrPara.ImplementZone-1);//当前灌溉分区（at_partition）  1分区-1,2分区-2
						
						}
						else//所有分区灌溉完成
						{		
							//改变工作状态
							MasterState = SYSTEM_IDLE;//空闲状态
							//停止计时
						  ClearCount(&WorkTime);
							//刷新界面
							SetScreen(LCD_MODELSEL_PAGE);
							//清空控制参数
							memset(&IrrPara,0,sizeof(IrrPara));
							//改变平台状态
							SetRemotePara(3,0);//设置停止-0，启动-1
							SetRemoteRead(2,0);//清除当前灌溉分区（at_partition）    1分区-1,2分区-2，空闲-0
							SetRemoteRead(0,0);//工作状态（work_state） 空闲-0，清水-1，肥水-2 
							SetRemoteRead(1,0);//清除倒计时（in_time）0	
							
							fsbz_3060_cx();//进行一次上传数据的程序
						}
					
					}
					break;
			}
		}

	}
}

