#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"

typedef struct
{
	u8 Enable;
	u8 CtrlType;
}ZoneCommand;//分区IO命令

typedef struct
{
	u8 IdleSta;
	u8 RWSta;
	u8 Error;
}IOCtrlPara;//IO控制参数

//分区命令类型
#define XREAD	0x01
#define YREAD	0x02
#define YSETSINGLE	0x03
#define YCLEAR	0x04
#define YSETMUL	0x05 

#define  DEV1READ 0x01 //温湿度\噪声\PM2.5\PM10\光照强度读取
#define  DEV2READ 0x02 //大气压\CO2读取
#define  DEV3READ 0X03	//风速
#define  DEV4READ 0X04	//风向
#define  DEV5READ 0X05	//雨量
#define  DEV6READ 0X06	//雨雪
#define  DEV5WRITE 0X07	//清除雨量

#ifndef ev
#define ev
extern u16 pm25,co2,temp,vol,humi,pm10,bmp,windspeed,winddir,rain,snow;
extern u32 light;
#endif

//io控制参数读写状态
#define READSTA 0x01
#define WRITESTA 0x02
extern const u8 auchCRCHi[];
extern const u8 auchCRCLo[];
extern 	u8 RainFlag ;
extern u8 RainfallClearF;
void ZoneCtrl_task(void *pdata);
void SetIOstate(u8 Num);
u16 CRC16(u8* puchMsg, u16 usDataLen);
u8 ZoneCtrl_Set(u8 Type);

#endif


