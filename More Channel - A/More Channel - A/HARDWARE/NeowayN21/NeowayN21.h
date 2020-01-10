#ifndef __NeowayN21_H
#define __NeowayN21_H

void	NeoWayN21_init(void);	//有方N21初始化
void	conN21(void);			//N21连接到阿里云
void	disconN21(void);		//断开IMQTT连接
void	N21GetTime(void);
unsigned char sendN21(char *data,unsigned char w);	//向订阅的topic发送数据

#endif
