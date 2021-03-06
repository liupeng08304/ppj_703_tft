#ifndef _LCD_H_
#define _LCD_H_

#include "stdint.h"
#include "nrf_gpio.h"
#include "utility.h"
#include "spi52.h"
#include "ex_flash.h"
#include "bsp.h"
#include "upfile.h"

//#define delay_ms(A ) nrf_delay_ms(A)

#define LCD_HEIGHT   96
#define LCD_WIDTH   (128)


#define START_SPI  0x80
#define END_SPI   0x40
#define EN_BLACK  0x01

//LCD的画笔颜色和背景色
extern uint16_t  POINT_COLOR;//默认红色
extern uint16_t  BACK_COLOR; //背景颜色.默认为白


//画笔颜色
#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40 //棕色
#define BRRED            0XFC07 //棕红色
#define GRAY             0X8430 //灰色





#define LCD_CMD  0  //写命令
#define LCD_DATA 1  //写数据



typedef union
{
    uint16_t colour;
    uint8_t buf[2];
} COLOUR_UNI;






#define SCK_HIGH() nrf_gpio_pin_set(LCD_SCLK_GPIO_Pin)
#define SCK_LOW() nrf_gpio_pin_clear(LCD_SCLK_GPIO_Pin)


#define SDIN_HIGH() nrf_gpio_pin_set(LCD_SDIN_GPIO_Pin)
#define SDIN_LOW() nrf_gpio_pin_clear(LCD_SDIN_GPIO_Pin)


#define RST_HIGH() nrf_gpio_pin_set(LCD_RST_GPIO_Pin)
#define RST_LOW() nrf_gpio_pin_clear(LCD_RST_GPIO_Pin)

#define RS_HIGH() nrf_gpio_pin_set(LCD_RS_GPIO_Pin)
#define RS_LOW() nrf_gpio_pin_clear(LCD_RS_GPIO_Pin)



#define CS_HIGH() nrf_gpio_pin_set(LCD_CS_GPIO_Pin)
#define CS_LOW() nrf_gpio_pin_clear(LCD_CS_GPIO_Pin)

#define LED_ON() nrf_gpio_pin_set(LCD_LED_GPIO_Pin)
#define LED_OFF() nrf_gpio_pin_clear(LCD_LED_GPIO_Pin)



#define STEP_CIRCLE_ADDR  EX_UI_ADDR
#define STEP_CIRCLE_ADDR_SIZE  (21*96*96*2)

#define HOHR_ADDR (STEP_CIRCLE_ADDR+STEP_CIRCLE_ADDR_SIZE)
#define HOHR_ADDR_SIZE (60*96*96*2)

#define TIME_CIRCLE_ADDR (HOHR_ADDR+HOHR_ADDR_SIZE)
#define TIME_CIRCLE_ADDR_SIZE (96*96*2)

#define MIN_ADDR (TIME_CIRCLE_ADDR+TIME_CIRCLE_ADDR_SIZE)
#define MIN_ADDR_SIZE (60*96*96*2)

#define BATTERY_ADDR (MIN_ADDR+MIN_ADDR_SIZE)  //0  1 2 3 4(full) 5% 10%
#define BATTERY_ADDR_1_SIZE (30*46*2)
#define BATTERY_ADDR_SIZE (7*BATTERY_ADDR_1_SIZE)


#define ICON3636_ADDR (BATTERY_ADDR+BATTERY_ADDR_SIZE)  //
#define ICON3636_1ADDR (36*36*2)
#define ICON_S(A)(ICON3636_ADDR+A*ICON3636_1ADDR-ICON3636_1ADDR)
#define ICON3636_ADDR_SIZE (44*ICON3636_1ADDR)
#define FACE_BOOK_ADDR ICON_S(1)
#define QQ_ADDR ICON_S(2)
#define SKYPE_ADDR  ICON_S(3)
#define SMS_ADDR  ICON_S(4)
#define TWITTER_ADDR  ICON_S(5)
#define WHATSAPP_ADDR  ICON_S(6)
#define EXERCISE1_ADDR  ICON_S(7)
#define EXERCISE2_ADDR  ICON_S(8)
#define EXERCISE3_ADDR  ICON_S(9)
#define EXERCISE4_ADDR  ICON_S(10)
#define DRINK_ADDR  ICON_S(11)
#define STEP1_ADDR  ICON_S(12)
#define STEP2_ADDR  ICON_S(13)
#define COFFEE_ADDR  ICON_S(14)
#define CALORIES_ADDR  ICON_S(15)
#define PHONE1_ADDR  ICON_S(16)
#define PHONE2_ADDR  ICON_S(17)
#define BLE1_ADDR  ICON_S(18)
#define BLE2_ADDR  ICON_S(19)
#define BLE3_ADDR  ICON_S(20)
#define DISTANCE1_ADDR  ICON_S(21)
#define DISTANCE2_ADDR  ICON_S(22)
#define ALARM1_ADDR  ICON_S(23)
#define ALARM2_ADDR  ICON_S(24)
#define ALARM3_ADDR  ICON_S(25)
#define UPGRADE_ADDR  ICON_S(26)
#define SLEEP1_ADDR  ICON_S(27)
#define SLEEP2_ADDR  ICON_S(28)
#define SLEEP3_ADDR  ICON_S(29)
#define DONE_ADDR  ICON_S(30)//完成
#define STEP_DONE1_ADDR  ICON_S(31)//计步达标
#define STEP_DONE2_ADDR  ICON_S(32)
#define STEP_DONE3_ADDR  ICON_S(33)
#define WECHAT_ADDR  ICON_S(34)
#define HR1_ADDR  ICON_S(35)
#define HR2_ADDR  ICON_S(36)
#define HR3_ADDR  ICON_S(37)
#define HR4_ADDR  ICON_S(38)
#define PHONE_SMS_ADDR  ICON_S(39)
#define BLOOD_ADDR  ICON_S(40)
#define MEDICINE_ADDR  ICON_S(41)
#define PAIR_ADDR  ICON_S(42)
#define SNAP_ADDR  ICON_S(43)
#define STEP_TIME_ADDR  ICON_S(44)








void oled_display_on(bool on_flg);
void LCD_Init(void);
void LCD_Clear(uint16_t color);
void TFT_ShowBmp(uint16_t x,uint16_t y,uint16_t xsize,uint16_t ysize,const uint8_t *p);
void ShowIcon(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t*pdata,uint8_t mode);
void ShowIcon_Flash(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint32_t addr,uint8_t mode);

void TFT_ShowBmp_Flash_POINT(uint16_t x,uint16_t y,uint16_t xsize,uint16_t ysize,uint32_t addr,uint8_t mode);
void TFT_ShowBmp_Flash(uint16_t x,uint16_t y,uint16_t xsize,uint16_t ysize,const uint32_t addr);
void TFT_ST7735defineScrollArea(int16_t tfa, int16_t bfa);
void TFT_ST7735scroll(uint16_t adrs);
//void LcdDislayInArea(uint8_t x,uint8_t y,uint8_t*str,uint8_t arer_size,uint8_t str_size,uint8_t *displace,bool hz);
//void ShowString(unsigned char* front,uint16_t xstart,uint16_t ystart,uint16_t xsize,uint16_t ysize,uint8_t *data,uint8_t len);
#endif


