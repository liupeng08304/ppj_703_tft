//Copyright (c) 2016
//OLED Driver

#include "app_oled_driver.h"
//#include "font.h"
#include <string.h>
#include "utility.h"
#include "stdio.h"
#include "font_band.h"

#define OLED_LIGHT  0x95

void Deactivate_Scroll(void);
void Horizontal_Scroll(unsigned char left, unsigned char Start_Page, unsigned char End_Page, unsigned char Interval);

/***********************************************************************
//SPI function
************************************************************************/
static void spi_pin_init(void)
{
    nrf_gpio_cfg_output(OLED_SPI_SCK);     //推挽输出
    nrf_gpio_cfg_output(OLED_SPI_MOSI);    //推挽输出
    nrf_gpio_cfg_output(OLED_SPI_MISO);    //推挽输出
    //nrf_gpio_cfg_input(spi_miso,GPIO_PIN_CNF_PULL_Pullup);//设置管脚位上拉输入
    nrf_gpio_cfg_output(OLED_SPI_CS);  //推挽输出
    nrf_gpio_cfg_output(OLED_RESET);   //推挽输出

    nrf_gpio_pin_set(OLED_SPI_CS);
}

static void spi_r_datcmd(unsigned char dat,unsigned char cmd)     //spi写cmd:0命令 1：数据
{
    unsigned char i;			 //data接受数据
    if(cmd)
        OLED_DC_Set();    //命令  dc=1
    else
        OLED_DC_Clr();		  //数据  dc=0
    OLED_CS_Clr();         //片选 cs=0
    for(i=0; i<8; i++)
    {
        OLED_SCLK_Clr();       //clk=0   时钟
        if(dat&0x80)
            OLED_SDIN_Set();
        else
            OLED_SDIN_Clr();
        OLED_SCLK_Set();       //clk=1
        dat<<=1;
    }
    OLED_CS_Set();          //片选cs=1
    OLED_DC_Set();   	       //dc=1
}


void writedata(unsigned char datatowrite)//写数据
{
    spi_r_datcmd(datatowrite,1);
}
/************************************************************
OLED DRIVER
*************************************************************/


//96*32
void oled_set_pos(unsigned char x, unsigned char y)
{
//#if(OLED_X_SIZE==128)
//    spi_r_datcmd(0xb0+y,OLED_CMD);//b0 ae
//    spi_r_datcmd((((x+0)&0xf0)>>4)|0x10,OLED_CMD);
//    spi_r_datcmd((x&0x0f),OLED_CMD);
//	#else
////    spi_r_datcmd(0xae+y+2,OLED_CMD);
////    spi_r_datcmd((((x+0x10)&0xf0)>>4)|0x10,OLED_CMD);
////    spi_r_datcmd((x&0x0f)|0x01,OLED_CMD);
//
//	   spi_r_datcmd(0xb0+y+2,OLED_CMD);//b0 ae
//    spi_r_datcmd((((x+0)&0xf0)>>4)|0x10,OLED_CMD);
//    spi_r_datcmd((x&0x0f),OLED_CMD);
//
//	#endif
    uint16_t x0,y0;
    x0=x+X_OFFSET;
    y0=y+Y_OFFSET;
    spi_r_datcmd(0xb0+y0,OLED_CMD);//b0 ae
    spi_r_datcmd(((x0&0xf0)>>4)|0x10,OLED_CMD);
    spi_r_datcmd((x0&0x0f),OLED_CMD);



}



static uint8_t led_on_flg=0;
void oled_display_on(void)          //开启oled
{
    spi_r_datcmd(0X8D,OLED_CMD);  //SET DCDC
    if((time.hour>7)&&(time.hour<19))
        spi_r_datcmd(0X95,OLED_CMD);  //DCDC ON
    else
        spi_r_datcmd(0X14,OLED_CMD);  //DCDC ON

    spi_r_datcmd(0XAF,OLED_CMD);  //DISPLAY ON
    led_on_flg=1;
}
void oled_display_off(void)     //关闭oled
{
    spi_r_datcmd(0X8D,OLED_CMD);  //SET DCDC
    spi_r_datcmd(0X10,OLED_CMD);  //DCDC OFF
    spi_r_datcmd(0XAE,OLED_CMD);  //DISPLAY OFF
    led_on_flg=0;
}

