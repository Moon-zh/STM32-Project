#include "communication.h"
#include "rs485.h"	 
#include "delay.h"
#include "Includes.h"
u8 s_u8State; //¼ì²â´®¿Ú3×´Ì¬
u8 f_u8State;//¼ì²â´®¿Ú4×´Ì¬
u8 com3state=0;//´®¿Ú3¹¤×÷×´Ì¬
u8 com4state=0;//´®¿Ú4¹¤×÷×´Ì¬
u8 startzone;//¿ªÆô·ÖÇø
u8 closezone;//¹Ø±Õ·ÖÇø
u8   s_u8State = 0;
u16 s_u16Pos = 0;
u16 s_u16Length = 0;
u8  f_u8State = 0;
u16 f_u16Pos = 0;
u16 f_u16Length = 0; 
u8 com3databuf[9];//´®¿Ú3Êý¾Ý
u8 com4databuf[24];//´®¿Ú4Êý¾Ý IO×´Ì¬Õ¼2¸ö×Ö½Ú£¬4¸öÁ÷Á¿±íÕ¼16¸ö×Ö½Ú
u8 send_count=0;//´®¿ÚÊý¾Ý·¢ËÍ´ÎÊý³¬Öµ
tagend tagendstr;//ÖÕ¶ËÍ¨ÐÅ½á¹¹Ìå

