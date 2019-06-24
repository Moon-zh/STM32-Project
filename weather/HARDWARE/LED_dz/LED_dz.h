#ifndef __LEDdz_H
#define __LEDdz_H

#include "sys.h"

#ifndef Env
#define Env
typedef struct
{
	u16 pm25,co2,temp,vol,humi,pm10,windspeed,winddir,rain,snow;
	u32 bmp,light;
}Environmental;
#endif

void	setdz(void);
void	setprogram1(Environmental data);
void	setprogram2(Environmental data);

#endif
