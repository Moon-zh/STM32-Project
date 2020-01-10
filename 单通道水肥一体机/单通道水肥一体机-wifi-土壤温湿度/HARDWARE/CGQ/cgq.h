#ifndef __CGQ_H
#define __CGQ_H

#include	"sys.h"


//自动模式下使用的标志 Guangai.
typedef struct
{
	u8 qdyzbz;//自动模式界面使用，启动阈值标志（采集土壤湿度，低于启动阈值开启浇水）
	u8 tzyzbz;//自动模式界面使用，停止阈值标志（采集土壤湿度，高于停止阈值停止浇水）
	u8 zdms;//自动模式 （默认为0: 0为关自动模式   1为开自动模式）
	u8 qdbz;//自动模式开启浇水或关闭浇水  （默认为0 :  1为已开启浇水  0为已关闭浇水）
	u8 yichi;//自动模式下，执行一次的标志	
}Zidongmoshi;


//发送查询相关地址下寄存器地址的数据
void sendinstruct_cgq(u8 addr,u8 jcqdz1,u8 jcqdz2);
//读取土壤温湿度传感器的数据
u8 readdata_cgq_tr(u8 ms);
//传感器-土壤温湿度数据处理
void cgq_tr_sjcl(void);
//传感器-土壤温湿度程序 （主要程序）
void CGQ_cx(void);



extern u8 cgq_trsd[2];//土壤湿度
extern u8 cgq_trwd[3];//土壤温度,[0]为1表示为负数，[0]为0表示为正数


extern u8 MasterState;//主程序状态变量
//自动模式下使用的标志
extern Zidongmoshi Guangai;


#endif

