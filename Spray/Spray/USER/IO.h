#ifndef __IO_H
#define __IO_H
#include "sys.h"
#include "main.h"
#include "rs485.h"
#include "check.h"
#include "delay.h"

void	IO_OutSet(u8 OutPort, u8 Value)	//µç´Å·§¿ØÖÆ
{
	switch(OutPort)
	{
		case 1:
			if(Value == 0)
			{
				PCout(7) = 0;
			}
			else
			{
				PCout(7) = 1;
			}
			break;
		
		case 2:
			if(Value == 0)
			{
				PCout(8) = 0;
			}
			else
			{
				PCout(8) = 1;
			}
			break;
			
		case 3:
			if(Value == 0)
			{
				PCout(9) = 0;
			}
			else
			{
				PCout(9) = 1;
			}
			break;
		
		case 4:
			if(Value == 0)
			{
				PAout(8) = 0;
			}
			else
			{
				PAout(8) = 1;
			}
			break;
		default:
			break;
	}
}

#endif
