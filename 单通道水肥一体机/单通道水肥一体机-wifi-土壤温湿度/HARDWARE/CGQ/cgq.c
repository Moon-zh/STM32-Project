#include	"cgq.h"

#include	"crc16.h"
#include	"rs485.h"
#include	"delay.h"

#include "UserCore.h"
#include "UserHmi.h"



//自动模式下使用的标志
Zidongmoshi Guangai;


/*
Modbus通信的传感器
*/


/*
发送查询相关地址下寄存器地址的数据
[0]:地址 [1]:功能码 [2][3]:起始地址  [4][5]:数据长度 [6][7]:校验码
*/
void sendinstruct_cgq(u8 addr,u8 jcqdz1,u8 jcqdz2)
{
	u8 xwsz[]={0x01,0x03,0x00,0x02,0x00,0x01,0x25,0xca};
	u16 crc16bz;
	u8 i;
	
	xwsz[0]=addr;//地址
	xwsz[2]=jcqdz1;//寄存器起始地址1
	xwsz[3]=jcqdz2;//寄存器起始地址2
	crc16bz=crc16table(xwsz,6);//进行CRC16校验
	xwsz[6]=crc16bz&0xff;
	xwsz[7]=crc16bz>>8;
	
	for(i=0;i<8;i++)
	{
		comSendChar(COM4,xwsz[i]);		
	}
	delay_ms(100);
	
}


u16 cgq_trwsd_hc[2];
/*
读取土壤温湿度传感器的数据
ms=0 温度：
ms=1 湿度：
*/
u8 readdata_cgq_tr(u8 ms)
{
	u8 buf[30];
	u8 len=0,crc16sz[2];
	u16 crc16bz;
	u16 data;
	
	memset(buf,0,sizeof buf);
	
	len=COM4GetBuf(buf,28);
	if(len==0)return 0;
	crc16bz=crc16table(buf,len-2);
	crc16sz[0]=crc16bz>>8;//高8位
	crc16sz[1]=crc16bz&0xff;//低8位
	
	comClearRxFifo(COM4);
	
	if((buf[len-1]==crc16sz[0])&&(buf[len-2]==crc16sz[1]))//校验CRC正确
	{
		if(ms==1)//土壤湿度
		{
				data=buf[3];
				data=data<<8;
				data|=buf[4];
			
		  	cgq_trwsd_hc[0]=data;
		}
		else	if(ms==0)//土壤温度
		{
				data=buf[3];
				data=data<<8;
				data|=buf[4];
				if(data>1000)//表示温度低于零度  65435
				{
					data=(0XFFFF-data)+1;
					
					data|=0x8000; //最高位为1，表示为温度低于零度
				}
			 	cgq_trwsd_hc[1]=data;
		}
		
		return 1;
	}
	return 0;	
	
}

u8 cgq_trsd[2];//土壤湿度
u8 cgq_trwd[3];//土壤温度,[0]为1表示为负数，[0]为0表示为正数
//传感器-土壤温湿度数据处理
void cgq_tr_sjcl(void)
{
	u16 a;
	
	//土壤湿度 345=34.5%RH
	a=cgq_trwsd_hc[0]/100;
	a*=10;
	a+=cgq_trwsd_hc[0]/10%10;
	cgq_trsd[0]=a;//土壤湿度（小数点前的十位和个位）
	cgq_trsd[1]=cgq_trwsd_hc[0]%10;//土壤湿度
	
	//土壤温度 456=45.6℃
	if(cgq_trwsd_hc[1]&0x8000==1)//温度低于0度
	{
		cgq_trwd[0]=1;
	}
	else	cgq_trwd[0]=0;
	cgq_trwsd_hc[1]=cgq_trwsd_hc[1]&0x7FFF;
	a=cgq_trwsd_hc[1]/100;
	a*=10;
	a+=cgq_trwsd_hc[1]/10%10;
	cgq_trwd[1]=a;//土壤温度
	cgq_trwd[2]=cgq_trwsd_hc[1]%10;//土壤温度
	
}