uint8_t oled_power_read(void)
{
    return led_on_flg;
}
uint8_t OLED_GRAM[8][128];

//void oled_fill(bool clear_flg)     //清屏函数
//{   //clear all 96*32
//    unsigned char i,n;
//    for(i=0; i<8; i++)
//    {
//        spi_r_datcmd (0xae+i,OLED_CMD);    //(0~7)
//        spi_r_datcmd (0x00,OLED_CMD);
//        spi_r_datcmd (0x10,OLED_CMD);
//        for(n=0; n<128; n++)
//			{
//				if(true==clear_flg)
//				{
//					spi_r_datcmd(0,OLED_DATA);
//					OLED_GRAM[i][n]=0;
//				}
//				else
//				{
//				spi_r_datcmd(OLED_GRAM[i][n],OLED_DATA);
//				}
//			}
//    }
//}


void oled_set_brightness(uint8_t value)
{
    spi_r_datcmd(0x81,OLED_CMD);//--set contrast control register 0x81
    spi_r_datcmd(value,OLED_CMD); // Set SEG Output Current Brightness CF
}

/*********************************************************************
* @fn      ss1306SetMirror
*
* @brief   set Mirrored display
*
* @param   en --1 enable Mirror , 0 resume normal display
*
* @return  None
*/
void ss1306SetMirror(uint8_t en)
{
    // the scan direction has been remapped in the oled design, so need reverse the setting here
    if(en == 0)
        en = 1;
    else
        en = 0 ;
    spi_r_datcmd((0xC0|(en<<3)),OLED_CMD);
    spi_r_datcmd((0xA0|en),OLED_CMD);
}


