#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "rs485.h"
#include "check.h"
#include "SETIO.h"
#include "stmflash.h"

#define FLASH_SAVE_ADDR  0X0802DD00

#define head 0xFE		//I0板地址(头帧)

u8 check_out[]	={head ,0x01 ,0x00 ,0x00 ,0x00 ,0x08 ,0x29 ,0xC3 ,0x00 ,0x00 ,0x00};	//查询继电器指令
u8 check_in[]	={head ,0x02 ,0x00 ,0x00 ,0x00 ,0x08 ,0x6D ,0xC3 ,0x00 ,0x00 ,0x00};	//查询光耦输入指令
u8 control[]	={head ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0xC5 ,0xD9 ,0x00 ,0x00 ,0x00};	//第四字节选择继电器开关 0为关 0xff为开
u8 control_all[]={head ,0x0F ,0x00 ,0x00 ,0x00 ,0x08 ,0x01 ,0xFF ,0xF1 ,0xD1 ,0x00};	//控制全部继电器第7个字节选择全部继电器开关

u8 check_out_back[]	={head ,0x01 ,0x01 ,0x00 ,0x00 ,0x00};				//返回继电器状态
u8 check_in_back[]	={head ,0x02 ,0x01 ,0x00 ,0x00 ,0x00};				//返回光耦状态
u8 check1_out_back[]={head ,0x05 ,0x00 ,0x00 ,0xFF ,0x00 ,0x98 ,0x35};	//返回单个继电器状态
u8 back_all[]       ={head ,0x0F ,0x00 ,0x00 ,0x00 ,0x08 ,0x40 ,0x02};

u8 setlinkage[]={head ,0x10 ,0x03 ,0xEB ,0x00 ,0x01 ,0x02 ,0x00 ,0x01 ,0x06 ,0x7F};		//设置为联动模式指令
u8 setnormal[] ={head ,0x10 ,0x03 ,0xEB ,0x00 ,0x01 ,0x02 ,0x00 ,0x00 ,0xC7 ,0xBF};		//设置为正常模式指令
u8 setaddr[]   ={head ,0x10 ,0x03 ,0xEA ,0x00 ,0x01 ,0x02 ,0x00 ,0x01 ,0x07 ,0xAE};		//设置继电器地址
u8 *tab[]={check_out,check_in,control,setlinkage,setnormal,setaddr};			//索引表
u8 *tab1[]={check_out,check_in,control,control_all,check_out_back,check_in_back,check1_out_back};

u8 Model=0;				//0为正常模式 1为联动模式
u8 zt_ks=0,zt_go=0;		//继电器与光耦状态存储
u8 addr0;				//存储用哪个地址发来的指令

void	saveaddr(u8 value)	//存储模块地址
{
	u8 i;
	for(i=0;i<7;i++){(tab1[i])[0]=value;}
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)&value,1);
}

void	send(u8 *a,u8 zt)	//485发送函数 包括CRC16校验	用于返回6字节的指令
{
	u16 b;
	a[0]=addr0;
	a[3]=zt;
	b=mc_check_crc16(a,4);
	a[4]=b>>8;
	a[5]=b&0xff;
	comSendBuf(COM2,a,6);
}

void	send1(u8 *a,u8 addr,u8 data)	//用于返回8字节的指令
{
	u16 b;
	a[0]=addr0;
	a[3]=addr;
	a[4]=data;
	b=mc_check_crc16(a,6);
	a[6]=b>>8;
	a[7]=b&0xff;
	comSendBuf(COM2,a,8);
}

u8		read(u8 *a)						//485读取函数
{
	u8 len;
	len=COM2GetBuf(a,15);
	if(len>=8)
	{
		delay_ms(100);
		return 1;
	}
	return 0;
}

