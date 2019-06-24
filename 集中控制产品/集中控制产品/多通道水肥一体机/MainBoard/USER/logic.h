#ifndef __LOGIC_H
#define __LOGIC_H
#include "Includes.h"


typedef struct
{
 u32 flower1;//水表1
 u32 flower2;//水表2
 u32 flower3;//水表3
 u32 flower4;//水表4
 u32 lastflower1;//上次水表1读数
 u32 lastflower2;//上次水表2读数
 u32 lastflower3;//上次水表3读数
 u32 lastflower4;//上次水表4读数	
 u32 startflower1;//起始水表1读数	
 u32 startflower2;//起始水表2读数	
 u32 startflower3;//起始水表3读数	
 u32 startflower4;//起始水表4读数		
}__attribute__((packed))fertask;

typedef union
{
	fertask ufertask;
  u8 flowerdatabuf[48];

}fertaskdata;

typedef struct
{
 float M_A;//a罐肥料
 float M_B;//B罐肥料
 float M_C;//C罐肥料
 float M_D;//D罐肥料
 float USE_M_A;//a罐肥料使用
 float USE_M_B;//B罐肥料
 float USE_M_C;//C罐肥料
 float USE_M_D;//D罐肥料
 u8 amount; //亩均施肥量
 u8 Th_A;//A浓度
 u8 Th_B;//A浓度
 u8 Th_C;//A浓度
 u8 Th_D;//A浓度	
 u8 formula_A;//配方A
 u8 formula_B;//配方B
 u8 formula_C;//配方C
 u8 formula_D;//配方D	
	
 float PH_A;//A的密度	
 float PH_B;//A的密度	
 float PH_C;//A的密度	
 float PH_D;//A的密度	
 float Q_A;//A开始流过的流量	
 float Q_B;//A开始流过的流量	
 float Q_C;//A开始流过的流量	
 float Q_D;//A开始流过的流量	
}__attribute__((packed))fercal;	//施肥计算需要的各项数据

//typedef union
//{
//	fertask ufertask;
//  u8 flowerdatabuf[64];

//}fertaskdata;
void FerLogic_task(void *pdata);
void Copy_Folwer(void);
void Chanel_Fer_Weight(u16 data);
float PH_Cal(float pot);
#endif