void oled_init(void)
{
    spi_pin_init();

    OLED_RST_Set();
    delay_ms(100);
    OLED_RST_Clr();
    delay_ms(100);
    OLED_RST_Set();

    spi_r_datcmd(0xAE,OLED_CMD);//--turn off oled panel

    spi_r_datcmd(0x00,OLED_CMD);//---set low column address 00
    spi_r_datcmd(0x10,OLED_CMD);//---set high column address 10

    spi_r_datcmd(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    spi_r_datcmd(0x81,OLED_CMD);//--set contrast control register 0x81
    spi_r_datcmd(0xff,OLED_CMD); // Set SEG Output Current Brightness CF

    spi_r_datcmd(0xA0,OLED_CMD);//--Set SEG/Column Mapping     0xa0???? 0xa1??
    spi_r_datcmd(0xC0,OLED_CMD);//Set COM/Row Scan Direction   0xc0???? 0xc8??




    spi_r_datcmd(0xA6,OLED_CMD);//--set normal display A6
    spi_r_datcmd(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    spi_r_datcmd(0x3f,OLED_CMD);//--1/64 duty
    spi_r_datcmd(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)0xD3
    spi_r_datcmd(0x00,OLED_CMD);//-not offset

    spi_r_datcmd(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    spi_r_datcmd(0xf0,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec

    spi_r_datcmd(0xD9,OLED_CMD);//--set pre-charge period
    spi_r_datcmd(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    spi_r_datcmd(0xDA,OLED_CMD);//--set com pins hardware configuration
    spi_r_datcmd(0x12,OLED_CMD);
    spi_r_datcmd(0xDB,OLED_CMD);//--set vcomh
    spi_r_datcmd(0x30,OLED_CMD);//Set VCOM Deselect Level  40
    spi_r_datcmd(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    spi_r_datcmd(0x02,OLED_CMD);//
    spi_r_datcmd(0x8D,OLED_CMD);//--set Charge Pump enable/disable
//    spi_r_datcmd(OLED_LIGHT,OLED_CMD);//--set(0x10) disable  0x95  0x14
    if((time.hour>7)&&(time.hour<19))
        spi_r_datcmd(0X95,OLED_CMD);  //DCDC ON
    else
        spi_r_datcmd(0X14,OLED_CMD);  //DCDC ON

    spi_r_datcmd(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    spi_r_datcmd(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7)
    spi_r_datcmd(0xAF,OLED_CMD);//--turn off oled panel
    oled_clear();
    oled_display_off();
#ifdef TRUN_180_EN
    ss1306SetMirror(0);
#endif
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Continuous Horizontal Scrolling (Partial or Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Rightward)
//       "0x01" (Leftward)
//    b: Define Start Page Address
//    c: Define End Page Address
//    d: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
//    e: Delay Time
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Horizontal_Scroll(unsigned char left, unsigned char Start_Page, unsigned char End_Page, unsigned char Interval)
{
#define   Write_Command(A) spi_r_datcmd(A,OLED_CMD);

//			x0=x+X_OFFSET;
//		y0=y+Y_OFFSET;


    oled_set_pos(28,2);
    Write_Command(0x26|left);			// Horizontal Scroll Setup
    Write_Command(0x00);			//           => (Dummy Write for First Parameter)
    Write_Command(Start_Page+Y_OFFSET);
    Write_Command(Interval);
    Write_Command(End_Page+Y_OFFSET);
    Write_Command(0x00);			// Activate Scrolling
    //Write_Command(0x26|a);			// Horizontal Scroll Setup
    Write_Command(0x7f);			//           => (Dummy Write for First Parameter)
    //Write_Command(b);
    //Write_Command(d);
    //Write_Command(c);
    Write_Command(0x2F);			// Activate Scrolling

}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Continuous Vertical / Horizontal / Diagonal Scrolling (Partial or Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Vertical & Rightward)
//       "0x01" (Vertical & Leftward)
//    b: Define Start Row Address (Horizontal / Diagonal Scrolling)
//    c: Define End Page Address (Horizontal / Diagonal Scrolling)
//    d: Set Top Fixed Area (Vertical Scrolling)
//    e: Set Vertical Scroll Area (Vertical Scrolling)
//    f: Set Numbers of Row Scroll per Step (Vertical / Diagonal Scrolling)
//    g: Set Time Interval between Each Scroll Step in Terms of Frame Frequency
//    h: Delay Time
//    * d+e must be less than or equal to the Multiplex Ratio...
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Continuous_Scroll(uint8_t left, uint8_t start_x,\
                       uint8_t end_page, uint8_t d,\
                       uint8_t e, uint8_t f, uint8_t inter)
{

    Write_Command(0xA3);			// Set Vertical Scroll Area
    Write_Command(d);			//   Default => 0x00 (Top Fixed Area)
    Write_Command(e);			//   Default => 0x40 (Vertical Scroll Area)

    Write_Command(0x29+left);			// Continuous Vertical & Horizontal Scroll Setup
    Write_Command(0x00);			//           => (Dummy Write for First Parameter)
    Write_Command(start_x);
    Write_Command(inter);
    Write_Command(end_page);
    Write_Command(f);
    Write_Command(0x2F);			// Activate Scrolling
//	Delay(h);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Deactivate Scrolling (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Deactivate_Scroll(void)
{
    Write_Command(0x2E);			// Deactivate Scrolling
}




void FillArea(uint16_t startx,uint16_t start_y,uint16_t xsize,uint16_t ysize)
{
    uint16_t y,x,pgs,st_page;



    st_page=start_y/8;
    pgs=ysize/8;

    if( (start_y%8)&&((st_page+pgs)<(OLED_Y_SIZE/8)))
    {
        pgs++;
    }

    for(y=0; y<pgs; y++)
    {
        oled_set_pos(startx,st_page+y);
        for(x=0; x<xsize; x++)
        {
            writedata(OLED_GRAM[st_page+y][startx+x]);
        }
    }

}

void oled_clear(void)
{
    uint8_t i,j;

    Deactivate_Scroll();
//
//    for(i=0; i<(OLED_Y_SIZE/8); i++)
//        for(j=0; j<OLED_X_SIZE; j++)
//            OLED_GRAM[i][j]=0;
//    FillArea(0,0,OLED_X_SIZE,OLED_Y_SIZE);


    for(i=0; i<8; i++)
        for(j=0; j<128; j++)
#ifdef	REVERSE_OLED
            OLED_GRAM[i][j]=0xff;
#else
            OLED_GRAM[i][j]=0;
#endif
    FillArea(0,0,128,64);
}



void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t)
{

    unsigned char pos,bx,temp=0;
    if(x>127||y>(64+7))
    {
        temp=2;
        return;
    }
    pos=y/8;
    bx=y%8;
    temp=(1<<(bx));

#ifdef  REVERSE_OLED
    if(t)OLED_GRAM[pos][x]&=~temp;
    else OLED_GRAM[pos][x]|=temp;
#else
    if(t)OLED_GRAM[pos][x]|=temp;
    else OLED_GRAM[pos][x]&=~temp;
#endif
}


void Show_icon(uint16_t startx,uint16_t starty,uint16_t xszie,uint16_t ysize,uint8_t* icon)
{

    //逐行扫描
//	uint16_t i,k,x0,y0,start=0;
//	uint8_t m;
//
//	for(i = 0;i < ysize;i++)
//	{
//		y0 = starty+i;
//
//		for(k = 0;k < xszie;k++)
//		{
//		  if(0==(k%8))
//		  {
//				m = icon[start++];
//		  }
//			x0 = startx +k;
////			OLED_DrawPoint(x0,y0,(m&0x80));
////			m<<=1;
//			OLED_DrawPoint(x0,y0,(m&0x01));
//			m>>=1;
//		}
//	}

//列行扫描
    uint16_t l_y,l_num,x0,y0,m,x,data_index=0,bit;
    l_num=ysize/8;
    if(ysize%8)
        l_num++;

    ysize=8*l_num;
    y0=starty;
    x0=startx;
    for(l_y=0; l_y<l_num; l_y++)
    {
        for(x=0; x<xszie; x++)
        {
            m=icon[data_index++];
            for(bit=0; bit<8; bit++)
            {
                OLED_DrawPoint(x0+x,y0+8*l_y+bit,(m&0x01));
                m>>=1;
            }
        }
    }

    FillArea(startx,starty,xszie,ysize);

}


void ShowString(unsigned char* front,uint16_t xstart,uint16_t ystart,uint16_t xsize,uint16_t ysize,uint8_t *data,uint8_t len)
{
    uint8_t i;
    uint16_t position;
    position=xstart;
    for(i=0; i<len; i++)
    {
        ///ASCII_8X12

        if(BIG_NUM[0]==front)
        {
            Show_icon(position,ystart,xsize,ysize,front+(data[i]-0x30)*BIG_NUM_SIZE);
        } else if(num6X12[0]==front)
        {
            if(data[i]==0x20)
                Show_icon(position,ystart,xsize,ysize,front+(data[i]-0x20+10)*12);
            else
                Show_icon(position,ystart,xsize,ysize,front+(data[i]-0x30)*12);


        } else 				if( (data[i]>=0x20)&&(data[i]<='z'))
            Show_icon(position,ystart,xsize,ysize,front+(data[i]-0x20)*16);

        position+=xsize;

    }
}

void oled_show_version(void)
{

    uint8_t buf[20];
    uint8_t len;
    len=(uint8_t)sprintf((char*)buf,"V%d.%d.%d",version[0],version[1],version[2]);
    ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-8*len)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)buf,len);
}


void oled_show_boot_info(void)
{


    Show_icon((OLED_X_SIZE-power_on_icon_x)/2,(OLED_Y_SIZE-power_on_icon_y)/2,power_on_icon_x,power_on_icon_y,(uint8_t*)power_on_icon);
//    ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-8*7)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)"iFit360",7);
}





void oled_show_battery_charging(unsigned char battery)
{
    //battery
    Show_icon((OLED_X_SIZE-34)/2,(OLED_Y_SIZE-16)/2,34,16,(unsigned char*)&battery34_16[(34*16/8)*battery]);

}





void ShowIconAndNum(uint8_t* icon,int num)
{

    uint8_t buf[6];
    uint8_t len;

    if( (icon!=icon16_16[15])||(num==0))  //升级的时候只有第0包才刷新icon
    {
        Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(unsigned char*)icon);
    }
    if(num>=100000)
        num%=100000;
    len=sprintf((char*)buf,"%d",num);
    ShowString((unsigned char*)BIG_NUM[0],(OLED_X_SIZE-len*BIG_NUM_X)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-BIG_NUM_Y)/2,BIG_NUM_X,BIG_NUM_Y,(uint8_t*)buf,len);


}


void ShowNotice(uint8_t icon_num,uint8_t *msg,uint8_t msglen)
{

    if(icon_num>10) return;/*超出通知的范围*/
    Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(uint8_t*)&icon16_16[icon_num][0]);



    if((msglen*8)>OLED_X_SIZE)
    {
        ShowString((unsigned char*)ASCII_8X16[0],0,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-16)/2,8,16,msg,msglen);
        Horizontal_Scroll(1,OLED_Y_SIZE/2/8,OLED_Y_SIZE/8,20);
    } else
    {
        ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-msglen*8)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-16)/2,8,16,msg,msglen);
    }
}