void	change_ks(u8 addr,u8 data)		//单个继电器状态改变
{
	u8 a;
	a=zt_ks;
	if(Model)return;		//联动模式直接跳出
	switch(addr)
	{
		case 0:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_9),GPIO_ResetBits(GPIOC,GPIO_Pin_9),zt_ks|=0x80;
				else GPIO_SetBits(GPIOC,GPIO_Pin_9),GPIO_SetBits(GPIOC,GPIO_Pin_9),zt_ks&=~0x80;
				break;
		case 1:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_8),GPIO_ResetBits(GPIOC,GPIO_Pin_8),zt_ks|=0x40;
				else GPIO_SetBits(GPIOC,GPIO_Pin_8),GPIO_SetBits(GPIOC,GPIO_Pin_8),zt_ks&=~0x40;
				break;
		case 2:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_7),GPIO_ResetBits(GPIOC,GPIO_Pin_7),zt_ks|=0x20;
				else GPIO_SetBits(GPIOC,GPIO_Pin_7),GPIO_SetBits(GPIOC,GPIO_Pin_7),zt_ks&=~0x20;
				break;
		case 3:
				if(data)GPIO_ResetBits(GPIOC,GPIO_Pin_6),GPIO_ResetBits(GPIOC,GPIO_Pin_6),zt_ks|=0x10;
				else GPIO_SetBits(GPIOC,GPIO_Pin_6),GPIO_SetBits(GPIOC,GPIO_Pin_6),zt_ks&=~0x10;
				break;
		case 4:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_15),GPIO_ResetBits(GPIOB,GPIO_Pin_15),zt_ks|=0x08;
				else GPIO_SetBits(GPIOB,GPIO_Pin_15),GPIO_SetBits(GPIOB,GPIO_Pin_15),zt_ks&=~0x08;
				break;
		case 5:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_14),GPIO_ResetBits(GPIOB,GPIO_Pin_14),zt_ks|=0x04;
				else GPIO_SetBits(GPIOB,GPIO_Pin_14),GPIO_SetBits(GPIOB,GPIO_Pin_14),zt_ks&=~0x04;
				break;
		case 6:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_13),GPIO_ResetBits(GPIOB,GPIO_Pin_13),zt_ks|=0x02;
				else GPIO_SetBits(GPIOB,GPIO_Pin_13),GPIO_SetBits(GPIOB,GPIO_Pin_13),zt_ks&=~0x02;
				break;
		case 7:
				if(data)GPIO_ResetBits(GPIOB,GPIO_Pin_12),GPIO_ResetBits(GPIOB,GPIO_Pin_12),zt_ks|=0x01;
				else GPIO_SetBits(GPIOB,GPIO_Pin_12),GPIO_SetBits(GPIOB,GPIO_Pin_12),zt_ks&=~0x01;
				break;
	}
//	if(a!=zt_ks)		//无变化不发送数据
	send1(check1_out_back,addr,data);
//	delay_ms(100);
}

void	change_all_ks(u8 a)				//全部继电器状态改变
{
	if(a&0x01)GPIO_ResetBits(GPIOC,GPIO_Pin_9);
	else GPIO_SetBits(GPIOC,GPIO_Pin_9);
	
	if(a&0x02)GPIO_ResetBits(GPIOC,GPIO_Pin_8);
	else GPIO_SetBits(GPIOC,GPIO_Pin_8);
	
	if(a&0x04)GPIO_ResetBits(GPIOC,GPIO_Pin_7);
	else GPIO_SetBits(GPIOC,GPIO_Pin_7);
	
	if(a&0x08)GPIO_ResetBits(GPIOC,GPIO_Pin_6);
	else GPIO_SetBits(GPIOC,GPIO_Pin_6);
	
	if(a&0x10)GPIO_ResetBits(GPIOB,GPIO_Pin_15);
	else GPIO_SetBits(GPIOB,GPIO_Pin_15);
	
	if(a&0x20)GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	else GPIO_SetBits(GPIOB,GPIO_Pin_14);
	
	if(a&0x40)GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	else GPIO_SetBits(GPIOB,GPIO_Pin_13);
	
	if(a&0x80)GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	else GPIO_SetBits(GPIOB,GPIO_Pin_12);
	
//	if(zt_ks!=a)zt_ks=a;			//无变化不发送数据
//	else return;

	send1(back_all,0,0);
	//send(check_out_back,zt_ks);
}

