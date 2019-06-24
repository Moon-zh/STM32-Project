#ifndef USER_H
#define USER_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"
#include "cmd_queue.h"

#define RED_COLOR_HMI       0xf800
#define BLUE_COLO_HMI      	0x001f
#define GREEN_COLOR_HMI     0x07c0
#define ORANGE_COLOR_HMI    0xfb20
#define GRAY_COLOR_HMI    	0xC618
#define YELLOW_COLOR_HMI    0xffe0
#define WHITE_COLOR_HMI     0xffff

#define WARINGPAGE_EMERGENCY 0x00
#define WARINGPAGE_CHARGE	0x01
#define WARINGPAGE_LOCATIONDEFEAT 0x02
#define WARINGPAGE_CUERROR		 0x03

extern u8 g_u8WiFiRestart;

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
	LCD_SAVE7_PAGE = 23, 			//23 四阶段保存
	LCD_USESELECT_PAGE = 24, 			//用户选择界面
	LCD_GUESTLOAD_PAGE = 25, 			//访客登录
	LCD_GUESTLOADERROR_PAGE = 26, 			//访客登录错误
	LCD_ADMINLOAD_PAGE = 27, 					//管理员登录
	LCD_ADMINLOADERROR_PAGE = 28, 			//管理员登录错误
	LCD_TASKRECORD_PAGE = 29, 			//任务记录
	LCD_GUESTVERIYFY_PAGE = 30, 			//访客密码校验
	LCD_NEWPASSWORD_PAGE = 31, 			//修改新密码
	LCD_PASSWORDSAVE_PAGE = 32, 					//密码保存成功
	LCD_UNCOMPLETWORK_PAGE = 33, 			//未完成任务提示	
	LCD_PASSWORDERROR_PAGE = 34, 					//密码错误
	LCD_NOTSAVE_PAGE = 35,			//未保存提示
	LCD_SETBUTONN_PAGE =36,    //设置压力按钮及分区
	LCD_ERROR_PAGE	=37,				//报警界面提示	
	LCD_WIFI_PAGE = 38,			//wifi配置界面
	LCD_WIFISAVE_PAGE =39		//wifi保存界面
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
	u16 starttime;//开始时间
	u16 lefttime;//剩余时间
	IrrPara Para;
	
}WorkPara;