void board2com(u8 readwrite,u8 ioadress,u8 iostate)
{
 u8 date[24]={0XAA,0X55,0X01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0XA5,0X5A};
 if(readwrite==0)//¶Á
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
void writetagend(u8 zone,u8 state)//ÒªÐ´µÄ·ÖÇø×´Ì¬
{
 u8 date[9]={0xaa,0x55,0,0x6,0x02,0,0,0xa5,0x5a};
    tagendstr.head_h=0xaa;
    tagendstr.head_l=0x55;
    tagendstr.ID=date[2]=(zone-1)/4+1;
    tagendstr.code=date[3]=0x6;
    tagendstr.datanum=0x02;
    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//¶ÔÓ¦·ÖÇøµç´Å·§
    tagendstr.solenoidvalve=date[6]=state;
    tagendstr.tail_h=0xa5;
    tagendstr.tail_l=0x5a;
		comSendBuf(COM3,date ,9); 
}
void readtagend(u8 zone)//Òª¶Á·ÖÇøµÄ×´Ì¬
{
 u8 date[9]={0xaa,0x55,0,0x6,0x02,0,0,0xa5,0x5a};
    tagendstr.head_h=0xaa;
    tagendstr.head_l=0x55;
    tagendstr.ID=date[2]=(zone-1)/4+1;
    tagendstr.code=date[3]=0x3;
    tagendstr.datanum=0x02;
    tagendstr.solenoidvalve=date[5]=zone-(zone-1)/4*4;//¶ÔÓ¦·ÖÇøµç´Å·§
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
	u8 u16SUM_Check;

	while ( comGetRxAvailableDataSize(COM3)>0 )
	{
		switch ( s_u8State )
		{
			case STATE_START:
				comGetChar ( COM3 , &u8Data );

				if ( ( u8Data == 0xaa ) && ( s_u16Pos == 0 ) ) //Ö¡Í·
				{
					buffer[s_u16Pos ++] = u8Data;
				}
				else if ( ( u8Data != 0xAA ) && ( s_u16Pos == 0 ) ) //Ö¡Í·
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
			case STATE_1://³ÖÐø½ÓÊÕºóÃæÊý¾Ý£¬È«²¿½ÓÊÕÍê
				comGetChar ( COM3 , &u8Data );
				buffer[s_u16Pos++] = u8Data;

				if ( s_u16Pos > 9 )
				{
					s_u16Pos = 0;
					memset ( com3databuf , 0 , 9 );
					s_u8State = STATE_START;
					return 0;
				}

				if ( s_u16Pos == 9 ) //½ÓÊÕÍê
				{
					s_u8State = STATE_START;
//							   u16DataLength = s_u16Length;
					s_u16Length = s_u16Pos;
					s_u16Pos = 0;

					//ÑéÖ¤½áÊøÂëºÍÐ£ÑéºÍ
					if ( ( buffer[s_u16Length - 1] == 0x5a ) && ( buffer[s_u16Length - 2]==0xa5 ) )  //
					{
						return s_u16Length;
					}
					else//Ê§°Ü
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

	return 0;//Ã»ÓÐÐÎ³ÉÍêÕûµÄÒ»Ö¡
}
u16 verifycom4recviedata(u8 *buffer)
{
	u8 u8Data = 0;
//	GR_U8 u8CountTem = 0 ,u8DataTem;
//	GR_U16 u16CRC ,u16DataLength = 0;
	u8 u16SUM_Check;

	while ( comGetRxAvailableDataSize(COM4)>0 )
	{
		switch ( s_u8State )
		{
			case STATE_START:
				comGetChar ( COM4 , &u8Data );

				if ( ( u8Data == 0xaa ) && ( f_u16Pos == 0 ) ) //Ö¡Í·
				{
					buffer[f_u16Pos ++] = u8Data;
				}
				else if ( ( u8Data != 0xAA ) && ( f_u16Pos == 0 ) ) //Ö¡Í·
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
			case STATE_1://³ÖÐø½ÓÊÕºóÃæÊý¾Ý£¬È«²¿½ÓÊÕÍê
				comGetChar ( COM4 , &u8Data );
				buffer[f_u16Pos++] = u8Data;

				if ( f_u16Pos > 24 )
				{
					f_u16Pos = 0;
					memset ( com4databuf , 0 , 24 );
					f_u8State = STATE_START;
					return 0;
				}

				if ( f_u16Pos ==24) //½ÓÊÕÍê
				{
					f_u8State = STATE_START;
//							   u16DataLength = s_u16Length;
					f_u16Length = f_u16Pos;
					f_u16Pos = 0;

					//ÑéÖ¤½áÊøÂëºÍÐ£ÑéºÍ
					if ( ( buffer[f_u16Length - 1] == 0x5a ) && ( buffer[f_u16Length - 2]==0xa5 ) )  //
					{
						return f_u16Length;
					}
					else//Ê§°Ü
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

	if ( f_u16Pos > 9 )
	{
		f_u16Pos = 0;
		f_u8State = STATE_START;
	}

	return 0;//Ã»ÓÐÐÎ³ÉÍêÕûµÄÒ»Ö¡
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
			case CMD_RETURN_REAL_TIME_DATA:
                com3state=0;//´Ë´ÎÐ´ÈÎÎñÍê³É
				send_count=0;
				break;
			default :
				break;
		}
	}
}
void  DepackCom4Data ( void )
{
	u8 u8SourceDataLength , u8DataLength , u8CMD;
	u8SourceDataLength = verifycom4recviedata ( com4databuf );

	if ( u8SourceDataLength != 0 )
	{
		u8CMD = com4databuf[3];

		switch ( u8CMD )
		{
			case 3:
				
			
				break;
			case CMD_RETURN_REAL_TIME_DATA:
                com4state=0;//´Ë´ÎÐ´ÈÎÎñÍê³É
				send_count=0;
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
    
     while(1)    
    {
     delay_ms(500);
     DepackCom3Data ();
     switch(com3state)
     {
         case 1: //¹Ø±Õ·ÖÇø
				 send_count++;
         writetagend(closezone,0);
         break;
         case 2://¿ªÆô·ÖÇø
					send_count++;
         writetagend(startzone,1);         
         break; 
         default:
         break;
     
     
     }
		 if(send_count>10)//·¢ËÍ´ÎÊý³¬¹ý10´Î
		 {
			 //Í£Ö¹ÈÎÎñ
		 
		 }
     
     
 
 
    }
}


