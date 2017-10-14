#ifndef BATTERY_ADC_H__
#define BATTERY_ADC_H__

#include "stdint.h"
#include "nrf_saadc.h"
#include "nrf_drv_saadc.h"
#include "nrf_log.h"



#define CHARGE_NORMAL 0
#define CHARGE_ING   1
#define CHAREG_END   2


typedef struct
{
	uint8_t charge_flg;
	uint16_t vol;
	uint16_t persent;
	uint16_t lever;
}STU_POWER;

extern STU_POWER StuPower;
extern const uint16_t VoltageTable[8] ;

 uint16_t Adc_Read(void);
//HR_CHANNEL  ADC_CONFIG_RES_8bit
void ConfigAdc(uint32_t channel,uint32_t adcbit);

uint16_t GetVol(void);

void TaskGetPower(void);
void power_charge_statues(void);
void InitPower(void);
uint8_t  charger_status(void);






 void InitAdc(void);
uint32_t StartAdc(nrf_saadc_input_t input,uint8_t times);
#endif


