#include "lcd.h"
#include "lcdfont.h"
//#include "delay.h"

//LCD的画笔颜色和背景色
uint16_t POINT_COLOR=WHITE;    //画笔颜色
uint16_t BACK_COLOR=BLACK;  //背景色


#define BUF_SIZE_SPI  512


#if (BUF_SIZE_SPI<256)
 #define lcd_spi_send(A,B) spi_send(MODE_LCD_SPI,A,B,0,0)

#else

void lcd_spi_send(uint8_t *datain,uint16_t len)
{
uint16_t left,sendlen,sended;
	
	left=len;
	sended=0;
	while(left)
	{
      if(left>240)
		  sendlen=240;
	  else
		 sendlen= left;
	  
	 spi_send(MODE_LCD_SPI,datain+sended,sendlen,0,0);
	  left-=sendlen;
	  sended+=sendlen;
	}


}
#endif
//LCD GPIO端口初始化
void LCD_GPIO_Init(void)
{
    nrf_gpio_cfg_output(LCD_SCLK_GPIO_Pin);
    SCK_LOW();


    nrf_gpio_cfg_output(LCD_SDIN_GPIO_Pin);
    SDIN_LOW();

    nrf_gpio_cfg_output(LCD_RST_GPIO_Pin);
    RST_HIGH();

    nrf_gpio_cfg_output(LCD_RS_GPIO_Pin);
    RS_LOW();


    nrf_gpio_cfg_output(LCD_CS_GPIO_Pin);
    CS_HIGH();

    nrf_gpio_cfg_output(LCD_LED_GPIO_Pin);
    LED_ON();
//  while(1);
}
//向ST7735R写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void LCD_WR_Byte(uint8_t dat,uint8_t cmd)
{

    if(0==cmd)
        RS_LOW();
    CS_LOW();
    lcd_spi_send(&dat,1);
    CS_HIGH();
    RS_HIGH();
}



void start_trancs_data(bool start)
{
    if(start==true)
    {
        RS_HIGH();
        CS_LOW();
    }
    else
    {
        CS_HIGH();
        RS_HIGH();
    }

}



//LCD 复位
void LCD_Reset(void)
{
    CS_HIGH();
    RST_HIGH();
    RST_LOW();
    delay_ms(100);
    RST_HIGH();
    delay_ms(50);
}


void EnLcdSpi(bool enflg)
{
    if(true==enflg)
    {
        EnableSpi(true,MODE_LCD_SPI,0xff,LCD_SDIN_GPIO_Pin,LCD_SCLK_GPIO_Pin);
    }
    else
    {
        EnableSpi(false,MODE_LCD_SPI,0xff,LCD_SDIN_GPIO_Pin,LCD_SCLK_GPIO_Pin);
    }


}

