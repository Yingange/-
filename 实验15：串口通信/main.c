/**************************************************************************************
*		              ����ͨ��ʵ��												  *
ʵ���������س����򿪴��ڵ������֣�������������Ϊ4800��ѡ���͵����ݾͿ�����ʾ
			�ڴ��������ϡ�
ע������ޡ�																				  
***************************************************************************************/

#include "reg52.h"			 //���ļ��ж����˵�Ƭ����һЩ���⹦�ܼĴ���
#include "oled.h"
#define u16 unsigned int 	  //���������ͽ�����������
#define u8  unsigned char 
#define uchar unsigned char
#define uint  unsigned int							   
unsigned char longstring1[]="�����������վ��ע�ⰲȫ��������������������\n\n";			//����ʾ�ַ�����Ҫʲô�ַ������ͱ༭ʲô�ַ������ɡ�
unsigned char longstring2[]="վ����";
unsigned char longstring3[]="�����δӺ����³��³���ע�ⰲȫ";
unsigned char longstring4[]="��ǰ�¶ȹ�����ע������";
//unsigned char exampledata[]="08:45:10";
unsigned char tostation[]="TO:";
unsigned char nextstation[]="NEXT:";
volatile unsigned char sending;

//#define GPIO_DIG P0
#define GPIO_KEY P1		  //���󰴼�
						  //OLED
						  // ����ģ��

sbit Gred=P2^7;	   //���⴫����

sbit RST=P2^1;	 //rtcʱ��
sbit IO=P2^2;
sbit SCK=P2^3;
sbit led=P2^0;
u8 KeyValue;	//������Ŷ�ȡ���ļ�ֵ	  �������ļ�ֵ�Ǵ�0 ~ F		  �������ʾʱ��a����������ʲô�źţ���������ߵ�ƽʹ���ŵ��м�·��Ϣ������󣬵��ǰ�����ֵ�ǳ���ȷ��
u8 station;
u8 stationx;	      
u8 figup=1;	
u8 fig_year_add=1;	   //������ֻ��һ����Ч
u8 fig_year_dec=1;	   //������ֻ��һ����Ч
u8 add_year_g=0;	   //��ĸ�λ
u8 add_year_s=0;	   //���ʮλ
u8 fig_month_add=1;	   //������ֻ��һ����Ч
u8 fig_month_dec=1;	   //������ֻ��һ����Ч
u8 add_month_g=0;	   //�µĸ�λ
u8 add_month_s=0;	   //�µ�ʮλ

u8 fig_day_add=1;	   //������ֻ��һ����Ч
u8 fig_day_dec=1;	   //������ֻ��һ����Ч
u8 add_day_g=0;	   //�յĸ�λ
u8 add_day_s=0;	   //�յ�ʮλ

u8 fig_hour_add=1;	   //������ֻ��һ����Ч
u8 fig_hour_dec=1;	   //������ֻ��һ����Ч
u8 add_hour_g=0;	   //ʱ�ĸ�λ
u8 add_hour_s=0;	   //ʱ��ʮλ

u8 fig_minut_add=1;	   //������ֻ��һ����Ч
u8 fig_minut_dec=1;	   //������ֻ��һ����Ч
u8 add_minut_g=0;	   //�ֵĸ�λ
u8 add_minut_s=0;	   //�ֵ�ʮλ

u8 fig_week_add=1;	   //������ֻ��һ����Ч
u8 fig_week_dec=1;	   //������ֻ��һ����Ч
u8 add_week_g=0;	   //�ܵĸ�λ
//u8 add_week_s=0;	   //�ܵ�ʮλ
u8 fig_retime=1;
//u8 code smgduan[17]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
//					0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};//��ʾ0~F��ֵ
u8 code staname[9]={'A','B','C','D','E','F','G','H'};		//վ����Ϣ
u8 stationow=0;

//DS1302��ַ����
#define ds1302_sec_add			0x80		//�����ݵ�ַ
#define ds1302_min_add			0x82		//�����ݵ�ַ
#define ds1302_hr_add			0x84		//ʱ���ݵ�ַ
#define ds1302_date_add			0x86		//�����ݵ�ַ
#define ds1302_month_add		0x88		//�����ݵ�ַ
#define ds1302_day_add			0x8a		//�������ݵ�ַ
#define ds1302_year_add			0x8c		//�����ݵ�ַ
#define ds1302_control_add		0x8e		//�������ݵ�ַ
#define ds1302_charger_add		0x90 					 
#define ds1302_clkburst_add		0xbe

