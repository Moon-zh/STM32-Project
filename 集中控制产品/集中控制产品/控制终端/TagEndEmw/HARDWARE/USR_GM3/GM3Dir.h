#ifndef __GM3DIR_H
#define __GM3DIR_H	
#include "sys.h"
#include "UserCore.h"


//设置GM3参数状态
#define GM3_SET_READY		0
#define GM3_SET_CHANGEMODE1	1
#define GM3_SET_CHANGEMODE2	2
#define GM3_SET_E0			3
#define GM3_SET_SETMODE		4
#define GM3_SET_OPENSOCKETA	5
#define GM3_SET_SETDEST		6
#define GM3_SET_SETLINKTYPE	7
#define GM3_SET_CLOSESOCKETB 8
#define GM3_SET_OPENHEART	9
#define GM3_SET_HEARTTIME	10
#define GM3_SET_HEARTDATA	11
#define GM3_SET_HEARTTP		12
#define GM3_SET_REGEN		13
#define GM3_SET_REGSND		14
#define GM3_SET_REGTP		15
#define GM3_SET_APN			16
#define GM3_SET_CLOUDEN		17
#define GM3_SET_CLOUDID		18
#define GM3_SET_CLOUDPA		19
#define GM3_SET_SAVE		20

//MODEBUS功能码
#define RD_COIL_STA		01	//读线圈
#define	RD_INPUT_STA	02	//读输入离散量
#define RD_HOLDING_REG	03	//读寄存器
#define	RD_INPUT_REG	04	//读输入寄存器
#define	WR_SINGLE_COIL	05	//写单个线圈
#define	WR_SINGLE_REG	06	//写单个寄存器
#define WR_MUL_COIL		15	//写多个线圈
#define	WR_MUL_REG		16	//写多个寄存器
#define INITIATIVE_COIL_STA	0X45//主动上报线圈
#define INITIATIVE_INPUT_STA 0X42//主动上报输入离散量
#define INITIATIVE_HOLDING_REG 0x46 //主动上报保持寄存器
#define INITIATIVE_INPUT_REG 0X44 //主动上报输入寄存器
//数据量的最大值定义
#define MAX_COIL_NUM  	88//必须为8的整数倍
#define MAX_INSTA_NUM 	48	//MAX_INSTA_NUM /8不能大于 256-9  帧长度不能大于256 必须为8的整数倍
#define MAX_REG_NUM		300
#define MAX_INREG_NUM	20

//modebus接收或者发送标志
#define RXFLAG	0
#define TXFLAG	1


typedef struct
{
	u8 buf[256];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;


typedef struct
{
	u8 Initiative;
	u8 Busy;
	u8 TxRxSwitch;
	u8 UnpackError;
	u8 DeviceID; 
}ModbusStruct;
typedef struct
{
	u16 DataAddr;// 目前未超过256所以在队列消息中使用的是u8类型
	u8 DataNum;//
	u8 DataType;
}InitativeStruct;

typedef struct
{
u8 fertilizer_Solenoid_valve; //上电默认电磁阀需要都关闭
/*对应吸肥电磁阀
bit0  吸肥电磁阀1  0代表这个电磁阀要关闭 1代表要开启 开启顺序 1-->5
bit1  吸肥电磁阀2
bit2  吸肥电磁阀3
bit3  吸肥电磁阀4
bit4  吸肥电磁阀5

*/
u8 water_Solenoid_valve;//灌水电磁阀 0关闭 1开启
u8 water_select;//棚分区选择
u16 fertilizer_Solenoid1_time; //肥泵1开启时间
u16 fertilizer_Solenoid2_time; //肥泵2开启时间
u16 fertilizer_Solenoid3_time; //肥泵3开启时间
u16 fertilizer_Solenoid4_time; //肥泵4开启时间
u16 fertilizer_Solenoid5_time; //肥泵5开启时间
	
u16 water_Solenoid_time;      //水泵开启时间


}GM3Struct;



extern ModbusStruct ModbusPara;
extern u8 ModbusCoil[MAX_COIL_NUM/8];//modbus线圈，共40个
extern u8 ModbusInSta[MAX_INSTA_NUM/8];//modbus输入离散变量










void SetMDCoil(u8 CoilNum, u8 Flag);
void SetInSta(u8 StaNum, u8 Flag);
void SetIniactivePara(InitativeStruct Para);
u8 PackSendRemoteQ(MsgStruct* MsgBlk);
void SetMulReg(u8* Buf);
void SetMulCoil(u8 *Buf);
u8 FrameRdCoil(u8* RxBuf, u8* TxBuf, u8 Flag);
u8 FrameRdInSta(u8* RxBuf, u8* TxBuf, u8 Flag);
u8 FrameRdReg(u8* RxBuf, u8* TxBuf);
u8 FrameRdInReg(u8* RxBuf, u8* TxBuf,u8 flag );
u8 FrameWrCoil(u8* RxBuf, u8* TxBuf);
u8 FrameWrReg(u8* RxBuf, u8* TxBuf);
void SetSingleCoil(u8 *Buf);
void SetSingleReg(u8* Buf);



#endif

