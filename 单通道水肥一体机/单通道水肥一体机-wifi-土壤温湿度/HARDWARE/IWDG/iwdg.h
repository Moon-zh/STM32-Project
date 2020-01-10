#ifndef __IWDG_H
#define __IWDG_H

#include	"sys.h"

void IWDG_Init(u8 prer,u16 rlr);//prer是预分频系数  rlr是重装值
void IWDG_Feed(void);//喂独立看门狗

#endif
