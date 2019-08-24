#ifndef __LEDdz_H
#define __LEDdz_H

#include "sys.h"

#ifndef Env
#define Env
typedef struct
{
	u16		airtemp;	//转为十进制后需要/10
	u16		airhumi;	//转为十进制后需要/10
	u16		soiltemp;	//转为十进制后需要/10
	u16		soilhumi;	//转为十进制后需要/10
	u16		CO2;
	u32		light;
	u16		EC;
}Environmental;
#endif

void	setdz(void);
void	setprogram1(u8 addr ,Environmental data);
//void	setprogram2(u8 addr ,Environmental data);

#endif
