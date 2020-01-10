#include "GM3Dir.h"
#include "Rs485.h"
#include "delay.h"
#include "string.h"    
#include "Includes.h"
#include "ZoneCtrl.h"
#include "UserCore.h"
#include "FlashDivide.h"
#include "W25Qxx.h"
#include "IO_BSP.h"

u8 ModbusCoil[MAX_COIL_NUM/8];//modbus线圈，共40个
u16 ModbusReg[MAX_REG_NUM];//modbus寄存器
u8 ModbusInSta[MAX_INSTA_NUM/8];//modbus输入离散变量
u16 ModebusInReg[MAX_INREG_NUM];//modbus输入寄存器
Ring NETCircle;
ModbusStruct ModbusPara;
InitativeStruct InitactivePara;
OS_EVENT * RemoteQMsg;
WorkPara Remote_Para;

void* 	RemoteMsgBlock[8];
OS_MEM* RemotePartitionPt;
u8 g_u8RemoteMsgMem[20][8];
//创建消息队列和内存块
//输出0：创建正常
//    1：消息队列创建失败
//	  2：内存块创建失败
u8 RemoteQInit(void)
{
	INT8U os_err;
	
	RemoteQMsg = OSQCreate ( RemoteMsgBlock, 8);
	
	if(RemoteQMsg == (OS_EVENT *)0)
	{
		return 1;
	}
	
	RemotePartitionPt = OSMemCreate (
										g_u8RemoteMsgMem,
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

u8 PackSendRemoteQ(MsgStruct* MsgBlk)
{
	INT8U os_err;
	MsgStruct * MsgTemp = NULL;
	MsgTemp = (MsgStruct *)OSMemGet(RemotePartitionPt,&os_err);
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
	os_err = OSQPost ( RemoteQMsg,(void*)MsgTemp );
	//发送消息失败释放内存
	if(os_err!=OS_ERR_NONE)
	{
		OSMemPut(RemotePartitionPt, ( void * )MsgTemp);
		return 2;
	}
	return 0;
}
//队列消息解析，消息内存释放
//输入：收到的消息队列指针
//输出：0：释放成功
//		1：释放失败
u8 DepackReceiveRemoteQ(MsgStruct * RemoteQ)
{
	u8 os_err;
	os_err = OSMemPut(RemotePartitionPt, ( void * )RemoteQ);
	if(os_err != OS_ERR_NONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//设置主动发送地址
void SetIniactivePara(InitativeStruct Para)
{
	memcpy(&InitactivePara,&Para,sizeof(InitactivePara));
}
//清空主动发送参数
void ClearIniactivePara(void)
{
	memset(&InitactivePara,0,sizeof(InitactivePara));
}
//StaNum从0号开始
//设置输入线圈的值
void SetInSta(u8 StaNum, u8 Flag)
{
	if(Flag == 1)
	{
		ModbusInSta[StaNum/8] |= 1<<(StaNum%8);
	}
	else if(Flag == 0)
	{
		ModbusInSta[StaNum/8] &= (1<<(StaNum%8)^0xff);
	}
	
}
//CoilNum从0号开始
//设置线圈的值
void SetMDCoil(u8 CoilNum, u8 Flag)
{
	if(Flag == 1)
	{
		ModbusCoil[CoilNum/8] |= 1<<(CoilNum%8);
	}
	else if(Flag == 0)
	{
		ModbusCoil[CoilNum/8] &= (1<<(CoilNum%8)^0xff);
	}
}

u8 NETSendBuf[256];
u8 NETSendBuf2[256];
//收发状态切换
//输入：切换值
void RTSwitach(u8 Val)
{
	ModbusPara.TxRxSwitch = Val;
}
//清空接收BUF
void ClearNetCircle(void)
{
	memset(&NETCircle, 0 , sizeof(NETCircle));
}
//清空发送BUF
void ClearNetSendBuf(void)
{
	memset(NETSendBuf, 0 , 256);
}
//分析收到的报文格式是否正确
//输入：接收到的报文
//返回：0：正常 1：数据地址无效 2：数据数量无效 3:地址+数据数量无效
u8 FormatVerify(u8 *Buf)
{
	u8 Max_Num;
	u8 FunCode = Buf[1];
	u16 StartAddr = Buf[2]<<8|Buf[3];
	u16 DataNum = Buf[4]<<8|Buf[5];
	
	if(FunCode == RD_COIL_STA || FunCode == WR_SINGLE_COIL)
	{
		Max_Num = MAX_COIL_NUM;
	}
	else if(FunCode == RD_INPUT_STA)
	{
		Max_Num = MAX_INSTA_NUM;
	}
	else if(FunCode == RD_HOLDING_REG || FunCode == WR_SINGLE_REG)
	{
		Max_Num = MAX_REG_NUM;
	}
	else if(FunCode == RD_INPUT_REG)
	{
		Max_Num = MAX_INREG_NUM;
	}
	if(StartAddr > Max_Num)
	{
		return 1;
	}
	if((DataNum == 0) || (DataNum > Max_Num))
	{
		if(FunCode != WR_SINGLE_COIL && FunCode != WR_SINGLE_REG)
			return 2;
	}
	if((StartAddr+DataNum) > Max_Num)
	{
		if(FunCode != WR_SINGLE_COIL && FunCode != WR_SINGLE_REG)
			return 3;
	}
	return 0;
}
//写开关量
//输入：接收到的报文
//Val错误不执行，不返回错误码
void SetSingleCoil(u8 *Buf)
{
	u16 DataAddr = Buf[2]<<8|Buf[3];
	u16 DataVal = Buf[4]<<8|Buf[5];
	MsgStruct Msgtemp;
	OS_CPU_SR  cpu_sr;

	//单个控制开关全部开启
	//跟策略相关的线圈，只能在策略未执行的时候控制
	if(DataAddr == 0x00)
	{
		if(DataVal == 0xff00)
		{
			Msgtemp.CmdType = MSG_START;
			Msgtemp.CmdSrc = GPRS_TASK_CODE;
			Msgtemp.CmdData[0] = Remote_Para.WorkModel;
			Msgtemp.CmdData[1] = Remote_Para.Zone;
			Msgtemp.CmdData[2] = Remote_Para.TimeH;
			Msgtemp.CmdData[3] = Remote_Para.TimeM;
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();
			SetScreen(LCD_STATESHOW_PAGE);
			WorkCountShow(Remote_Para.TimeH,Remote_Para.TimeM);
		}
		else if(DataVal == 0x0000)
		{
			Msgtemp.CmdType = MSG_STOP;
			Msgtemp.CmdSrc = GPRS_TASK_CODE;
			OS_ENTER_CRITICAL();
			PackSendMasterQ(&Msgtemp);
			OS_EXIT_CRITICAL();
		}
	}
}
u8 alfredTest = 0;
void SetSingleReg(u8* Buf)
{
//	StrategyStruct CmdStrategy;
	u16 DataAddr = Buf[2]<<8|Buf[3];
	u16 DataVal = Buf[4]<<8|Buf[5];
//	OS_CPU_SR  cpu_sr;
	if(DataAddr == 0x00)
	{
		Remote_Para.WorkModel = (u8) DataVal;
		ModbusReg[0] = Remote_Para.WorkModel;
	}
	else if(DataAddr == 0x01 )
	{
		Remote_Para.Zone = (u8) DataVal;
		ModbusReg[1] = Remote_Para.Zone;
	}
	else if(DataAddr == 0x02 )
	{
		Remote_Para.TimeH = DataVal/60;
		Remote_Para.TimeM = DataVal%60;
		ModbusReg[2] = Remote_Para.TimeH*60+Remote_Para.TimeM;
	}
	
}
//解析从云端收到的消息
//输入：收到的报文
u8 UpackPlatform (u8*Buf)
{
	u8 Error = 0;
	if(Buf[0] != ModbusPara.DeviceID)
	{
		//ERROR
		RTSwitach(TXFLAG);
		return 5;
	}
	Error = FormatVerify(Buf);
	switch(Buf[1])
	{
		case RD_COIL_STA:
		case RD_INPUT_STA:
		case RD_HOLDING_REG:
		case RD_INPUT_REG:
			RTSwitach(TXFLAG);
			break;
		case WR_SINGLE_COIL:
			SetSingleCoil(Buf);
			RTSwitach(TXFLAG);
			break;
		case WR_SINGLE_REG:
			SetSingleReg(Buf);
			RTSwitach(TXFLAG);
			break;
		case WR_MUL_COIL:
			//该功能暂时不启用
			break;
		case WR_MUL_REG:
			//该功能暂时不启用
			break;
		case INITIATIVE_COIL_STA:
			ClearNetCircle();
			ModbusPara.Busy = 0;
			break;
		case INITIATIVE_INPUT_STA:
			ClearNetCircle();
			ModbusPara.Busy = 0;
			break;
		
		case INITIATIVE_HOLDING_REG:
			ClearNetCircle();
			ModbusPara.Busy = 0;
			break;
		
		case INITIATIVE_INPUT_REG:
			ClearNetCircle();
			ModbusPara.Busy = 0;
			break;
		default:
			Error = 4;//功能码异常
			RTSwitach(TXFLAG);
			break;
	}
	return Error;
	
}
//组异常帧
//输入：收到的报文 错误码 发送Buf
//返回：要发送的字节数
//备注：错误码 1：数据地址错误 2：数据数量错误 3：数据地址+数据数量错误 4：功能码错误
u8 FrameError(u8 *Buf, u8 ErrorCode,u8* SendBuf)
{
	u8 Cnt = 0;
	u16 Crc = 0;
	OS_CPU_SR  cpu_sr;
	
	SendBuf[Cnt++] = ModbusPara.DeviceID;
	SendBuf[Cnt++] = 0x80+Buf[1];
	SendBuf[Cnt++] = ErrorCode;
	OS_ENTER_CRITICAL();
	//公共函数使用
	Crc = CRC16(SendBuf,Cnt);
	OS_EXIT_CRITICAL();
	SendBuf[Cnt++] = (u8)(Crc>>8&0xff);
	SendBuf[Cnt++] = (u8)(Crc&0xff);
	return Cnt;
	
}
//整个数组按位右移函数,
//待验证
void ArrRightShift(u8 *Des, u8 *Src, u8 MoveStep, u8 ArrSize)
{
	u8 i,j,Temp;
	u8 Verify = 0;
	j = MoveStep/8 ;
	Temp = MoveStep%8;
	//确认数确定，由右移位数确定,用于取出右移到前面八位中的位
	for(i=0; i<Temp; i++)
	{
		Verify |= 1<<i;
	}
	for(i = 0; j < ArrSize; i++,j++)
	{
		Des[i] = Src[j]>>Temp;
		if(j+1 == ArrSize)
			break;
		Des[i] = Des[i]|((Src[j+1]&Verify)<<(8-Temp));
	}
}
//组读线圈的帧
//输入：接收到的报文，发送Buf
//返回：已经写了的字节数
u8 FrameRdCoil(u8* RxBuf, u8* TxBuf, u8 Flag)
{
	u8 Cnt = 2,i,j,Temp = 0;
	u8 CoilArr[MAX_COIL_NUM/8];
	u16 ReDataAddr = RxBuf[2]<<8|RxBuf[3];
	u16 ReDataNum =  RxBuf[4]<<8|RxBuf[5];
	u16 SendByte = (ReDataNum-1)/8+1;//逻辑有错误,现已修改，需测试
	if(Flag == 1)
	{
		TxBuf[Cnt++] = RxBuf[2];
		TxBuf[Cnt++] = RxBuf[3];
		TxBuf[Cnt++] = 0x0;//发送数据不会超过256
		TxBuf[Cnt++] = (u8)(ReDataNum&0x00ff);
		TxBuf[Cnt++] = (u8)(SendByte&0x00ff);
	}
	else 
	{
		TxBuf[Cnt++] = (u8)SendByte&0x00ff;
	}
	ArrRightShift(CoilArr,ModbusCoil,(u8)ReDataAddr,MAX_COIL_NUM/8);
	//TxBuf[Cnt++] = (u8)SendByte&0x00ff;
	for(i=0; i<SendByte; i++)
	{
		TxBuf[Cnt++] = CoilArr[i];
	}
	if(ReDataNum%8!=0)
	{
		for(j = 0;j < ReDataNum%8;j++)
		{	
			Temp = Temp|(1<<j);
		}
		TxBuf[Cnt-1] |=  CoilArr[i]&Temp;
	}
	
	return Cnt;
}
//组读输入离散量
//非标准写法，只适用于现在16个  只支持全部查询，不支持单个查询
u8 FrameRdInSta(u8* RxBuf, u8* TxBuf, u8 Flag)
{
	u8 Cnt = 2;
	u8 i = 0;
	if(Flag == 1)
	{
		TxBuf[Cnt++] = RxBuf[2];
		TxBuf[Cnt++] = RxBuf[3];
		TxBuf[Cnt++] = MAX_INSTA_NUM>>8;
		TxBuf[Cnt++] = MAX_INSTA_NUM%256;
		TxBuf[Cnt++] = MAX_INSTA_NUM/8;
	}
	else
	{
		TxBuf[Cnt++] = MAX_INSTA_NUM;
	}
	for(i = 0;i< MAX_INSTA_NUM/8 ; i++)
	{
		TxBuf[Cnt++] = ModbusInSta[i];
	}
	//TxBuf[Cnt++] = ModbusInSta[1];
	return Cnt;
}
//组读寄存器
//输入：收到的报文，发送的BUF
//返回：已经写了的字节数
u8 FrameRdReg(u8* RxBuf, u8* TxBuf,u8 flag)
{
	u8 Cnt = 2,i;
	u16 ReDataAddr = RxBuf[2]<<8|RxBuf[3];
	u16 ReDataNum =  RxBuf[4]<<8|RxBuf[5];
	if(flag == 1)
	{
		TxBuf[Cnt++] = RxBuf[2];//地址
		TxBuf[Cnt++] = RxBuf[3];
		TxBuf[Cnt++] = RxBuf[4];//数量
		TxBuf[Cnt++] = RxBuf[5];
	}
	TxBuf[Cnt++] = ReDataNum<<1;
	//FlashReadWaterTime((u8*)&ModbusReg[0]);
	//	FlashReadFertilizerTime((u8*)&ModbusReg[1]);
	for(i = 0; i < ReDataNum; i++,ReDataAddr++)
	{
		TxBuf[Cnt++] =(u8) ((ModbusReg[ReDataAddr]&0xff00)>>8);
		TxBuf[Cnt++] =(u8) (ModbusReg[ReDataAddr]&0x00ff);
	}
	return Cnt;
}
//组读输入寄存器
//输入：收到的报文，发送的BUF
//返回：已经写了的字节数
u8 FrameRdInReg(u8* RxBuf, u8* TxBuf,u8 flag )
{
	u8 Cnt = 2,i;
	u16 ReDataAddr = RxBuf[2]<<8|RxBuf[3];
	u16 ReDataNum =  RxBuf[4]<<8|RxBuf[5];
	
	if(flag == 1)
	{
		TxBuf[Cnt++] = RxBuf[2];//地址
		TxBuf[Cnt++] = RxBuf[3];
		TxBuf[Cnt++] = RxBuf[4];//数量
		TxBuf[Cnt++] = RxBuf[5];
		TxBuf[Cnt++] = RxBuf[5]*2;//字节数
	}
	else
	{
		TxBuf[Cnt++] = ReDataNum<<1;
	}
	for(i = 0; i < ReDataNum; i++,ReDataAddr++)
	{
		TxBuf[Cnt++] =(u8) ((ModebusInReg[ReDataAddr]&0xff00)>>8);
		TxBuf[Cnt++] =(u8) (ModebusInReg[ReDataAddr]&0x00ff);
	}
	return Cnt;
}

//组写线圈
//输入：收到的报文，发送的BUF
//返回：已经写了的字节数
u8 FrameWrCoil(u8* RxBuf, u8* TxBuf)
{
	TxBuf[2] = RxBuf[2];
	TxBuf[3] = RxBuf[3];
	TxBuf[4] = RxBuf[4];
	TxBuf[5] = RxBuf[5];
	return 6;
}
//组写寄存器
//输入：收到的报文，发送的BUF
//返回：已经写了的字节数
u8 FrameWrReg(u8* RxBuf, u8* TxBuf)
{
	TxBuf[2] = RxBuf[2];
	TxBuf[3] = RxBuf[3];
	TxBuf[4] = RxBuf[4];
	TxBuf[5] = RxBuf[5];
	return 6;
}
//正常组帧
//输入：收到的报文 发送Buf
//返回：要发送的字节数
u8 FrameNormal(u8* RxBuf, u8* TxBuf)
{
	u8 Cnt = 0;
	OS_CPU_SR  cpu_sr;
	u16 Crc = 0;
	TxBuf[0] = ModbusPara.DeviceID;
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
			Cnt = FrameRdReg(RxBuf,TxBuf,0);
			break;
		case RD_INPUT_REG:
			Cnt = FrameRdInReg(RxBuf,TxBuf,0);
			break;
		case WR_SINGLE_COIL:
			Cnt = FrameWrCoil(RxBuf,TxBuf);
			break;
		case WR_SINGLE_REG:
			delay_ms(50);
			Cnt = FrameWrReg(RxBuf,TxBuf);
			break;
		case INITIATIVE_COIL_STA:
			Cnt = FrameRdCoil(RxBuf,TxBuf,1);
			break;
		case INITIATIVE_INPUT_STA:
			Cnt = FrameRdInSta(RxBuf,TxBuf,1);
			break;
		case INITIATIVE_HOLDING_REG:
			Cnt = FrameRdReg(RxBuf,TxBuf,1);
			break;
		case INITIATIVE_INPUT_REG:
			//主动上报输入寄存器
			Cnt = FrameRdInReg(RxBuf,TxBuf,1);
			break;
	}
	
	OS_ENTER_CRITICAL();
	//公共函数使用
	Crc = CRC16(TxBuf,Cnt);
	OS_EXIT_CRITICAL();
	
	TxBuf[Cnt++] = (u8)((Crc>>8)&0xff);
	TxBuf[Cnt++] = (u8)(Crc&0xff);
	return Cnt;
}
//主动上报组帧
//输入: 主动上报参数
//返回: 发送的字节数
u8 FrameInitactive(InitativeStruct Para)
{
	u8 TempBuf[6];
	u8 Cnt;
	
	TempBuf[1] = Para.DataType;
	TempBuf[2] = (u8)((Para.DataAddr>>8)&0xff);
	TempBuf[3] = (u8)(Para.DataAddr&0x00ff);
	TempBuf[4] = 0;
	TempBuf[5] = Para.DataNum;
	Cnt = FrameNormal(TempBuf,NETSendBuf2);
	return Cnt;
}

//void GM3_check_cmd(u8*str)
//GM3发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* GM3_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)NETCircle.buf,(const char*)str);
	return (u8*)strx;
}
//gm3发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 GM3_send_cmd(u8 *cmd,u8 *ack,u16 waittime, u8 flag)
{
	u8 res=0; 
	u8*p = NULL;
//	u8 uCnt = 0;

	if(flag)//
	{
		u3_printf("%s\r\n",cmd);//发送命令
	}
	else 
	{
		u3_printf("%s",cmd);//发送命令
	}
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{ 
			delay_ms(10);
			//串口收到新的数据，并判断是否有为有效数据
			COMGetBuf(COM1 ,NETCircle.buf, 256);
			{
				p = GM3_check_cmd(ack);
				if(p)
				{
					*p = 1;//破坏字符串
					break;//得到有效数据 
				}
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
u8 GprspPraSetF = 0;
u8 GprsParaSetState = 0;//\"00013629000000000003\"
u8 DevIdBuf[39] = {'A','T','+','C','L','O','U','D','=',
				'0','0','0','1','3','7','1','2',
				'0','0','0','0','0','0','0','0','0','0','0','8',44,
				'5','q','h','O','K','b','x','u',0};
u8 DevPaBuf[24] = {'A','T','+','C','L','O','U','D','P','A','=',
				'\"','5','q','h','O','K','b','x','u','\"',0};
//"AT+CLOUDPA=\"wfM9uJs3\""

void gprs_task(void *pdata)
{
	//u8 Com3RxBuf[10]={0};
	u8 SendNum = 0;
	u8 os_err;
	MsgStruct * pMsgBlk = NULL;
	MsgStruct Msgtemp;
	RemoteQInit();
	IO_OutSet(6,1);
	delay_ms(1000);
	ModbusPara.DeviceID = 1;
	while(1)
	{
		delay_ms(100);
		if(GprspPraSetF == 1)//GPRS设置
		{
			switch(GprsParaSetState)
			{
				case GM3_SET_READY:
					delay_ms(150);
					//初始化com3接收BUF
					Uart1VarInit();
					GprsParaSetState = GM3_SET_CHANGEMODE1;
					break;
				case GM3_SET_CHANGEMODE1:
					//if(GM3_send_cmd("+++","a",30,0) == 0)
					if(GM3_send_cmd("AT","a",30,0) == 0)
					{
						GprsParaSetState = GM3_SET_CHANGEMODE2;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CHANGEMODE2:
 					if(GM3_send_cmd("a","+ok",30,0) == 0)
					{
						GprsParaSetState = GM3_SET_E0;
						//GprsParaSetState = GM3_SET_SETMODE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_E0:
					if(GM3_send_cmd("AT+E=OFF","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SETMODE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_SETMODE:
					if(GM3_send_cmd("AT+WKMOD=NET","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_OPENSOCKETA;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_OPENSOCKETA:
					if(GM3_send_cmd("AT+SOCKAEN=ON","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SETDEST;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_SETDEST:
					if(GM3_send_cmd("AT+SOCKA=TCP,data.mingjitech.com,15000","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SETLINKTYPE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_SETLINKTYPE:
					if(GM3_send_cmd("AT+SOCKASL=long","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_CLOSESOCKETB;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CLOSESOCKETB:
					if(GM3_send_cmd("AT+SOCKBEN=OFF","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_OPENHEART;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_OPENHEART:
					if(GM3_send_cmd("AT+HEARTEN=ON","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_HEARTTIME;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_HEARTTIME:
					if(GM3_send_cmd("AT+HEARTTM=50","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_HEARTDATA;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_HEARTDATA:
					if(GM3_send_cmd("AT+HEARTDT=4F4B","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_HEARTTP;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_HEARTTP:
					if(GM3_send_cmd("AT+HEARTSND=NET","OK",900,1) == 0)
					{
						GprsParaSetState = GM3_SET_REGEN;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_REGEN:
					if(GM3_send_cmd("AT+REGEN=ON","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_REGSND;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_REGSND:
					if(GM3_send_cmd("AT+REGSND=link","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_REGTP;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_REGTP:
					if(GM3_send_cmd("AT+REGTP=CLOUD","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_CLOUDID;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_APN:
					if(GM3_send_cmd("AT+APN=CMNET,,,","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_CLOUDEN;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CLOUDEN:
					if(GM3_send_cmd("AT+CLOUDEN=ON","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_CLOUDID;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CLOUDID:
					if(GM3_send_cmd(DevIdBuf,"OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SAVE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CLOUDPA:
					//if(GM3_send_cmd("AT+CLOUDPA=\"wfM9uJs3\"","OK",30,1) == 0)
					if(GM3_send_cmd(DevPaBuf,"OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SAVE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_SAVE:
					GM3_send_cmd("AT+S","OK",30,1);
					GprspPraSetF = 0;
					break;
				default:
						break;
			}
		}
		else if(GprspPraSetF == 0)
		{
			//解析
			if((ModbusPara.TxRxSwitch == 0) && (COMGetBuf(COM1 ,NETCircle.buf, 256) > 0)&&(ModbusPara.Busy == 0))
			{
				ModbusPara.Busy = 1;
				ModbusPara.UnpackError = UpackPlatform(NETCircle.buf);
				ClearNetSendBuf();
			}
			//发送
			if(ModbusPara.TxRxSwitch == 1)
			{
				if(ModbusPara.UnpackError != 0)
				{
					//组错误帧，填写异常码
					SendNum = FrameError(NETCircle.buf,ModbusPara.UnpackError,NETSendBuf);
				}
				else
				{
					//根据功能码组帧
					SendNum = FrameNormal(NETCircle.buf,NETSendBuf);
				}
				RTSwitach(RXFLAG);
				ClearNetCircle();
				comSendBuf(COM1,NETSendBuf,SendNum);
				ModbusPara.Busy = 0;
				delay_ms(500);
			}
			else if(ModbusPara.Busy == 0)
			{
				pMsgBlk = ( MsgStruct *) OSQPend ( RemoteQMsg,
						10,
						&os_err );
				
				if(os_err == OS_ERR_NONE)
				{
					memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgStruct) );
					DepackReceiveRemoteQ(pMsgBlk);
					InitactivePara.DataAddr =Msgtemp.CmdData[0];
					InitactivePara.DataNum =Msgtemp.CmdData[1];
					InitactivePara.DataType =Msgtemp.CmdType;
					//主动发送函数
					SendNum = FrameInitactive(InitactivePara);
					ClearIniactivePara();
					comSendBuf(COM1,NETSendBuf2,SendNum);
					delay_ms(500);
				}
			}
		}
	}
}

