#ifndef FLOW_H
#define FLOW_H


#include "includes.h"
typedef struct
{

u32 Flowmeter1; //������1 עˮ��ŷ�1
u32 Flowmeter2; //������2 עˮ��ŷ�2
u32 Flowmeter3; //������3	עˮ��ŷ�3
u32 Flowmeter4; //������4	עˮ��ŷ�4
//u32 Flowmeter5; //������5 עˮ��ŷ�5

//u32 Flowmeter6;  //������6		EC
//u16 Flowmeter7; //������7	  PH





}Flowmeter;
typedef union
{
Flowmeter flower;
u8 databuf[16];

}FlowmeterStruct;
#endif