void oled_display_on(bool on_flg)
{
    static uint8_t on_flg_b=0xff;

    if(on_flg_b==on_flg) return;
    EnLcdSpi(true);

    if(true==on_flg)
    {
        LCD_WR_Byte(0x11,LCD_CMD); //Sleep out
        delay_ms(120);
        LED_ON();
        LCD_WR_Byte(0x29,LCD_CMD); //Display on
    }
    else
    {
        LCD_WR_Byte(0x28,LCD_CMD); //Display off
        LCD_WR_Byte(0x10,LCD_CMD); //Sleep in
        LED_OFF();

    }
    EnLcdSpi(false);
    on_flg_b=on_flg;
}
void LCD_GPIO_test(void)
{

    nrf_gpio_cfg_output(LCD_CS_GPIO_Pin);
    nrf_gpio_cfg_output(LCD_RS_GPIO_Pin);
    nrf_gpio_cfg_output(LCD_RST_GPIO_Pin);
    nrf_gpio_cfg_output(LCD_SDIN_GPIO_Pin);
    nrf_gpio_cfg_output(LCD_LED_GPIO_Pin);
    nrf_gpio_cfg_output(LCD_SCLK_GPIO_Pin);

    while(1)
    {
        SCK_LOW();
        SCK_HIGH();

        SDIN_LOW();//ok
        SDIN_HIGH();

        RST_HIGH();//ok
        RST_LOW();

        RS_LOW();
        RS_HIGH();


        CS_HIGH();
        CS_LOW();
        LED_ON();
        LED_OFF();
    }




}
//LCD 初始化
void LCD_Init(void)
{
//	LCD_GPIO_test();
    LCD_GPIO_Init();             //端口初始化
    LCD_Reset();                 //LCD复位，在LCD初始化之前

    delay_ms(120);
    EnLcdSpi(true);
    LCD_WR_Byte(0x11,LCD_CMD); //Sleep out
    delay_ms(120);

    LCD_WR_Byte(0x28,LCD_CMD); //Display off
    LED_OFF();

    LCD_WR_Byte(0xB1,LCD_CMD); //帧率控制
    LCD_WR_Byte(0x01,LCD_DATA);
    LCD_WR_Byte(0x2C,LCD_DATA);
    LCD_WR_Byte(0x2D,LCD_DATA);

    LCD_WR_Byte(0xB2,LCD_CMD);
    LCD_WR_Byte(0x01,LCD_DATA);
    LCD_WR_Byte(0x2C,LCD_DATA);
    LCD_WR_Byte(0x2D,LCD_DATA);

    LCD_WR_Byte(0xB3,LCD_CMD);
    LCD_WR_Byte(0x01,LCD_DATA);
    LCD_WR_Byte(0x2C,LCD_DATA);
    LCD_WR_Byte(0x2D,LCD_DATA);
    LCD_WR_Byte(0x01,LCD_DATA);
    LCD_WR_Byte(0x2C,LCD_DATA);
    LCD_WR_Byte(0x2D,LCD_DATA);

    LCD_WR_Byte(0xB4,LCD_CMD);
    LCD_WR_Byte(0x03,LCD_DATA);

    LCD_WR_Byte(0xB6,LCD_CMD);
    LCD_WR_Byte(0xB4,LCD_DATA);
    LCD_WR_Byte(0xF0,LCD_DATA);

    LCD_WR_Byte(0xC0,LCD_CMD);
    LCD_WR_Byte(0xA2,LCD_DATA);
    LCD_WR_Byte(0x02,LCD_DATA);
    LCD_WR_Byte(0x84,LCD_DATA);

    LCD_WR_Byte(0xC1,LCD_CMD);
    LCD_WR_Byte(0xC5,LCD_DATA);

    LCD_WR_Byte(0xC2,LCD_CMD);
    LCD_WR_Byte(0x0A,LCD_DATA);
    LCD_WR_Byte(0x00,LCD_DATA);

    LCD_WR_Byte(0xC3,LCD_CMD);
    LCD_WR_Byte(0x8A,LCD_DATA);
    LCD_WR_Byte(0x2A,LCD_DATA);

    LCD_WR_Byte(0xC4,LCD_CMD);
    LCD_WR_Byte(0x8A,LCD_DATA);
    LCD_WR_Byte(0xEE,LCD_DATA);

    LCD_WR_Byte(0xC5,LCD_CMD);
    LCD_WR_Byte(0x0A,LCD_DATA);

    LCD_WR_Byte(0x36,LCD_CMD); //MX, MY, RGB mode
    LCD_WR_Byte(0xC8,LCD_DATA);  //0x68

    LCD_WR_Byte(0xe0,LCD_CMD);
    LCD_WR_Byte(0x02,LCD_DATA);
    LCD_WR_Byte(0x1c,LCD_DATA);
    LCD_WR_Byte(0x07,LCD_DATA);
    LCD_WR_Byte(0x12,LCD_DATA);
    LCD_WR_Byte(0x37,LCD_DATA);
    LCD_WR_Byte(0x32,LCD_DATA);
    LCD_WR_Byte(0x29,LCD_DATA);
    LCD_WR_Byte(0x2d,LCD_DATA);
    LCD_WR_Byte(0x29,LCD_DATA);
    LCD_WR_Byte(0x25,LCD_DATA);
    LCD_WR_Byte(0x2b,LCD_DATA);
    LCD_WR_Byte(0x39,LCD_DATA);
    LCD_WR_Byte(0x00,LCD_DATA);
    LCD_WR_Byte(0x01,LCD_DATA);
    LCD_WR_Byte(0x03,LCD_DATA);
    LCD_WR_Byte(0x10,LCD_DATA);

    LCD_WR_Byte(0xe1,LCD_CMD);
    LCD_WR_Byte(0x03,LCD_DATA);
    LCD_WR_Byte(0x1d,LCD_DATA);
    LCD_WR_Byte(0x07,LCD_DATA);
    LCD_WR_Byte(0x06,LCD_DATA);
    LCD_WR_Byte(0x2e,LCD_DATA);
    LCD_WR_Byte(0x2c,LCD_DATA);
    LCD_WR_Byte(0x29,LCD_DATA);
    LCD_WR_Byte(0x2d,LCD_DATA);
    LCD_WR_Byte(0x2e,LCD_DATA);
    LCD_WR_Byte(0x2e,LCD_DATA);
    LCD_WR_Byte(0x37,LCD_DATA);
    LCD_WR_Byte(0x3f,LCD_DATA);
    LCD_WR_Byte(0x00,LCD_DATA);
    LCD_WR_Byte(0x00,LCD_DATA);
    LCD_WR_Byte(0x02,LCD_DATA);
    LCD_WR_Byte(0x10,LCD_DATA);



    LCD_WR_Byte(0xF0,LCD_CMD);
    LCD_WR_Byte(0x01,LCD_DATA);
    LCD_WR_Byte(0xF6,LCD_CMD);
    LCD_WR_Byte(0x00,LCD_DATA);

    LCD_WR_Byte(0x3A,LCD_CMD); //65k mode
    LCD_WR_Byte(0x05,LCD_DATA);
//     LCD_WR_Byte(0x29,LCD_CMD); //Display on
//  LCD_WR_Byte(0x28,LCD_CMD); //Display off
    EnLcdSpi(false);


    oled_display_on(true);
    LCD_Clear(RED);
    oled_display_on(false);

}
//设置lcd显示区域，在此区域写点数据自动换行
//xy起点和终点
static void LCD_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end)
{


    uint8_t buf[4];
    CS_LOW();
    RS_LOW();
    buf[0]=0x2a;
    lcd_spi_send(buf,1);
    RS_HIGH();
    buf[0]=0;
    buf[1]=x_start;
    buf[2]=0;
    buf[3]=x_end;
    lcd_spi_send(buf,4);

    RS_LOW();
    buf[0]=0x2b;
    lcd_spi_send(buf,1);
    RS_HIGH();
    buf[0]=0;
    buf[1]=y_start;
    buf[2]=0;
    buf[3]=y_end;
    lcd_spi_send(buf,4);

    RS_LOW();
    buf[0]=0x2c;
    lcd_spi_send(buf,1);
    RS_HIGH();
    CS_HIGH();


}

