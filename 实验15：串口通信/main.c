/**************************************************************************************
*		              串口通信实验												  *
实现现象：下载程序后打开串口调试助手，将波特率设置为4800，选择发送的数据就可以显示
			在串口助手上。
注意事项：无。																				  
***************************************************************************************/

#include "reg52.h"			 //此文件中定义了单片机的一些特殊功能寄存器
#include "oled.h"
#define u16 unsigned int 	  //对数据类型进行声明定义
#define u8  unsigned char 
#define uchar unsigned char
#define uint  unsigned int							   
unsigned char longstring1[]="车辆起步请扶好站好注意安全请主动给老弱病残让座\n\n";			//待显示字符，需要什么字符串，就编辑什么字符串即可。
unsigned char longstring2[]="站到了";
unsigned char longstring3[]="请依次从后门下车下车请注意安全";
unsigned char longstring4[]="当前温度过高请注意防暑降温";
//unsigned char exampledata[]="08:45:10";
unsigned char tostation[]="TO:";
unsigned char nextstation[]="NEXT:";
volatile unsigned char sending;

//#define GPIO_DIG P0
#define GPIO_KEY P1		  //矩阵按键
						  //OLED
						  // 语音模块

sbit Gred=P2^7;	   //红外传感器

sbit RST=P2^1;	 //rtc时钟
sbit IO=P2^2;
sbit SCK=P2^3;
sbit led=P2^0;
u8 KeyValue;	//用来存放读取到的键值	  ，按键的键值是从0 ~ F		  数码管显示时，a脚无论输入什么信号，都是输出高电平使引脚得有几路信息输出错误，但是按键键值非常正确。
u8 station;
u8 stationx;	      
u8 figup=1;	
u8 fig_year_add=1;	   //按键的只有一次有效
u8 fig_year_dec=1;	   //按键的只有一次有效
u8 add_year_g=0;	   //年的个位
u8 add_year_s=0;	   //年的十位
u8 fig_month_add=1;	   //按键的只有一次有效
u8 fig_month_dec=1;	   //按键的只有一次有效
u8 add_month_g=0;	   //月的个位
u8 add_month_s=0;	   //月的十位

u8 fig_day_add=1;	   //按键的只有一次有效
u8 fig_day_dec=1;	   //按键的只有一次有效
u8 add_day_g=0;	   //日的个位
u8 add_day_s=0;	   //日的十位

u8 fig_hour_add=1;	   //按键的只有一次有效
u8 fig_hour_dec=1;	   //按键的只有一次有效
u8 add_hour_g=0;	   //时的个位
u8 add_hour_s=0;	   //时的十位

u8 fig_minut_add=1;	   //按键的只有一次有效
u8 fig_minut_dec=1;	   //按键的只有一次有效
u8 add_minut_g=0;	   //分的个位
u8 add_minut_s=0;	   //分的十位

u8 fig_week_add=1;	   //按键的只有一次有效
u8 fig_week_dec=1;	   //按键的只有一次有效
u8 add_week_g=0;	   //周的个位
//u8 add_week_s=0;	   //周的十位
u8 fig_retime=1;
//u8 code smgduan[17]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
//					0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};//显示0~F的值
u8 code staname[9]={'A','B','C','D','E','F','G','H'};		//站点信息
u8 stationow=0;

//DS1302地址定义
#define ds1302_sec_add			0x80		//秒数据地址
#define ds1302_min_add			0x82		//分数据地址
#define ds1302_hr_add			0x84		//时数据地址
#define ds1302_date_add			0x86		//日数据地址
#define ds1302_month_add		0x88		//月数据地址
#define ds1302_day_add			0x8a		//星期数据地址
#define ds1302_year_add			0x8c		//年数据地址
#define ds1302_control_add		0x8e		//控制数据地址
#define ds1302_charger_add		0x90 					 
#define ds1302_clkburst_add		0xbe

//初始时间定义
uchar time_buf[8] = {0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00};//初始时间异常的重要啊。 否则不能正常显示
uchar readtime[16];//当前时间
uchar sec_buf=0;  //秒缓存
uchar sec_flag=0; //秒标志位

