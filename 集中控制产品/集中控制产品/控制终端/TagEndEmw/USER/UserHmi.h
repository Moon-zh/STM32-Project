#ifndef USER_H
#define USER_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"
#include "cmd_queue.h"

#define RED_COLOR_HMI       0xf800
#define BLUE_COLO_HMI      0x001f
#define GREEN_COLOR_HMI     0x07c0
#define GRAY_COLOR_HMI    	0xC618
#define YELLOW_COLOR_HMI    0xffe0
#define WHITE_COLOR_HMI     0xffff

#define WARINGPAGE_EMERGENCY 0x00
#define WARINGPAGE_CHARGE	0x01
#define WARINGPAGE_LOCATIONDEFEAT 0x02
#define WARINGPAGE_CUERROR		 0x03


typedef enum _AGVSM_LCDPageNumber
{
	LCD_STANDBY_PAGE = 0,			//0 ��������
	LCD_START_PAGE = 1,				//1 ��������
	LCD_IRRSTEP1_PAGE = 2,			//2 ��Ȳ�����һ��
	LCD_WATERSTEP2_PAGE = 3, 		//3 ��ˮ�����ڶ���
	LCD_FERSTEP2_PAGE = 4,			//4	��ˮ�����ڶ���
	LCD_WATERSTEP3_PAGE = 5,		//5 ��ˮ����������
	LCD_FERSTEP3_PAGE = 6,			//6 ��ˮ����������
	LCD_FERSTEP4_PAGE = 7,			//7 ��ˮ�������Ĳ�
	LCD_WATERSHOW_PAGE = 8,			//8 ��ˮ��ʾ
	LCD_FERSHOW_PAGE = 9, 			//9 ��ˮ��ʾ
	LCD_SETSTEP1_PAGE = 10,			//10 ���õ�һ��
	LCD_SETSTEP2_PAGE = 11,			//11 ���õڶ���
	LCD_SETSTEP3_PAGE = 12,			//12 ���õ�����
	LCD_SETSTEP4_PAGE = 13,			//13 ���õ��Ĳ�
	LCD_SETSTEP5_PAGE = 14,			//14 ���õ��岽
	LCD_SETSTEP6_PAGE = 15,			//15 ���õ�����
	LCD_SETSTEP7_PAGE = 16, 		//16 ���õ��߲�
	LCD_SAVE1_PAGE = 17, 			//17 Ũ�ȱ���
	LCD_SAVE2_PAGE = 18, 			//18 �����������
	LCD_SAVE3_PAGE = 19, 			//19 ���Ϸʱ���
	LCD_SAVE4_PAGE = 20, 			//20 һ�׶α���
	LCD_SAVE5_PAGE = 21, 			//21 ���׶α���
	LCD_SAVE6_PAGE = 22, 			//22 ���׶α���
	LCD_SAVE7_PAGE = 23 			//23 �Ľ׶α���
	
} AGVSM_LCDPageNumber;


typedef union
{
	u16 WaterRecord[60];
	u8 ElseRecord[120];
}HmiRecord;

typedef union
{
	u16 WaterTime;
	u16	IrrPjNum;	//��1��ʼ
}IrrPara;

typedef struct
{
	u32 WaterPartition;
	u32 FerPartition;
	IrrPara Para;
}WorkPara;


extern char g_u8SysTemPassword[9];

void Task_HMIMonitor ( void * parg );
void NumberToASCII ( u8 *Arry , u16 Number);
void DisCountDown(u16 RemainderMinute);
void MainPageDisSta(u8 State);
void WaterStaShow(u8 State);
void FertilizerStaShow(u8 State);
void SetRadiotube(u8 Num, u8 Value);
void SetSwitchAlarm(void);
void Refresh_ACTION_Page(void);
void Refresh_Main_Page(void);
void PasswordSetShow(void);
void BasicInfoShow(void);
void GreenHouseInfoShow(void);
void IrrProjectShow(void);
void AmountSetShow(void);
void DosingSetShow(void);
void WaterSetShow(void);
//void WorkStateShow(void);
void IrrDetailShow(void);
void SerialNumShow(u8 PageNum,u8 Addr,u8 Num,u8 StarNum);
void IrrProjectNumShow(void);
void IrrPartitonNumShow(void);
void FlashReadWiFi(u8* _pBuf);
void FlashWriteWiFi(u8* _pBuf);
#endif