//快速画一个点
//x,y:坐标
//color:颜色
static void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
    uint8_t buf[6];
    CS_LOW();
    RS_LOW();
    buf[0]=0x2a;
    lcd_spi_send(buf,1);
    RS_HIGH();
    buf[0]=0;
    buf[1]=x;
    buf[2]=0;
    buf[3]=x;
    lcd_spi_send(buf,4);

    RS_LOW();
    buf[0]=0x2b;
    lcd_spi_send(buf,1);
    RS_HIGH();
    buf[0]=0;
    buf[1]=y;
    buf[2]=0;
    buf[3]=y;
    lcd_spi_send(buf,4);


    RS_LOW();
    buf[0]=0x2c;
    lcd_spi_send(buf,1);
    RS_HIGH();

    buf[4]=color>>8;
    buf[5]=color;
    lcd_spi_send(buf+4,2);

    CS_HIGH();
    RS_HIGH();


}


//LCD清屏函数
//color:填充的函数
void LCD_Clear(uint16_t color)///可以优化
{
    uint32_t i,m,len=0;
    uint8_t buf[BUF_SIZE_SPI];


    EnLcdSpi(true);


    LCD_SetRegion(0,0,LCD_WIDTH-1,LCD_HEIGHT-1);
    i=LCD_WIDTH*LCD_HEIGHT;

    start_trancs_data(true);

    for(m=0; m<i; m++)
    {
        buf[len++]=color>>8;
        buf[len++]=color;
		
		if(len>=BUF_SIZE_SPI)
		{
        lcd_spi_send(buf,len);
		len=0;
		}
    }
	if(len)
	{
	lcd_spi_send(buf,len);
	len=0;
	}
    start_trancs_data(false);
    EnLcdSpi(false);


}