//功能:延时1毫秒
//入口参数:x
//出口参数:无
//说明:晶振为12M
void Delay_xms(uint x)
{
  uint i,j;
  for(i=0;i<x;i++)
    for(j=0;j<112;j++);
}
//DS1302初始化函数
void ds1302_init(void) 
{
	RST=0;			//RST脚置低
	SCK=0;			//SCK脚置低
}
//向DS1302写入一字节数据
void ds1302_write_byte(uchar addr, uchar d) 
{
	uchar i;
	RST=1;					//启动DS1302总线	
	//写入目标地址：addr
	addr = addr & 0xFE;   //最低位置零，寄存器0位为0时写，为1时读
	for (i = 0; i < 8; i ++) {
		if (addr & 0x01) {
			IO=1;
			}
		else {
			IO=0;
			}
		SCK=1;      //产生时钟
		SCK=0;
		addr = addr >> 1;
		}	
	//写入数据：d
	for (i = 0; i < 8; i ++) {
		if (d & 0x01) {
			IO=1;
			}
		else {
			IO=0;
			}
		SCK=1;    //产生时钟
		SCK=0;
		d = d >> 1;
		}
	RST=0;		//停止DS1302总线
}

//从DS1302读出一字节数据
uchar ds1302_read_byte(uchar addr) {

	uchar i,temp;	
	RST=1;					//启动DS1302总线
	//写入目标地址：addr
	addr = addr | 0x01;    //最低位置高，寄存器0位为0时写，为1时读
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
	//输出数据：temp
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
	RST=0;					//停止DS1302总线
	return temp;
}

//向DS302写入时钟数据
void ds1302_write_time(void) 
{
	ds1302_write_byte(ds1302_control_add,0x00);			//关闭写保护 
	ds1302_write_byte(ds1302_sec_add,0x80);				//暂停时钟 
	//ds1302_write_byte(ds1302_charger_add,0xa9);	    //涓流充电 
	ds1302_write_byte(ds1302_year_add,time_buf[1]);		//年 
	ds1302_write_byte(ds1302_month_add,time_buf[2]);	//月 
	ds1302_write_byte(ds1302_date_add,time_buf[3]);		//日 
	ds1302_write_byte(ds1302_hr_add,time_buf[4]);		//时 
	ds1302_write_byte(ds1302_min_add,time_buf[5]);		//分
	ds1302_write_byte(ds1302_sec_add,time_buf[6]);		//秒
	ds1302_write_byte(ds1302_day_add,time_buf[7]);		//周 
	ds1302_write_byte(ds1302_control_add,0x80);			//打开写保护     
}

//从DS302读出时钟数据
void ds1302_read_time(void)  
{
	time_buf[1]=ds1302_read_byte(ds1302_year_add);		//年 
	time_buf[2]=ds1302_read_byte(ds1302_month_add);		//月 
	time_buf[3]=ds1302_read_byte(ds1302_date_add);		//日 
	time_buf[4]=ds1302_read_byte(ds1302_hr_add);		//时 
	time_buf[5]=ds1302_read_byte(ds1302_min_add);		//分 
	time_buf[6]=(ds1302_read_byte(ds1302_sec_add))&0x7f;//秒，屏蔽秒的第7位，避免超出59
	time_buf[7]=ds1302_read_byte(ds1302_day_add);		//周 	
}

//void Ds1302Init()
//{
////	uchar n;
//	Ds1302Write(0x8E,0X00);		 //禁止写保护，就是关闭写保护功能
////	for (n=0; n<7; n++)//写入7个字节的时钟信号：分秒时日月周年
////	{
////		Ds1302Write(WRITE_RTC_ADDR[n],TIME[n]);	
////	}
//	ds1302_write_time();
//	Ds1302Write(0x8E,0x80);		 //打开写保护功能
//}

/*******************************************************************************
* 函 数 名         : delay
* 函数功能		   : 延时函数，i=1时，大约延时10us
*******************************************************************************/
void delay(u16 i)
{
	while(i--);	
}

