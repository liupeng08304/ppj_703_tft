#ifndef __APP_OLED_DRIVER_H
#define __APP_OLED_DRIVER_H

#include "nrf_gpio.h"
#include <stdbool.h>
#include "SoftwareRTC.h"
#include "bsp.h"



//// To be used with
//typedef struct
//{
//    uint16_t year;    // 2000+
//    uint8_t month;    // 0-11
//    uint8_t day;      // 0-30
//    uint8_t seconds;  // 0-59
//    uint8_t minutes;  // 0-59
//    uint8_t hour;     // 0-23
//}UTCTimeStruct;

typedef enum {
    InCharging = 0,       /* Device connect to the plugs*/
    ChargingComplete,      /* Show charging complete & still connect to plugs*/
    NoCharge           /* Show not connect to plugs*/
} Charging_State;


#define OLED_CMD  0
#define OLED_DATA 1
#define OLED_MODE 0

#define OLED_CS_Clr()  nrf_gpio_pin_clear(OLED_SPI_CS)       //spi片选
#define OLED_CS_Set()  nrf_gpio_pin_set(OLED_SPI_CS)

#define OLED_RST_Clr() nrf_gpio_pin_clear(OLED_RESET)      //oled复位信号
#define OLED_RST_Set() nrf_gpio_pin_set(OLED_RESET)

#define OLED_DC_Clr() nrf_gpio_pin_clear(OLED_SPI_MISO)       //数据输入
#define OLED_DC_Set() nrf_gpio_pin_set(OLED_SPI_MISO)

#define OLED_SCLK_Clr() nrf_gpio_pin_clear(OLED_SPI_SCK)     //时钟信号
#define OLED_SCLK_Set() nrf_gpio_pin_set(OLED_SPI_SCK)

#define OLED_SDIN_Clr() nrf_gpio_pin_clear(OLED_SPI_MOSI)   //数据输出
#define OLED_SDIN_Set() nrf_gpio_pin_set(OLED_SPI_MOSI)

#define spi_mosi_in   ((NRF_GPIO->IN >> OLED_SPI_MISO) & 0x1UL)        //输入spi_miso

#define spi_mosi_pin_in()  do { NRF_GPIO->DIRCLR = (1UL << OLED_SPI_MISO);  } while(0)   /*!< Configures SDA pin as input  */
#define spi_mosi_pin_out() do { NRF_GPIO->DIRSET = (1UL << OLED_SPI_MISO);  } while(0)   /*!< Configures SDA pin as output */

/*________________________________________________________________________*/

#define OledSetPos(page, column)								oled_set_pos(column, (2 + page))

//
#define OLED_SCREEN_ID_COLUMN_OFFSET							(0)
#define OLED_SCREEN_INFO_COLUMN_OFFSET						(17 + 1 + 2) //(17)
//
#define OLED_SCREEN_ID_ICON_W_SIZE								(17)
#define OLED_SCREEN_ID_ICON_H_SIZE								(32)
#define OLED_SCREEN_ID_ICON_MAP_SIZE							(68) //(52) //Bytes
#define OLED_SCREEN_INFO_ICON_W_SIZE							(12) //(13)
#define OLED_SCREEN_INFO_ICON_H_SIZE							(32)
#define OLED_SCREEN_INFO_ICON_MAP_SIZE						(48) //(52) //Bytes
//
#define OLED_INFO_START_FRAME_STILL_TIMEE					(10)
#define OLED_INFO_END_FRAME_STILL_TIMEE						(10)
//
#define OLED_SCREEN_NULL_ID												0xff
#define OLED_SCREEN_REFRESH_ID										0xfe

#define OLED_SCREEN_EVENT_CALL_ID									0x01//0x99//0x01
#define OLED_SCREEN_EVENT_SMS_ID									0x02//0x9a//0x02
#define OLED_SCREEN_EVENT_GENERIC_ID							0x03//0x9b//0x03
#define OLED_SCREEN_EVENT_GMSG_ID									0x04//0x9c//0x04
#define OLED_SCREEN_EVENT_WHATSAPP_ID							0x05//0x9d//0x05
#define OLED_SCREEN_EVENT_FACEBOOK_ID							0x06//0x9e//0x06
#define OLED_SCREEN_EVENT_TWITTER_ID							0x07//0x9f//0x07
#define OLED_SCREEN_EVENT_SKYPE_ID								0x08//0xa0//0x08

