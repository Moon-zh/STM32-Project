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

u8 ModbusCoil[MAX_COIL_NUM/8];//modbus��Ȧ����40��
u16 ModbusReg[MAX_REG_NUM];//modbus�Ĵ���
u8 ModbusInSta[MAX_INSTA_NUM/8];//modbus������ɢ����
u16 ModebusInReg[MAX_INREG_NUM];//modbus����Ĵ���
Ring NETCircle;
ModbusStruct ModbusPara;
InitativeStruct InitactivePara;
//�����������͵�ַ
void SetIniactivePara(InitativeStruct Para)
{
	memcpy(&InitactivePara,&Para,sizeof(InitactivePara));
}
//����������Ͳ���
void ClearIniactivePara(void)
{
	memset(&InitactivePara,0,sizeof(InitactivePara));
}
//StaNum��0�ſ�ʼ
//����������Ȧ��ֵ
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
//CoilNum��0�ſ�ʼ
//������Ȧ��ֵ
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
//�շ�״̬�л�
//���룺�л�ֵ
void RTSwitach(u8 Val)
{
	ModbusPara.TxRxSwitch = Val;
}
//��ս���BUF
void ClearNetCircle(void)
{
	memset(&NETCircle, 0 , sizeof(NETCircle));
}
//��շ���BUF
void ClearNetSendBuf(void)
{
	memset(NETSendBuf, 0 , 256);
}
//�����յ��ı��ĸ�ʽ�Ƿ���ȷ
//���룺���յ��ı���
//���أ�0������ 1�����ݵ�ַ��Ч 2������������Ч 3:��ַ+����������Ч
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
//д������
//���룺���յ��ı���
//Val����ִ�У������ش�����
void SetSingleCoil(u8 *Buf)
{
	u16 DataAddr = Buf[2]<<8|Buf[3];
	u16 DataVal = Buf[4]<<8|Buf[5];
//	MsgStruct Msgtemp;
//	OS_CPU_SR  cpu_sr;

	//�������ƿ���ȫ������
	//��������ص���Ȧ��ֻ���ڲ���δִ�е�ʱ�����
	if(DataAddr <= 0x04)
	{
		if(DataVal == 0xff00)
		{
			IO_OutSet(DataAddr+1,1);
			SetMDCoil(DataAddr,1);
		}
		else if(DataVal == 0x0000)
		{
			IO_OutSet(DataAddr+1,0);
			SetMDCoil(DataAddr,0);
		}
	}
}
void SetSingleReg(u8* Buf)
{
	StrategyStruct CmdStrategy;
	u16 DataAddr = Buf[2]<<8|Buf[3];
	u16 DataVal = Buf[4]<<8|Buf[5];
	OS_CPU_SR  cpu_sr;
	if(DataAddr == 0x00)
	{
		if(DataVal > 1)
			return;
		CmdStrategy.PumpWFlag = DataVal;
		OS_ENTER_CRITICAL();
		if(SetStrategy(&CmdStrategy) == 0)
		{
			//д�ɹ�
		}
		OS_EXIT_CRITICAL();
	}
	else if(DataAddr == 0x01)
	{
		if(DataVal > 1)
			return;
		
		OS_ENTER_CRITICAL();
		FlashWriteWaterTime((u8*)&DataVal);
		OS_EXIT_CRITICAL();
	}
	else if(DataAddr >= 0x02 && DataAddr <= 0x07)
	{
		if(DataVal > 6)
			return;
		OS_ENTER_CRITICAL();
		FlashWriteFertilizerTime((u8*)&DataVal);
		OS_EXIT_CRITICAL();
	}
	else if(DataAddr == 0x08)
	{
		if(DataVal > 32 || DataVal == 0)
			return;
		CmdStrategy.Zone = (u8) DataVal;
		OS_ENTER_CRITICAL();
		if(SetStrategy(&CmdStrategy) == 0)
		{
			//д�ɹ�
		}
		OS_EXIT_CRITICAL();
	}
	else if(DataAddr == 0x09)
	{
		if(DataVal > 1440 || DataVal == 0)
			return;
		CmdStrategy.WorkHour =  DataVal/60;
		CmdStrategy.WorkMinute =  DataVal%60;
		OS_ENTER_CRITICAL();
		if(SetStrategy(&CmdStrategy) == 0)
		{
			//д�ɹ�
		}
		OS_EXIT_CRITICAL();
	}
	
}
//�������ƶ��յ�����Ϣ
//���룺�յ��ı���
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
			//�ù�����ʱ������
			break;
		case WR_MUL_REG:
			//�ù�����ʱ������
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
			Error = 4;//�������쳣
			RTSwitach(TXFLAG);
			break;
	}
	return Error;
	
}
//���쳣֡
//���룺�յ��ı��� ������ ����Buf
//���أ�Ҫ���͵��ֽ���
//��ע�������� 1�����ݵ�ַ���� 2�������������� 3�����ݵ�ַ+������������ 4�����������
u8 FrameError(u8 *Buf, u8 ErrorCode,u8* SendBuf)
{
	u8 Cnt = 0;
	u16 Crc = 0;
	OS_CPU_SR  cpu_sr;
	
	SendBuf[Cnt++] = ModbusPara.DeviceID;
	SendBuf[Cnt++] = 0x80+Buf[1];
	SendBuf[Cnt++] = ErrorCode;
	OS_ENTER_CRITICAL();
	//��������ʹ��
	Crc = CRC16(SendBuf,Cnt);
	OS_EXIT_CRITICAL();
	SendBuf[Cnt++] = (u8)(Crc>>8&0xff);
	SendBuf[Cnt++] = (u8)(Crc&0xff);
	return Cnt;
	
}
//�������鰴λ���ƺ���,
//����֤
void ArrRightShift(u8 *Des, u8 *Src, u8 MoveStep, u8 ArrSize)
{
	u8 i,j,Temp;
	u8 Verify = 0;
	j = MoveStep/8 ;
	Temp = MoveStep%8;
	//ȷ����ȷ����������λ��ȷ��,����ȡ�����Ƶ�ǰ���λ�е�λ
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
//�����Ȧ��֡
//���룺���յ��ı��ģ�����Buf
//���أ��Ѿ�д�˵��ֽ���
u8 FrameRdCoil(u8* RxBuf, u8* TxBuf, u8 Flag)
{
	u8 Cnt = 2,i,j,Temp = 0;
	u8 CoilArr[MAX_COIL_NUM/8];
	u16 ReDataAddr = RxBuf[2]<<8|RxBuf[3];
	u16 ReDataNum =  RxBuf[4]<<8|RxBuf[5];
	u16 SendByte = (ReDataNum-1)/8+1;//�߼��д���,�����޸ģ������
	if(Flag == 1)
	{
		TxBuf[Cnt++] = RxBuf[2];
		TxBuf[Cnt++] = RxBuf[3];
		TxBuf[Cnt++] = 0x0;//�������ݲ��ᳬ��256
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
//���������ɢ��
//�Ǳ�׼д����ֻ����������16��  ֻ֧��ȫ����ѯ����֧�ֵ�����ѯ
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
//����Ĵ���
//���룺�յ��ı��ģ����͵�BUF
//���أ��Ѿ�д�˵��ֽ���
u8 FrameRdReg(u8* RxBuf, u8* TxBuf)
{
	u8 Cnt = 2,i;
	u16 ReDataAddr = RxBuf[2]<<8|RxBuf[3];
	u16 ReDataNum =  RxBuf[4]<<8|RxBuf[5];
	TxBuf[Cnt++] = ReDataNum<<1;
	FlashReadWaterTime((u8*)&ModbusReg[0]);
	FlashReadFertilizerTime((u8*)&ModbusReg[1]);
	for(i = 0; i < ReDataNum; i++,ReDataAddr++)
	{
		TxBuf[Cnt++] =(u8) ((ModbusReg[ReDataAddr]&0xff00)>>8);
		TxBuf[Cnt++] =(u8) (ModbusReg[ReDataAddr]&0x00ff);
	}
	return Cnt;
}
//�������Ĵ���
//���룺�յ��ı��ģ����͵�BUF
//���أ��Ѿ�д�˵��ֽ���
u8 FrameRdInReg(u8* RxBuf, u8* TxBuf)
{
	u8 Cnt = 2,i;
	u16 ReDataAddr = RxBuf[2]<<8|RxBuf[3];
	u16 ReDataNum =  RxBuf[4]<<8|RxBuf[5];
	TxBuf[Cnt++] = ReDataNum<<1;
	//ModebusInReg[0] = WarterRemainderTime;
	//ModebusInReg[1] = FertilizerRemainderTime;
	for(i = 0; i < ReDataNum; i++,ReDataAddr++)
	{
		TxBuf[Cnt++] =(u8) ((ModebusInReg[ReDataAddr]&0xff00)>>8);
		TxBuf[Cnt++] =(u8) (ModebusInReg[ReDataAddr]&0x00ff);
	}
	return Cnt;
}
//��д��Ȧ
//���룺�յ��ı��ģ����͵�BUF
//���أ��Ѿ�д�˵��ֽ���
u8 FrameWrCoil(u8* RxBuf, u8* TxBuf)
{
	TxBuf[2] = RxBuf[2];
	TxBuf[3] = RxBuf[3];
	TxBuf[4] = RxBuf[4];
	TxBuf[5] = RxBuf[5];
	return 6;
}
//��д�Ĵ���
//���룺�յ��ı��ģ����͵�BUF
//���أ��Ѿ�д�˵��ֽ���
u8 FrameWrReg(u8* RxBuf, u8* TxBuf)
{
	TxBuf[2] = RxBuf[2];
	TxBuf[3] = RxBuf[3];
	TxBuf[4] = RxBuf[4];
	TxBuf[5] = RxBuf[5];
	return 6;
}
//������֡
//���룺�յ��ı��� ����Buf
//���أ�Ҫ���͵��ֽ���
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
			Cnt = FrameRdReg(RxBuf,TxBuf);
			break;
		case RD_INPUT_REG:
			Cnt = FrameRdInReg(RxBuf,TxBuf);
			break;
		case WR_SINGLE_COIL:
			Cnt = FrameWrCoil(RxBuf,TxBuf);
			break;
		case WR_SINGLE_REG:
			Cnt = FrameWrReg(RxBuf,TxBuf);
			break;
		case INITIATIVE_COIL_STA:
			Cnt = FrameRdCoil(RxBuf,TxBuf,1);
			break;
		case INITIATIVE_INPUT_STA:
			Cnt = FrameRdInSta(RxBuf,TxBuf,1);
			break;
	}
	
	OS_ENTER_CRITICAL();
	//��������ʹ��
	Crc = CRC16(TxBuf,Cnt);
	OS_EXIT_CRITICAL();
	
	TxBuf[Cnt++] = (u8)((Crc>>8)&0xff);
	TxBuf[Cnt++] = (u8)(Crc&0xff);
	return Cnt;
}
//�����ϱ���֡
//����: �����ϱ�����
//����: ���͵��ֽ���
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
//GM3���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* GM3_check_cmd(u8 *str)
{
	char *strx=0;
	strx = strstr((const char*)NETCircle.buf,(const char*)str);
	return (u8*)strx;
}
//gm3��������
//cmd:���͵������ַ���(����Ҫ���ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 GM3_send_cmd(u8 *cmd,u8 *ack,u16 waittime, u8 flag)
{
	u8 res=0; 
	u8*p = NULL;
//	u8 uCnt = 0;

	if(flag)//
	{
		u3_printf("%s\r\n",cmd);//��������
	}
	else 
	{
		u3_printf("%s",cmd);//��������
	}
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{ 
			delay_ms(10);
			//�����յ��µ����ݣ����ж��Ƿ���Ϊ��Ч����
			COMGetBuf(COM3 ,&NETCircle, 256);
			{
				p = GM3_check_cmd(ack);
				if(p)
				{
					*p = 1;//�ƻ��ַ���
					break;//�õ���Ч���� 
				}
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
u8 GprspPraSetF = 0;
u8 GprsParaSetState = 0;//\"00013629000000000003\"
u8 DevIdBuf[34] = {'A','T','+','C','L','O','U','D','I','D','=',
				'\"','0','0','0','1','3','7','1','2',
				'0','0','0','0','0','0','0','0','0','0','0','1','\"',0};
u8 DevPaBuf[24] = {'A','T','+','C','L','O','U','D','P','A','=',
				'\"','5','q','h','O','K','b','x','u','\"',0};
//"AT+CLOUDPA=\"wfM9uJs3\""
u8 Gm3Taskin = 0;
u8 Gm3Taskout = 0;

void gprs_task(void *pdata)
{
	//u8 Com3RxBuf[10]={0};
	u8 SendNum = 0;
	delay_ms(1000);
	ModbusPara.DeviceID = 1;
	while(1)
	{
		delay_ms(100);
		Gm3Taskin++;
		if(GprspPraSetF == 1)//GPRS����
		{
			switch(GprsParaSetState)
			{
				case GM3_SET_READY:
					delay_ms(150);
					//��ʼ��com3����BUF
					Uart1VarInit();
					GprsParaSetState = GM3_SET_CHANGEMODE1;
					break;
				case GM3_SET_CHANGEMODE1:
					if(GM3_send_cmd("+++","a",30,0) == 0)
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
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_E0:
					if(GM3_send_cmd("ATE0","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SETMODE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					
					break;
				case GM3_SET_SETMODE:
					if(GM3_send_cmd("AT+WKMOD=\"NET\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_OPENSOCKETA;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_OPENSOCKETA:
					if(GM3_send_cmd("AT+SOCKAEN=\"on\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SETDEST;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_SETDEST:
					if(GM3_send_cmd("AT+SOCKA=\"TCP\",\"data.mingjitech.com\",15000","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_SETLINKTYPE;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_SETLINKTYPE:
					if(GM3_send_cmd("AT+SOCKASL=\"long\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_CLOSESOCKETB;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CLOSESOCKETB:
					if(GM3_send_cmd("AT+SOCKBEN=\"off\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_OPENHEART;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_OPENHEART:
					if(GM3_send_cmd("AT+HEARTEN=\"on\"","OK",30,1) == 0)
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
					if(GM3_send_cmd("AT+HEARTDT=\"4F4B\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_HEARTTP;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_HEARTTP:
					if(GM3_send_cmd("AT+HEARTTP=\"NET\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_REGEN;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_REGEN:
					if(GM3_send_cmd("AT+REGEN=\"off\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_REGSND;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_REGSND:
					if(GM3_send_cmd("AT+REGSND=\"link\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_REGTP;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_REGTP:
					if(GM3_send_cmd("AT+REGTP=\"IMEI\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_APN;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_APN:
					if(GM3_send_cmd("AT+APN=\"CMNET\",\"\",\"\"","OK",30,1) == 0)
					{
						GprsParaSetState = GM3_SET_CLOUDEN;
					}
					else
					{
						GprsParaSetState = GM3_SET_READY;
					}
					break;
				case GM3_SET_CLOUDEN:
					if(GM3_send_cmd("AT+CLOUDEN=\"on\"","OK",30,1) == 0)
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
						GprsParaSetState = GM3_SET_CLOUDPA;
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
			//����
			if((ModbusPara.TxRxSwitch == 0) && (COMGetBuf(COM3 ,&NETCircle, 256) > 4)&&(ModbusPara.Busy == 0))
			{
				ModbusPara.Busy = 1;
				ModbusPara.UnpackError = UpackPlatform(NETCircle.buf);
				ClearNetSendBuf();
			}
			//����
			if(ModbusPara.TxRxSwitch == 1)
			{
				if(ModbusPara.UnpackError != 0)
				{
					//�����֡����д�쳣��
					SendNum = FrameError(NETCircle.buf,ModbusPara.UnpackError,NETSendBuf);
				}
				else
				{
					//���ݹ�������֡
					SendNum = FrameNormal(NETCircle.buf,NETSendBuf);
				}
				RTSwitach(RXFLAG);
				ClearNetCircle();
				comSendBuf(COM3,NETSendBuf,SendNum);
				ModbusPara.Busy = 0;
				delay_ms(500);
			}
			else if((ModbusPara.Busy == 0) && (ModbusPara.Initiative == 1))
			{
				//�������ͺ���
				SendNum = FrameInitactive(InitactivePara);
				ClearIniactivePara();
				comSendBuf(COM3,NETSendBuf2,SendNum);
				ModbusPara.Initiative = 0;
				delay_ms(500);
			}
		}
		Gm3Taskout++;
	}
}
