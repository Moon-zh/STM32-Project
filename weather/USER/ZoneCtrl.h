#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H	
#include"sys.h"

typedef struct
{
	u8 Enable;
	u8 CtrlType;
}ZoneCommand;//����IO����

typedef struct
{
	u8 IdleSta;
	u8 RWSta;
	u8 Error;
}IOCtrlPara;//IO���Ʋ���

//������������
#define XREAD	0x01
#define YREAD	0x02
#define YSETSINGLE	0x03
#define YCLEAR	0x04
#define YSETMUL	0x05 

#define  DEV1READ 0x01 //��ʪ��\����\PM2.5\PM10\����ǿ�ȶ�ȡ
#define  DEV2READ 0x02 //����ѹ\CO2��ȡ
#define  DEV3READ 0X03	//����
#define  DEV4READ 0X04	//����
#define  DEV5READ 0X05	//����
#define  DEV6READ 0X06	//��ѩ
#define  DEV5WRITE 0X07	//�������

#ifndef ev
#define ev
extern u16 pm25,co2,temp,vol,humi,pm10,bmp,windspeed,winddir,rain,snow;
extern u32 light;
#endif

//io���Ʋ�����д״̬
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


