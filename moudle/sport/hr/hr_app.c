#include "hr_app.h"
#include "bsp.h"
#include "nrf_gpio.h"
#include "Battery_ADC.h"
#include "utility.h"
#include "stdio.h"
#include "string.h"
#include "pedometer_klx.h"
#include "data_use_api.h"
#include "hrs3300.h"
#include "bsp.h"
#include "ui_time.h"
#include "iic_gpio.h"
#include "lis3dh_driver.h"
STU_HR StuHr;
uint8_t start_bp=0;
uint8_t hr_ok=4;
volatile uint16_t irvalue=0;
APP_TIMER_DEF(br_bp_rtc_time);
bool is_bp_detect=false;

void hr_bp_event_handler(void * val);

uint8_t GetHrState(STU_HR *stuhr)
{
if(stuhr)
{
stuhr[0]=StuHr;
}
return StuHr.hr_statues;
}



void start_hr_bp_time(bool startflg)
{
    uint32_t err_code;

    if(true==startflg)
    {
        err_code = app_timer_start(br_bp_rtc_time,  APP_TIMER_TICKS(20) , NULL);
    } else
    {
        err_code = app_timer_stop(br_bp_rtc_time);

    }
    APP_ERROR_CHECK(err_code);
}

 void hrs3300_power_on(bool onflg)
{
    if(true==onflg)
    {
		HRS_I2C_CLK_HIGH;
		HRS_I2C_DATA_HIGH;
		nrf_gpio_pin_set(HRS_POWER);
    } else
    {
		HRS_I2C_CLK_LOW;
		HRS_I2C_DATA_LOW;
		nrf_gpio_pin_clear(HRS_POWER);
    }
}





void InitHeatRate(void)
{
	uint32_t err_code;
    StuHr.hr_fielt=60;
    StuHr.auto_detect_hr=false;
    StuHr.wearFlg=true;
    StuHr.hr_statues=stop;

	err_code = app_timer_create(&br_bp_rtc_time,
	APP_TIMER_MODE_REPEATED,
	hr_bp_event_handler);
	APP_ERROR_CHECK(err_code);
	
	

	HRS_I2C_DATA_OUTPUT;
	nrf_gpio_cfg_output(HRS_POWER);
	nrf_gpio_cfg_output(HRS_SCL_PIN);
	nrf_gpio_cfg_output(HRS_REV);
	nrf_gpio_pin_clear(HRS_REV);
	HRS_I2C_CLK_LOW;
	HRS_I2C_DATA_LOW;
	nrf_gpio_pin_clear(HRS_POWER);
	
}

void start_hr3300(bool detect_bp,bool start)
{

	is_bp_detect=detect_bp;

		//
		if(start==true)
		{
			hrs3300_power_on(start);
			delay_ms(10);
			hr_ok=4;
			if(false==Hrs3300_chip_init())// initial hrm chip
			{
			hr_ok=0;
			}
			Hrs3300_chip_enable();
			Hrs3300_set_exinf(0,0,0,0,0,0);
			
			#ifdef USE_LOW_RAM_HR
			Hrs3300_alg_open();
			#else
			if(true==detect_bp)
			Hrs3300_bp_alg_open(); // bp alg initial
			else
			Hrs3300_alg_open();
			#endif
			LOG("open 3300\r\n");
			start_hr_bp_time(start);
			
		}else
		{
			LOG("close 3300\r\n");
			start_hr_bp_time(start);
			Hrs3300_chip_disable();
			delay_ms(1);
			hrs3300_power_on(start);
		}
		
		return;
	

}




bool get_blood_statues(uint8_t* bloodhigh,uint8_t* bloodlow)
{

	if(StuHr.bpend==GET_BP)
	{
	*bloodhigh=StuHr.bph;
	*bloodlow=StuHr.bpl;
	return true;
	}		
	return false;

}



void hr_data_in(uint8_t hr)
{
	
	if( (hr>40)&&(hr<180)&&(  StuHr.hr!=hr))
	{
		StuHr.hr=hr;
		StuHr.hr_fielt=hr;
		StuHr.hr_statues=get_hr;
		StuHr.get_unit++;
		LOG("get hr\r\n");
	}
	if((StuHr.auto_detect_hr==true)&&((StuHr.work_time>=AUTO_TEST_TIME_OUT)||(StuHr.get_unit>1)))
	{
		SendSem(SEND_HR);
		StuHr.hr_statues=stop;
	}
}




void hr_bp_event_handler(void * val)
{
    static uint8_t inter;
    uint8_t hr,wear_flg;

	    inter++;
		#ifdef USE_LOW_RAM_HR
		if(inter&0x01)
		{
		heart_rate_meas_timeout_handler(&hr,&wear_flg,&StuHr.bpend,&StuHr.bph,&StuHr.bpl);//StuHr.wearFlg
		StuHr.work_time+=40;
		hr_data_in(hr);
		}
		#else
		if(true==is_bp_detect)
		{
			blood_presure_meas_timeout_handler(&StuHr.bpend,&StuHr.bph,&StuHr.bpl);
		}
		else
		{
			if(inter&0x01)
			{
			heart_rate_meas_timeout_handler(&hr,&wear_flg,&StuHr.bpend,&StuHr.bph,&StuHr.bpl);//StuHr.wearFlg
			StuHr.work_time+=40;
			hr_data_in(hr);
				
			}
		}
		#endif

}






