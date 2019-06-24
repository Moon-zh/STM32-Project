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

#define IDLECOUNTNUM 3000 //5分钟 3000/60/10 = 5

typedef enum _AGVSM_LCDPageNumber
{
	LCD_STANDBY_PAGE = 0,       //0 待机界面
	LCD_START_PAGE = 1,			//1 开始界面
	LCD_ADDFER_PAGE = 2,		//2 加肥界面
	LCD_ADDWATER_PAGE = 3,		//3 加水界面
	//非控制界面--联系我们
	LCD_STIRE_PAGE = 4, 		//4 搅拌界面
	//非控制界面--保存成功界面
	LCD_SET_PAGE = 5,			//5设置界面
	LCD_SAVE_PAGE = 6,			//6保存界面
	LCD_WIFISET_PAGE = 8,		//WIFI设置
	LCD_WIFISAVE_PAGE = 9,		//保存界面
} AGVSM_LCDPageNumber;
typedef struct
{
	u8 PumpWFlag;	//水泵是否开启
	u8 PumpFFlag;	//肥泵是否开启
	u8 Passageway[6]; //通道阀位控制
	u32 Zone;		//分区阀控制
	u8 WorkHour;	//工作时长(h)
	u8 WorkMinute;	//工作时长(m)
	u8 WorkDay;		//工作周几
	u8 StartHour;	//开始小时
	u8 StartMinute;	//开始分钟
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
