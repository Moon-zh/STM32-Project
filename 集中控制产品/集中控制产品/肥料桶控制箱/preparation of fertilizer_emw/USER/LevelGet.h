#ifndef __LEVELGET_H
#define __LEVELGET_H		

#include "sys.h"
#include "includes.h"  

#define DEV1READ 0x01
#define DEV2READ 0x02
#define DEV3READ 0x03
#define DEV4READ 0x04
#define DEV5READ 0x05
#define DEV6READ 0x06

typedef struct
{
	u8 IdleSta;
	u8 RWSta;
	u8 Error;
}LevelCtrlPara;

typedef struct
{
	u8 Enable;
	u8 CtrlType;
}LevelCommand;

void LevelGet_task(void *pdata);
void CycleUpLevel(void);

#endif
