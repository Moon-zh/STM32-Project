#ifndef USER_H
#define USER_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"
#include "cmd_queue.h"
//#include "cmd_process.h"

#define RED_COLOR_HMI       0xf800
#define BLUE_COLO_HMI      0x001f
#define GREEN_COLOR_HMI     0x07c0
#define YELLOW_COLOR_HMI    0xffe0
#define WHITE_COLOR_HMI     0xffff

#define WARINGPAGE_EMERGENCY 0x00
#define WARINGPAGE_CHARGE	0x01
#define WARINGPAGE_LOCATIONDEFEAT 0x02
#define WARINGPAGE_CUERROR		 0x03

#define IDLECOUNTNUM 3000 //5���� 3000/60/10 = 5

typedef enum _AGVSM_LCDPageNumber
{
	LCD_STANDBY_PAGE = 0,       //0 ��������
	LCD_START_PAGE = 1,			//1 ��ʼ����
	LCD_ADDFER_PAGE = 2,		//2 �ӷʽ���
	LCD_ADDWATER_PAGE = 3,		//3 ��ˮ����
	//�ǿ��ƽ���--��ϵ����
	LCD_STIRE_PAGE = 4, 		//4 �������
	//�ǿ��ƽ���--����ɹ�����
	LCD_SET_PAGE = 5,			//5���ý���
	LCD_SAVE_PAGE = 6,			//6�������
	LCD_WIFISET_PAGE = 8,		//WIFI����
	LCD_WIFISAVE_PAGE = 9,		//�������
} AGVSM_LCDPageNumber;
typedef struct
{
	u8 PumpWFlag;	//ˮ���Ƿ���
	u8 PumpFFlag;	//�ʱ��Ƿ���
	u8 Passageway[6]; //ͨ����λ����
	u32 Zone;		//����������
	u8 WorkHour;	//����ʱ��(h)
	u8 WorkMinute;	//����ʱ��(m)
	u8 WorkDay;		//�����ܼ�
	u8 StartHour;	//��ʼСʱ
	u8 StartMinute;	//��ʼ����
}ProjectStruct;
void Task_HMIMonitor ( void * parg );
void NumberToASCII ( u8 *Arry , u16 Number);
void DisCountDown(u16 RemainderMinute);
void MainPageDisSta(u8 State);
void WaterStaShow(u8 State);
void FertilizerStaShow(u8 State);
void SetRadiotube(u8 Num, u8 Value);
void RecoverWateradd(u8 num);
void RecoverStir(u8 num);
void SetSwitchAlarm(void);
void SetWateradd(u8 num );
void SetStir(u8 num);
void ParaShow(void);
extern u8  _60SecCnt[5];

#endif
