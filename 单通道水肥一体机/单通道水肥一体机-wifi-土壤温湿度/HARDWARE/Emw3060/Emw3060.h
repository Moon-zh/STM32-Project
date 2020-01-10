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


//鉴权信息
extern char ProductKey0[20];
extern char DeviceName0[50];
extern char DeviceSecret0[50];

extern char *ProductKey;
extern char *DeviceName;
extern char *DeviceSecret;



//json包内容  topic上报目录
static char 	*topicw=	"thing/event/property/post\"";
static char      *postw="\\\"params\\\":{";

static char  *wartopicw=	"user/war\"";
static char   *warpostw="\\\"params\\\":{";


//wifi
extern char ssid[30];  //wifi名称
extern char password[30]; //wifi密码
extern char ipword[30];//IP地址
extern char zwword[30];//子网掩码
extern char wgword[30];//网关地址
extern char ymword[30];//域名


extern u8 MasterState;//主程序状态变量
extern u8 fsbz_3060;//每一次接收启动或停止之后，都先上传一次现在的数据
extern u8 fsbz_3060_bf;//发送数据变量的备份，在开启之后马上停止的时候才使用

void	Emw3060_LED_Init(void);				//EMWLED初始化
void	Emw3060_init(void);//wifiEMW初始化
void	Emw3060_con(void);//EMW连接阿里云
unsigned char sendEmw(char *data,unsigned char w);////EMW上报数据,数据发送到云平台
void SetRemoteRead(u8 addr,s16 value);
void SetRemotePara(u8 addr,s16 value);
u8 PackSendRemoteQ(MsgStruct* MsgBlk);//消息队列，//消息打包发走

/*主程序-调用上传数据程序*/
void fsbz_3060_cx(void);


#define EmwLED1 PAout(11) //wifi模组上面的两个LED灯
#define EmwLED2 PAout(12)

#endif