//传感器-土壤温湿度程序 （主要程序）
void CGQ_cx(void)
{	
	u8 sz_hc[20];//缓冲数组
	MsgStruct Msgtemp;
  WorkPara Hmi_Para_Zidong;	
	

	if(Guangai.zdms==1)//开启了自动模式
	{
			//获取土壤湿度
			sendinstruct_cgq(0x01,0x00,0x02);//发送查询指令
			delay_ms(300);
			readdata_cgq_tr(1);
			delay_ms(200);			
			
			//获取土壤温度
			sendinstruct_cgq(0x01,0x00,0x03);//发送查询指令
			delay_ms(300);
			readdata_cgq_tr(0);
			delay_ms(200);			
		
			
			cgq_tr_sjcl();//传感器-土壤温湿度数据处理
			
			//土壤温湿度值上传到触摸屏界面
			sprintf((char *)sz_hc,"%d.%d%%RH",cgq_trsd[0],cgq_trsd[1]);
			SetTextValue(8,1,sz_hc);//界面-土壤湿度值
			if(cgq_trwd[0]==1)//温度零下
			{
				sprintf((char *)sz_hc,"一");
				SetTextValue(8,8,sz_hc);	
			}
			else	
			{
				sprintf((char *)sz_hc,"  ");
				SetTextValue(8,8,sz_hc);				
			}
			sprintf((char *)sz_hc,"%d.%d℃",cgq_trwd[1],cgq_trwd[2]);
			SetTextValue(8,2,sz_hc);//界面-土壤温度值	


			if((cgq_trsd[0]<Guangai.qdyzbz)&&(Guangai.zdms==1))//土壤湿度小于设置启动阈值且自动模式开启，开启浇水
			{
					Guangai.qdbz=1;//自动模式开启浇水
					if(Guangai.yichi==0)//执行一次的标志
					{
						Hmi_Para_Zidong.WorkModel = 1;//清水
						Hmi_Para_Zidong.Zone = 0x03;//默认开启两个分区
						Hmi_Para_Zidong.TimeH =12;//灌溉时长
						Hmi_Para_Zidong.TimeM =0;//灌溉时长						
						
						Msgtemp.CmdSrc = SCREEN_TASK_CODE;
						Msgtemp.CmdType = MSG_START;//启动
						Msgtemp.CmdData[0] = Hmi_Para_Zidong.WorkModel;//区分：是清水还是施肥
						Msgtemp.CmdData[1] = Hmi_Para_Zidong.Zone;//要执行几个分区
						Msgtemp.CmdData[2] = Hmi_Para_Zidong.TimeH;//时间：小时
						Msgtemp.CmdData[3] = Hmi_Para_Zidong.TimeM;//时间：分钟
						PackSendMasterQ(&Msgtemp);//消息打包发走	

						//触摸屏，设置当前灌溉状态
						sprintf((char *)sz_hc,"开启清水灌溉");
						SetTextValue(8,4,sz_hc);						
						
						Guangai.yichi=1;
					}
			}
			else	if((cgq_trsd[0]>=(Guangai.tzyzbz))&&(Guangai.qdbz==1))//土壤湿度大于等于设置停止阈值且已经开始浇水，停止浇水
			{
					Guangai.qdbz=0;//自动模式关闭浇水
					Guangai.yichi=0;//标志清除
					
					//发送停止消息
					Msgtemp.CmdSrc = SCREEN_TASK_CODE;
					Msgtemp.CmdType = MSG_STOP;  //结束
					//Msgtemp.CmdData[0] = 1;//灌溉类型(to_type)    默认-0，清水-1，肥水-2  平台判断的不能为0
					Msgtemp.CmdData[1] = 0;//需要灌溉分区（need_partition）  1分区-1,2分区-2,1和2分区-3
					Msgtemp.CmdData[2] = 0;//灌溉时常（小时）
					Msgtemp.CmdData[3] = 0;//灌溉时常（分钟）	
					PackSendMasterQ(&Msgtemp);	

			  	//触摸屏，设置当前灌溉状态
					sprintf((char *)sz_hc,"关闭清水灌溉");
					SetTextValue(8,4,sz_hc);				
			}
			
			if((Guangai.qdbz==1)&&(MasterState==0))//还需要浇水，而之前设置的倒计时结束了。 工作程序为空闲了，需要重新发送启动
			{
					Guangai.yichi=0;//在执行一次清水，启动	
			}
	}

									
	
	
}


