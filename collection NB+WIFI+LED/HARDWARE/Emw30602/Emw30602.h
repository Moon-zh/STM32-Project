#ifndef __Emw30602_H
#define __Emw30602_H

void	Emw3060_init2(void);
void	Emw3060_con2(void);
unsigned char sendEmw2(char *data,unsigned char w);

extern char ssid[20];
extern char password[20];

#define Emw2LED1 PAout(12)
#define Emw2LED2 PAout(11)

extern unsigned char Emwled2;

#endif
