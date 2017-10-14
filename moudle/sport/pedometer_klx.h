#ifndef PEDOMETER_KLX_H__
#define PEDOMETER_KLX_H__
#include "stdint.h"
#include "stdbool.h"
#include "bsp.h"

#define TIME_1_SEC   25


#define FILTER_POINT  5

typedef struct
{
    uint16_t buffer[FILTER_POINT];
    uint16_t amp_thres;
    uint8_t interval_thres;
} STU_G_SENSOR;



typedef enum
{
    motion_static,
    motion_working,
    motion_running,
    motion_other
} ENUM_MOTION;


extern ENUM_MOTION EnumMotion;

///*********************************
//10000--1g     10---0.4s
//**********************************/
//void InitCntStep(uint16_t amp_thres, uint8_t interval_thres);/*10000*20/256=780  ,10*/

///*10000---1g*/
//uint16_t step_counter(uint16_t g_sensorin);

#define BEGIN_STEP  12
//#define BEGIN_STEP_NIGHT  20
#define ERROR_CNTS   50
#define IN_STEP_ERROR_CNTS   2

uint8_t SportsProcess(uint8_t get_data_item,uint32_t *AccSensorData);
void InitStep(void);

#endif


