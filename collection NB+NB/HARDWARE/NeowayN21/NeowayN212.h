#ifndef __NeowayN21_H
#define __NeowayN21_H

//json包内容  topic上报目录

void	NeoWayN21_init2(void);	//有方N21初始化
void	conN212(void);			//N21连接到阿里云
void	disconN212(void);		//断开IMQTT连接
unsigned char sendN212(char *data,unsigned char w);	//向订阅的topic发送数据

#endif
