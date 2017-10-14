#ifndef __SEM_51822_H___
#define __SEM_51822_H___

#include "stdint.h"
#include "stdbool.h"
typedef enum
{
NO_SEM=0,
PRESS_KEY,//
TIME_MINUTE,
END_MOTOR,
LOW_POWER,//
NOTICE_COM,//
GET_G_SENSOR,
CHARGE,
CHARGE_DOWN,
OLED_ON,
OLED_OFF,
REFLASH_UI,
STEP_COME,
INIT_UI,
SEND_HR,
SEND_REAL_TIME_DATA,
}ENUM_SEM;


void SendSem(ENUM_SEM Sem);
ENUM_SEM GetSem(void);
void SemEmpty(void);
bool IsSemEmpty(void);

#endif



