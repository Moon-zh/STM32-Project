#ifndef __FLASHDIVIDE_H
#define __FLASHDIVIDE_H 			   
#include "sys.h"  
#define ADDR_Log				0x0000
#define	ADDR_Fer				0x4000
#define ADDR_Fer0				0x3F00
#define ADDR_mem				0xF000
void	FlashWriteLog(u8* buf,u8 addr,u8 Num);
void	FlashReadLog(u8* buf,u8 addr);
void	FlashWriteFer(u8* buf,u8 addr,u8 Num);
void	FlashReadFer(u8* buf,u8 addr);
void	FlashWriteFer0(u8* buf);
void	FlashReadFer0(u8* buf);
void	FlashWriteMem(u8* buf);
void	FlashReadMem(u8* buf);

#endif
