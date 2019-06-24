#include "PCHmi.h"
#include "Rs485.h"
//#include "GM3Dir.h"
#include "Delay.h"
#include "ZoneCtrl.h"
#include "FlashDivide.h"

u8 PC_Rev_Buf[256];
u8 PC_MasterSend_Buf[256];
u8 PC_SlaveSend_Buf[256];
PCCtrlStruct PCCtrl;
InitativeStruct PCMasterInitative;

OS_EVENT * PCMasterQMsg;
void* 	PCMasterMsgBlock[4];
OS_MEM* PCMasterPartitionPt;
u8 g_u8PCMasterMsgMem[20][4];
//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 PCMasterQInit(void)
{
	INT8U os_err;
	
	PCMasterQMsg = OSQCreate ( PCMasterMsgBlock, 4);
	
	if(PCMasterQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	PCMasterPartitionPt = OSMemCreate (
										g_u8PCMasterMsgMem,
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

u8 PackSendPCMasterQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(PCMasterPartitionPt,&os_err);
	if(MsgTemp == NULL)
	{
		return 1;
	}
	MsgTemp ->CmdType = MsgBlk->CmdType;
	MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
	MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
	MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
	os_err = OSQPost ( PCMasterQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(PCMasterPartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceivePCMasterQ(MsgStruct * PCMasterQ)
{
	u8 os_err;
	os_err = OSMemPut(PCMasterPartitionPt, ( void * )PCMasterQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//从机发送标志位
//输入:设置值
//输出:无
void SetSlaveTxFlag(u8 Value)
{
	PCCtrl.SlaveTxFlag = Value;
}
//接收函数
//输入:接收BUF首地址
//返回: 0:无报文 >0:实际接收字节数
u8 PC_Rev(u8*Buf)
{
	u8 RevLen = 0;
	RevLen = COMGetBuf(COM4,PC_Rev_Buf,256);
	return RevLen;
}
//判断接收报文类型
//输入:接收BUF地址
//返回:接收到的报文类型 0 接收错误 1 从机接收 2 主机接收  
u8 PC_RevType(u8*Buf)
{
	u8 RevType = 0;
	//
	if(((Buf[1]>=0x01)&&(Buf[1]<=0x06))||(Buf[1]==0x0f)||(Buf[1]==0x10))
	{
		RevType = 1;
	}
	else if((Buf[1]==0x42)||((Buf[1]>=0x44)&&(Buf[1]<=0x46)))
	{
		RevType = 2;
	}
	return RevType;
}
//验证从机收到的报文是否有错误
//输入:收到的报文,收到的字节数
//返回:0:无错误 1:从机地址错误 2:报文长度不够 3:地址+数量超限 4:CRC错误 5:长度错误
u8 Verify_PCSlaveRev(u8* RevBuf,u8 RevLen)
{
	u8 DevID = RevBuf[0];
	u8 FunCode = RevBuf[1];
	u16 StartAddr = (RevBuf[2]<<8)|RevBuf[3];
	u16 DataNum = (RevBuf[4]<<8)|RevBuf[5];
	u16 Crc = (RevBuf[RevLen-2]<<8)|RevBuf[RevLen-1];
	if(DevID!= 0x01)
	{
		return 1;
	}
	if(RevLen<5)
	{
		return 2;
	}
	if(FunCode==WR_SINGLE_COIL)
	{
		if(StartAddr>PC_COIL_NUM)
		{
			return 3;
		}
	}
	else if((FunCode==RD_COIL_STA)||(FunCode==WR_MUL_COIL))
	{
		if((StartAddr+DataNum)>PC_COIL_NUM)
		{
			return 3;
		}
		if(FunCode==WR_MUL_COIL)
		{
			if((RevLen-9)!=RevBuf[6])
			{
				return 5;
			}
		}
	}
	else if(FunCode == RD_INPUT_STA)

	{
		if((StartAddr+DataNum)>PC_INSTA_NUM)
		{
			return 3;
		}
	}
	else if(FunCode == WR_SINGLE_REG)
	{
		if(StartAddr>PC_REG_NUM)
		{
			return 3;
		}
	}
	else if((FunCode==RD_HOLDING_REG)||(FunCode==WR_MUL_REG))
	{
		if((StartAddr+DataNum)>PC_REG_NUM)
		{
			return 3;
		}
		if(FunCode==WR_MUL_REG)
		{
			if((RevLen-9)!=RevBuf[6])
			{
				return 5;
			}
		}
	}
	else if(FunCode == RD_INPUT_REG)
	{
		if((StartAddr+DataNum)>PC_INREG_NUM)
		{
			return 3;
		}
	}
	if(Crc!=CRC16(RevBuf,RevLen-2))
	{
		return 4;
	}
	return 0;
}
//解析收到的从机报文
//输入:接收到的报文,收到的字节数
//返回:0:无错误 1:从机地址错误 2:报文长度不够 3:地址+数量超限
u8 PC_SlaveUnpack(u8* RevBuf,u8 RevLen)
{
	u8 ReturnCode = 0;
	ReturnCode = Verify_PCSlaveRev(RevBuf,RevLen);
	if(ReturnCode!=0)
	{
		return ReturnCode;
	}
	switch(RevBuf[1])
	{
		case RD_COIL_STA:
		case RD_INPUT_STA:
		case RD_HOLDING_REG:
		case RD_INPUT_REG:
			break;
		case WR_SINGLE_COIL:
			SetSingleCoil(RevBuf);
			break;
		case WR_SINGLE_REG:
			SetSingleReg(RevBuf);
			break;
		case WR_MUL_COIL:
			SetMulCoil(RevBuf);
			break;
		case WR_MUL_REG:
			SetMulReg(RevBuf);
			break;
		default:
			break;
	}
	return ReturnCode;
}
//从机组帧函数
//输入:接收Buf首地址 ,发送Buf首地址
//返回:发送字节数
u8 PC_SlavePack(u8* RxBuf,u8* TxBuf)
{
	u8 Cnt = 0;
	u16 Crc = 0;
	TxBuf[0] = 0x01;
	TxBuf[1] = RxBuf[1];
	switch(RxBuf[1])
	{
		case RD_COIL_STA:
			Cnt = FrameRdCoil(RxBuf,TxBuf,0);
			break;
		case RD_INPUT_STA:
			Cnt = FrameRdInSta(RxBuf,TxBuf,0);
			break;
		case RD_HOLDING_REG:
			Cnt = FrameRdReg(RxBuf,TxBuf);
			break;
		case RD_INPUT_REG:
			Cnt = FrameRdInReg(RxBuf,TxBuf,0);
			break;
		case WR_SINGLE_COIL:
			Cnt = FrameWrCoil(RxBuf,TxBuf);
			break;
		case WR_SINGLE_REG:
			Cnt = FrameWrReg(RxBuf,TxBuf);
			break;
		case WR_MUL_COIL:
			Cnt = FrameWrCoil(RxBuf,TxBuf);
			break;
		case WR_MUL_REG:
			Cnt = FrameWrReg(RxBuf,TxBuf);
			break;
	}
	Crc = CRC16(TxBuf,Cnt);
	TxBuf[Cnt++] = (u8)(Crc>>8&0xff);
	TxBuf[Cnt++] = (u8)(Crc&0xff);
	return Cnt;
}
//组错误帧
//输入:错误类型 发送Buf首地址
//返回:发送字节数
u8 PC_ErrorPack(u8 ErrType,u8* RxBuf,u8* TxBuf)
{
	u8 Cnt = 0;
	u16 Crc = 0;
	TxBuf[Cnt++] = 0x01;
	TxBuf[Cnt++] = RxBuf[1]+0x80;
	TxBuf[Cnt++] = ErrType;
	Crc = CRC16(TxBuf,Cnt);
	TxBuf[Cnt++] = (u8)(Crc>>8&0xff);
	TxBuf[Cnt++] = (u8)(Crc&0xff);
	return Cnt;
}
//验证主机收到的报文是否有错误
//输入:收到的报文,收到的字节数
//返回:0:无错误 1:从机地址错误 2:报文长度不够 3:CRC错误
u8 Verify_PCMasterRev(u8* RevBuf,u8 RevLen)
{
	u8 DevID = RevBuf[0];
//	u8 FunCode = RevBuf[1];
//	u16 StartAddr = (RevBuf[2]<<8)|RevBuf[3];
//	u16 DataNum = (RevBuf[4]<<8)|RevBuf[5];
	u16 Crc = (RevBuf[RevLen-2]<<8)|RevBuf[RevLen-1];
	Crc = CRC16(RevBuf,RevLen-2);
	if(DevID!=0x01)
	{
		return 1;
	}
	if(RevLen<5)
	{
		return 2;
	}
	if(Crc!=CRC16(RevBuf,RevLen-2))
	{
		return 3;
	}
	return 0;
	
}
//主机报文解析
//输入:接收BUF首地址 ,接收字节数
//返回: 0:无错误 1:从机地址错误 2:报文长度不够 3:CRC错误 4:功能码错误
u8 PC_MasterUnpack(u8* RevBuf,u8 RevLen,u8 Code)
{
	u8 ReturnCode = 0;
	ReturnCode = Verify_PCSlaveRev(RevBuf,RevLen);
	if(ReturnCode==0)
	{
		if(Code == RevBuf[1])
		{
			ReturnCode = 0;
		}
		else
		{
			ReturnCode = 4;
		}
	}
	return ReturnCode;
}
//主机组帧
//输入:主机结构体,发送Buf首地址
//返回:发送字节数
void PC_MasterPack(InitativeStruct Initative,u8* TxBuf)
{
	u8 Cnt = 0;
	u16 Crc = 0;
	u8 RxBuf[6];
	//与GM3Dirv一致
	RxBuf[1] = Initative.DataType;
	RxBuf[2] = (u8)((Initative.DataAddr>>8)&0xff);
	RxBuf[3] = (u8)(Initative.DataAddr&0x00ff);
	RxBuf[4] = 0;
	RxBuf[5] = Initative.DataNum;
	//
	TxBuf[Cnt++] = 0x01;
	TxBuf[Cnt++] = Initative.DataType;
	switch(Initative.DataType)
	{
		case INITIATIVE_COIL_STA:
			Cnt = FrameRdCoil(RxBuf,TxBuf,1);
			break;
		case INITIATIVE_INPUT_STA:
			Cnt = FrameRdInSta(RxBuf,TxBuf,1);
			break;
		case INITIATIVE_HOLDING_REG:
			break;
		case INITIATIVE_INPUT_REG:
			Cnt = FrameRdInReg(RxBuf,TxBuf,1);
			break;
		default:
			break;
	}
	Crc = CRC16(TxBuf,Cnt);
	TxBuf[Cnt++] = (u8)(Crc>>8&0xff);
	TxBuf[Cnt++] = (u8)(Crc&0xff);
	comSendBuf(COM4,TxBuf,Cnt);
}
void PCHmi_task(void *pdata)
{
	u8 os_err;
	u8 RevSize = 0;
	u8 SendSize = 0;
	u8 SlaveUnpackR = 0;//从机解帧返回值
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
//	MsgStruct MsgtempBlk;
	PCMasterQInit();
	while(1)
	{
		delay_ms(20);
		//接收数据
		RevSize = PC_Rev(PC_Rev_Buf);
		if(RevSize>0)
		{
			PCCtrl.RevType = PC_RevType(PC_Rev_Buf);
			//RevSize = 0;
		}
		if(PCCtrl.RevType == 1)
		{
			PCCtrl.RevType = 0;
			SlaveUnpackR = PC_SlaveUnpack(PC_Rev_Buf,RevSize);
			if(SlaveUnpackR == 0)
			{
				SendSize = PC_SlavePack(PC_Rev_Buf,PC_SlaveSend_Buf);
			}
			else
			{
				SendSize = PC_ErrorPack(SlaveUnpackR,PC_Rev_Buf,PC_SlaveSend_Buf);
			}
			comSendBuf(COM4,PC_SlaveSend_Buf,SendSize);
			delay_ms(20);//防止从机与主机的帧一起发出去
		}
		switch(PCCtrl.MasterState)
		{
			case 0:
				pMsgBlk = ( MsgStruct *) OSQPend ( PCMasterQMsg,
							3,
							&os_err );
				
				if(os_err == OS_ERR_NONE)
				{
					memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
					DepackReceivePCMasterQ(pMsgBlk);
					PCMasterInitative.DataType = Msgtemp.CmdType;
					PCMasterInitative.DataAddr = (Msgtemp.CmdData[0]<<8)|Msgtemp.CmdData[1];
					PCMasterInitative.DataNum = Msgtemp.CmdSrc;
					PC_MasterPack(PCMasterInitative,PC_SlaveSend_Buf);
					PCCtrl.MasterState = 1;//进入接收状态
				}
				break;
			case 1:
				if(PCCtrl.RevType == 2)
				{
					PCCtrl.RevType = 0;
					if(PC_MasterUnpack(PC_Rev_Buf,RevSize,PCMasterInitative.DataType)==0)
					{
						PCCtrl.MasterErrCnt = 0;
						PCCtrl.MasterState = 0;//进入等待发送状态
						PCCtrl.Off_Line = 0;//置错误标志
					}
				}
				else
				{
					if((PCCtrl.MasterErrCnt++)%50 == 0)
					{
						PCCtrl.MasterState = 2;//进入重发状态
					}
					if(PCCtrl.MasterErrCnt>260)
					{
						PCCtrl.MasterState = 0;//进入等待发送状态
						PCCtrl.Off_Line = 1;//置错误标志
						PCCtrl.MasterErrCnt = 0;
					}
				}
				break;
			case 2:
				PC_MasterPack(PCMasterInitative,PC_SlaveSend_Buf);
				PCCtrl.MasterState = 1;//进入接收状态
				break;
		}
	}
}