/*******************************************************************************
* 函数名         :UsartInit()
* 函数功能		   :设置串口
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
//void UsartInit()
//{
//	SCON=0X50;			//设置为工作方式1
//	TMOD=0X20;			//设置计数器工作方式2
//	PCON=0X80;			//波特率加倍
//	TH1=0XF3;				//计数器初始值设置，注意波特率是4800的
//	TL1=0XF3;				// 为了完成语音模块的正常播报，需要将波特率更改为9600
//	ES=1;						//打开接收中断
//	EA=1;						//打开总中断
//    REN=1;        //允许接收 
//	TR1=1;					//打开计数器
//}

//串口初始化(系统晶振为12MHz)
void Uart_init(uchar Baud_flag)
{
   TMOD = 0x20;       //T1 2
   PCON = 0x00;       //SMOD = 0
   SCON = 0x50;	      //串口1 8
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
	default:          //默认为2400
	  TH1=0xf3;
	  TL1=0xf3;
	break;
   }
   TR1 = 1;	          //启动定时器1								
   ES=1;              //开串口中断
   EA=1;              //开总中断   
   REN=1;    
}

void sendchar(unsigned char d)		  //发送一个字节的数据，形参d即为待发送数据。
{
 
 SBUF=d; //将数据写入到串口缓冲
 sending=1;	 //设置发送标志
 while(sending); //等待发送完毕
}

void sendstring(unsigned char * pd)		    //发送一个字符串，当遇到'/0'结束发送。
{
 while((*pd)!='\0') 
 {
  sendchar(*pd); //发送一个字符
  pd++;  //移动到下一个字符
 }
}

/*******************************************************************************
* 函 数 名         : KeyDown
* 函数功能		   : 检测有按键按下并读取键值
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void KeyDown(void)
{
	char a=0;
	GPIO_KEY=0x0f;
	if(GPIO_KEY!=0x0f)//读取按键是否按下
	{
		delay(1000);//延时10ms进行消抖
		if(GPIO_KEY!=0x0f)//再次检测键盘是否按下
		{	
			//测试列
			GPIO_KEY=0X0F;
			switch(GPIO_KEY)
			{
				case(0X07):	KeyValue=0;break;		 //0111
				case(0X0b):	KeyValue=1;break;		 //1011
				case(0X0d): KeyValue=2;break;		 //1101
				case(0X0e):	KeyValue=3;break;		 //1110
			}
			//测试行
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
				case(0): station=0;figup=1;break; //	上行播报
				case(1): station=7;figup=0;break; //	下行播报
				case(2): sendchar(staname[stationow]);
		                 sendstring(longstring2);
				         sendstring(longstring3); break;				 //	重复播报
				case(3): station=0;figup=1;
//				         add_year_g=0; add_year_s=0;
//						 add_month_g=0; add_month_s=0;
//						 add_day_g=0;add_day_s=0;
//						 add_hour_g=0; add_hour_s=0;
//						 add_minut_g=0; add_minut_s=0;
//						 add_week_g=0;
                         fig_retime=0;
						 break;		       	 //	初始站点

				case(4):fig_year_add=0;led=0;break;				 // 年++
				case(5):fig_year_dec=0;led=0;break;		 //年--
				case(6):fig_month_add=0;led=0;break;		 //月++
				case(7):fig_month_dec=0;led=0;break;		 //月--

				case(8):fig_day_add=0;led=0;break;		 //日++
				case(9):fig_day_dec=0;led=0;break;		 //日--
				case(10):fig_hour_add=0;led=0;break;		 //时++
				case(11):fig_hour_dec=0;led=0;break;		 //时--

				case(12):fig_minut_add=0;led=0;break;		 //分++
				case(13):fig_minut_dec=0;led=0;break;		 //分--
				case(14):fig_week_add=0;led=0;break;		 //周++
				case(15):fig_week_dec=0;led=0;break;		 //周--

	    
		    	}	
			while((a<50)&&(GPIO_KEY!=0xf0))	 //检测按键松手检测,当后续有程序的时候，只有当手离开按键后，后续的程序才会接着执行。
		    	{
				delay(1000);
				a++;
		    	}
		}
	}
}
/*******************************************************************************
* 函 数 名       : main
* 函数功能		 : 主函数
* 输    入       : 无
* 输    出    	 : 无
*******************************************************************************/
void main()
{
// u8 t;
unsigned char show_year[5];
unsigned char show_month[3];
unsigned char show_day[3];
unsigned char show_h_m_s[9];
unsigned char show_week[2];
unsigned char show_tosp[2];		 //当前到站
unsigned char show_nxsp[2];		//下一站
 OLED_Init();			//初始化OLED  
 OLED_Clear(); 
 Delay_xms(50);//等待系统稳定
 ds1302_init(); //DS1302初始化
 Uart_init(2); //波特率初始化为9600
 Delay_xms(10);
 ds1302_write_time(); //写入初始值

//	UsartInit();  //	串口初始化
//	LSA=0; //给一个数码管提供位选
//	LSB=0;
//	LSC=0;
	while(1)
	{
	      KeyDown();		   //按键判断函数
		  ds1302_read_time();  //读取时间 
		  //年
		  if(fig_year_add==0 && KeyValue==4)
		   {
		     add_year_g=add_year_g+1; //分离出年个位 	  自增
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
		     add_year_g=add_year_g-1; //分离出年个位 	自减
			 fig_year_dec=1; 
			 led=1;
			 if((time_buf[1]&0x0F)+add_year_g==0XFF)
			   {
			     add_year_s=add_year_s-1;
			     add_year_g=9;	
			   }
		   }
		   //月
		   if(fig_month_add==0 && KeyValue==6)
		   {
		     add_month_g=add_month_g+1; //分离出月个位 	  自增
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
		     add_month_g=add_month_g-1; //分离出月个位 		自减
			 fig_month_dec=1; 
			 led=1;
			 if((time_buf[2]&0x0F)+add_month_g==0XFF)
			   {
			     add_month_s=add_month_s-1;
			     add_month_g=9;	
			   }
		   }  

		   //日
		   if(fig_day_add==0 && KeyValue==8)
		   {
		     add_day_g=add_day_g+1; //分离出日个位 	  自增
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
		     add_day_g=add_day_g-1; //分离出日个位 		自减
			 fig_day_dec=1; 
			 led=1;
			 if((time_buf[3]&0x0F)+add_day_g==0XFF)
			   {
			     add_day_s=add_day_s-1;
			     add_day_g=9;	
			   }
		   }  

		   //时
		   if(fig_hour_add==0 && KeyValue==10)
		   {
		     add_hour_g=add_hour_g+1; //分离出时个位 	  自增
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
		     add_hour_g=add_hour_g-1; //分离出时个位 		自减
			 fig_hour_dec=1; 
			 led=1;
			 if((time_buf[4]&0x0F)+add_hour_g==0XFF)
			   {
			     add_hour_s=add_hour_s-1;
			     add_hour_g=9;	
			   }
		   }  
		   //分
		   if(fig_minut_add==0 && KeyValue==12)
		   {
		     add_minut_g=add_minut_g+1; //分离出分个位 	  自增
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
		     add_minut_g=add_minut_g-1; //分离出分个位 		自减
			 fig_minut_dec=1; 
			 led=1;
			 if((time_buf[5]&0x0F)+add_minut_g==0xff)
			   {
			     add_minut_s=add_minut_s-1;
			     add_minut_g=9;	
			   }
		   }  
		   //周
		   if(fig_week_add==0 && KeyValue==14)
		   {
		     add_week_g=add_week_g+1; //分离出周个位 	  自增
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
		     add_week_g=add_week_g-1; //分离出周个位 		自减
			 fig_week_dec=1; 
			 led=1;
			 if((time_buf[7]&0x07)+add_week_g==0)
			   {
//			     add_week_s=add_week_s-1;
			     add_minut_g=7;	
			   }
		   } 


		  readtime[0]=(time_buf[0]>>4)+'0';   //分离出年千位
		  show_year[0]=readtime[0];
		  readtime[1]=(time_buf[0]&0x0F)+'0'; //分离出年百位 
 		  show_year[1]=readtime[1];
		  readtime[2]=(time_buf[1]>>4)+'0'+add_year_s;   //分离出年十位
		  show_year[2]=readtime[2];
		  readtime[3]=(time_buf[1]&0x0F)+'0'+add_year_g; //分离出年个位
		  show_year[3]=readtime[3];		
		  show_year[4]='\0';

		  time_buf[1]=(readtime[2]-'0')<<4+(readtime[3]-'0');

		  readtime[4]=(time_buf[2]>>4)+'0'+add_month_s;   //分离出月十位
		  show_month[0]=readtime[4];
		  readtime[5]=(time_buf[2]&0x0F)+'0'+add_month_g; //分离出月个位 
		  show_month[1]=readtime[5];
		  show_month[2]='\0';

		  time_buf[2]=(readtime[4]-'0')<<4+(readtime[5]-'0');
		
		  readtime[6]=(time_buf[3]>>4)+'0'+add_day_s;   //分离出日十位
		  show_day[0]=readtime[6];
		  readtime[7]=(time_buf[3]&0x0F)+'0'+add_day_g; //分离出日个位
		  show_day[1]=readtime[7]; 
		  show_day[2]='\0';

		  time_buf[3]=(readtime[6]-'0')<<4+(readtime[7]-'0');

		  readtime[14]=(time_buf[7]&0x07)+'0'+add_week_g; //星期
          show_week[0]=readtime[14];
		  show_week[1]='\0';
		  
		  time_buf[7]=(readtime[14]-'0');		  		

		  readtime[8]=(time_buf[4]>>4)+'0'+add_hour_s;   //分离出小时十位
		  show_h_m_s[0]=readtime[8];
		  readtime[9]=(time_buf[4]&0x0F)+'0'+add_hour_g; //分离出小时个位
		  show_h_m_s[1]=readtime[9];
		  
		  time_buf[4]=(readtime[8]-'0')<<4+(readtime[9]-'0');		  
		  		  
		  show_h_m_s[2]=':';		   
		  readtime[10]=(time_buf[5]>>4)+'0'+add_minut_s;   //分离出分钟十位
		  show_h_m_s[3]=readtime[10];

		  readtime[11]=(time_buf[5]&0x0F)+'0'+add_minut_g; //分离出分钟个位
		  show_h_m_s[4]=readtime[11];
		  show_h_m_s[5]=':';

		  time_buf[5]=(readtime[10]-'0')<<4+(readtime[11]-'0');

		  readtime[12]=(time_buf[6]>>4)+'0';   //分离出秒钟十位
		  show_h_m_s[6]=readtime[12];
		  readtime[13]=(time_buf[6]&0x0F)+'0'; //分离出秒钟个位 
		  show_h_m_s[7]=readtime[13];
		  show_h_m_s[8]='\0';		  		    
		  readtime[15]='\0'; //分离出秒钟个位   

//		if(fig_retime==0)
//		{
//           ds1302_init(); //DS1302初始化
//           ds1302_write_time(); //写入初始值
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
//请依次从后门下车下车注意安全
//		delay_ms(100);
		OLED_ShowCHinese(0,4,24);//请				 
		OLED_ShowCHinese(18,4,25);//依
		OLED_ShowCHinese(36,4,26);//次
		OLED_ShowCHinese(54,4,27);//从
		OLED_ShowCHinese(72,4,28);//后
		OLED_ShowCHinese(90,4,29);//门
		OLED_ShowCHinese(108,4,30);//下
		OLED_ShowCHinese(0,6,31);//车				 
		OLED_ShowCHinese(18,6,15);//请
		OLED_ShowCHinese(36,6,20);//注
		OLED_ShowCHinese(54,6,21);//意
		OLED_ShowCHinese(72,6,22);//安
		OLED_ShowCHinese(90,6,23);//全
//	   	delay_ms(100);
			 								 
		 while(Gred==0);
//车辆起步请扶好站好注意安全
		OLED_ShowCHinese(0,4,11);//车				 
		OLED_ShowCHinese(18,4,12);//辆
		OLED_ShowCHinese(36,4,13);//起
		OLED_ShowCHinese(54,4,14);//步
		OLED_ShowCHinese(72,4,15);//请
		OLED_ShowCHinese(90,4,16);//扶
		OLED_ShowCHinese(108,4,17);//好
		OLED_ShowCHinese(0,6,18);//站				 
		OLED_ShowCHinese(18,6,19);//好
		OLED_ShowCHinese(36,6,20);//注
		OLED_ShowCHinese(54,6,21);//意
		OLED_ShowCHinese(72,6,22);//安
		OLED_ShowCHinese(90,6,23);//全
		delay_ms(50);
		//请主动给老弱病残让座
	    OLED_ShowCHinese(0,4,32);//请				 
		OLED_ShowCHinese(18,4,33);//主
		OLED_ShowCHinese(36,4,34);//动
		OLED_ShowCHinese(54,4,35);//给
		OLED_ShowCHinese(72,4,36);//老
		OLED_ShowCHinese(90,4,37);//弱
		OLED_ShowCHinese(108,4,38);//病
		OLED_ShowCHinese(0,6,39);//残						 
		OLED_ShowCHinese(18,6,40);//等
		OLED_ShowCHinese(36,6,41);//乘
		OLED_ShowCHinese(54,6,42);//客
		OLED_ShowCHinese(72,6,43);//让
		OLED_ShowCHinese(90,6,44);//座
//		delay_ms(100);	
				 sendstring(longstring1); 
				 sendchar('\n');
		   }
		}


