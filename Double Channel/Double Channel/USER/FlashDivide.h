#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_Log				0x0000
#define ADDR_mem				0xF000
void	FlashWriteLog(u8* buf,u8 addr,u8 Num);
void	FlashReadLog(u8* buf,u8 addr);
void	FlashWriteMem(u8* buf);
void	FlashReadMem(u8* buf);

#endif