void TFT_ShowBmp(uint16_t x,uint16_t y,uint16_t xsize,uint16_t ysize,const uint8_t *p)
{
    uint32_t size,temp;
	uint8_t buf[BUF_SIZE_SPI];
	uint32_t len=0;
    EnLcdSpi(true);

    LCD_SetRegion(x,y,x+xsize-1,y+ysize-1);

    temp=(uint32_t)xsize*ysize;
    start_trancs_data(true);
    for(size=0; size<temp; size++)
    {

		buf[len++]=*(p+size*2+1);
        buf[len++]=*(p+size*2+0);
		if(len>=BUF_SIZE_SPI)
		{
        lcd_spi_send(buf,len);
		len=0;
		}
    }
	if(len)
	{
	lcd_spi_send(buf,len);
	}
    start_trancs_data(false);
    EnLcdSpi(false);
}
//ShowIcon( (LCD_WIDTH-8)/2,(LCD_HEIGHT-12)/2,8,12,(uint8_t*)icon_ble812,START_SPI|END_SPI);
void ShowIcon(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t*pdata,uint8_t mode)
{
    uint32_t size,temp,tmp32;
    uint16_t colour,colour_p,colour_b;
    uint32_t xp,yp,line_bytes,offset,i;
	uint8_t buf[BUF_SIZE_SPI];
	uint32_t len=0;

	colour_p = (POINT_COLOR>>8) | (POINT_COLOR<<8);
    colour_b = (BACK_COLOR>>8) | (BACK_COLOR<<8);
	
    if(mode&START_SPI)
        EnLcdSpi(true);

    LCD_SetRegion(x,y,x+xsize-1,y+ysize-1);

    temp=(uint32_t)xsize*ysize;

    xp=0;
    yp=0;
    line_bytes = xsize/8;
    offset = xsize%8;
    if(offset)
    {
        line_bytes++;
        offset = (8-offset);
    }

    start_trancs_data(true);

    tmp32=0;
    for(i=0; i<line_bytes; i++)
    {
        tmp32<<=8;
        tmp32|=pdata[i];
    }
    tmp32>>=offset;

    for(size=0; size<temp; size++)
    {
        colour=colour_b;//POINT_COLOR;
		 xp++;
        if(tmp32&(1<<(xsize-xp)))
        {
            colour=colour_p;
        }
		
		buf[len++]=colour;
		buf[len++]=colour>>8;
		if(len>=BUF_SIZE_SPI)
		{
        lcd_spi_send(buf,len);
		len=0;
		}

        if(xp>=xsize)
        {
            xp=0;
            yp++;
            pdata+=line_bytes;
            tmp32=0;
            for(i=0; i<line_bytes; i++)
            {
                tmp32<<=8;
                tmp32|=pdata[i];
            }
            tmp32>>=offset;
        }


    }
	
	if(len)
	{
	lcd_spi_send(buf,len);
	}
    start_trancs_data(false);

    if(mode&END_SPI)
        EnLcdSpi(false);
}

