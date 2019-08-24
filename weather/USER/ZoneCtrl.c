#include"ZoneCtrl.h"
#include"Rs485.h"
#include"includes.h"
#include "delay.h"
#include "UserCore.h"
#include "GM3Dir.h"
#include "LED_dz.h"
Environmental sensor;

ZoneCommand ZoneCtrl;
IOCtrlPara ZonePara;
u8 ZoneSendBuf[50];
u32 IOState;
u8 DevID = 0;//u8 ClearRainFlag ;
/*16位CRC校验表*/
const u8 auchCRCHi[]={
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,

0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40
};
const u8 auchCRCLo[]={
0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,0x07,0xc7,0x05,0xc5,0xc4,0x04,
0xcc,0x0c,0x0d,0xcd,0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,0x08,0xc8,
0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,
0x14,0xd4,0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,0x11,0xd1,0xd0,0x10,
0xf0,0x30,0x31,0xf1,0x33,0xf3,0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,0x3b,0xfb,0x39,0xf9,0xf8,0x38,
0x28,0xe8,0xe9,0x29,0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,0xec,0x2c,
0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,

0xa0,0x60,0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,0xa5,0x65,0x64,0xa4,
0x6c,0xac,0xad,0x6d,0xaf,0x6f,0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,
0xb4,0x74,0x75,0xb5,0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,0x70,0xb0,
0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,
0x9c,0x5c,0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,0x99,0x59,0x58,0x98,
0x88,0x48,0x49,0x89,0x4b,0x8b,0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,0x40
};

u16 CRC16(u8* puchMsg, u16 usDataLen)
{
	u8 uchCRCHi=0xff;
	u8 uchCRCLo=0xff;
	u8 uIndex;
	
	while(usDataLen--)
	{
		uIndex=uchCRCHi^*(puchMsg++);
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo=auchCRCLo[uIndex];
	}
	return uchCRCHi<<8|uchCRCLo;
}

//清除发送buf
void ZoneBufClear(void)
{
	u8 Cnt = 0;
	for(Cnt=0;Cnt<sizeof(ZoneSendBuf);Cnt++)
	{
		ZoneSendBuf[Cnt++] = 0;
	}
}
void SetIOstate(u8 Num)
{
		IOState = IOState|(1<<(Num-1));
}
//初始化ZoneCtrl
void ZoneCtrl_Init(void)
{
	ZoneCtrl.Enable = 0;
	ZoneCtrl.CtrlType = 0;
}
//分区电磁阀的设置
u8 ZoneCtrl_Set(u8 Type)
{
	if(ZoneCtrl.Enable == 1)
	{
		return 0;
	}
	ZoneCtrl.Enable = 1;
	ZoneCtrl.CtrlType = Type;
	return 1;
}
//初始化ZonePara
void ZonePara_Init(void)
{
	ZonePara.IdleSta = 0;
	ZonePara.RWSta = 0;
	ZonePara.Error = 0;
	
}
//读取模块输入
u8 XRead_Handl(u8*Buf)
{
	Buf[1] = 0x02;
	Buf[2] = 0x00;
	Buf[3] = 0x20;
	Buf[4] = 0x00;
	Buf[5] = 0x04;
	return 6;
}

