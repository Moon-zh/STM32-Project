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
	LCD_STANDBY_PAGE = 0,			//0 待机界面
	LCD_START_PAGE = 1,				//1 启动界面
	LCD_IRRSTEP1_PAGE = 2,			//2 灌溉操作第一步
	LCD_WATERSTEP2_PAGE = 3, 		//3 清水操作第二步
	LCD_FERSTEP2_PAGE = 4,			//4	肥水操作第二步
	LCD_WATERSTEP3_PAGE = 5,		//5 清水操作第三步
	LCD_FERSTEP3_PAGE = 6,			//6 肥水操作第三步
	LCD_FERSTEP4_PAGE = 7,			//7 肥水操作第四步
	LCD_WATERSHOW_PAGE = 8,			//8 清水显示
	LCD_FERSHOW_PAGE = 9, 			//9 肥水显示
	LCD_SETSTEP1_PAGE = 10,			//10 设置第一步
	LCD_SETSTEP2_PAGE = 11,			//11 设置第二步
	LCD_SETSTEP3_PAGE = 12,			//12 设置第三步
	LCD_SETSTEP4_PAGE = 13,			//13 设置第四步
	LCD_SETSTEP5_PAGE = 14,			//14 设置第五步
	LCD_SETSTEP6_PAGE = 15,			//15 设置第六步
	LCD_SETSTEP7_PAGE = 16, 		//16 设置第七步
	LCD_SAVE1_PAGE = 17, 			//17 浓度保存
	LCD_SAVE2_PAGE = 18, 			//18 分区面积保存
	LCD_SAVE3_PAGE = 19, 			//19 复合肥保存
	LCD_SAVE4_PAGE = 20, 			//20 一阶段保存
	LCD_SAVE5_PAGE = 21, 			//21 二阶段保存
	LCD_SAVE6_PAGE = 22, 			//22 三阶段保存
	LCD_SAVE7_PAGE = 23 			//23 四阶段保存
	
} AGVSM_LCDPageNumber;


typedef union
{
	u16 WaterRecord[60];
	u8 ElseRecord[120];
}HmiRecord;

typedef union
{
	u16 WaterTime;
	u16	IrrPjNum;	//从1开始
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

