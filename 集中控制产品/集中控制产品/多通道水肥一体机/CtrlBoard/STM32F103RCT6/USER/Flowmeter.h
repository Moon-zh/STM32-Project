#ifndef FLOW_H
#define FLOW_H


#include "includes.h"
typedef struct
{

u32 Flowmeter1; //流量计1 注水电磁阀1
u32 Flowmeter2; //流量计2 注水电磁阀2
u32 Flowmeter3; //流量计3	注水电磁阀3
u32 Flowmeter4; //流量计4	注水电磁阀4
//u32 Flowmeter5; //流量计5 注水电磁阀5

//u32 Flowmeter6;  //流量计6		EC
//u16 Flowmeter7; //流量计7	  PH





}Flowmeter;
typedef union
{
Flowmeter flower;
u8 databuf[16];

}FlowmeterStruct;
#endif