void lcd_swap_buf(uint8_t *buf,uint16_t len)
{
	uint32_t i,k;
	uint8_t tmp;
	k=len/2;
	for(i=0;i<k;i++)
	{

	tmp=buf[i*2];
	buf[i*2]=buf[i*2+1];
	buf[i*2+1]=tmp;
	}

}


void TFT_ShowBmp_Flash(uint16_t x,uint16_t y,uint16_t xsize,uint16_t ysize,const uint32_t addr)
{
    uint32_t temp,numbers,len,j,lastlen;

	uint8_t buf[BUF_SIZE_SPI];


    EnLcdSpi(true);

    LCD_SetRegion(x,y,x+xsize-1,y+ysize-1);

    lastlen=0;
    len=BUF_SIZE_SPI;
    temp=(uint32_t)xsize*ysize*2;
    numbers=temp/BUF_SIZE_SPI;
    if(temp%BUF_SIZE_SPI)
    {
        numbers++;
        lastlen=temp%BUF_SIZE_SPI;

    }
    start_trancs_data(true);


    for(j=0; j<numbers; j++)
    {
		if((j==(numbers-1))&&(lastlen))
        {
            len=lastlen;
        } 
        if(uifilecrc!=0xffff)
		{
            SPI_FLASH_ReadCont((uint8_t *)buf,addr+j*BUF_SIZE_SPI,len);
			lcd_swap_buf(buf,len);
			lcd_spi_send(buf,len);
		}

    }


    start_trancs_data(false);
    EnLcdSpi(false);
}


void TFT_ShowBmp_Flash_POINT(uint16_t x,uint16_t y,uint16_t xsize,uint16_t ysize,uint32_t addr,uint8_t mode)
{
    uint32_t i,temp,numbers,len,j,lastlen,xP,xE,yP,point_len;//,yE;
    COLOUR_UNI Colour;
	uint8_t buf[BUF_SIZE_SPI];
    EnLcdSpi(true);

    lastlen=0;
    len=BUF_SIZE_SPI;
    temp=(uint32_t)xsize*ysize*2;
    numbers=temp/BUF_SIZE_SPI;
    if(temp%BUF_SIZE_SPI)
    {
        numbers++;
        lastlen=temp%BUF_SIZE_SPI;

    }
    xP=x;
    yP=y;
    xE=x+xsize;
	point_len=len/2;
    for(j=0; j<numbers; j++)
    {
        if((j==(numbers-1))&&(lastlen))
        {
            len=lastlen;
			point_len=len/2;
        }

		
        if(uifilecrc!=0xffff)
            SPI_FLASH_ReadCont((uint8_t *)buf,addr+j*BUF_SIZE_SPI,len);


        for(i=0; i<point_len; i++)
        {

            Colour.buf[0]=*(buf+i*2+0);
            Colour.buf[1]=*(buf+i*2+1);

            if(Colour.colour!=BLACK)
                LCD_Fast_DrawPoint(xP,yP,Colour.colour);
            else if(mode&0x01)
                LCD_Fast_DrawPoint(xP,yP,BACK_COLOR);

            xP++;
            if(xP>=xE)
            {
                xP=x;
                yP++;
            }
        }
    }

    EnLcdSpi(false);
}



void TFT_ST7735defineScrollArea(int16_t tfa, int16_t bfa)
{

    EnLcdSpi(true);
    LCD_WR_Byte(0x33,LCD_CMD);


    LCD_WR_Byte(tfa>>8,LCD_DATA);
    LCD_WR_Byte(tfa>>0,LCD_DATA);

    LCD_WR_Byte(bfa>>8,LCD_DATA);
    LCD_WR_Byte(bfa>>0,LCD_DATA);

    EnLcdSpi(false);
}

void TFT_ST7735scroll(uint16_t adrs) {
    EnLcdSpi(true);
    LCD_WR_Byte(0x37,LCD_CMD);


    LCD_WR_Byte(adrs>>8,LCD_DATA);
    LCD_WR_Byte(adrs>>0,LCD_DATA);
    EnLcdSpi(false);
}