//Show step
void oled_show_step(unsigned int num)
{
    ShowIconAndNum((uint8_t*)&icon16_16[11][0],num);
}

//Show Hr
void oled_show_HR(uint8_t num,uint8_t a_b)
{
    static uint8_t numb;
    uint8_t buf[32];
    uint8_t len,i;
    if(a_b)
    {
        if(a_b==2)
        {
            numb=200;
        }
        Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(uint8_t*)icon16_16[12]);
    } else
    {
        for(i=0; i<32; i++)
            buf[i]=0;
        Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,buf);
    }

    if(numb!=num)
    {
        numb=num;
        if(num==0)
        {
            len=2;
            ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-16)/2,8,16,(uint8_t*)"--",len);
        }
//			else if(num==200)
//			{
//			len=2;
//			ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-16)/2,8,16,(uint8_t*)"XX",len);
//			}
        else
        {
            len=sprintf((char*)buf,"%d",num);
            ShowString((unsigned char*)BIG_NUM[0],(OLED_X_SIZE-len*BIG_NUM_X)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-BIG_NUM_Y)/2,BIG_NUM_X,BIG_NUM_Y,(uint8_t*)buf,len);
        }
    }


}
//void oled_show_HR(uint8_t num,uint8_t a_b)
//{
//	  static uint8_t numb;
//	  uint8_t buf[32];
//		uint8_t len,i;
//		if(a_b)
//		{
//			if(a_b==2)
//			{
//			numb=200;
//			}
//			Show_icon(OLED_X_SIZE-20+2,(OLED_Y_SIZE/2-16)/2,16,16,(uint8_t*)icon16_16[12]);
//		}else
//		{
//		for(i=0;i<32;i++)
//			buf[i]=0;
//		Show_icon(OLED_X_SIZE-20+2,(OLED_Y_SIZE/2-16)/2,16,16,buf);
//		}
//
//	  if(numb!=num)
//		{
//			numb=num;
//			if(num==0)
//			{
//			len=2;
//			ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-20-len*8)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)"--",len);
//			}else
//			{
//			len=sprintf((char*)buf,"%d",num);
//			ShowString((unsigned char*)BIG_NUM[0],(OLED_X_SIZE-20-len*BIG_NUM_X)/2,(OLED_Y_SIZE-BIG_NUM_Y)/2,BIG_NUM_X,BIG_NUM_Y,(uint8_t*)buf,len);
//			}
//	}
//
//
//}