#define OLED_SCREEN_EVENT_QQ_ID							0x09//0x9f//0x07
#define OLED_SCREEN_EVENT_WEICHART_ID								0x0a//0xa0//0x08
#define OLED_SCREEN_EVENT_SNAPCHAT_ID  0x0b
///// OLED Home Screen
#define OLED_HOME_SCREEN_NULL_ID									0xff
#define OLED_HOME_SCREEN_REFRESH_ID								0xfe

#define OLED_HOME_SCREEN_REFRESH_ALL_ID						0xfd
#define OLED_HOME_SCREEN_REFRESH_TIME_ID					0xfc
#define OLED_HOME_SCREEN_REFRESH_DATE_ID					0xfb
#define OLED_HOME_SCREEN_REFRESH_NOTIFY_ID				0xfa
#define OLED_HOME_SCREEN_REFRESH_BATTERY_ID				0xf9
#define OLED_HOME_SCREEN_REFRESH_ONLY_BATTERY_ID	0xf8

#define OLED_HOME_SCREEN_REFRESH_DYNAMIC_ID				0xf7

//Time area
#define OLED_HOME_TIME_ICON_COUNT									(4 + 1)
#define OLED_HOME_TIME_ICON_W_SIZE								(14)
#define OLED_HOME_TIME_ICON_H_SIZE								(32)
#define OLED_HOME_TIME_ICON_MAP_SIZE							(14 * 4)

#define OLED_HOME_SCREEN_REFRESH_TIME_AREA_MASK		(1 << 0)

//Date area
#define OLED_HOME_DATE_ICON_COUNT									(5)
#define OLED_HOME_DATE_ICON_W_SIZE								(6)
#define OLED_HOME_DATE_ICON_H_SIZE								(8)
#define OLED_HOME_DATE_ICON_MAP_SIZE							(6 * 1)

#define OLED_HOME_DATE_ICON_PAG_BASE							(3)
#define OLED_HOME_DATE_ICON_COL_BASE							(((14 * 4) + 6 + 4) - 1)

#define OLED_HOME_SCREEN_REFRESH_DATE_AREA_MASK		(1 << 1)

//Battery area
#define OLED_HOME_BAT_ICON_COUNT									(1)
#define OLED_HOME_BAT_ICON_W_SIZE									(20)
#define OLED_HOME_BAT_ICON_H_SIZE									(8)
#define OLED_HOME_BAT_ICON_MAP_SIZE								(20 * 1)

#define OLED_HOME_BAT_ICON_PAG_BASE								(0)
#define OLED_HOME_BAT_ICON_COL_BASE								(((14 * 4) + 6 + 4) + 5 - 1)

#define OLED_HOME_SCREEN_REFRESH_BAT_AREA_MASK		(1 << 2)
//
#define OLED_HOME_ONLY_BAT_ICON_COUNT							(1)
//#ifdef CUS_SINGAPORE
#define OLED_HOME_ONLY_BAT_ICON_W_SIZE						(58)
#define OLED_HOME_ONLY_BAT_ICON_H_SIZE						(24)
#define OLED_HOME_ONLY_BAT_ICON_MAP_SIZE					(174)


#define OLED_HOME_ONLY_BAT_ICON_PAG_BASE					(0)
#define OLED_HOME_ONLY_BAT_ICON_COL_BASE					((96 - OLED_HOME_ONLY_BAT_ICON_W_SIZE) / 2)
//#else
//#define OLED_HOME_ONLY_BAT_ICON_W_SIZE						(64)
//#define OLED_HOME_ONLY_BAT_ICON_H_SIZE						(32)
//#define OLED_HOME_ONLY_BAT_ICON_MAP_SIZE					(64 * 4)

//#define OLED_HOME_ONLY_BAT_ICON_PAG_BASE					(0)
//#define OLED_HOME_ONLY_BAT_ICON_COL_BASE					((96 - OLED_HOME_ONLY_BAT_ICON_W_SIZE) / 2)
//#endif

#define OLED_HOME_SCREEN_REFRESH_ONLY_BAT_AREA_MASK				(1 << 3)

//Notify area
#define OLED_HOME_NOTIFY_ICON_COUNT								(3)
#define OLED_HOME_NOTIFY_ICON_W_SIZE							(10)
#define OLED_HOME_NOTIFY_ICON_H_SIZE							(16)
#define OLED_HOME_NOTIFY_ICON_MAP_SIZE						(10 * 2)

