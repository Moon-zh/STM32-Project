#ifndef __Emw3060_H
#define __Emw3060_H


//json包内容  topic上报目录
static char 	*topicw=	"thing/event/property/post\"";
static char      *postw="\\\"params\\\":{";

static char  *wartopicw=	"user/war\"";
static char   *warpostw="\\\"params\\\":{";

void	Emw3060_init(void);
unsigned char Emw3060_con(void);
unsigned char sendEmw(char *data,unsigned char w);

#define EmwLED1 PAout(1)
#define EmwLED2 PAout(0)

extern unsigned char Emwled;

#endif
