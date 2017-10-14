#ifndef ALARM_H___
#define ALARM_H___

#include "stdint.h"
#include "stdbool.h"

#define ALARM_MEDICION 		0x00
#define ALARM_DRINKS  		(ALARM_MEDICION+1)
#define ALARM_COFFEE 			(ALARM_DRINKS+1)
#define ALARM_ALARM 			(ALARM_COFFEE+1)
#define ALARM_SLEEP 			(ALARM_ALARM+1)
#define ALARM_EXERCISE 		(ALARM_SLEEP+1)
#define ALARM_RUN 				(ALARM_EXERCISE+1)
#define ALARM_GET_TARGET 				(ALARM_RUN+1)

void TaskAlarmCheck(void);


//发送电量低于20%
void SendPower20Notice(void);


void TaskSendentaryCheck(bool wear);




void TestPrintfSedentary(void);

void SendMacShowNotice(void);

#endif