#if(BORD&USE_BLOOD)
//Show blood
void oled_show_blood(uint8_t hight,uint8_t low,uint8_t a_b)
{
    static uint16_t numb;
    uint16_t tmp;

    uint8_t buf[32];
    uint8_t len;
    if(a_b)
    {
        if(a_b==2)
        {
            numb=0xffff;
        }
        Show_icon(0,(24-16)/2,16,16,(uint8_t*)blood1616[2]);
        Show_icon(0,OLED_Y_SIZE/2+(24-16)/2,16,16,(uint8_t*)blood1616[0]);

        Show_icon(OLED_X_SIZE-20,(OLED_Y_SIZE/2-20)/2,20,20,(uint8_t*)icon_mmhg2020);
        Show_icon(OLED_X_SIZE-20,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-20)/2,20,20,(uint8_t*)icon_mmhg2020);
    } else
    {

        Show_icon(0,(24-16)/2,16,16,(uint8_t*)blood1616[3]);
        Show_icon(0,OLED_Y_SIZE/2+(24-16)/2,16,16,(uint8_t*)blood1616[1]);
    }

    tmp=hight;
    tmp=(tmp<<8)|low;
    if(numb!=tmp)
    {
        numb=tmp;
        if(tmp==0)
        {
            len=2;
            ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8-22),(OLED_Y_SIZE/2-16)/2,8,16,(uint8_t*)"--",len);
            ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8-22),OLED_Y_SIZE/2+(OLED_Y_SIZE/2-16)/2,8,16,(uint8_t*)"--",len);
        } else
        {
            len=sprintf((char*)buf,"%03d",hight);
            ShowString((unsigned char*)BIG_NUM[0],OLED_X_SIZE-len*BIG_NUM_X-22,(OLED_Y_SIZE/2-BIG_NUM_Y)/2,BIG_NUM_X,BIG_NUM_Y,(uint8_t*)buf,len);


            len=sprintf((char*)buf,"%d",low);
            ShowString((unsigned char*)BIG_NUM[0],OLED_X_SIZE-len*BIG_NUM_X-22,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-BIG_NUM_Y)/2,BIG_NUM_X,BIG_NUM_Y,(uint8_t*)buf,len);

        }

    }


}
#endif
//Show calories
void oled_show_calories(unsigned int num)
{
    ShowIconAndNum((uint8_t*)&icon16_16[13][0],num);
}

