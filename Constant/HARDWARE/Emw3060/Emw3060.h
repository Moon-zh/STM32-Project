#ifndef __Emw3060_H
#define __Emw3060_H

void	Emw3060_init(void);
void	Emw3060_con(void);
unsigned char sendEmw(char *data,unsigned char w);

#define EmwLED1 PAout(1)
#define EmwLED2 PAout(0)

extern unsigned char Emwled;

#endif
