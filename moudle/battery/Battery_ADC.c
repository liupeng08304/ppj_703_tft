#include "Battery_ADC.h"
#include "nrf_gpio.h"
#include "utility.h"
#include "sem.h"
#include "bsp.h"
#include "hr_app.h"
#include "alarm.h"



STU_POWER StuPower;


uint8_t times_v;
int avg_sample;
void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{


    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {

//         float steinhart;
//         float sampleAVG;
        int i;
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, times_v);
        APP_ERROR_CHECK(err_code);
        for (i = 0; i < times_v; i++)
        {
            avg_sample += p_event->data.done.p_buffer[i]; // take N samples in a row
        }
        avg_sample /= i; // average all the samples out

    }
}



void InitAdc(void)
{
    ret_code_t err_code;


    err_code=nrf_drv_saadc_init(0,saadc_callback);
    APP_ERROR_CHECK(err_code);







}


// NRF_SAADC_INPUT_AIN6
// StartAdc(NRF_SAADC_INPUT_AIN3,4);
uint32_t StartAdc(nrf_saadc_input_t input,uint8_t times)
{
//    static uint8_t sec=0;
// 	if(sec++<8) return 0;
// 	sec=0;
//		ret_code_t err_code;
    nrf_saadc_value_t m_buffer_pool[100];



    nrf_saadc_channel_config_t channel_cfg=NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(input);
//		err_code=
    nrf_drv_saadc_channel_init(0,&channel_cfg);
// 		APP_ERROR_CHECK(err_code);


//		err_code=
    nrf_drv_saadc_buffer_convert(m_buffer_pool,times);
// 		APP_ERROR_CHECK(err_code);
    times_v=times;


    avg_sample=-100;
    while(avg_sample==-100)
    {
        if(NRF_SUCCESS!=nrf_drv_saadc_sample())
        {
            LOG("err\r\n");
        }
        delay_ms(1);
    }

    LOG("vol:%d %d\r\n",avg_sample,avg_sample*V_A/V_B);

    return avg_sample;

}




uint8_t hight_power_flg=0;





const uint16_t VoltageTable[8] =
{
//        4142,4011,3912,3831,3774,3734,3688,3605
    4142,3980,3880,3803,3752,3717,3683,3610
};
static const uint8_t PercentageTable[] =
{
    100,86,72,58,43,29,15,1
};



/*************************************************************************
* charger status
* 2 -> NoCharge; 1 -> ChargingComplete； 0 -> inCharging
                 28 CHARGER_CONNECTED    29 CHARGER_CHARGING
NoCharge               High                High
InCharging             Low                 High
ChargingComplete       High                Low
**************************************************************************/


/*0--> normal   1charge   2 charge complete*/
uint8_t  charger_status(void)
{


    bool connected = false;
    bool  charging = false;


    if(nrf_gpio_pin_read(CHARGER_CONNECTED_PIN))//usb connect
    {
        connected=true;
    }

    if(0==nrf_gpio_pin_read(CHARGER_CHARGING_PIN))
    {
        charging=true;
    }


    if(false==connected)
    {
        hight_power_flg=0;
        return CHARGE_NORMAL;
    } else
    {

        if(true ==charging)
        {
            if(hight_power_flg>29)
            {
                return CHARGE_NORMAL;
            } else
            {
                return CHARGE_ING;
            }

        } else
        {
            if(hight_power_flg>9)
                return CHAREG_END;
            else
                return CHARGE_ING;
        }
    }




}


uint8_t cal_percentage(uint16_t volatage)
{
    uint8_t length ;
    uint8_t i  = 0;

    if(CHAREG_END==charger_status())
    {
        return 100;
    }
    length=8;
    //find the first value which is < volatage
    for(i = 0; i< length ; i++)
    {
        if(volatage >= VoltageTable[i])
        {
            break;
        }
    }

    if( i>= length)
    {
        return 0;
    }

    if(i == 0)
    {
        return 100;
    }

    return (volatage - VoltageTable[i])/((VoltageTable[i-1] - VoltageTable[i])/(PercentageTable[i-1] - PercentageTable[i])) + PercentageTable[i];

}







STU_POWER StuPower;


uint8_t CalcVoltageLever(uint8_t persent)
{
    uint8_t value=0;
    value=persent/20;
    if(value>=5)
        return 4;
    else
        return value;

}

 uint8_t usb_in=0;
void InitPower(void)
{
    uint8_t i;
    uint32_t voltmp=0;
    nrf_gpio_cfg_input(CHARGER_CONNECTED_PIN,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CHARGER_CHARGING_PIN,NRF_GPIO_PIN_PULLUP);

   StuPower.charge_flg=CHARGE_NORMAL;

   usb_in= nrf_gpio_pin_read(CHARGER_CONNECTED_PIN);
   for(i=0; i<10; i++)
   {
       voltmp+=GetVol();
       delay_ms(100);
   }
   StuPower.vol=voltmp/10;
   StuPower.persent=cal_percentage(StuPower.vol);
   StuPower.lever=CalcVoltageLever(StuPower.persent);

}




void power_charge_statues(void)
{
    if(CHARGE_ING==charger_status())
    {
        if(StuPower.charge_flg!=CHARGE_ING)
        {
            StuPower.charge_flg=CHARGE_ING;
            /*发送消息，系统在充电*/
            SendSem(CHARGE);
        }

    } else
    {
        if(StuPower.charge_flg==CHARGE_ING)
        {
            StuPower.charge_flg=CHARGE_NORMAL;
            SendSem(CHARGE_DOWN);
        }
    }


}

//uint16_t vol;
uint16_t GetVol(void)
{
 
   return (StartAdc(BAT_CHANNEL,5)*V_A/V_B);
}

void TaskGetPower(void)
{
    static uint8_t send_lower_power=1;
    uint32_t vol,volb;
    STU_HR stuhr;
    volb=StuPower.vol;
    GetHrState(&stuhr);
    if(stuhr.hr_statues!=stop) return ;

    vol= GetVol();
    vol=((vol<<1)+(volb<<3))/10;
    StuPower.vol=vol;
    StuPower.persent=cal_percentage(StuPower.vol);
    StuPower.lever=CalcVoltageLever(StuPower.persent);

    if((1==send_lower_power)&&(StuPower.persent>40))
        send_lower_power=0;

    if((0==send_lower_power)&&(StuPower.persent<=20))
    {
        SendPower20Notice();
        send_lower_power=1;
    }

///////////////////////////////////////////////////////////////////////////////
    if(StuPower.charge_flg==CHARGE_NORMAL)
    {
        if(StuPower.vol<(VoltageTable[7]+5))//3615
            SendSem(LOW_POWER);
    }

    if(StuPower.vol>=4100)
    {
        if(hight_power_flg<100)
            hight_power_flg++;
    } else
    {
        hight_power_flg=0;
    }

}



