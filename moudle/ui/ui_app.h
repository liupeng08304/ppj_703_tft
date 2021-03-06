#ifndef UI_51822_H__
#define  UI_51822_H__
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "bsp.h"
#include "hr_app.h"
#include "protocol.h"
#include "ui_time.h"
#include "Battery_ADC.h"
typedef enum
{
    OLED_OFF_MENU=0,
    TIME_DIS_MENU,
    STEP_DIS_MENU,
    HR_DIS_MENU,
    BLOOD_DIS_MENU,
    DISTANCE_DIS_MENU,
    K_DIS_MENU,
    SPORT_TIME_MENU,
    SLEEP_DIS_MENU,
    PAIR_DIS_MENU,
    MSG_DIS_MENU,
    CHARGE_DIS_MENU,
    ALARM_DIS_MENU,

} ENUM_INDEX;

typedef ENUM_INDEX (*UiFunction)(ENUM_SEM sem);
typedef struct
{
    ENUM_INDEX index;
    uint8_t position;
} STU_ENABLE_INDEX;


extern ENUM_INDEX currentIndex;

void task_ui(void);
ENUM_INDEX LcdDisplayOff(ENUM_SEM sem);
ENUM_INDEX LcdDisplayTime(ENUM_SEM sem);
ENUM_INDEX LcdDisplaySteps(ENUM_SEM sem);
ENUM_INDEX LcdDisplayNotice(ENUM_SEM sem);
ENUM_INDEX LcdDisplayCharge(ENUM_SEM sem);
ENUM_INDEX LcdDisplayDistance(ENUM_SEM sem);
ENUM_INDEX LcdDisplayAlarm(ENUM_SEM sem);
ENUM_INDEX LcdDisplayK(ENUM_SEM sem);
ENUM_INDEX LcdDisplayHr(ENUM_SEM sem);
ENUM_INDEX LcdDisplaySportTime(ENUM_SEM sem);
ENUM_INDEX LcdDisplayBlood(ENUM_SEM sem);
ENUM_INDEX LcdDisplaySleep(ENUM_SEM sem);
ENUM_INDEX LcdDisplayPair(ENUM_SEM sem);




void ShowStartTime(void);
void ShowPowerLow(void);
void flash_oled_in_sleep_mode(uint8_t return_low_power_mode);
bool wait_key_to_power_on(void);
bool wait_key_to_power_off(void);
void showUpgredePersent(uint32_t persent);
void display_in_sleep_mode(uint8_t return_low_power_mode);

void TaskPrintfFile(uint8_t *dataout,uint16_t len);

//显示一定时间的版本号
void show_test_mode(uint8_t sec);


void show_ana_time(void);
bool wait_key_to_power_off(void);







#endif