typedef struct
{
	u8 task_num;
	u16 start_year; //任务开始年
	u8 start_month;	//任务开始月
	u8 start_day;	//任务开始月
	u8 start_hour;	//任务开始时
	u8 start_minitue;	//任务开始分
	u8 start_second;	//任务开始秒
  u8 irrigated_mode;//灌溉模式
  u16 irrigated_time;//灌溉时长
  u8 formula;     //配方名称
  u16 fertilizing_amount;//亩均施肥量
	u8 select_zone;//选择分区
  u8 stop_mode; //停止方式	

}__attribute__((packed))CtaskRecord;
typedef union
{
	CtaskRecord record;
	u8 task[16];
}__attribute__((packed))CtaskRecordunion; //任务存储读取结构体
//任务执行结构体
typedef struct
{
 
 u8 curtargetnum;//当前执行的任务号
 u8 lasttarget_id;//上一个任务id	
 u8 currenttarget_id;//当前任务ID
 u8 Ctrl_update_state; //需要更新控件的状态
 u8 targetnum[32];//最大可以存取32个任务
// u16 curtargetstarttime;//当前任务开启时间
 u8 curtargetyear; 
 u8 curtargetmon;
 u8 curtargetday;
 u8 curtargethour;
 u8 curtargetmin;
 u8 curtargetsec;
 u8 pausehour;	//暂停时的小时
 u8 pausemin;		//暂停时的分钟
 u8 pausesec;		//暂停时的秒数
 u16 pausetime; //暂停的时间
 u8 pause_flag;//暂停标志
 u16 tasktimeing;//进行时间
 u8 wateringprogram;//灌溉程序
 u8 fer_onoff;//水泵开启关闭状态
 u8 fer_chanle;//施肥阀通道
 u8 curtarget_mode;  //灌溉模式
 u16 curtarget_time;  //灌溉时长
 u8 curtarget_front_time;//前置浇水
 u16 curtarget_fertilizing_time;//施肥时长
 u8 curtarget_back_time;//后置浇水
 u8 curtarget_stopmode;//停止模式
 u8 formula; //配方
 u16 fertilizing_amount;//亩均施肥量
 u16 currenttargetlefttime;//当前任务剩余时间
 u8 curtarget_max_num;//当前任务的最大数量

}__attribute__((packed))Targetrecord;
typedef union
{
	Targetrecord trecord;
	u8 target[67];
}__attribute__((packed))Targetred; //任务存储读取结构体
//设置存储参数
typedef struct
{
		u16 P_A;// A罐浓度
		u16 P_B;// B罐浓度
		u16 P_C;// C罐浓度
		u16 P_D;// D罐浓度   
    u16 Partition1;//1分区亩数 
    u16 Partition2;//2分区亩数
    u16 Partition3;//3分区亩数
    u16 Partition4;//1分区亩数
    u16 Partition5;//5分区亩数
    u16 Partition6;//6分区亩数
    u16 Partition7;//7分区亩数 
    u16 Partition8;//8分区亩数
    u16 Partition9;//9分区亩数
    u16 Partition10;//10分区亩数
    u16 Partition11;//11分区亩数
    u16 Partition12;//12分区亩数    
    u16 Partition13;//13分区亩数
    u16 Partition14;//14分区亩数
    u16 Partition15;//15分区亩数
    u16 Partition16;//16分区亩数
    u16 Partition17;//1分区亩数  //后16分区暂时不用
    u16 Partition18;//2分区亩数
    u16 Partition19;//3分区亩数
    u16 Partition20;//1分区亩数
    u16 Partition21;//5分区亩数
    u16 Partition22;//6分区亩数
    u16 Partition23;//7分区亩数 
    u16 Partition24;//8分区亩数
    u16 Partition25;//9分区亩数
    u16 Partition26;//10分区亩数
    u16 Partition27;//11分区亩数
    u16 Partition28;//12分区亩数    
    u16 Partition29;//13分区亩数
    u16 Partition30;//14分区亩数
    u16 Partition31;//15分区亩数
    u16 Partition32;//16分区亩数    
    u16 Complex_Front_time;//复合肥前置浇水时间  
    u16 Complex_Fer_time; //复合肥施肥时间    
    u16 Complex_Back_time; //复合肥后置时间     
    u16 Complex_A_Fer;//A罐肥料  
    u16 Complex_B_Fer; //B罐肥料    
    u16 Complex_C_Fer; //C罐肥料    
    u16 Complex_D_Fer;//D罐肥料
    u16 Complex_Fer_amount;//复合亩均施肥量   
    u16 First_Front_time;//第一阶段前置浇水时间  
    u16 First_Fer_time; //第一阶段施肥时间    
    u16 First_Back_time; //第一阶段后置时间     
    u16 First_A_Fer;//第一阶段A罐肥料  
    u16 First_B_Fer; //第一阶段B罐肥料    
    u16 First_C_Fer; //第一阶段C罐肥料    
    u16 First_D_Fer;//第一阶段D罐肥料
    u16 First_Fer_amount;//第一阶段亩均施肥量       
    u16 Second_Front_time;//第二阶段前置浇水时间  
    u16 Second_Fer_time; //第二阶段施肥时间    
    u16 Second_Back_time; //第二阶段后置时间     
    u16 Second_A_Fer;//第二阶段A罐肥料  
    u16 Second_B_Fer; //第二阶段B罐肥料    
    u16 Second_C_Fer; //第二阶段C罐肥料    
    u16 Second_D_Fer;//第二阶段D罐肥料
    u16 Second_Fer_amount;//第二阶段亩均施肥量   
    u16 Third_Front_time;//第三阶段前置浇水时间  
    u16 Third_Fer_time; //第三阶段施肥时间    
    u16 Third_Back_time; //第三阶段后置时间        
    u16 Third_A_Fer;//第三阶段A罐肥料  
    u16 Third_B_Fer; //第三阶段B罐肥料    
    u16 Third_C_Fer; //第三阶段C罐肥料    
    u16 Third_D_Fer;//第三阶段D罐肥料
    u16 Third_Fer_amount;//第三阶段亩均施肥量       
    u16 Four_Front_time;//第四阶段前置浇水时间  
    u16 Four_Fer_time; //第四阶段施肥时间    
    u16 Four_Back_time; //第四阶段后置时间     
    u16 Four_A_Fer;//第四阶段A罐肥料  
    u16 Four_B_Fer; //第四阶段B罐肥料    
    u16 Four_C_Fer; //第四阶段C罐肥料    
    u16 Four_D_Fer;//第四阶段D罐肥料
    u16 Four_Fer_amount;//第四阶段亩均施肥量         
    u16 PartitionEnable[16];
}__attribute__((packed))SetPara;
typedef union
{
	SetPara set;
	u16 FlashPara[92];
}__attribute__((packed))FlashSet; //设置参数保存
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
void Save_TaskRecord(u8 tasknum,u8 read,u8 write);
void time_copy(void);
void Water_Program(u8 program);
void Display_SetButonn(void);
#endif