//Show calories
void oled_show_sporttime(unsigned int min)
{

    uint8_t t_x,t_y;
    Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(uint8_t*)&icon_1616_alarm[8][0]);

    //len=sprintf((char*)buf,"%02dh:%02dm",min/60,min%60);
//		ShowString((unsigned char*)num11_20[0],(OLED_X_SIZE-len*11)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-20)/2,11,20,(uint8_t*)buf,len);
    //	ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-8*len)/2,OLED_Y_SIZE/2+(OLED_Y_SIZE/2-16)/2,8,16,(uint8_t*)buf,len);


    t_x=(OLED_X_SIZE-(BIG_NUM_SIZE+4))/2;
    t_y=OLED_Y_SIZE/2+(OLED_Y_SIZE/2-BIG_NUM_Y)/2;
    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[min/60%10][0]);
    t_x+=BIG_NUM_X;
    Show_icon(t_x,t_y,4,20,(unsigned char*)&dot4_20[0]);
    t_x+=4;
    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[min%60/10][0]);
    t_x+=BIG_NUM_X;
    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[min%60%10][0]);
}

//Show upgrade
void oled_show_upgrade(unsigned int num)
{
    ShowIconAndNum((uint8_t*)&icon16_16[15][0],num);


}


//void oled_show_ansy(void)
//{
//		oled_clear();
//		oled_display_on();
//		Show_icon((OLED_X_SIZE-24)/2,(OLED_Y_SIZE-24)/2,24,24,(unsigned char*)icon_ansy_2424);


//}

//void oled_show_system(uint8_t perseng,uint8_t *version,uint16_t hr_v)
//{
//	uint8_t buf[20],len;
////	oled_clear();

//	len=sprintf((char*)buf,"vol:%d",perseng);
//	ShowString((unsigned char*)ASCII_8X16[0],0,0,8,16,buf,len);
//		len=sprintf((char*)buf,"%02x-%02x-%02x",version[0],version[1],version[2]);
//	ShowString((unsigned char*)ASCII_8X16[0],0,16,8,16,buf,len);
//		len=sprintf((char*)buf,"h_r:%d",hr_v);
//	ShowString((unsigned char*)ASCII_8X16[0],0,32,8,16,buf,len);
//
//
//
//}
//Show Distance
void oled_show_distance(unsigned int num)
{
//ShowIconAndNum(,num/10);

    uint8_t buf[6];
    uint8_t len,startx,starty;
    uint8_t point;
    Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(uint8_t*)&icon16_16[14][0]);


    //point3_2
    point=num%10;
    num=num/10;
    if(num>=1000)
        num%=1000;
    len=sprintf((char*)buf,"%d",num);
    startx=(OLED_X_SIZE-(len+1)*BIG_NUM_X-3)/2;
    starty=OLED_Y_SIZE/2+(OLED_Y_SIZE/2-BIG_NUM_Y)/2;
    ShowString((unsigned char*)BIG_NUM[0],startx,starty,BIG_NUM_X,BIG_NUM_Y,(uint8_t*)buf,len);
    Show_icon(startx+len*BIG_NUM_X,starty+BIG_NUM_Y-2,3,2,(uint8_t*)&point3_2);
    Show_icon(startx+len*BIG_NUM_X+3,starty,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[point][0]);

}