void	read_go()		//读取光耦状态
{
	u8 a;
	a=zt_go;
	zt_go=0;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)<<1;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)<<2;
	zt_go|=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)<<3;
	zt_go|=GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_4)<<4;
	zt_go|=GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)<<5;
	zt_go|=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)<<6;
	zt_go|=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)<<7;
	zt_go=~zt_go;
	
	if(Model)					//联动模式控制
	{
		change_all_ks(zt_go);
		delay_ms(50);
		if(zt_go!=a)			//输入无变化时不发送数据
		send(check_in_back,zt_go);
	}
}

void	pdcmd(u8 *a)	//判断指令
{
	u8 k,g,sum;
	u16 checkcrc=0;
	if(!((a[0]==0xfe)|(a[0]==control[0])))
	{
		comClearRxFifo(COM2);
		return ;
	}
	for(g=0;g<6;g++)
	{
		sum=0;
		for(k=0;k<11;k++)	//判断是否有相同的数据
		{
			if(a[k]==(tab[g])[k])sum++;
			else if(k==0){if(a[k]==0xFE)sum++;}
		}
		if(sum>=5)			//判断是否是8字节的指令
		{
			if(tab[g]==check_out){if(a[1]==1){addr0=a[0];send(check_out_back,zt_ks);return ;}}		//继电器状态查询
			else if(tab[g]==check_in){if(a[1]==2){addr0=a[0];send(check_in_back,zt_go);return;}}	//光耦状态查询
			else if((a[3]==0xEB)&(a[8]==1)){Model=1;comSendBuf(COM2,a,11);return ;}					//设置为联动状态
			else if((a[3]==0xEB)&(a[8]==0)){Model=0;comSendBuf(COM2,a,11);return ;}					//设置为正常状态
			else if(a[3]==0xEA)												//设置模块地址
				{
					checkcrc=mc_check_crc16(a,9);
					comSendBuf(COM2,a,11);
					if(((checkcrc>>8)==a[9])&&((checkcrc&0xff)==a[10]))
					saveaddr(a[8]);
					return ;
				}
		}
		if(sum>=4)				//判断是否是继电器控制指令
		{
			if((a[0]==0xFE)|(a[0]==control[0]))
			{
				addr0=a[0];
				checkcrc=mc_check_crc16(a,6);
				if(((checkcrc>>8)==a[6])&&((checkcrc&0xff)==a[7]))
				{
					if(a[1]==0x05)	//判断是否第二字节为单个继电器控制命令
					{
						change_ks(a[3],a[4]);
						return ;
					}
				}
				checkcrc=mc_check_crc16(a,8);
				if(((checkcrc>>8)==a[8])&&((checkcrc&0xff)==a[9]))
				{
					if(a[1]==0x0f)	//判断是否第二字节为全部继电器控制命令
					{
						change_all_ks(a[7]);
						return ;
					}
				}
			}
		}
	}
}

void	init()			//系统初始化
{
	SystemInit();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	bsp_InitUart();			//初始化RS485
	LED_Init();		  		//初始化与LED连接的硬件接口	
	GO_Init();				//光耦初始化
	KS_Init();				//继电器初始化
	
}

int 	main(void)
{	 
	u8 t=0;
	u8 buf0[20];
	u16 bz=0xFE;
	u8 *rs485buf=buf0;
	
	init();
	delay_ms(1000);
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)&bz,1);		//读取flash内存储的地址
	if((bz==0)|(bz==0xFF)){bz=0xFE;STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)&bz,1);}	//如果为非法地址则写入FE
	saveaddr(bz);
	
	while(1)
	{
		if(read(rs485buf))pdcmd(rs485buf);	//读取到485指令后判断指令
		read_go();							//读取光耦状态

		t++; 
		delay_ms(10);
		if(t==20)
		{			
			LED0=!LED0;//提示系统正在运行	
			t=0;
		}		   
	} 
}