/*
start_flg 为真开始监测,否则为假
*/
void StartHeatRate(uint8_t start_flg)
{
    if(true==start_flg)
    {
		StuHr.bpend=0;
		start_hr3300(start_bp,start_flg);
        StuHr.hr=0;
        StuHr.work_time=0;
        StuHr.get_unit=0;
        StuHr.hr_statues=hr_play;
    } else
    {

		start_hr3300(start_bp,start_flg);
		start_bp=0;
        StuHr.work_time=0;
        StuHr.hr_statues=stop;

    }
    StuHr.auto_detect_hr=false;
}




/*靠近检测*/
uint8_t  ProximityDet(uint16_t *dif_value)
{
    uint32_t i=0,sum=0;//,sum1;
	uint16_t value;

	start_hr3300(false,true);
	while(i < 10)
	{
		value=Hrs3300_read_hrs();
		sum +=value;
		delay_ms(42);
		i++;
		//LOGRTT("%d-%d\r\n",value,i);
		if(value >= ReadProx())
		{
		break;
		}
	}
	LOG("%d-%d-%d\r\n",sum,value,i);
	start_hr3300(false,false);
    irvalue=sum;
	LOG("sum:%d\r\n",sum);
    if(sum >= ReadProx())
        return 1;
    else
        return 0;
}






/*佩戴监测*/
bool HrCheckWear(uint16_t *hr_detect)
{

if((nrf_gpio_pin_read(CHARGER_CONNECTED_PIN))||(StuEeprom.StuPara.static_min>60))//usb connect
{
	StuHr.wearFlg=false;
	return StuHr.wearFlg;
}
    

#if(0==(BORD&USE_HR))
    StuHr.wearFlg=true;
    return StuHr.wearFlg;
#else
    StuHr.wearFlg=ProximityDet(hr_detect);
    return StuHr.wearFlg;
#endif
}


bool ReadWearFlg(void)
{
    return StuHr.wearFlg;
}


/*1min扫描一次*/
void TaskCheckWearAndTestHr(void)
{

#define DETECT_HR_MIN 5   //1 2 5 


        if((nrf_gpio_pin_read(CHARGER_CONNECTED_PIN))||(StuEeprom.StuPara.static_min>60))//usb connect
        {
            StuHr.wearFlg=false;
            return ;
        }
    
#if(0==(BORD&USE_HR))
    return;
#else
    if(StuHr.hr_statues!=stop) return;
    if(time.minutes%DETECT_HR_MIN) return;
    HrCheckWear(0);
    TaskAutoDetectHeartRate(StuEeprom.StuPara.hr_inter,time,ReadWearFlg());
#endif
}




uint8_t first_test_hr=0;
/*
心率定时测量

inter_min10
0停止
1：	10min
2:	20min
3:	30min
*/
void TaskAutoDetectHeartRate(uint8_t inter_min10,RTC_UTCTimeStruct tm,bool wear)
{
    uint8_t interval;

    if(wear==false)  return;
    if(EnumMotion==motion_working)  return;
    interval=inter_min10*10;
    if(0==inter_min10) return;



    if(tm.minutes%interval) return;

    StartHeatRate(true);
    StuHr.auto_detect_hr=true;
    first_test_hr=1;
}

//来通知的时候关闭HR
void NoticeCloseHr(void)
{

    if( (StuHr.auto_detect_hr==false)&&(StuHr.hr_statues!=stop))
    {
        StartHeatRate(false);
    }
}


//写测试到的心率
void WriteHeatRate(uint32_t timesec)
{
    uint8_t buf[20];
    static uint8_t minb=0xff;
    if( (StuHr.hr_fielt<45)||(StuHr.hr_fielt>180)) return;

    if(minb==time.minutes)   return;

    minb=time.minutes;

    //step :type+sec+step(4byte)+step_sec(2byte)+distance_cm(2byte)+calories(2byte)
//HR:  type+sec+ hr(1byte)
    memset(buf,0xff,20);
    buf[0]=StuHr.hr_fielt;
    usr_api_write(0xa8,buf,timesec);

}



void test1min_a_hr(void)
{
    if( (StuHr.hr_fielt<45)||(StuHr.hr_fielt>180))
        StuHr.hr_fielt=46;

    WriteHeatRate(StuEeprom.StuPara.time);
    StuHr.hr_fielt++;
}



/*
自动测试的时候超出范围重新测试一次
*/
void save_hr_or_redetect(void)
{
    uint8_t save=1;
    if((true==StuHr.auto_detect_hr)&&(first_test_hr))
    {
        //restart
        if( (StuHr.hr_fielt<44)||(StuHr.hr_fielt>120))
        {
            StartHeatRate(false);
            StartHeatRate(true);
            StuHr.auto_detect_hr=true;
            first_test_hr=0;
            save=0;
        }
    }
    if(save)
    {
        WriteHeatRate(StuEeprom.StuPara.time);
        StartHeatRate(false);
    }
}