//读取百叶箱信息
u8 ReadBlinds(u8* Buf, u8 DevAddr)
{
	Buf[0] = DevAddr;
	Buf[1] = 0x03;
	Buf[2] = 0x00;
	Buf[3] = 0x00;
	Buf[4] = 0x00;
	Buf[5] = 0x0d;
	return 6;
	
}
//解析百叶箱信息
u8 UnframeBlinds(u8*ReBuf)
{	
	if(ReBuf[0] == 0x01)
	{
		if(ReBuf[1] == 0x03)
		{
			sensor.humi=ModebusInReg[0] = (ReBuf[3]<<8)|ReBuf[4];
			if(sensor.humi>990)sensor.humi=990;
			sensor.temp=ModebusInReg[1] = (ReBuf[5]<<8)|ReBuf[6];
			sensor.pm25=(ModebusInReg[2] = (ReBuf[11]<<8)|ReBuf[12]);
			sensor.light=(ModebusInReg[3] = (ReBuf[17]<<8)|ReBuf[18])<<16;
			sensor.light|=ModebusInReg[4] = (ReBuf[19]<<8)|ReBuf[20];
			sensor.pm10=(ModebusInReg[5] = (ReBuf[21]<<8)|ReBuf[22]);
			sensor.vol=ModebusInReg[6] = (ReBuf[27]<<8)|ReBuf[28];
			
			return 1;
		}
	}
	else if(ReBuf[0] == 0x02)
	{
		if(ReBuf[1] == 0x03)
		{
			sensor.bmp=(ModebusInReg[7] = (ReBuf[23]<<8)|ReBuf[24])<<16;
			sensor.bmp|=ModebusInReg[8] = (ReBuf[25]<<8)|ReBuf[26];
			sensor.co2=ModebusInReg[9] = (ReBuf[13]<<8)|ReBuf[14];
			return 1;
		}
	}
	return 0;
}
//读取风速
u8 ReadAnemometer(u8* Buf, u8 DevAddr)
{
	Buf[0] = DevAddr;
	Buf[1] = 0x03;
	Buf[2] = 0x00;
	Buf[3] = 0x16;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	return 6;
}
//解析风速
u8 UnframeAnemometer(u8 * ReBuf)
{
	if(ReBuf[1] == 0x03)
	{
		sensor.windspeed=ModebusInReg[10] = (ReBuf[3]<<8)|ReBuf[4];
		return 1;
	}
	return 0;
}
//读风向
u8 ReadVane(u8* Buf, u8 DevAddr)
{
	Buf[0] = DevAddr;
	Buf[1] = 0x03;
	Buf[2] = 0x00;
	Buf[3] = 0x17;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	return 6;
}
//解析风速
u8 UnframeVane(u8 * ReBuf)
{
	if(ReBuf[1] == 0x03)
	{
		ModebusInReg[11] = (ReBuf[3]<<8)|ReBuf[4];
//		if(((ModebusInReg[11]>=348)&&(ModebusInReg[11]<=360))
//			&&((ModebusInReg[11]<=11)))
//		{
//			ModebusInReg[11] = 1;//南
//		}
//		else if((ModebusInReg[11]>11)&&(ModebusInReg[11]<=33))
//		{
//			ModebusInReg[11] = 2;//西南偏南
//		}
//		else if((ModebusInReg[11]>33)&&(ModebusInReg[11]<=56))
//		{
//			ModebusInReg[11] = 3;//西南
//		}
//		else if((ModebusInReg[11]>56)&&(ModebusInReg[11]<=78))
//		{
//			ModebusInReg[11] = 4;//西南偏西
//		}
//		else if((ModebusInReg[11]>78)&&(ModebusInReg[11]<=101))
//		{
//			ModebusInReg[11] = 5;//西
//		}
//		else if((ModebusInReg[11]>101)&&(ModebusInReg[11]<=123))
//		{
//			ModebusInReg[11] = 6;//西北偏西
//		}
//		else if((ModebusInReg[11]>123)&&(ModebusInReg[11]<=146))
//		{
//			ModebusInReg[11] = 7;//西北
//		}
//		else if((ModebusInReg[11]>146)&&(ModebusInReg[11]<=168))
//		{
//			ModebusInReg[11] = 8;//西北偏北
//		}
//		else if((ModebusInReg[11]>168)&&(ModebusInReg[11]<=191))
//		{
//			ModebusInReg[11] = 9;//北
//		}
//		else if((ModebusInReg[11]>191)&&(ModebusInReg[11]<=213))
//		{
//			ModebusInReg[11] = 10;//东北偏北
//		}
//		else if((ModebusInReg[11]>213)&&(ModebusInReg[11]<=236))
//		{
//			ModebusInReg[11] = 11;//东北
//		}
//		else if((ModebusInReg[11]>236)&&(ModebusInReg[11]<=258))
//		{
//			ModebusInReg[11] = 12;//东北偏东
//		}
//		else if((ModebusInReg[11]>258)&&(ModebusInReg[11]<=281))
//		{
//			ModebusInReg[11] = 13;//东
//		}
//		else if((ModebusInReg[11]>281)&&(ModebusInReg[11]<=303))
//		{
//			ModebusInReg[11] = 14;//东南偏南
//		}
//		else if((ModebusInReg[11]>303)&&(ModebusInReg[11]<=326))
//		{
//			ModebusInReg[11] = 15;//东南
//		}
//		else if((ModebusInReg[11]>326)&&(ModebusInReg[11]<=348))
//		{
//			ModebusInReg[11] = 16;//东南偏南
//		}
		sensor.winddir=ModebusInReg[11];
		return 1;
	}
	return 0;
}
//读取雨量
u8 ReadRainfall(u8* Buf, u8 DevAddr)
{
	Buf[0] = DevAddr;
	Buf[1] = 0x03;
	Buf[2] = 0x01;
	Buf[3] = 0x05;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	return 6;
}
//解析雨量
u8 UnframeRainfall(u8 * ReBuf)
{
	if(ReBuf[1] == 0x03)
	{
		sensor.rain=ModebusInReg[12] = (ReBuf[3]<<8)|ReBuf[4];
		return 1;
	}
	else if(ReBuf[1] == 0x10)
	{
		return 1;
	}
	return 0;
}
//初始化雨量
u8 SetRainfall(u8* Buf, u8 DevAddr)
{
	Buf[0] = DevAddr;
	Buf[1] = 0x10;
	Buf[2] = 0x00;
	Buf[3] = 0x00;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	Buf[6] = 0x02;
	Buf[7] = 0x00;
	Buf[8] = 0x00;
	return 9;
}