#define OLED_HOME_NOTIFY_ICON_PAG_BASE							(1)
#define OLED_HOME_NOTIFY_ICON_COL_BASE							OLED_HOME_DATE_ICON_COL_BASE

#define OLED_HOME_SCREEN_REFRESH_NOTIFY_AREA_MASK		(1 << 4)


#define OLED_PANEL_CONTENT_CALLING_INDEX						(1)
#define OLED_PANEL_CONTENT_HOME_INDEX								(2)
#define OLED_PANEL_CONTENT_BATTERY_ONLY_INDEX				(3)


//void Show_icon1(uint16_t startx,uint16_t starty,uint16_t xszie,uint16_t ysize,uint8_t* icon);


void oled_show_distance(unsigned int num);
//void oled_show_calories(unsigned char x,unsigned char y,unsigned short num);
//void oled_show_activity(unsigned char x,unsigned char y,unsigned short num);
//void oled_show_charging(unsigned char batt,Charging_State ch_stat,bool display);
//void oled_show_interaction(unsigned char IconTypeId,unsigned char *pStr,unsigned char Length);
//void oled_show_turn_over_wrist(bool on);
//static void spi_pin_init(void);
//static void spi_r_datcmd(unsigned char dat,unsigned char cmd); //spi写cmd:0命令 1：数据
//static unsigned char spi_rw(unsigned char dat);     					//spi读写

//void OLED_Fill(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char dot);
//void OLED_DrawPoint(unsigned char x,unsigned char y,unsigned char t);
//void oled_set_pos(unsigned char x, unsigned char y); //移动点

//void oled_display_1232(unsigned char x,unsigned char y,unsigned char data,unsigned char x_offset);
//void oled_display_2832(unsigned char x,unsigned char y,unsigned char data);
//void oled_display_3232(unsigned char x,unsigned char y,unsigned char data);
//void oled_display_6432(unsigned char x,unsigned char y,unsigned char data);
//void oled_display_59(unsigned char x,unsigned char y,unsigned char data);
//void oled_display_dot(unsigned char x,unsigned char y,unsigned char x_offset);
//void oled_display_1232num2(unsigned char x,unsigned char y,unsigned char num);
//void oled_display_59num2(unsigned char x,unsigned char y,unsigned char num);
//void oled_display_816(unsigned char x,unsigned char y,unsigned char x_offset,unsigned char y_offset,unsigned char data);
/****显示的界面函数*START********************************************************************************/

void oled_clear(void);
void oled_set_brightness(uint8_t value);
void oled_display_on(void);
void oled_display_off(void);
void oled_init(void);

/*主界面  参数1：时间  参数2： 电池容量  参数3：是否连接蓝牙*/
/*注意：参数2范围 ： 0 ~ 4*/
void oled_show_time(RTC_UTCTimeStruct tm,unsigned char batter_state, bool my_bond_stat,bool use_am_pm);

void oled_show_step(unsigned int num); /*脚步  计量*/

/*参数1： OLED_SCREEN_EVENT_CALL_ID 来电提醒功能
**        OLED_SCREEN_EVENT_SMS_ID  短信提醒功能
*/
//void OledPanelCallingDisplay(unsigned char IconTypeId, unsigned char *pStr, unsigned char Length);

void oled_show_boot_info(void);/*开机界面*/


/*充电图标
**注意：参数1： 范围 0 ~ 4
**/
void oled_show_battery_charging(unsigned char battery);


void oled_show_version(void);
void oled_show_HR(uint8_t num,uint8_t a_b);



void ShowNotice(uint8_t icon_num,uint8_t *msg,uint8_t msglen);

void oled_show_version_real(void);

void oled_show_upgrade(unsigned int num);

void oled_show_alarm(uint8_t hour,uint8_t min,bool use_am_pm,uint8_t ararm_type);

void oled_show_calories(unsigned int num);

uint8_t oled_power_read(void);
/****显示的界面函数*END********************************************************************************/
void oled_show_sporttime(unsigned int min);


void oled_show_result(uint8_t result);

void  show_battery(uint8_t battery);

void ShowString(unsigned char* front,uint16_t xstart,uint16_t ystart,uint16_t xsize,uint16_t ysize,uint8_t *data,uint8_t len);

void GUI_Line(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char colour);
void GUI_Full(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t colour);


void  DisplayQrCode(uint8_t startx,uint8_t starty,uint8_t*data,uint8_t len);
void oled_show_blood(uint8_t hight,uint8_t low,uint8_t a_b);

#endif

