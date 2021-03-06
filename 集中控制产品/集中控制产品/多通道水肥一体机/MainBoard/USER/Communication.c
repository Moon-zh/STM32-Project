#include "communication.h"
#include "rs485.h"
#include "delay.h"
#include "Includes.h"
#include "UserHmi.h"
#include "logic.h"
u8 Com_State=0;
u8 s_u8State; //检测串口3状态
u8 f_u8State;//检测串口4状态
u8 com_conect_state=0;//串口连接状态
u8 com3state=0;//串口3工作状态
u8 com4state=2;//串口4工作状态
u8 startzone;//开启分区
u8 closezone;//关闭分区
u8 Com3_Send_Count=0;//重发机制
u8 Com4_Send_Count=0;//重发机制
u8 Com3_Conect_Error=0;//终端连接错误标志
u8 Com4_Conect_Error=0;//肥水阀连接错误标志
extern Targetred ctargetrcord;//当前任务记录
extern fertaskdata fertasktimedate;//水表实时数据读取
//以下数据为校验用

u8   s_u8State = 0;
u16 s_u16Pos = 0;
u16 s_u16Length = 0;
u8  f_u8State = 0;
u16 f_u16Pos = 0;
u16 f_u16Length = 0;
u8 com3databuf[9];//串口3数据
u8 com4databuf[24];//串口4数据 IO状态占2个字节，4个流量表占16个字节
u8 send_count=0;//串口3数据发送次数超值
u8 send4_count=0;//串口4数据发送次数超值
tagend tagendstr;//终端通信结构体
OS_EVENT * ComQMsg;
void* 	ComMsgBlock[4];
OS_MEM* ComPartitionPt;
u8 g_u8ComMsgMem[20][4];
MsgComStruct ComMsg;
u8 ComQInit(void)
{
    INT8U os_err;

    ComQMsg = OSQCreate ( ComMsgBlock, 4);

    if(ComQMsg == (OS_EVENT *)0)
    {
        return 1;
    }

    ComPartitionPt = OSMemCreate (
                         g_u8ComMsgMem,
                         20,
                         4,
                         &os_err
                     );
    if(os_err != OS_ERR_NONE)
    {
        return 2;
    }
    return 0;
}
u8 DepackReceiveComQ(MsgComStruct * MasterQ)
{
    u8 os_err;
    os_err = OSMemPut(ComPartitionPt, ( void * )MasterQ);
    if(os_err != OS_ERR_NONE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

u8 PackSendComQ(MsgComStruct* MsgBlk)
{
    INT8U os_err;
    MsgComStruct * MsgTemp = NULL;
    MsgTemp = (MsgComStruct *)OSMemGet(ComPartitionPt,&os_err);
    if(MsgTemp == NULL)
    {
        return 1;
    }
    MsgTemp ->CmdType = MsgBlk->CmdType;
    MsgTemp ->CmdSrc = MsgBlk->CmdSrc;
    MsgTemp ->CmdData[0] = MsgBlk->CmdData[0];
    MsgTemp ->CmdData[1] = MsgBlk->CmdData[1];
    os_err = OSQPost ( ComQMsg,(void*)MsgTemp );
    //发送消息失败释放内存
    if(os_err!=OS_ERR_NONE)
    {
        OSMemPut(ComPartitionPt, ( void * )MsgTemp);
        return 2;
    }
    return 0;
}

void board2com(u8 readwrite,u8 ioadress,u8 iostate)
{
    u8 date[24]= {0XAA,0X55,0X01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0XA5,0X5A};
    if(readwrite==0)//读
    {
        date[3]=0X03;

    }
    else
    {
        date[3]=0X06;
        date[4]=ioadress;
        date[5]=iostate;
    }
    comSendBuf(COM4,date ,24);
}
void writetagend(u8 zone,u8 state)//要写的分区状态
{
    u8 date[9]= {0xaa,0x55,0,0x6,0x02,0,0,0xa5,0x5a};
    tagendstr.head_h=0xaa;
    tagendstr.head_l=0x55;
    tagendstr.ID=date[2]=(zone-1)/4+1;
    tagendstr.code=date[3]=0x6;
    tagendstr.datanum=0x02;
    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//对应分区电磁阀
    tagendstr.solenoidvalve=date[6]=state;
    tagendstr.tail_h=0xa5;
    tagendstr.tail_l=0x5a;
    comSendBuf(COM3,date ,9);
}
void Resetall(u8 zone)//要写的分区状态
{
    u8 date[9]= {0xaa,0x55,0,0x0f,0x02,0,0,0xa5,0x5a};
    tagendstr.head_h=0xaa;
    tagendstr.head_l=0x55;
    tagendstr.ID=date[2]=(zone-1)/4+1;
    tagendstr.code=date[3]=0x6;
    tagendstr.datanum=0x02;
    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//对应分区电磁阀
    tagendstr.solenoidvalve=date[6]=0;
    tagendstr.tail_h=0xa5;
    tagendstr.tail_l=0x5a;
    comSendBuf(COM3,date ,9);
}
void readtagend(u8 zone)//要读分区的状态
{
    u8 date[9]= {0xaa,0x55,0,0x03,0x02,0,0,0xa5,0x5a};
    tagendstr.head_h=0xaa;
    tagendstr.head_l=0x55;
    tagendstr.ID=date[2]=(zone-1)/4+1;
    tagendstr.code=date[3]=0x3;
    tagendstr.datanum=0x02;
    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//对应分区电磁阀
    tagendstr.solenoidvalve=date[6]=0;
    tagendstr.tail_h=0xa5;
    tagendstr.tail_l=0x5a;
    comSendBuf(COM3,date ,9);
}
u16 verifycom3recviedata(u8 *buffer)
{
    u8 u8Data = 0;
//	GR_U8 u8CountTem = 0 ,u8DataTem;
//	GR_U16 u16CRC ,u16DataLength = 0;
//	u8 u16SUM_Check;

    while ( comGetRxAvailableDataSize(COM3)>0 )
    {
        switch ( s_u8State )
        {
        case STATE_START:
            comGetChar ( COM3 , &u8Data );

            if ( ( u8Data == 0xaa ) && ( s_u16Pos == 0 ) ) //帧头
            {
                buffer[s_u16Pos ++] = u8Data;
            }
            else if ( ( u8Data != 0xAA ) && ( s_u16Pos == 0 ) ) //帧头
            {
                s_u16Pos = 0;
                continue;
            }
            else
            {
                buffer[s_u16Pos ++] = u8Data;
            }

            if(( u8Data != 0x55 ) && ( s_u16Pos == 2 ))
            {
                s_u16Pos = 0;
                return s_u16Pos;
            }
            //s_u16Pos ++;

            if ( ( s_u16Pos == 3 ) && ( buffer[2] == tagendstr.ID ) )
            {
                s_u8State = STATE_1;
            }
            else if ( ( s_u16Pos == 3 ) && ( buffer[2] != tagendstr.ID ) )
            {
                s_u16Pos = 0;
                return s_u16Pos;
            }

            break;
        case STATE_1://持续接收后面数据，全部接收完
            comGetChar ( COM3 , &u8Data );
            buffer[s_u16Pos++] = u8Data;

            if ( s_u16Pos > 9 )
            {
                s_u16Pos = 0;
                memset ( com3databuf , 0 , 9 );
                s_u8State = STATE_START;
                return 0;
            }

            if ( s_u16Pos == 9 ) //接收完
            {
                s_u8State = STATE_START;
//							   u16DataLength = s_u16Length;
                s_u16Length = s_u16Pos;
                s_u16Pos = 0;

                //验证结束码和校验和
                if ( ( buffer[s_u16Length - 1] == 0x5a ) && ( buffer[s_u16Length - 2]==0xa5 ) )  //
                {
                    return s_u16Length;
                }
                else//失败
                {
                    memset ( com3databuf , 0 , s_u16Length );
                    return 0;
                }
            }

            break;
        default:
            break;
        }
    }

    if ( s_u16Pos > 9 )
    {
        s_u16Pos = 0;
        s_u8State = STATE_START;
    }

    return 0;//没有形成完整的一帧
}
u16 verifycom4recviedata(u8 *buffer)
{
    u8 u8Data = 0;
//	GR_U8 u8CountTem = 0 ,u8DataTem;
//	GR_U16 u16CRC ,u16DataLength = 0;
//	u8 u16SUM_Check;

    while ( comGetRxAvailableDataSize(COM4)>0 )
    {
        switch ( f_u8State )
        {
        case STATE_START:
            comGetChar ( COM4 , &u8Data );

            if ( ( u8Data == 0xaa ) && ( f_u16Pos == 0 ) ) //帧头
            {
                buffer[f_u16Pos ++] = u8Data;
            }
            else if ( ( u8Data != 0xAA ) && ( f_u16Pos == 0 ) ) //帧头
            {
                f_u16Pos = 0;
                continue;
            }
            else
            {
                buffer[f_u16Pos ++] = u8Data;
            }

            if(( u8Data != 0x55 ) && ( f_u16Pos == 2 ))
            {
                f_u16Pos = 0;
                return f_u16Pos;
            }
            //s_u16Pos ++;

            if ( ( f_u16Pos == 3 ) && ( buffer[2] == 1 ) )
            {
                f_u8State = STATE_1;
            }
            else if ( ( f_u16Pos == 3 ) && ( buffer[2] != 1 ) )
            {
                f_u16Pos = 0;
                return f_u16Pos;
            }

            break;
        case STATE_1://持续接收后面数据，全部接收完
            comGetChar ( COM4 , &u8Data );
            buffer[f_u16Pos++] = u8Data;

            if ( f_u16Pos > 24 )
            {
                f_u16Pos = 0;
                memset ( com4databuf , 0 , 24 );
                f_u8State = STATE_START;
                return 0;
            }

            if ( f_u16Pos ==24) //接收完
            {
                f_u8State = STATE_START;
//							   u16DataLength = s_u16Length;
                f_u16Length = f_u16Pos;
                f_u16Pos = 0;

                //验证结束码和校验和
                if ( ( buffer[f_u16Length - 1] == 0x5a ) && ( buffer[f_u16Length - 2]==0xa5 ) )  //
                {
                    return f_u16Length;
                }
                else//失败
                {
                    memset ( com4databuf , 0 , f_u16Length );
                    return 0;
                }
            }

            break;
        default:
            break;
        }
    }

    if ( f_u16Pos > 24 )
    {
        f_u16Pos = 0;
        f_u8State = STATE_START;
    }

    return 0;//没有形成完整的一帧
}
void  DepackCom3Data ( void )
{
    u8 u8SourceDataLength , u8CMD;
    u8SourceDataLength = verifycom3recviedata ( com3databuf );

    if ( u8SourceDataLength != 0 )
    {
        u8CMD = com3databuf[3];

        switch ( u8CMD )
        {
        case 3:

            send_count=0;
            break;
        case CMD_RETURN_REAL_TIME_DATA:
            com3state=4;//此次写任务完成
            send_count=0;
            break;
        default :
            break;
        }
    }
}
void  DepackCom4Data ( void )
{
    u8 u8SourceDataLength , i , u8CMD;
    u8SourceDataLength = verifycom4recviedata ( com4databuf );

    if ( u8SourceDataLength != 0 )
    {
        u8CMD = com4databuf[3];

        switch ( u8CMD )
        {
        case 3:
            for(i=6; i<22; i++)
            {
                fertasktimedate.flowerdatabuf[i-6]=com4databuf[i];
            }
//		    com4state=0;//此次写任务完成
            send4_count=0;
            break;
        case CMD_RETURN_REAL_TIME_DATA:
            com4state=2;//此次写任务完成
            send4_count=0;
            break;
        default :
            break;
        }
    }
}

void CHECKCtrl_task(void *pdata)
{
    u8 os_err;
    pdata=pdata;
    MsgComStruct * pMsgBlk = NULL;
    MsgComStruct Msgtemp;
    MsgComStruct MsgtempBlk;
//	MsgStruct Msgtemp1;
    MsgStruct MsgtempBlk1;
    //OS_CPU_SR  cpu_sr;
    ComQInit();
    while(1)
    {
        delay_ms(300);
        if(com3state==4)//与终端连接之后再开始心跳
            Com3_Send_Count++;
        if(com4state==2)//与卡2连接之后再开始心跳
            Com4_Send_Count++;
        DepackCom3Data ();
        DepackCom4Data ();
        switch(com3state)
        {
        case 1: //关闭分区
            send_count++;
            writetagend(closezone,0);
            break;
        case 2://开启分区
            send_count++;
            writetagend(startzone,1);
        case 3:
            send_count++;
            Resetall(closezone);
            break;
        case 4://心跳读取
            if(Com3_Send_Count>=4)
            {   send_count++;
                Com3_Send_Count=0;
                MsgtempBlk.CmdType = COM_READ; //发送消息告诉状态机启动任务
                MsgtempBlk.CmdData[0]= 1;//灌溉模式
                PackSendComQ(&MsgtempBlk);
            }
            break;
        default:
            break;


        }
        switch(com4state)
        {
        case 1: //开启电磁阀
            send4_count++;
            board2com(1,ctargetrcord.trecord.fer_onoff,ctargetrcord.trecord.fer_chanle);//选择要开启的
            break;

        case 2://心跳读取
            if(Com4_Send_Count>=5)
            {
                Com4_Send_Count=0;
                send4_count++;
                MsgtempBlk.CmdType = COM_READ; //发送消息告诉状态机启动任务
                MsgtempBlk.CmdData[0]= 2;//串口4发送心跳
                PackSendComQ(&MsgtempBlk);
            }
            break;
        default:
            break;


        }
        if((send_count>20)&&(Com3_Conect_Error==0))//发送次数超过10次
        {
            com3state=0; //陷入死循环
            Com3_Conect_Error=1;
            send_count=0;
            MsgtempBlk1.CmdType = MSG_ALARM; //发送消息告诉状态机启动任务
            MsgtempBlk1.CmdData[0]= PRESS_BUTTON_ERROR;//水表错误
            PackSendMasterQ(&MsgtempBlk1);	 		        //停止任务

        }
        else if((send_count>20)&&(Com3_Conect_Error==1))
        {
            com3state=0; //陷入死循环
            send_count=0;
            Com3_Conect_Error=0;

        }

        if((send4_count>10)&&(Com4_Conect_Error==0))//发送次数超过10次
        {
            com4state=0;
            send4_count=0;
            Com4_Conect_Error=1;
            MsgtempBlk1.CmdType = MSG_ALARM; //发送消息告诉状态机启动任务
            MsgtempBlk1.CmdData[0]= PRESS_BUTTON_ERROR;//水表错误
            PackSendMasterQ(&MsgtempBlk1);	 		        //停止任务

        }
        else if((send4_count>10)&&(Com4_Conect_Error==1))
        {
            com4state=0; //陷入死循环
            send4_count=0;
            Com4_Conect_Error=0;

        }

        pMsgBlk = ( MsgComStruct *) OSQPend ( ComQMsg,
                                              10,
                                              &os_err );


        if(os_err == OS_ERR_NONE)
        {
            memcpy((u8*)&Msgtemp,(u8*)pMsgBlk,sizeof(MsgComStruct) );
            DepackReceiveComQ(pMsgBlk);
            switch(Msgtemp.CmdType)
            {
            case COM_READ:
                if(Msgtemp.CmdData[0]==1)//串口3读
                {
                    readtagend(startzone); //需要防呆机制 终端自身要加

                }
                else if(Msgtemp.CmdData[0]==2)//串口4读
                {
                    com4state =2;
                    board2com(0,ctargetrcord.trecord.fer_onoff,ctargetrcord.trecord.fer_chanle);//选择要开启的

                }
                break;
            case COM_WRITE:
                if(Msgtemp.CmdData[0]==1)//串口3写
                {
                    if(Msgtemp.CmdData[1]==1) //关闭分区
                    {
                        Com3_Send_Count=0;//清心跳计时
                        com3state =1;
                        send_count++;
                        writetagend(closezone,0);

                    }
                    else if(Msgtemp.CmdData[1]==2)//开启分区
                    {
                        Com3_Send_Count=0;//清心跳计时
                        send_count++;
                        com3state =2;
                        writetagend(startzone,1);


                    }
                    else if(Msgtemp.CmdData[1]==5)//全关分区
                    {


                    }

                }
                else if(Msgtemp.CmdData[0]==2)//写串口4
                {
                    if(Msgtemp.CmdData[1]==1) //开启分区
                    {
                        Com4_Send_Count=0;//清心跳计时
                        com4state =1;
                        send4_count++;
                        board2com(1,ctargetrcord.trecord.fer_onoff,ctargetrcord.trecord.fer_chanle);//选择要开启的

                    }
                    if(Msgtemp.CmdData[1]==4) //关闭分区
                    {
                        Com4_Send_Count=0;//清心跳计时
                        com4state =1;
                        send4_count++;
                        board2com(1,0,ctargetrcord.trecord.fer_chanle);//选择要开启的

                    }
                    if(Msgtemp.CmdData[1]==3) //全关分区
                    {
                        Com4_Send_Count=0;//清心跳计时
                        com4state =1;
                        send4_count++;
                        board2com(1,2,0);//选择要开启的

                    }
                }
                break;



            }
        }


    }
}

