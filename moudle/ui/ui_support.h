#ifndef UI_SHOW_SUPPORT_H___
#define UI_SHOW_SUPPORT_H___
#include "lcd.h"
#include "SoftwareRTC.h"
#include "font_band.h"
#include "in_flash_manage.h"
#include "nrf_log.h"

void lcd_show_time(uint8_t type,RTC_UTCTimeStruct  tm,unsigned char batter_state, bool my_bond_stat);
void lcd_show_step(uint32_t step,uint32_t goal,uint8_t cnt);
void lcd_show_HR(uint8_t num,uint8_t a_b);
void lcd_show_distance(uint32_t display_value,uint8_t a_b);
void lcd_show_K(uint32_t display_value,uint8_t a_b);
void lcd_show_sport_time(uint32_t display_value,uint8_t a_b);
void LcdShowNotice(uint8_t icon_num,uint8_t *msg,uint8_t msglen);
void lcd_show_battery_charging(uint8_t battery_lever);
void lcd_show_battery(uint8_t persent,uint8_t a_b);
void lcd_show_alarm(uint8_t hour,uint8_t min,bool use_am_pm,uint8_t ararm_type);
void lcd_show_sleep(uint32_t display_value,uint8_t a_b);
void lcd_show_pair(uint8_t a_b);
void DisplayHrTime(RTC_UTCTimeStruct  tm,uint8_t hr);

void Printfdata(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t*buf,uint8_t len,uint8_t*front,uint16_t size);
void oled_show_version(void);
#endif