//读取雨
u8 ReadRain(u8* Buf, u8 DevAddr)
{
	Buf[0] = DevAddr;
	Buf[1] = 0x03;
	Buf[2] = 0x00;
	Buf[3] = 0x010;
	Buf[4] = 0x00;
	Buf[5] = 0x01;
	return 6;
}
u8 RainFlag = 0;
u8 UnframeRain(u8 * ReBuf)
{
	if(ReBuf[1] == 0x03)
	{
		sensor.snow=ModebusInReg[13] = (ReBuf[3]<<8)|ReBuf[4];
		RainFlag = ModebusInReg[13];
		return 1;
	}
	return 0;
}

void Frame_Send(u8 CmdType ,u8*Buf)
{
	u8 Cnt = 0;
	u16 Crc= 0;
	OS_CPU_SR  cpu_sr;
	
	//判断该控制哪个io模块//适用于分区电磁阀
	//根据命令类型填充所要发送指令
	switch(CmdType)
	{
		case DEV1READ:
			Cnt = ReadBlinds(Buf, 1);
			DevID = 1;
			break;
		case DEV2READ:
			Cnt = ReadBlinds(Buf, 2);
			DevID = 2;
			break;
		case DEV3READ:
			Cnt = ReadAnemometer(Buf, 3);
			DevID = 3;
			break;
		case DEV4READ:
			Cnt = ReadVane(Buf, 4);
			DevID = 4;
			break;
		case DEV5READ:
			Cnt = ReadRainfall(Buf, 5);
			DevID = 5;
			break;
		case DEV6READ:
			Cnt = ReadRain(Buf, 6);
			DevID = 6;
			break;
		case DEV5WRITE:
			Cnt = SetRainfall(Buf, 5);
			DevID = 5;
			break;
		default:
			break;
	}
	
	OS_ENTER_CRITICAL();
	Crc = CRC16(Buf , Cnt);
	OS_EXIT_CRITICAL();
	Buf[Cnt++] = (u8)((Crc>>8)&0xff);
	Buf[Cnt++] = (u8)(Crc&0xff);
	comSendBuf(COM4,Buf,Cnt);
	ZonePara.RWSta = READSTA;//已经发送完毕，转换为读取状态
	
}
//解析输出状态
u8 Unfreame_YSta(u8* ReceiveBuf)
{
	if(ReceiveBuf[3] > 0x0f )
	{
		return 0;
	}
	IOState = (ReceiveBuf[3]<<((ReceiveBuf[0]-2)*4)) 
				& (IOState|(0x0F<<((ReceiveBuf[0]-2)*4)));
	return 1;
}
//解析收到的报文
u8 Unfreame_Recive(u8* Buf)
{
	OS_CPU_SR  cpu_sr;
	u8 ReBuf[50] = {0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0};
	u16 Crc;
	u8 ReValue = 0;
	u8 BufLen = 0;
	//是否收到数据判断
	BufLen = COM4GetBuf(ReBuf,100);
	if(BufLen <= 5)
	{
		return 0;
	}

	//地址判断
	if(ReBuf[0] != DevID)
	{
		return 0;
	}
	//CRC判断
	Crc = ReBuf[BufLen - 2]<<8|ReBuf[BufLen - 1];
	
	OS_ENTER_CRITICAL();
	if(Crc != CRC16(ReBuf, BufLen - 2))
	{
		OS_EXIT_CRITICAL();
		return 0;
	}
	OS_EXIT_CRITICAL();
	//分析报文
	switch(ReBuf[0])
	{
		case 0x01:
		case 0x02:
			ReValue = UnframeBlinds(ReBuf);
			break;
		case 0x03:
			ReValue = UnframeAnemometer(ReBuf);
			break;
		case 0x04:
			ReValue = UnframeVane(ReBuf);
			break;
		case 0x05:
			ReValue = UnframeRainfall(ReBuf);
			break;
		case 0x06 :
			ReValue = UnframeRain(ReBuf);
			break;
		default:
			break;
	}
	return ReValue;	
}
void CmdComletion(u8 MsgType,u8 Addr, u8 Flag)
{
	OS_CPU_SR  cpu_sr;
	
	MsgStruct Msgtemp;
	Msgtemp.CmdType = MsgType;
	Msgtemp.CmdSrc = ZONE_TASK_CODE;
	Msgtemp.CmdData[0] = Addr;
	Msgtemp.CmdData[1] = Flag;
	OS_ENTER_CRITICAL();
	PackSendMasterQ(&Msgtemp);
	OS_EXIT_CRITICAL();
}
u8 RainfallClearF = 0;
//分区控制任务
// alfred
// 添加局部变量更新消息判断数据读取与控制
u8 alfredZoneCnt[5];
u8 ZoneTaskin = 0;
u8 ZoneTaskout = 0;
void ZoneCtrl_task(void *pdata)
{
	ZoneCtrl_Init();
	ZonePara_Init();
	ZoneBufClear();
	while(1)
	{
		delay_ms(100);//需要测试多久合适 alfred to be continue
		ZoneTaskin++;
		//分区空闲
		alfredZoneCnt[0]++;
		if(ZonePara.IdleSta == 0)
		{
			if(ZoneCtrl.Enable == 1)
			{
				ZonePara.RWSta = WRITESTA;
				ZonePara.IdleSta = 1;
			}
			else 
			{
				if(RainfallClearF == 1)
				{
					RainfallClearF = 0;
					ZoneCtrl_Set(7);
				}
				else
				{
					if(DevID < 6)
						ZoneCtrl_Set(DevID+1);
					else
						ZoneCtrl_Set(1);
						
				}
			}
		}
		else if(ZonePara.IdleSta == 1)//分区忙
		{
			switch(ZonePara.RWSta)
			{
				case READSTA:
					if(Unfreame_Recive(ZoneSendBuf))//解析收到的报文
					{
						//CmdComletion(MSG_ZONEALARM,ZoneSendBuf[0],0);
						ZoneBufClear();
						ZoneCtrl_Init();
						ZonePara_Init();
					}
					else 
					{
						if(ZonePara.Error++ >=  5)
						{
							if(ZonePara.Error > 10)//重发一次收不到数据认为模块掉线
							{
								//CmdComletion(MSG_ZONEALARM,ZoneSendBuf[0],1);
								ZoneBufClear();
								ZoneCtrl_Init();
								ZonePara_Init();
							}
							else if(ZonePara.Error == 5)
							{
								ZonePara.RWSta = 0x02;//5次收不到数据重发一次
							}
						}
					}
					break;
				case WRITESTA:
					Frame_Send(ZoneCtrl.CtrlType, ZoneSendBuf);//组帧发送
					alfredZoneCnt[1]++;
					break;
				default://收到错误数据初始化所有
					ZoneBufClear();
					ZoneCtrl_Init();
					ZonePara_Init();
					break;
			}
		}
		ZoneTaskout++;
	}
}
