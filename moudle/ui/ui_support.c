#include "ui_support.h"
#include "hr_app.h"


uint32_t disvalueb=0xffffffff;

static uint32_t get_int(uint8_t a_b_in,uint8_t max_item)
{
	static uint8_t icon_position_cnt=0;
	if(a_b_in==0xff)
	{
	icon_position_cnt=0;
	}
	icon_position_cnt++;
	if(icon_position_cnt>max_item)
	icon_position_cnt=1;

	return icon_position_cnt-1;
}
//显示时间
void lcdshowTime(uint8_t hour,uint8_t min,uint8_t sec,bool use_am_pm,uint8_t start_y,uint8_t use_red,uint8_t use_black)
{
    uint8_t t_x=5,t_y,hourdisplay;
    hourdisplay=hour;

    //time  12:34
    t_y=start_y+(LCD_HEIGHT-start_y-34)/2;
    if(true==use_am_pm)
    {
        if(hourdisplay>12)
        {
            hourdisplay-=12;
        }
        t_x=(LCD_WIDTH-24*4-6-18)/2;
    }
    else
    {
        t_x=(LCD_WIDTH-24*4-6)/2;
    }

    ShowIcon(t_x,t_y,24,34,(uint8_t*)&icon_timer2434[hourdisplay/10][0],use_black);
    t_x+=24;
    if(use_red)
    {
        POINT_COLOR=RED;
        ShowIcon(t_x,t_y,24,34,(uint8_t*)&icon_timer2434[hourdisplay%10+10][0],use_black);
        POINT_COLOR=WHITE;
    }
    else
    {
        ShowIcon(t_x,t_y,24,34,(uint8_t*)&icon_timer2434[hourdisplay%10][0],use_black);
    }
    t_x+=24;
    ShowIcon(t_x,t_y,6,34,(uint8_t*)&icon_colon0634[0],use_black);
    t_x+=6;
    ShowIcon(t_x,t_y,24,34,(uint8_t*)&icon_timer2434[min/10][0],use_black);
    t_x+=24;
    ShowIcon(t_x,t_y,24,34,(uint8_t*)&icon_timer2434[min%10][0],use_black);

    if(true==use_am_pm)//icon_pm1512
    {
        ShowIcon(LCD_WIDTH-17,t_y+2,15,12,(hour<12)? (unsigned char*)icon_am1512:(unsigned char*)icon_pm1512,use_black);
        ShowIcon(LCD_WIDTH-18,t_y+17+2,9,12,(uint8_t*)&icon_num0912[sec/10][0],use_black);
        ShowIcon(LCD_WIDTH-9,t_y+17+2,9,12,(uint8_t*)&icon_num0912[sec%10][0],use_black);

    }

}
void DisplayAnaTime(uint8_t hour,uint8_t min)
{
    uint32_t position1,position2;
    static uint16_t minb=0;
    uint16_t minnow;

    minnow=hour*60+min;

    if(hour&0x80)
    {
        minb=minnow+1;
        hour&=0x7f;
    }
    if(minb!=minnow)
    {
        minb=minnow;

        if(hour>=12)
            hour-=12;

        position1=HOHR_ADDR+(hour*60+min)/12*(96*96*2);
        position2=MIN_ADDR+min*(96*96*2);
        TFT_ShowBmp_Flash((LCD_WIDTH-96)/2,(LCD_HEIGHT-96)/2,96,96,HOHR_ADDR+60*(96*96*2));
        TFT_ShowBmp_Flash_POINT((LCD_WIDTH-96)/2,(LCD_HEIGHT-96)/2,96,96,position1,0);
        TFT_ShowBmp_Flash_POINT((LCD_WIDTH-96)/2,(LCD_HEIGHT-96)/2,96,96,position2,0);
    }
}
void Displayhhmm(uint8_t start_x,uint8_t start_y,uint8_t hh,uint8_t mm,bool is_use_read)
{
    uint8_t len,i,buf[40];
    len=sprintf((char*)buf,"%d",hh/10%10);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }

    BACK_COLOR=BLACK;
    Printfdata(start_x,start_y,24,34,buf,len,(uint8_t*)&icon_timer2434[0][0],102);
    BACK_COLOR=BLACK;

    start_x += 24;
    len=sprintf((char*)buf,"%d",hh%10);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }

    BACK_COLOR=BLACK;
    if (is_use_read == true)
    {
        POINT_COLOR=RED;
        buf[0] += 10;
    }

    Printfdata(start_x,start_y,24,34,buf,len,(uint8_t*)&icon_timer2434[0][0],102);
    POINT_COLOR=WHITE;
    BACK_COLOR=BLACK;

    start_x += 24;
    ShowIcon(start_x,start_y,6,34,(uint8_t*)&icon_colon0634[0],START_SPI|END_SPI);
    start_x += 6;
    len=sprintf((char*)buf,"%02d",mm);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    BACK_COLOR=BLACK;
    Printfdata(start_x,start_y,24,34,buf,len,(uint8_t*)&icon_timer2434[0][0],102);
    BACK_COLOR=BLACK;
}
void DisplayMonthDay(uint8_t start_x,uint8_t start_y,uint8_t month,uint8_t day)
{
    uint8_t len,i,buf[40];
	month = month%13;
    len=sprintf((char*)buf,"%02d",month);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }

    BACK_COLOR=BLACK;
    Printfdata(start_x,start_y,9,12,buf,len,(uint8_t*)&icon_num0912[0][0],24);
    BACK_COLOR=BLACK;
    start_x += 9*len;
    ShowIcon(start_x,start_y,9,12,(uint8_t*)&icon_num0912[10][0],START_SPI|END_SPI);

    start_x += 9;
	day = day%32;
    len=sprintf((char*)buf,"%02d",day);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    BACK_COLOR=BLACK;
    Printfdata(start_x,start_y,9,12,buf,len,(uint8_t*)&icon_num0912[0][0],24);
    BACK_COLOR=BLACK;
}
void DisplayDaySecond(uint8_t start_x,uint8_t start_y,uint8_t data)
{
    uint8_t len,i,buf[40];
    len=sprintf((char*)buf,"%02d",data);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }

    BACK_COLOR=BLACK;
    Printfdata(start_x,start_y,9,12,buf,len,(uint8_t*)&icon_num0912[0][0],24);
    BACK_COLOR=BLACK;