void oled_show_result(uint8_t result)
{
    uint8_t len;
    oled_clear();
    if(0==result)/*成功*/
    {
        len=3;
        ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)"ok!",len);

        return ;
    } else 	if(1==result)/*time out*/
    {
        len=8;
        ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)"time out",len);

    } else 	if(2==result)/*crc*/
    {
        len=7;
        ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)"crc err",len);

    } else 	if(3==result)/*file err*/
    {
        len=8;
        ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-len*8)/2,(OLED_Y_SIZE-16)/2,8,16,(uint8_t*)"file err",len);

    }
}




//显示时间
void showTime(uint8_t hour,uint8_t min,bool use_am_pm,uint8_t start_y)
{
    //	showTime(tm,use_am_pm,6,8);
    uint8_t t_x=5,t_y,hourdisplay;
    hourdisplay=hour;

    //time  12:34
    t_y=start_y+(OLED_Y_SIZE-start_y-BIG_NUM_Y)/2;
    if(true==use_am_pm)
    {
        if(hourdisplay>12)
        {
            hourdisplay-=12;
        }
        t_x=(OLED_X_SIZE-BIG_NUM_X*4-4-14)/2;//4*11+4+2+12
        t_x+=3;/////////////////////////////////2016-12-14
    }
    else
    {
        t_x=(OLED_X_SIZE-BIG_NUM_X*4-4)/2;
    }

    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[hourdisplay/10][0]);
    t_x+=BIG_NUM_X;
    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[hourdisplay%10][0]);
    t_x+=BIG_NUM_X;
    Show_icon(t_x,t_y,4,20,(unsigned char*)&dot4_20[0]);
    t_x+=4;
    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[min/10][0]);
    t_x+=BIG_NUM_X;
    Show_icon(t_x,t_y,BIG_NUM_X,BIG_NUM_Y,(unsigned char*)&BIG_NUM[min%10][0]);

    t_x-=(BIG_NUM_X*3+4);
    if(true==use_am_pm)
    {
        t_y=t_y+(BIG_NUM_Y-8);
        t_x=OLED_X_SIZE-14;//
        Show_icon(t_x,t_y,12,8,(hour<12)? (unsigned char*)am12_8:(unsigned char*)pm12_8);
    }

}



void oled_show_alarm(uint8_t hour,uint8_t min,bool use_am_pm,uint8_t ararm_type)
{

    Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(unsigned char*)icon_1616_alarm[ararm_type]);
    showTime(hour,min,use_am_pm,OLED_Y_SIZE/2);

}



//Show time
//Charging_State global_device_charging_state;
void oled_show_time(RTC_UTCTimeStruct  tm,unsigned char batter_state, bool my_bond_stat,bool use_am_pm)
{


    uint8_t t_x;

    //data  01-09  9月1日
    t_x=0;
    Show_icon(t_x,2,6,9,(unsigned char*)&num6_9[(tm.day/10)*12]);
    t_x+=6;
    Show_icon(t_x,2,6,9,(unsigned char*)&num6_9[(tm.day%10)*12]);
    t_x+=6;
    Show_icon(t_x,2,6,9,(unsigned char*)&num6_9[10*12]);
    t_x+=6;
    Show_icon(t_x,2,6,9,(unsigned char*)&num6_9[(tm.month/10)*12]);
    t_x+=6;
    Show_icon(t_x,2,6,9,(unsigned char*)&num6_9[(tm.month%10)*12]);
    t_x+=6;

    //battery
    Show_icon(OLED_X_SIZE-18-1,2,18,9,(unsigned char*)&battery18_9[36*batter_state]);

    //ble
    if(true==my_bond_stat)
    {
        Show_icon(30+(OLED_X_SIZE-50-8)/2,2,8,9,(unsigned char*)ble8_9);//num 11为6*8的蓝牙图标
    }

    //time
    showTime(tm.hour,tm.minutes,use_am_pm,8+4);/////////////////////////////////2016-12-14
}




