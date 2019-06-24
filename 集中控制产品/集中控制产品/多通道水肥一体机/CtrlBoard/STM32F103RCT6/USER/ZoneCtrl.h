#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"

typedef struct
{
	u8 Enable;
	u8 CtrlNum;
	u8 CtrlType;
}ZoneCommand;//分区IO命令

typedef struct
{
	u8 IdleSta;
	u8 RWSta;
	u8 Error;
}IOCtrlPara;//IO控制参数
typedef struct
{
	u8 IOid;   // ioID 485id地址
	u8 IOread_write;//io读写 0读 1写
	u8 IORegister; //io寄存器地址
	u8 ioaction;  //ff 对应寄存器置1 00 对应寄存器置0
	u8 IOdata;		//io读取时对应的Y地址 Y1 0X01 Y2 0X02 Y3 0X04 Y4 0X08 Y5 0X10 Y6 0X20 Y7 0X40 Y8 0X80
}IOPara;//I参数

//分区命令类型
#define XREAD	0x01
#define YREAD	0x02
#define YSETSINGLE	0x03
#define YCLEAR	0x04
#define YSETMUL	0x05 

//io控制参数读写状态
#define READSTA 0x01
#define WRITESTA 0x02
extern const u8 auchCRCHi[];
extern const u8 auchCRCLo[];
void ZoneCtrl_task(void *pdata);
void IO_READ_Input_Data(u8 id);
void IO_RESETALL_DATA(u8 id);
void IO_SET_DATA(u8 id,u8 dataadress,u8 action);
//void SetIOstate(u8 Num);
u16 CRC16(u8* puchMsg, u16 usDataLen);
//u8 ZoneCtrl_Set(u8 Type,u8 Num);

#endif