//    start_x += 12*len;
}
void DisplayNumber(uint8_t start_x,uint8_t start_y,uint8_t data,bool is_use_read,uint8_t x_size,uint8_t y_size,uint8_t *from,uint8_t unint_size)
{
    uint8_t len,i,buf[40];

    len=sprintf((char*)buf,"%d",data);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }

    BACK_COLOR=BLACK;
    if (is_use_read == true)
    {
        POINT_COLOR=RED;
        buf[0] += 10;
    }

    Printfdata(start_x,start_y,x_size,y_size,buf,len,(uint8_t*)from,unint_size);
    POINT_COLOR=WHITE;
    BACK_COLOR=BLACK;
}
//Show time
//Charging_State global_device_charging_state;
void lcd_show_time(uint8_t type,RTC_UTCTimeStruct  tm,unsigned char batter_state, bool my_bond_stat)
{

    uint8_t start_x,start_y;
    bool use_am_pm = true;
//     LOG("type:%d\r\n",type);
// 	my_bond_stat= true;
    uint8_t hr;
    start_y = start_x;
    start_x = start_y;
	hr=StuHr.hr_fielt;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////表盘
    if(type==4)
    {
        DisplayAnaTime((batter_state&0x80)?tm.hour|0x80:tm.hour,tm.minutes);
        return ;
    }
	batter_state&=0x0f;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////时间
    if (type == 3)
    {
        DisplayNumber(33,11,tm.hour/10,false,24,34,(uint8_t*)&icon_timer2434[0][0],102);
        DisplayNumber(33+24,11,tm.hour%10,true,24,34,(uint8_t*)&icon_timer2434[0][0],102);
        DisplayNumber(33,LCD_HEIGHT-10-34,tm.minutes/10,false,24,34,(uint8_t*)&icon_timer2434[0][0],102);
        DisplayNumber(33+24,LCD_HEIGHT-10-34,tm.minutes%10,false,24,34,(uint8_t*)&icon_timer2434[0][0],102);
    } else
    {
        if ( type == 0 )
        {
            start_x = 11;
            start_y = 33;
        }else if (type == 1)
        {
            start_x = 0;
            start_y = 29;
        }else if (type == 2)
        {
            start_x = 12;
            start_y = 29;
        }
        Displayhhmm(start_x,start_y,tm.hour,tm.minutes,true);//16:27
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////蓝牙
    if (type == 0)
    {
        start_x = 60;
        start_y = 2;
    }else if (type == 1)
    {
        start_x = 0;
        start_y = 2;
    }else if (type == 2)
    {
        start_x = 12;
        start_y = 2;
    }
    if ((type == 0) || (type == 1) || (type == 2))
    {
        if (my_bond_stat)
            ShowIcon(start_x,start_y,8,12,(uint8_t*)icon_ble812,START_SPI|END_SPI);
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////电池
    if (type == 0)
    {
        start_x = 89;
        start_y = 2;
    }else if (type == 1)
    {
        start_x = LCD_WIDTH-27;
        start_y = 2;
    }else if (type == 2)
    {
        start_x = 89;
        start_y = 2;
    }
    if ((type == 0) || (type == 1) || (type == 2))
    {
        if( (batter_state&0x7f) < 2)
        {
            POINT_COLOR=RED;//默认红色
        }
        ShowIcon( LCD_WIDTH-13-27,4,27,12,(uint8_t*)&icon_batter2712[(batter_state&0x7f)%5][0],START_SPI|END_SPI);
        POINT_COLOR=WHITE;//默认
		
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////am or pm
    if (type == 0)
    {
        start_x = 11;
        start_y = 2;
    }else if (type == 1)
    {
        start_x = 110;
        start_y = 29;
    }
    if ((type == 0) || (type == 1) )
    {
        if (use_am_pm == true)//
            ShowIcon(start_x,start_y,15,12,(tm.hour<12)? (unsigned char*)icon_am1512:(unsigned char*)icon_pm1512,START_SPI|END_SPI);
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////数字  月份或日
    if (type == 0 )
    {
        start_x = 11;
        start_y = LCD_HEIGHT-7-12;

    } else if (type == 3 )
    {
        start_x = 83;
        start_y = 33;
    }
    if ( (type ==0) || (type ==3))
    {
        DisplayMonthDay(start_x,start_y,tm.month,tm.day);//0912  06/24
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////月份  英文
	if (type == 1)
    {
        start_x = 47;
        start_y = LCD_HEIGHT-6-12;
    }
    else if (type == 2)
    {
        start_x = 52;
        start_y = LCD_HEIGHT-6-12;
    }
    if ( (type == 1) || (type == 2))
    {
        tm.month = 8;
        ShowIcon(start_x,start_y,35,12,(uint8_t*)&icon_month3512[tm.month][0],START_SPI|END_SPI);
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////星期  英文
    if (type == 0)
    {
        start_x = 91;
        start_y = LCD_HEIGHT-12-7;
    }
    else if (type == 1)
    {
        start_x = 0;
        start_y = LCD_HEIGHT-6-12;
    }
    else if (type == 2)
    {
        start_x = 12;
        start_y = LCD_HEIGHT-6-12;
    } else if (type == 3)
    {
        start_x = 84;
        start_y = LCD_HEIGHT-12-32;
    }
    ShowIcon(start_x,start_y,25,12,(uint8_t*)&icon_week2512[tm.week][0],START_SPI|END_SPI);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////秒
    if  (type == 1)
    {
        start_x = 110;
        start_y = 51;
		DisplayNumber(start_x,start_y,tm.seconds,false,9,12,(uint8_t*)&icon_num0912[0][0],24);
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////日
    if (type == 1)
    {
        start_x = 110;
        start_y = LCD_HEIGHT-6-12;
    } else if (type == 2)
    {
        start_x = 98;
        start_y = LCD_HEIGHT-6-12;
    }
    if ( (type == 1) ||(type == 2) )
    {
		DisplayNumber(start_x,start_y,tm.day%32,false,9,12,(uint8_t*)&icon_num0912[0][0],24);
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////心率
    if (type == 3)
    {
		POINT_COLOR=RED;//默认红色
		ShowIcon(3,LCD_HEIGHT-24-26,26,26,(uint8_t*)icon_hr2626,START_SPI|END_SPI);
		POINT_COLOR=WHITE;//默认
        DisplayNumber(3,27,hr,false,14,18,(uint8_t*)&icon_num1418[0][0],36);
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

//icon_num1418[10][42]
void Printfdata(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t*buf,uint8_t len,uint8_t*front,uint16_t size)
{
    uint8_t data[64];
    uint8_t i,len1,mode=START_SPI;
    if(len==0) return;
    if(len==1) mode=START_SPI|END_SPI;

    for(i=0; i<len; i++)
    {
        if( (buf[i]>=0x20)&&(buf[i]<='z'))
        {
            data[i]=buf[i]-0x20;
        } else
        {
            data[i]=buf[i];
        }
    }

    i=0;
    ShowIcon(x,y,xsize,ysize,front+size*data[i],mode|0x01);
    i++;
    x+=xsize;

    if(len>2)
    {
        len1=len-1;
        for(; i<len1; i++)
        {
            ShowIcon(x,y,xsize,ysize,front+size*data[i],0x01);
            x+=xsize;
        }
    }
    ShowIcon(x,y,xsize,ysize,front+size*data[i],END_SPI|0x01);
}

void LcdDislayInArea(uint8_t x,uint8_t y,uint8_t*str,uint8_t arer_size,uint8_t str_size,uint8_t *displace,bool hz)
{

    uint8_t buf[20];
    uint8_t i,s_place;
    memset(buf,0x20,20);

    if(str_size<=arer_size)
    {
        memcpy(buf,str,str_size);
    } else if(
        (displace[0]!=0)&&
        (displace[0]!=(arer_size+str_size))
    )
    {
        if(displace[0]<=arer_size)
        {
            for(i=(arer_size-displace[0]); i<arer_size; i++)
            {
                buf[i]=*str++;
            }
        } else
        {
            for(i=0; i<arer_size; i++)
            {
                s_place=displace[0]-arer_size+i;
                if(s_place>=str_size)
                {
                    break;
                }
                buf[i]=str[s_place];
            }

        }

    }
    if(false==hz)
    {

        Printfdata(x,y,12,32,buf,arer_size,(uint8_t*)&ascii_1232[0][0],64);
    }
    else
    {
    }

    displace[0]++;
    if(true==hz)
    {
        displace[0]++;
        if(displace[0]%2)
        {
            displace[0]++;
        }
    }

    if(displace[0]>=(arer_size+str_size))
    {
        displace[0]=0;
    }


}
//Show step
void lcd_show_step(uint32_t step,uint32_t goal,uint8_t cnt)
{
    static uint32_t stetb;
    uint8_t buf[5];
    uint32_t persent;

    persent=step*100/goal;

    if(persent>=100)
    {
        persent=100;
    }
    persent=persent/5;

    buf[4]=step%10;
    buf[3]=step%100/10;
    buf[2]=step%1000/100;
    buf[1]=step%10000/1000;
    buf[0]=step%100000/10000;

    if(cnt==0xff)
    {
        TFT_ShowBmp_Flash((LCD_WIDTH-96)/2,(LCD_HEIGHT-96)/2,96,96,STEP_CIRCLE_ADDR+persent*(96*96*2));//
    }

    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,15,36,36,(cnt&0x01) ? STEP1_ADDR:STEP2_ADDR);
// 0xff

    if((0xff==cnt)||(stetb!=step))
    {
        stetb=step;
        BACK_COLOR=0x2988;
        Printfdata(29,59,14,18,buf,5,(uint8_t*)&icon_num1418[0][0],36);
        BACK_COLOR=BLACK;
    }

}


void showUpgredePersent(uint16_t persent)
{
	static uint8_t pb;
	uint8_t buf[20];
	uint8_t len;
//133代表升级错误    100代表升级成功     188 erase
	

	
	
	POINT_COLOR=RED;//默认
	if (133 == persent)
	{
		LCD_Clear(BLACK);
		len=(uint8_t)sprintf((char*)buf,"err");
		Printfdata((LCD_WIDTH-len*12)/2,(LCD_HEIGHT-32)/2,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
		POINT_COLOR=WHITE;//默认
		return;
	}else 	if (188 == persent)
	{
		oled_display_on(true);
	    LCD_Clear(BLACK);
		pb = 0xff;
		len=(uint8_t)sprintf((char*)buf,"init..");
		Printfdata((LCD_WIDTH-len*12)/2,(LCD_HEIGHT-32)/2,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
		POINT_COLOR=WHITE;//默认
		return;
	}else if (100 == persent)
	{
		LCD_Clear(BLACK);
		len=(uint8_t)sprintf((char*)buf,"ok!");
		Printfdata((LCD_WIDTH-len*12)/2,(LCD_HEIGHT-32)/2,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
		POINT_COLOR=WHITE;//默认
		return;
	}else if (100 < persent)
	{
		oled_display_on(true);
		LCD_Clear(BLACK);
		len=(uint8_t)sprintf((char*)buf,"%d",persent);
		Printfdata((LCD_WIDTH-len*12)/2,(LCD_HEIGHT-32)/2,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
		POINT_COLOR=WHITE;//默认
		return;
	}
	else if(persent!=pb)
    {
        LOG("per:%d\r\n",persent);
		if(pb==0xff)
			   LCD_Clear(BLACK);
        pb=persent;
        ShowIcon((LCD_WIDTH-24*2)/2,(LCD_HEIGHT-34)/2,24,34,(uint8_t*)&icon_timer2434[persent/10][0],START_SPI);
        ShowIcon((LCD_WIDTH-24*2)/2+24,(LCD_HEIGHT-34)/2,24,34,(uint8_t*)&icon_timer2434[persent%10][0],END_SPI);
		POINT_COLOR=WHITE;//默认
    }
        

}
//Show Hr
void lcd_show_HR(uint8_t num,uint8_t a_b)
{
    uint8_t len = 0,buf[5],i;
    if (num == 0)
    {
        //BACK_COLOR=red;
        ShowIcon(24,16,4,4,(uint8_t*)point_0404,START_SPI);
        ShowIcon(24,42,4,4,(uint8_t*)point_0404,0);
		for (i=55; i<=92; i+=4)
		{
		        ShowIcon(i,43,2,3,(uint8_t*)icon_line0203,0);
		}
        ShowIcon(96,43,2,3,(uint8_t*)icon_line0203,END_SPI);
        //BACK_COLOR=black;
        TFT_ShowBmp_Flash(10,LCD_HEIGHT-36-6,36,36,(a_b&0x01)? HR4_ADDR : HR3_ADDR);
    } else
    {
        len=sprintf((char*)buf,"%d",num%1000);
        for(i=0; i<len; i++)
        {
            buf[i]-=0x30;
        }
        BACK_COLOR=BLACK;
        Printfdata((LCD_WIDTH-14*len)/2,(LCD_HEIGHT-18)/2,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
        BACK_COLOR=BLACK;

        if(num>150)
		{
			TFT_ShowBmp_Flash(10,18-16,36,36,(a_b&0x01)?HR1_ADDR: HR2_ADDR);
			ShowIcon(26,(LCD_HEIGHT-4)/2,4,4,(uint8_t*)point_0404,START_SPI);
			ShowIcon(26,LCD_HEIGHT-16-2,4,4,(uint8_t*)point_0404,END_SPI);
		}
        else if( (num<150) && (num>120))
		{
			TFT_ShowBmp_Flash(10,(LCD_HEIGHT-36)/2,36,36,(a_b&0x01)?HR1_ADDR : HR2_ADDR);//120-150
			ShowIcon(26,18,4,4,(uint8_t*)point_0404,START_SPI);
			ShowIcon(26,LCD_HEIGHT-16-2,4,4,(uint8_t*)point_0404,END_SPI);
		}
        else
		{
			TFT_ShowBmp_Flash(10,LCD_HEIGHT-16-18,36,36,(a_b&0x01)?HR1_ADDR : HR2_ADDR);//90-120
			ShowIcon(26,18,4,4,(uint8_t*)point_0404,START_SPI);
			ShowIcon(26,(LCD_HEIGHT-4)/2,4,4,(uint8_t*)point_0404,END_SPI);
		}
    }
}

void lcd_show_distance(uint32_t display_value,uint8_t a_b)
{
    uint8_t len = 3,buf[7];
    uint8_t start_x,i;


    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,9,36,36,(get_int(a_b,2))?DISTANCE1_ADDR: DISTANCE2_ADDR);

	if((disvalueb==display_value)&&(a_b!=0xff)) return;
	
	disvalueb=display_value;
	
    len=sprintf((char*)buf,"%d",display_value/100%100);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x = (LCD_WIDTH-14*(len+2)-15-9)/2;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;
    start_x += 14*len+4;//点
    ShowIcon(start_x,72,4,4,(uint8_t*)point_0404,START_SPI|END_SPI);
    len=sprintf((char*)buf,"%02d",display_value%100);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x += 5;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

    start_x += 16+14;
    ShowIcon(start_x,64,15,12,(uint8_t*)icon_km1512,START_SPI|END_SPI);

}


void lcd_show_K(uint32_t display_value,uint8_t a_b)
{
    uint8_t len = 1,buf[7];
    uint8_t start_x,i;

    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,9,36,36,CALORIES_ADDR);

    len=sprintf((char*)buf,"%d",display_value);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x = (LCD_WIDTH-14*len)/2;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

}
void lcd_show_sleep(uint32_t display_value,uint8_t a_b)
{

    uint8_t buf[20],len,i,start_x;

    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,9,36,36, SLEEP1_ADDR+ICON3636_1ADDR*get_int(a_b,3));

    len=sprintf((char*)buf,"%d",display_value/60);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x = (LCD_WIDTH-14*(len+4))/2;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

    start_x += 14*len+7;//h
    ShowIcon(start_x,58+18-12,5,12,(uint8_t*)icon_h0512,START_SPI|END_SPI);

    len=sprintf((char*)buf,"%02d",display_value%60);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x += 7;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

    start_x += 14*2+7;//m
    ShowIcon(start_x,58+18-12,7,12,(uint8_t*)icon_m0712,START_SPI|END_SPI);
}

void lcd_show_sport_time(uint32_t display_value,uint8_t a_b)
{
    uint8_t len = 4,buf[7];
    uint8_t start_x,i;
 
    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,9,36,36,STEP_TIME_ADDR);


    len=sprintf((char*)buf,"%02d",display_value/60%60);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x = (LCD_WIDTH-14*4-9)/2;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

    start_x += 14*2+4;
    ShowIcon(start_x,60,4,4,(uint8_t*)point_0404,START_SPI);
    ShowIcon(start_x,69,4,4,(uint8_t*)point_0404,END_SPI);

    len=sprintf((char*)buf,"%02d",display_value%60);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x += 9;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

}


void lcd_show_battery_charging(uint8_t battery_lever)
{
   //0  1 2 3 4(full) 5% 10%
	if(battery_lever>6)
		battery_lever=6;
   TFT_ShowBmp_Flash((LCD_WIDTH-30)/2,(LCD_HEIGHT-46)/2,30,46,BATTERY_ADDR+BATTERY_ADDR_1_SIZE*battery_lever);
  
}

void lcd_show_battery(uint8_t persent,uint8_t a_b)
{
    uint8_t len = 1,buf[7],i,start_x;
    extern uint8_t mac_data[6];

    if(5==persent)
        TFT_ShowBmp_Flash((LCD_WIDTH-30)/2,(LCD_HEIGHT-46)/2,30,46,BATTERY_ADDR+BATTERY_ADDR_1_SIZE*5);
    else if(20==persent)
        TFT_ShowBmp_Flash((LCD_WIDTH-30)/2,(LCD_HEIGHT-46)/2,30,46,BATTERY_ADDR+BATTERY_ADDR_1_SIZE*6);
    else if(30==persent)/*????*/
    {
        TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,(LCD_HEIGHT-36)/2,36,36,EXERCISE1_ADDR+a_b*(36*36*2));
    }
    else if(40==persent)/*??*/
    {
        len = 2;
        TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,(LCD_HEIGHT/2-36)/2,36,36,PAIR_ADDR);
        len=sprintf((char*)buf,"%02X%02X",0X11,0X22);
        for(i=0; i<len; i++)
        {
            buf[i]-=0x30;
        }
        start_x = (LCD_WIDTH-14*4)/2;
        BACK_COLOR=BLACK;
        Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
        BACK_COLOR=BLACK;

    }
}
void lcd_show_alarm(uint8_t hour,uint8_t min,bool use_am_pm,uint8_t ararm_type)
{
    /* 0x00 吃药 0x01喝水 0x02 咖啡  0x03闹钟 0x04睡眠  0x05锻炼 0x06跑步*/

//0 medcion //1 drinks //2 coffee //3 alarm //4 sleep //5 exercise //6 run //7 get target //8 sport  time
    uint8_t len = 4,buf[7];
    uint8_t start_x,i;
    uint32_t addr;

    if (0==ararm_type ) addr = MEDICINE_ADDR;
    else if (1==ararm_type ) addr =DRINK_ADDR;
    else if (2==ararm_type ) addr =COFFEE_ADDR;
    else if (3==ararm_type ) addr =ALARM1_ADDR;
    else if (4==ararm_type ) addr =SLEEP3_ADDR;
    else if (5==ararm_type ) addr =EXERCISE3_ADDR;
    else if (6==ararm_type ) addr =EXERCISE3_ADDR;//////////////没有////////////////////

    if (ararm_type <= 6)
        TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,9,36,36,addr);


    len=sprintf((char*)buf,"%02d",hour%60);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x = (LCD_WIDTH-14*4-9)/2;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

    start_x += 14*2+4;
    ShowIcon(start_x,60,4,4,(uint8_t*)point_0404,START_SPI);
    ShowIcon(start_x,69,4,4,(uint8_t*)point_0404,END_SPI);

    len=sprintf((char*)buf,"%02d",min%60);
    for(i=0; i<len; i++)
    {
        buf[i]-=0x30;
    }
    start_x += 9;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,14,18,buf,len,(uint8_t*)&icon_num1418[0][0],36);
    BACK_COLOR=BLACK;

}


void LcdShowNotice(uint8_t icon_num,uint8_t *msg,uint8_t msglen)
{
    /*电话   sms  do_activity  wechat qq snapchat facebook twitter  whatsapp skype*/
    //NEW_CALL=0x99,	NEW_MSG,	GENERIC_NOTIFICATION,	NEW_GMSG,	WHATSAPP,	FACEBOOK,	TWITTER,SKYPE,	QQ,	WEICHAR,	SNAPCHAT, 运动提醒
    const uint32_t picture_buf[]= {PHONE1_ADDR,SMS_ADDR,ALARM1_ADDR,PHONE_SMS_ADDR,WHATSAPP_ADDR,FACE_BOOK_ADDR,TWITTER_ADDR,SKYPE_ADDR,QQ_ADDR,WECHAT_ADDR,SNAP_ADDR,EXERCISE1_ADDR };

    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,9,36,36,picture_buf[icon_num%12]);

    if(msglen>10)
        msglen = 10;

    BACK_COLOR=BLACK;
    Printfdata((LCD_WIDTH-msglen*12)/2,58,12,32,msg,msglen,(uint8_t*)&ascii_1232[0][0],64);
    BACK_COLOR=BLACK;

}



//extern void get_mac(uint8_t *mac);
//a_b 0---2
void lcd_show_pair(uint8_t a_b)
{

    uint8_t buf[20],mac[6],len,start_x;

    get_mac(mac);
    TFT_ShowBmp_Flash((LCD_WIDTH-36)/2,(LCD_HEIGHT/2-36)/2,36,36,BLE1_ADDR+(uint32_t)get_int(a_b,3)*ICON3636_1ADDR);

    len=sprintf((char*)buf,"%02X%02X",mac[4],mac[5]);
    start_x = (LCD_WIDTH-12*len)/2;
    BACK_COLOR=BLACK;
    Printfdata(start_x,58,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
    BACK_COLOR=BLACK;


}


void oled_show_version(void)
{

    uint8_t buf[20];
    uint8_t len;
    len=(uint8_t)sprintf((char*)buf,"V%d.%d.%d",version[0],version[1],version[2]);
    Printfdata((LCD_WIDTH-len*12)/2,(LCD_HEIGHT-32)/2,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
}





