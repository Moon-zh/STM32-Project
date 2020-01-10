#ifndef __Emw3060_H
#define __Emw3060_H
#include "UserCore.h"
#include "string.h"    
#include "Includes.h"



typedef struct
{
	u8 buf[512];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;
#define ParaNum 4


//��Ȩ��Ϣ
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;



//json������  topic�ϱ�Ŀ¼
static char 	*topicw=	"thing/event/property/post\"";
static char      *postw="\\\"params\\\":{";

static char  *wartopicw=	"user/war\"";
static char   *warpostw="\\\"params\\\":{";


//wifi
extern char ssid[30];  //wifi����
extern char password[30]; //wifi����
extern char ipword[30];//IP��ַ
extern char zwword[30];//��������
extern char wgword[30];//���ص�ַ
extern char ymword[30];//����


extern u8 MasterState;//������״̬����
extern u8 fsbz_3060;//ÿһ�ν���������ֹ֮ͣ�󣬶����ϴ�һ�����ڵ�����
extern u8 fsbz_3060_bf;//�������ݱ����ı��ݣ��ڿ���֮������ֹͣ��ʱ���ʹ��

void	Emw3060_LED_Init(void);				//EMWLED��ʼ��
void	Emw3060_init(void);//wifiEMW��ʼ��
void	Emw3060_con(void);//EMW���Ӱ�����
unsigned char sendEmw(char *data,unsigned char w);////EMW�ϱ�����,���ݷ��͵���ƽ̨
void SetRemoteRead(u8 addr,s16 value);
void SetRemotePara(u8 addr,s16 value);
u8 PackSendRemoteQ(MsgStruct* MsgBlk);//��Ϣ���У�//��Ϣ�������

/*������-�����ϴ����ݳ���*/
void fsbz_3060_cx(void);


#define EmwLED1 PAout(11) //wifiģ�����������LED��
#define EmwLED2 PAout(12)

#endif