//��ʼʱ�䶨��
uchar time_buf[8] = {0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00};//��ʼʱ���쳣����Ҫ���� ������������ʾ
uchar readtime[16];//��ǰʱ��
uchar sec_buf=0;  //�뻺��
uchar sec_flag=0; //���־λ

//����:��ʱ1����
//��ڲ���:x
//���ڲ���:��
//˵��:����Ϊ12M
void Delay_xms(uint x)
{
  uint i,j;
  for(i=0;i<x;i++)
    for(j=0;j<112;j++);
}
//DS1302��ʼ������
void ds1302_init(void) 
{
	RST=0;			//RST���õ�
	SCK=0;			//SCK���õ�
}
//��DS1302д��һ�ֽ�����
void ds1302_write_byte(uchar addr, uchar d) 
{
	uchar i;
	RST=1;					//����DS1302����	
	//д��Ŀ���ַ��addr
	addr = addr & 0xFE;   //���λ���㣬�Ĵ���0λΪ0ʱд��Ϊ1ʱ��
	for (i = 0; i < 8; i ++) {
		if (addr & 0x01) {
			IO=1;
			}
		else {
			IO=0;
			}
		SCK=1;      //����ʱ��
		SCK=0;
		addr = addr >> 1;
		}	
	//д�����ݣ�d
	for (i = 0; i < 8; i ++) {
		if (d & 0x01) {
			IO=1;
			}
		else {
			IO=0;
			}
		SCK=1;    //����ʱ��
		SCK=0;
		d = d >> 1;
		}
	RST=0;		//ֹͣDS1302����
}

//��DS1302����һ�ֽ�����
uchar ds1302_read_byte(uchar addr) {

	uchar i,temp;	
	RST=1;					//����DS1302����
	//д��Ŀ���ַ��addr
	addr = addr | 0x01;    //���λ�øߣ��Ĵ���0λΪ0ʱд��Ϊ1ʱ��
	for (i = 0; i < 8; i ++) {
		if (addr & 0x01) {
			IO=1;
			}
		else {
			IO=0;
			}
		SCK=1;
		SCK=0;
		addr = addr >> 1;
		}	
	//������ݣ�temp
	for (i = 0; i < 8; i ++) {
		temp = temp >> 1;
		if (IO) {
			temp |= 0x80;
			}
		else {
			temp &= 0x7F;
			}
		SCK=1;
		SCK=0;
		}	
	RST=0;					//ֹͣDS1302����
	return temp;
}

//��DS302д��ʱ������
void ds1302_write_time(void) 
{
	ds1302_write_byte(ds1302_control_add,0x00);			//�ر�д���� 
	ds1302_write_byte(ds1302_sec_add,0x80);				//��ͣʱ�� 
	//ds1302_write_byte(ds1302_charger_add,0xa9);	    //������ 
	ds1302_write_byte(ds1302_year_add,time_buf[1]);		//�� 
	ds1302_write_byte(ds1302_month_add,time_buf[2]);	//�� 
	ds1302_write_byte(ds1302_date_add,time_buf[3]);		//�� 
	ds1302_write_byte(ds1302_hr_add,time_buf[4]);		//ʱ 
	ds1302_write_byte(ds1302_min_add,time_buf[5]);		//��
	ds1302_write_byte(ds1302_sec_add,time_buf[6]);		//��
	ds1302_write_byte(ds1302_day_add,time_buf[7]);		//�� 
	ds1302_write_byte(ds1302_control_add,0x80);			//��д����     
}

//��DS302����ʱ������
void ds1302_read_time(void)  
{
	time_buf[1]=ds1302_read_byte(ds1302_year_add);		//�� 
	time_buf[2]=ds1302_read_byte(ds1302_month_add);		//�� 
	time_buf[3]=ds1302_read_byte(ds1302_date_add);		//�� 
	time_buf[4]=ds1302_read_byte(ds1302_hr_add);		//ʱ 
	time_buf[5]=ds1302_read_byte(ds1302_min_add);		//�� 
	time_buf[6]=(ds1302_read_byte(ds1302_sec_add))&0x7f;//�룬������ĵ�7λ�����ⳬ��59
	time_buf[7]=ds1302_read_byte(ds1302_day_add);		//�� 	
}

