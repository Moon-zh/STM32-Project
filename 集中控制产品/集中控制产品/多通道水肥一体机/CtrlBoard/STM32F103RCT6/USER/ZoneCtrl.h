#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"

typedef struct
{
	u8 Enable;
	u8 CtrlNum;
	u8 CtrlType;
}ZoneCommand;//����IO����

typedef struct
{
	u8 IdleSta;
	u8 RWSta;
	u8 Error;
}IOCtrlPara;//IO���Ʋ���
typedef struct
{
	u8 IOid;   // ioID 485id��ַ
	u8 IOread_write;//io��д 0�� 1д
	u8 IORegister; //io�Ĵ�����ַ
	u8 ioaction;  //ff ��Ӧ�Ĵ�����1 00 ��Ӧ�Ĵ�����0
	u8 IOdata;		//io��ȡʱ��Ӧ��Y��ַ Y1 0X01 Y2 0X02 Y3 0X04 Y4 0X08 Y5 0X10 Y6 0X20 Y7 0X40 Y8 0X80
}IOPara;//I����

//������������
#define XREAD	0x01
#define YREAD	0x02
#define YSETSINGLE	0x03
#define YCLEAR	0x04
#define YSETMUL	0x05 

//io���Ʋ�����д״̬
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