//		sendstring(readtime);
//        sendchar(KeyValue);
//		sendstring("aaaa");

//		GPIO_DIG=smgduan[KeyValue];	  //	 此处可以根据键值的不同进行独立功能的开发 
//		sendc();
		OLED_Clear();
	    OLED_ShowString(0,0,show_year,12); 
		OLED_ShowCHinese(36,0,7);//年	 四位
	    OLED_ShowString(54,0,show_month,12); 
		OLED_ShowCHinese(72,0,8);//月		 两位
	    OLED_ShowString(90,0,show_day,12); 
		OLED_ShowCHinese(108,0,9);//日		   两位
		OLED_ShowCHinese(0,2,10);//周
	    OLED_ShowString(18,2,show_week,12); 	 //星期二  一位
	   	OLED_ShowString(36,2,show_h_m_s,12);  //时分秒 共8位

		OLED_ShowString(36,3,tostation,12); 
	    OLED_ShowString(60,3,show_tosp,12); 
		OLED_ShowString(72,3,nextstation,12); 
	    OLED_ShowString(112,3,show_nxsp,12); 

////请依次从后门下车下车注意安全
////		delay_ms(100);
//		OLED_ShowCHinese(0,4,24);//请				 
//		OLED_ShowCHinese(18,4,25);//依
//		OLED_ShowCHinese(36,4,26);//次
//		OLED_ShowCHinese(54,4,27);//从
//		OLED_ShowCHinese(72,4,28);//后
//		OLED_ShowCHinese(90,4,29);//门
//		OLED_ShowCHinese(108,4,30);//下
//		OLED_ShowCHinese(0,6,31);//车				 
//		OLED_ShowCHinese(18,6,15);//请
//		OLED_ShowCHinese(36,6,20);//注
//		OLED_ShowCHinese(54,6,21);//意
//		OLED_ShowCHinese(72,6,22);//安
//		OLED_ShowCHinese(90,6,23);//全
////	   	delay_ms(100);
////请主动给老弱病残让座
//	    OLED_ShowCHinese(0,4,32);//请				 
//		OLED_ShowCHinese(18,4,33);//主
//		OLED_ShowCHinese(36,4,34);//动
//		OLED_ShowCHinese(54,4,35);//给
//		OLED_ShowCHinese(72,4,36);//老
//		OLED_ShowCHinese(90,4,37);//弱
//		OLED_ShowCHinese(108,4,38);//病
//		OLED_ShowCHinese(0,6,39);//残						 
//		OLED_ShowCHinese(18,6,40);//等
//		OLED_ShowCHinese(36,6,41);//乘
//		OLED_ShowCHinese(54,6,42);//客
//		OLED_ShowCHinese(72,6,43);//让
//		OLED_ShowCHinese(90,6,44);//座
////		delay_ms(100);
	}		
}

/*******************************************************************************
* 函数名         : Usart() interrupt 4
* 函数功能		  : 串口通信中断函数
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
void Usart() interrupt 4
{

  if(RI)    //收到数据
  {
     RI=0;   //清中断请求
  }
 else      //发送完一字节数据
  {
    TI=0;
    sending=0;  //清正在发送标志
  }
}