//void Ds1302Init()
//{
////	uchar n;
//	Ds1302Write(0x8E,0X00);		 //��ֹд���������ǹر�д��������
////	for (n=0; n<7; n++)//д��7���ֽڵ�ʱ���źţ�����ʱ��������
////	{
////		Ds1302Write(WRITE_RTC_ADDR[n],TIME[n]);	
////	}
//	ds1302_write_time();
//	Ds1302Write(0x8E,0x80);		 //��д��������
//}

/*******************************************************************************
* �� �� ��         : delay
* ��������		   : ��ʱ������i=1ʱ����Լ��ʱ10us
*******************************************************************************/
void delay(u16 i)
{
	while(i--);	
}

/*******************************************************************************
* ������         :UsartInit()
* ��������		   :���ô���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
//void UsartInit()
//{
//	SCON=0X50;			//����Ϊ������ʽ1
//	TMOD=0X20;			//���ü�����������ʽ2
//	PCON=0X80;			//�����ʼӱ�
//	TH1=0XF3;				//��������ʼֵ���ã�ע�Ⲩ������4800��
//	TL1=0XF3;				// Ϊ���������ģ���������������Ҫ�������ʸ���Ϊ9600
//	ES=1;						//�򿪽����ж�
//	EA=1;						//�����ж�
//    REN=1;        //������� 
//	TR1=1;					//�򿪼�����
//}

//���ڳ�ʼ��(ϵͳ����Ϊ12MHz)
void Uart_init(uchar Baud_flag)
{
   TMOD = 0x20;       //T1 2
   PCON = 0x00;       //SMOD = 0
   SCON = 0x50;	      //����1 8
   switch(Baud_flag)
   {   	
    case 0x00:        //2400 11.0592M:0xf4
	  TH1=0xf3;
	  TL1=0xf3;
	break;
	case 0x01:        //4800 11.0592M:0xfa
	  TH1=0xf9;
	  TL1=0xf9;
	break;
    case 0x02:        //9600 11.0592M:0xfd
	  TH1=0xfd;
	  TL1=0xfd;
	break;
    case 0x03:        //19200 11.0592M:0xfe
      TH1=0xfd;
	  TL1=0xfd;
    break;
	default:          //Ĭ��Ϊ2400
	  TH1=0xf3;
	  TL1=0xf3;
	break;
   }
   TR1 = 1;	          //������ʱ��1								
   ES=1;              //�������ж�
   EA=1;              //�����ж�   
   REN=1;    
}

void sendchar(unsigned char d)		  //����һ���ֽڵ����ݣ��β�d��Ϊ���������ݡ�
{
 
 SBUF=d; //������д�뵽���ڻ���
 sending=1;	 //���÷��ͱ�־
 while(sending); //�ȴ��������
}

void sendstring(unsigned char * pd)		    //����һ���ַ�����������'/0'�������͡�
{
 while((*pd)!='\0') 
 {
  sendchar(*pd); //����һ���ַ�
  pd++;  //�ƶ�����һ���ַ�
 }
}

/*******************************************************************************
* �� �� ��         : KeyDown
* ��������		   : ����а������²���ȡ��ֵ
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void KeyDown(void)
{
	char a=0;
	GPIO_KEY=0x0f;
	if(GPIO_KEY!=0x0f)//��ȡ�����Ƿ���
	{
		delay(1000);//��ʱ10ms��������
		if(GPIO_KEY!=0x0f)//�ٴμ������Ƿ���
		{	
			//������
			GPIO_KEY=0X0F;
			switch(GPIO_KEY)
			{
				case(0X07):	KeyValue=0;break;		 //0111
				case(0X0b):	KeyValue=1;break;		 //1011
				case(0X0d): KeyValue=2;break;		 //1101
				case(0X0e):	KeyValue=3;break;		 //1110
			}
			//������
			GPIO_KEY=0XF0;
			switch(GPIO_KEY)
		    	{
				case(0X70):	KeyValue=KeyValue;break;
				case(0Xb0):	KeyValue=KeyValue+4;break;
				case(0Xd0): KeyValue=KeyValue+8;break;
				case(0Xe0):	KeyValue=KeyValue+12;break;
		    	}
		    switch(KeyValue)
		    	{
				case(0): station=0;figup=1;break; //	���в���
				case(1): station=7;figup=0;break; //	���в���
				case(2): sendchar(staname[stationow]);
		                 sendstring(longstring2);
				         sendstring(longstring3); break;				 //	�ظ�����
				case(3): station=0;figup=1;
//				         add_year_g=0; add_year_s=0;
//						 add_month_g=0; add_month_s=0;
//						 add_day_g=0;add_day_s=0;
//						 add_hour_g=0; add_hour_s=0;
//						 add_minut_g=0; add_minut_s=0;
//						 add_week_g=0;
                         fig_retime=0;
						 break;		       	 //	��ʼվ��

				case(4):fig_year_add=0;led=0;break;				 // ��++
				case(5):fig_year_dec=0;led=0;break;		 //��--
				case(6):fig_month_add=0;led=0;break;		 //��++
				case(7):fig_month_dec=0;led=0;break;		 //��--

				case(8):fig_day_add=0;led=0;break;		 //��++
				case(9):fig_day_dec=0;led=0;break;		 //��--
				case(10):fig_hour_add=0;led=0;break;		 //ʱ++
				case(11):fig_hour_dec=0;led=0;break;		 //ʱ--

				case(12):fig_minut_add=0;led=0;break;		 //��++
				case(13):fig_minut_dec=0;led=0;break;		 //��--
				case(14):fig_week_add=0;led=0;break;		 //��++
				case(15):fig_week_dec=0;led=0;break;		 //��--

	    
		    	}	
			while((a<50)&&(GPIO_KEY!=0xf0))	 //��ⰴ�����ּ��,�������г����ʱ��ֻ�е����뿪�����󣬺����ĳ���Ż����ִ�С�
		    	{
				delay(1000);
				a++;
		    	}
		}
	}
}
/*******************************************************************************
* �� �� ��       : main
* ��������		 : ������
* ��    ��       : ��
* ��    ��    	 : ��
*******************************************************************************/
void main()
{
// u8 t;
unsigned char show_year[5];
unsigned char show_month[3];
unsigned char show_day[3];
unsigned char show_h_m_s[9];
unsigned char show_week[2];
unsigned char show_tosp[2];		 //��ǰ��վ
unsigned char show_nxsp[2];		//��һվ
 OLED_Init();			//��ʼ��OLED  
 OLED_Clear(); 
 Delay_xms(50);//�ȴ�ϵͳ�ȶ�
 ds1302_init(); //DS1302��ʼ��
 Uart_init(2); //�����ʳ�ʼ��Ϊ9600
 Delay_xms(10);
 ds1302_write_time(); //д���ʼֵ

//	UsartInit();  //	���ڳ�ʼ��
//	LSA=0; //��һ��������ṩλѡ
//	LSB=0;
//	LSC=0;
	while(1)
	{
	      KeyDown();		   //�����жϺ���
		  ds1302_read_time();  //��ȡʱ�� 
		  //��
		  if(fig_year_add==0 && KeyValue==4)
		   {
		     add_year_g=add_year_g+1; //��������λ 	  ����
			 fig_year_add=1; 

			 if((time_buf[1]&0x0F)+add_year_g==9)
			   {
			   	 add_year_g=0;	
			     add_year_s=add_year_s+1;

			   }
			  led=1;
		   } 
		   if(fig_year_dec==0 && KeyValue==5)
		   {
		     add_year_g=add_year_g-1; //��������λ 	�Լ�
			 fig_year_dec=1; 
			 led=1;
			 if((time_buf[1]&0x0F)+add_year_g==0XFF)
			   {
			     add_year_s=add_year_s-1;
			     add_year_g=9;	
			   }
		   }
		   //��
		   if(fig_month_add==0 && KeyValue==6)
		   {
		     add_month_g=add_month_g+1; //������¸�λ 	  ����
			 if((time_buf[2]&0x0F)+add_month_g==10)
			   {
			   	 add_month_g=0;	
			     add_month_s=add_month_s+1;

			   }
			  led=1;
			  fig_month_add=1; 
		   } 

		   if(fig_month_dec==0 && KeyValue==7)
		   {
		     add_month_g=add_month_g-1; //������¸�λ 		�Լ�
			 fig_month_dec=1; 
			 led=1;
			 if((time_buf[2]&0x0F)+add_month_g==0XFF)
			   {
			     add_month_s=add_month_s-1;
			     add_month_g=9;	
			   }
		   }  

		   //��
		   if(fig_day_add==0 && KeyValue==8)
		   {
		     add_day_g=add_day_g+1; //������ո�λ 	  ����
			 if((time_buf[3]&0x0F)+add_day_g==10)
			   {
			   	 add_day_g=0;	
			     add_day_s=add_day_s+1;

			   }
			  led=1;
			  fig_day_add=1; 
		   } 

		   if(fig_day_dec==0 && KeyValue==9)
		   {
		     add_day_g=add_day_g-1; //������ո�λ 		�Լ�
			 fig_day_dec=1; 
			 led=1;
			 if((time_buf[3]&0x0F)+add_day_g==0XFF)
			   {
			     add_day_s=add_day_s-1;
			     add_day_g=9;	
			   }
		   }  

		   //ʱ
		   if(fig_hour_add==0 && KeyValue==10)
		   {
		     add_hour_g=add_hour_g+1; //�����ʱ��λ 	  ����
			 if((time_buf[4]&0x0F)+add_hour_g==10)
			   {
			   	 add_hour_g=0;	
			     add_hour_s=add_hour_s+1;

			   }
			  led=1;
			  fig_hour_add=1; 
		   } 

		   if(fig_hour_dec==0 && KeyValue==11)
		   {
		     add_hour_g=add_hour_g-1; //�����ʱ��λ 		�Լ�
			 fig_hour_dec=1; 
			 led=1;
			 if((time_buf[4]&0x0F)+add_hour_g==0XFF)
			   {
			     add_hour_s=add_hour_s-1;
			     add_hour_g=9;	
			   }
		   }  
		   //��
		   if(fig_minut_add==0 && KeyValue==12)
		   {
		     add_minut_g=add_minut_g+1; //������ָ�λ 	  ����
			 if((time_buf[5]&0x0F)+add_minut_g==10)
			   {
			   	 add_minut_g=0;	
			     add_minut_s=add_minut_s+1;

			   }
			  led=1;
			  fig_minut_add=1; 
		   } 

		   if(fig_minut_dec==0 && KeyValue==13)
		   {
		     add_minut_g=add_minut_g-1; //������ָ�λ 		�Լ�
			 fig_minut_dec=1; 
			 led=1;
			 if((time_buf[5]&0x0F)+add_minut_g==0xff)
			   {
			     add_minut_s=add_minut_s-1;
			     add_minut_g=9;	
			   }
		   }  
		   //��
		   if(fig_week_add==0 && KeyValue==14)
		   {
		     add_week_g=add_week_g+1; //������ܸ�λ 	  ����
			 if((time_buf[7]&0x07)+add_week_g==8)
			   {
			   	 add_week_g=1;	
//			     add__s=add_minut_s+1;

			   }
			  led=1;
			  fig_week_add=1; 
		   } 

		   if(fig_week_dec==0 && KeyValue==15)
		   {
		     add_week_g=add_week_g-1; //������ܸ�λ 		�Լ�
			 fig_week_dec=1; 
			 led=1;
			 if((time_buf[7]&0x07)+add_week_g==0)
			   {
//			     add_week_s=add_week_s-1;
			     add_minut_g=7;	
			   }
		   } 


		  readtime[0]=(time_buf[0]>>4)+'0';   //�������ǧλ
		  show_year[0]=readtime[0];
		  readtime[1]=(time_buf[0]&0x0F)+'0'; //��������λ 
 		  show_year[1]=readtime[1];
		  readtime[2]=(time_buf[1]>>4)+'0'+add_year_s;   //�������ʮλ
		  show_year[2]=readtime[2];
		  readtime[3]=(time_buf[1]&0x0F)+'0'+add_year_g; //��������λ
		  show_year[3]=readtime[3];		
		  show_year[4]='\0';

		  time_buf[1]=(readtime[2]-'0')<<4+(readtime[3]-'0');

		  readtime[4]=(time_buf[2]>>4)+'0'+add_month_s;   //�������ʮλ
		  show_month[0]=readtime[4];
		  readtime[5]=(time_buf[2]&0x0F)+'0'+add_month_g; //������¸�λ 
		  show_month[1]=readtime[5];
		  show_month[2]='\0';

		  time_buf[2]=(readtime[4]-'0')<<4+(readtime[5]-'0');
		
		  readtime[6]=(time_buf[3]>>4)+'0'+add_day_s;   //�������ʮλ
		  show_day[0]=readtime[6];
		  readtime[7]=(time_buf[3]&0x0F)+'0'+add_day_g; //������ո�λ
		  show_day[1]=readtime[7]; 
		  show_day[2]='\0';

		  time_buf[3]=(readtime[6]-'0')<<4+(readtime[7]-'0');

		  readtime[14]=(time_buf[7]&0x07)+'0'+add_week_g; //����
          show_week[0]=readtime[14];
		  show_week[1]='\0';
		  
		  time_buf[7]=(readtime[14]-'0');		  		

		  readtime[8]=(time_buf[4]>>4)+'0'+add_hour_s;   //�����Сʱʮλ
		  show_h_m_s[0]=readtime[8];
		  readtime[9]=(time_buf[4]&0x0F)+'0'+add_hour_g; //�����Сʱ��λ
		  show_h_m_s[1]=readtime[9];
		  
		  time_buf[4]=(readtime[8]-'0')<<4+(readtime[9]-'0');		  
		  		  
		  show_h_m_s[2]=':';		   
		  readtime[10]=(time_buf[5]>>4)+'0'+add_minut_s;   //���������ʮλ
		  show_h_m_s[3]=readtime[10];

		  readtime[11]=(time_buf[5]&0x0F)+'0'+add_minut_g; //��������Ӹ�λ
		  show_h_m_s[4]=readtime[11];
		  show_h_m_s[5]=':';

		  time_buf[5]=(readtime[10]-'0')<<4+(readtime[11]-'0');

		  readtime[12]=(time_buf[6]>>4)+'0';   //���������ʮλ
		  show_h_m_s[6]=readtime[12];
		  readtime[13]=(time_buf[6]&0x0F)+'0'; //��������Ӹ�λ 
		  show_h_m_s[7]=readtime[13];
		  show_h_m_s[8]='\0';		  		    
		  readtime[15]='\0'; //��������Ӹ�λ   

//		if(fig_retime==0)
//		{
//           ds1302_init(); //DS1302��ʼ��
//           ds1302_write_time(); //д���ʼֵ
//		   fig_retime=1;	
//		}
  		if(Gred==0)
		{								  	
		   delay(100);
		   if(Gred==0)
		   {
				 sendchar(staname[station]);
				 show_tosp[0]=staname[station];
				 show_tosp[1]='\0';
				 if(figup==0)
				   {
				   	stationx=station-1;
				    station--;

					if(station==0XFF )
     				   {
					    station=7;
					   }
					if(stationx==0xff)
     				   {				
						stationx=7;
					   }
					}

     			 else
				   {
				  	stationx=station+1; 
				    station++;

					if(station==8)
					 {
				      station=0;	
					 }
					if(stationx==8)
					 {
					  stationx=0;
					 }
				   }

				 show_nxsp[0]=staname[stationx];
				 show_nxsp[1]='\0';
		         sendstring(longstring2);
				 sendstring(longstring3);
				 sendchar('\n');
				 stationow= station;
//�����δӺ����³��³�ע�ⰲȫ
//		delay_ms(100);
		OLED_ShowCHinese(0,4,24);//��				 
		OLED_ShowCHinese(18,4,25);//��
		OLED_ShowCHinese(36,4,26);//��
		OLED_ShowCHinese(54,4,27);//��
		OLED_ShowCHinese(72,4,28);//��
		OLED_ShowCHinese(90,4,29);//��
		OLED_ShowCHinese(108,4,30);//��
		OLED_ShowCHinese(0,6,31);//��				 
		OLED_ShowCHinese(18,6,15);//��
		OLED_ShowCHinese(36,6,20);//ע
		OLED_ShowCHinese(54,6,21);//��
		OLED_ShowCHinese(72,6,22);//��
		OLED_ShowCHinese(90,6,23);//ȫ
//	   	delay_ms(100);
			 								 
		 while(Gred==0);
//�����������վ��ע�ⰲȫ
		OLED_ShowCHinese(0,4,11);//��				 
		OLED_ShowCHinese(18,4,12);//��
		OLED_ShowCHinese(36,4,13);//��
		OLED_ShowCHinese(54,4,14);//��
		OLED_ShowCHinese(72,4,15);//��
		OLED_ShowCHinese(90,4,16);//��
		OLED_ShowCHinese(108,4,17);//��
		OLED_ShowCHinese(0,6,18);//վ				 
		OLED_ShowCHinese(18,6,19);//��
		OLED_ShowCHinese(36,6,20);//ע
		OLED_ShowCHinese(54,6,21);//��
		OLED_ShowCHinese(72,6,22);//��
		OLED_ShowCHinese(90,6,23);//ȫ
		delay_ms(50);
		//��������������������
	    OLED_ShowCHinese(0,4,32);//��				 
		OLED_ShowCHinese(18,4,33);//��
		OLED_ShowCHinese(36,4,34);//��
		OLED_ShowCHinese(54,4,35);//��
		OLED_ShowCHinese(72,4,36);//��
		OLED_ShowCHinese(90,4,37);//��
		OLED_ShowCHinese(108,4,38);//��
		OLED_ShowCHinese(0,6,39);//��						 
		OLED_ShowCHinese(18,6,40);//��
		OLED_ShowCHinese(36,6,41);//��
		OLED_ShowCHinese(54,6,42);//��
		OLED_ShowCHinese(72,6,43);//��
		OLED_ShowCHinese(90,6,44);//��
//		delay_ms(100);	
				 sendstring(longstring1); 
				 sendchar('\n');
		   }
		}


//		sendstring(readtime);
//        sendchar(KeyValue);
//		sendstring("aaaa");

//		GPIO_DIG=smgduan[KeyValue];	  //	 �˴����Ը��ݼ�ֵ�Ĳ�ͬ���ж������ܵĿ��� 
//		sendc();
		OLED_Clear();
	    OLED_ShowString(0,0,show_year,12); 
		OLED_ShowCHinese(36,0,7);//��	 ��λ
	    OLED_ShowString(54,0,show_month,12); 
		OLED_ShowCHinese(72,0,8);//��		 ��λ
	    OLED_ShowString(90,0,show_day,12); 
		OLED_ShowCHinese(108,0,9);//��		   ��λ
		OLED_ShowCHinese(0,2,10);//��
	    OLED_ShowString(18,2,show_week,12); 	 //���ڶ�  һλ
	   	OLED_ShowString(36,2,show_h_m_s,12);  //ʱ���� ��8λ

		OLED_ShowString(36,3,tostation,12); 
	    OLED_ShowString(60,3,show_tosp,12); 
		OLED_ShowString(72,3,nextstation,12); 
	    OLED_ShowString(112,3,show_nxsp,12); 

////�����δӺ����³��³�ע�ⰲȫ
////		delay_ms(100);
//		OLED_ShowCHinese(0,4,24);//��				 
//		OLED_ShowCHinese(18,4,25);//��
//		OLED_ShowCHinese(36,4,26);//��
//		OLED_ShowCHinese(54,4,27);//��
//		OLED_ShowCHinese(72,4,28);//��
//		OLED_ShowCHinese(90,4,29);//��
//		OLED_ShowCHinese(108,4,30);//��
//		OLED_ShowCHinese(0,6,31);//��				 
//		OLED_ShowCHinese(18,6,15);//��
//		OLED_ShowCHinese(36,6,20);//ע
//		OLED_ShowCHinese(54,6,21);//��
//		OLED_ShowCHinese(72,6,22);//��
//		OLED_ShowCHinese(90,6,23);//ȫ
////	   	delay_ms(100);
////��������������������
//	    OLED_ShowCHinese(0,4,32);//��				 
//		OLED_ShowCHinese(18,4,33);//��
//		OLED_ShowCHinese(36,4,34);//��
//		OLED_ShowCHinese(54,4,35);//��
//		OLED_ShowCHinese(72,4,36);//��
//		OLED_ShowCHinese(90,4,37);//��
//		OLED_ShowCHinese(108,4,38);//��
//		OLED_ShowCHinese(0,6,39);//��						 
//		OLED_ShowCHinese(18,6,40);//��
//		OLED_ShowCHinese(36,6,41);//��
//		OLED_ShowCHinese(54,6,42);//��
//		OLED_ShowCHinese(72,6,43);//��
//		OLED_ShowCHinese(90,6,44);//��
////		delay_ms(100);
	}		
}

/*******************************************************************************
* ������         : Usart() interrupt 4
* ��������		  : ����ͨ���жϺ���
* ����           : ��
* ���         	 : ��
*******************************************************************************/
void Usart() interrupt 4
{

  if(RI)    //�յ�����
  {
     RI=0;   //���ж�����
  }
 else      //������һ�ֽ�����
  {
    TI=0;
    sending=0;  //�����ڷ��ͱ�־
  }
}