void  show_battery(uint8_t battery)
{
    extern uint8_t mac_data[6];
//	char buf[10],len;
    if(5==battery)
        Show_icon((OLED_X_SIZE-64)/2,(OLED_Y_SIZE-48)/2,64,48,(unsigned char*)&battery5[0]);
    else if(20==battery)
        Show_icon((OLED_X_SIZE-64)/2,(OLED_Y_SIZE-48)/2,64,48,(unsigned char*)&battery20[0]);
    else if(30==battery)
        Show_icon((OLED_X_SIZE-30)/2,(OLED_Y_SIZE-30)/2,30,30,(unsigned char*)&sedentary_icon[0]);
    else if(40==battery)
    {

//		mac_data[4]=0x5d;
//		mac_data[5]=0xef;

        Show_icon((OLED_X_SIZE-16)/2,(OLED_Y_SIZE/2-16)/2,16,16,(uint8_t*)icon_pair16_16);
        //  len=sprintf(buf,"%02x%02x",mac_data[4],mac_data[5]);
        //  ShowString((unsigned char*)ASCII_8X16[0],(OLED_X_SIZE-32)/2,24,8,16,(uint8_t*)buf,len);

        uint8_t t_x=(OLED_X_SIZE-4*11)/2,t_y=OLED_Y_SIZE/2+(OLED_Y_SIZE/2-20)/2,tmp;

        tmp=mac_data[4]>>4;
        Show_icon(t_x,t_y,11,20,(unsigned char*)&icon_num_11_20[tmp][0]);
        t_x+=11;
        tmp=mac_data[4]&0x0f;
        Show_icon(t_x,t_y,11,20,(unsigned char*)&icon_num_11_20[tmp][0]);
        t_x+=11;
        tmp=mac_data[5]>>4;
        Show_icon(t_x,t_y,11,20,(unsigned char*)&icon_num_11_20[tmp][0]);
        t_x+=11;
        tmp=mac_data[5]&0x0f;
        Show_icon(t_x,t_y,11,20,(unsigned char*)&icon_num_11_20[tmp][0]);



    }
}


void GUI_Full(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t colour)
{
    uint8_t i,j;
    for(j = y0; j <= y1; j ++)
        for(i = x0; i <= x1; i ++)
            OLED_DrawPoint(i,j,colour);
}

void GUI_Line(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char colour)
{
    int temp;
    int dx,dy;
    int s1,s2,status,i;
    int Dx,Dy,sub;

    dx = x1 - x0;
    if(dx >= 0)
        s1 = 1;
    else
        s1 = -1;
    dy = y1 - y0;
    if(dy >= 0)
        s2 = 1;
    else
        s2 =-1;


    Dx = (x1>x0)? (x1-x0):(x0-x1) ;//  abs(x1-x0);
    Dy = (y1>y0)? (y1-y0):(y0-y1) ;//abs(y1-y0);

    if(Dy > Dx)
    {
        temp = Dx;
        Dx = Dy;
        Dy = temp;
        status = 1;
    }
    else
        status = 0;


    /*********Bresenham???????????********/
    sub = Dy + Dy - Dx;                 //?1?????????
    for(i = 0; i < Dx; i ++)
    {

        OLED_DrawPoint(x0,y0,colour);           //??
        if(sub >= 0)
        {
            if(status == 1)               //???Y??,x??1
                x0 += s1;
            else                     //???X??,y??1
                y0 += s2;
            sub -= (Dx + Dx);                 //?????????
        }
        if(status == 1)
            y0 += s2;
        else
            x0 += s1;
        sub += Dy + Dy;

    }
}



#include "qrencode.h"

void  DisplayQrCode(uint8_t startx,uint8_t starty,uint8_t*data,uint8_t len)
{
    uint16_t x,y;
//uint8_t	data1,k;
//	uint8_t data2,l;
//	uint8_t buf[WDB*WD];
    uint8_t buftmp[1024];
    strinbuf=buftmp;
    qrframe=(buftmp+270);
    rlens=(qrframe+600);



    memcpy(strinbuf,data,len);
    qrencode();

    oled_display_on();
    oled_clear();

    for (x= 0; x <WD ; x++) //WDB*8
    {
        for (y = 0; y < WD; y++)
            OLED_DrawPoint(startx+x,starty+y,!QRBIT(x,y));
    }
    FillArea(startx,starty,WD,WD);


}


