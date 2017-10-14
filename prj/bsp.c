#include "bsp.h"
#include "s132config.h"
#include "app_button.h"
#include "nrf_soc.h"
#include "SoftwareRTC.h"
#include "Battery_ADC.h"
#include "wdt.h"
#include "ui_app.h"
#include "ui_app.h"
#include "app_timer.h"
#include "upfile.h"
#include "alarm.h"
#include "lis3dh_driver.h"
#include "sleep.h"
#include "hr_app.h"
#include "queueu51822.h"
#include "app_timer.h"
#include "in_flash_manage.h"
#include "pedometer_klx.h"


uint8_t g_sensor_ok=2;
uint32_t error_code1;
uint8_t tmp_sec;
uint8_t min;

APP_TIMER_DEF(m_rtc_time_id);
void rtc_event_handler(void * val)
{

  
    StuEeprom.StuPara.time+=10;
	tmp_sec=0;
	
    min=(StuEeprom.StuPara.time % 3600UL) / 60UL;
    if(min!=time.minutes)
        SendSem(TIME_MINUTE);



}


void GetTimeAndSec(  RTC_UTCTimeStruct *time1)
{

  ConvertToUTCTime(time1,StuEeprom.StuPara.time+tmp_sec/2);

}

void RtcTimeInit(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_rtc_time_id,
                                APP_TIMER_MODE_REPEATED,
                                rtc_event_handler);
    APP_ERROR_CHECK(err_code);



    err_code = app_timer_start(m_rtc_time_id,  APP_TIMER_TICKS(10000) , NULL);
    APP_ERROR_CHECK(err_code);
}


APP_TIMER_DEF(m_acc_time_id);

void acc_time_event_handler(void * val)
{

    tmp_sec++;

    SendSem(GET_G_SENSOR);

}
void init_acc_time(void)
{

    uint32_t err_code;
    err_code = app_timer_create(&m_acc_time_id,
                                APP_TIMER_MODE_REPEATED,
                                acc_time_event_handler);
    APP_ERROR_CHECK(err_code);

}


void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if((KEY_PIN==pin_no)&&(button_action==APP_BUTTON_PUSH))
    {
        SendSem(PRESS_KEY);
        LOG("key\r\n");
    }

}


void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {KEY_PIN, APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_PULLUP,button_event_handler}
    };

	delay_ms(200);
	nrf_gpio_pin_set(KEY_IC_POWER_PIN);

	
	
	app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]),APP_TIMER_TICKS(100));
	err_code = app_button_enable();
	APP_ERROR_CHECK(err_code);
}




void start_acc_time(bool start_flg)
{
    uint32_t err_code;
    if(true==start_flg)
    {
        err_code = app_timer_start(m_acc_time_id,  APP_TIMER_TICKS(500) , NULL);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        err_code = app_timer_stop(m_acc_time_id);
        APP_ERROR_CHECK(err_code);
    }

}
void LowerPower(void)
{
    power_manage();
    V_FeedWdog();

}



bool wait_key_to_power_on(void)
{
	
	#if 1
    static uint8_t sta=0,press_wait_time;

    switch(sta)
    {
    case 0:/*等待按键释放*/
        if(ReadKey())/*释放了*/
        {
            delay_ms(50);
            if(ReadKey())/*释放了*/
            {
                press_wait_time=20;
                sta++;
            }
        }
        break;
    case 1:/*等待按下*/
        if(!ReadKey())
        {
            press_wait_time=20;
            start_sys_time(START_TIME,true);
            sta++;
        }
        break;
    case 2:/*按下时间计算时间*/
        if(ReadKey())/*释放了*/
        {
            start_sys_time(START_TIME,false);
            sta=1;
            break;
        }
        while(GetSem()==REFLASH_UI)
        {
            press_wait_time--;
            if(0==press_wait_time)
            {
                start_sys_time(START_TIME,false);
                return  true;
            }
        }
        break;

    }

#endif

    return false;
}


void ManagePowerOn(void)
{
   uint32_t secb;
    uint8_t ansy=1;
    /*电压要大于3.5v  关机标志，充电 开机*/


    /*开机对电压的要求*/
    while(1)//3650   (3615关机，开机3620 改为3650)
    {
		if(StuEeprom.StuPara.time!=secb)
		{
			if(GetVol()>(VoltageTable[7]+40))
			{
			break;
			}
			secb=StuEeprom.StuPara.time;
		}
        LowerPower();
       display_in_sleep_mode(0);///////////////////////////////xxxx

    }
   display_in_sleep_mode(1);///////////////////////////////xxxx

    /*key*/
   buttons_init();

    /*显示版本号等待按下*/
    if(StuEeprom.StuPara.ask_version)
    {
        while(true!=wait_key_to_power_off())
        {
            LowerPower();
        }
        StuEeprom.StuPara.ask_version=0;
        ansy=1;

    }


    /*有关机标志，需要等待按键开机，先释放，然后按下*/
    if(StuEeprom.StuPara.ask_power_off)
    {

        if(2==StuEeprom.StuPara.ask_power_off)
        {
            while(charger_status()==CHARGE_NORMAL)
            {
                LowerPower();//
            }
        }
        else
        {
            while((true!=wait_key_to_power_on())&&(charger_status()==CHARGE_NORMAL))
            {
                LowerPower();//
            }
        }

    }



    if(StuEeprom.StuPara.ask_reset)
    {
        StuEeprom.StuPara.ask_reset=0;
        ansy=1;
        /*调用开机界面*/
      //  ShowStartUi();///////////////////////////////xxxx
    }



    if(StuEeprom.StuPara.ask_power_off)
    {
        EEpromSetParaToFactory();
        erase_sleep_flash();
        DeleteQueue();
        StuEeprom.StuPara.ask_power_off=0;
        ansy=1;
    }

    if(ansy)
    {
        EepromAnsy();
    }

}


//删除历史数据中和今天相同的计步数据
void delete_history_data_same_to_today(void)
{

    RTC_UTCTimeStruct time_totay_data;
    uint32_t today_data_in_sec,i;
    time_totay_data.year=time.year;
    time_totay_data.month=time.month;
    time_totay_data.day=time.day;
    time_totay_data.hour=0;
    time_totay_data.minutes=0;
    time_totay_data.seconds=0;
    today_data_in_sec=convert_time_to_Second(time_totay_data);

    /*查看日期是否有重复的，和今天相同日期的删除*/
    for(i=0; i<HISTORY_DATA_ITEM; i++)
    {
        if(StuEeprom.StuPara.StuHistory[i].time==today_data_in_sec)
        {
            StuEeprom.StuPara.StuHistory[i].time=0;
        }

    }

}



void data_process_if_change_day(void)
{
//  #define BEFORE_ZERO
    static uint8_t day_before=0xff;
    static uint32_t sec;

    RTC_UTCTimeStruct time_tmp;
    if(day_before==0xff)
    {
        day_before=time.day;
        sec=StuEeprom.StuPara.time;
        return;
    }

    if(time.day!=day_before)
    {

        if(StuEeprom.StuPara.time>sec)//8--->9,时间往前走，保存之前的，现在的为0
        {
            ConvertToUTCTime(&time_tmp,sec);
            if(StuEeprom.StuPara.step)
                write_para_history(time_tmp.year,time_tmp.month,time_tmp.day);
            StuEeprom.StuPara.step=0;
            StuEeprom.StuPara.step_sec=0;
            StuEeprom.StuPara.distance_cm=0;
            StuEeprom.StuPara.calories=0;
            StuEeprom.StuPara.ask_reset=0;
        }
        else  //8--->7,8号不保存，7号为0/或者7号为之前的步数
        {

#ifdef BEFORE_ZERO//8--->7,时间后退。之前的不保存，现在的为0
            StuEeprom.StuPara.step=0;
            StuEeprom.StuPara.step_sec=0;
            StuEeprom.StuPara.distance_cm=0;
            StuEeprom.StuPara.calories=0;
            StuEeprom.StuPara.ask_reset=0;
#else  //8--->7,之前的不保存，现在的为之前的数据

#endif
            delete_history_data_same_to_today();
        }


        EepromAnsy();

        /*凌晨清零 并且重启手环*/
        if((time.minutes==0)&&(time.hour==0))
        {
            CallErr(DISPLAY_HELLO_UI+1,0,(const uint8_t *)"save_last_day and off");
        }


    }

    day_before=time.day;
    sec=StuEeprom.StuPara.time;
}
void AccForLcdOnOff(int16_t xvalue,int16_t yvalue,int16_t zvalue,uint32_t total)
{
    static uint8_t sta=0;
    static uint16_t TrigTime=0;
    uint16_t tmp;
    if(TrigTime<99)
    {
        TrigTime++;
    }

    switch(sta)
    {
        case 0:
        case 1:
        case 2:
            tmp=abs(xvalue);
            if( (tmp>850)||(yvalue<-900)) /*下垂或者侧放  900 700   700*/
            {
                sta++;
                if(tmp>850)
                {
                    TrigTime=0;
                    SendSem(OLED_OFF);

                }
                if(3==sta)
                    TrigTime=0;
            }
            else
            {
                sta=0;
            }
            break;
        case 3:
        case 4:
        case 5:
            zvalue=-zvalue;
            if(zvalue>800)/*水平的时候亮屏幕*/
            {
                sta++;
                if(sta>5)
                {
                    TrigTime=0;
                    SendSem(OLED_ON);
                    sta=0;
                }
            }

            break;
    }


}


void calc_step_time(uint8_t change_step,uint32_t current_step)
{
    static uint8_t in_working=0,half_sec=0;

    if(change_step)
    {
        in_working=4;
    }
    if(in_working)
        in_working--;

    half_sec++;
    if(in_working)
    {
        if(half_sec&0x01)
        {
            StuEeprom.StuPara.step_sec++;
        }
    }


}

void ana_step_datas_in_500ms(uint8_t change_step,uint32_t total_step)
{
    if(change_step)
    {
        StuEeprom.StuPara.distance_cm=total_step*StuEeprom.StuPara.hight*75/175;
        StuEeprom.StuPara.calories=(total_step*StuEeprom.StuPara.weight*StuEeprom.StuPara.hight/170)>>10;
    }
    calc_step_time(change_step,total_step);

}



void updateDateInMin(bool save_flg)
{
    bool wear_flg;
    /*1min调用一次*/
    ConvertToUTCTime(&time,StuEeprom.StuPara.time);
    if(true==save_flg)
    {
        TaskCheckWearAndTestHr();//
        TaskGetPower();
        TaskAlarmCheck();
        wear_flg=ReadWearFlg();
        TaskSendentaryCheck(wear_flg);
        TaskSleep(time.hour,time.minutes,time.week,(true==wear_flg)? WEAR:UN_WEAR);
    }
    data_process_if_change_day();
    if(time.minutes%10)  return;
    /*10min调用一次*/
    if(true==save_flg)
        EepromAnsy();/*保存一次现场数据*/
}

// #define MAX_ACC  20
//  AxesRaw_t g_axes[MAX_ACC];
// uint8_t accin=0,accout=0,acc_have_flg=0;

// #define BLE_BUF_SIZE 10
// static bool BleQueueflg=false;
// static uint8_t BleIn=0,BleOut=0;
// static uint8_t BleBuf[BLE_BUF_SIZE][21];

// bool BleQueueIn(uint8_t*datain,uint8_t len)
// {

// 		memcpy(&BleBuf[BleIn][1],datain,len);
// 		BleBuf[BleIn][0]=len;
// 		BleIn++;
// 		BleIn%=BLE_BUF_SIZE;
// 		BleQueueflg=true;
// 		return true;


// }

// void BleQueueAllOut(void)
// {
// 	BleOut=0;
// 	BleIn=0;
// 	BleQueueflg = false; 

// }
// bool BleQueueOut(uint8_t*dataout,uint8_t* len)
// {

// 	if(BleQueueflg!=true) 
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		*len=BleBuf[BleOut][0];
// 		memcpy(dataout,&BleBuf[BleOut][1],*len);
// 		BleOut++;
// 		BleOut %= BLE_BUF_SIZE; 
// 		if(BleOut==BleIn)
// 		{
// 		BleQueueflg = false; 
// 		}
// 		return true;
// 	}
// }

 AxesRaw_t axes[32];
 uint8_t acc_cnt=0,acc_max=0;
 
 AxesRaw_t get_acc_data(void)
 {
	 uint8_t cnt;
	 cnt=acc_cnt;
	 acc_cnt++;
 return axes[cnt%acc_max];
}
uint8_t  TaskSport(void)
{
    uint8_t get_data_item;
    uint8_t i,add_step;
    int16_t xt,yt,zt;

    uint32_t AccSensorData[32];

    if(g_sensor_ok==0) return 0;
    get_data_item=GetData(axes);
//	LOG("cnt:%d,now:%d\r\n",acc_cnt,get_data_item);
	acc_cnt=0;
	acc_max=get_data_item;
    for(i = 0; i < get_data_item; i++)
    {
        xt=((int16_t)axes[i].AXIS_X)>>4;
        yt=((int16_t)axes[i].AXIS_Y)>>4;
        zt=((int16_t)axes[i].AXIS_Z)>>4;
        AccSensorData[i] = xt*xt+yt*yt+zt*zt;
        AccSensorData[i] /= 100;

        CallByGsensor(xt,yt,zt);
        if(OLED_AUTO_ON==StuEeprom.StuPara.oled_auto_on)
        {

//            //水平  0           0               -1024
//            //下垂  1024    0             0
//            //侧放  0             -1024           0
//            AccForLcdOnOff(xt,yt,zt,AccSensorData[i]);

            AccForLcdOnOff(-yt,xt,zt,AccSensorData[i]);

        }

    }


    add_step=SportsProcess(get_data_item,AccSensorData);
    if(add_step)
    {

        StuEeprom.StuPara.step+=add_step;
        LOG("add step:%d\r\n",StuEeprom.StuPara.step);
    }
    return add_step;

}



APP_TIMER_DEF(m_motor_timer_id);          ;
uint8_t motorworktimes=0, on_100uints, off_100uints,len_sta;
static void motor_timeout_handler(void * p_context)
{
    static uint8_t endtick=0;
    switch(len_sta)
    {
        case 0:
            if(motorworktimes)
            {
                endtick=on_100uints;
                MOTOR_ON();
                len_sta++;
            }
            break;
        case 1:
            endtick--;
            if(0==endtick)
            {
                endtick=off_100uints;
                MOTOR_OFF();
                len_sta++;
            }
            break;
        case 2:
            endtick--;
            if(0==endtick)
            {
                motorworktimes--;
                if(0==motorworktimes)
                {
                    len_sta=0;
                    SendSem(END_MOTOR);
                    break;
                }
                else
                {
                    endtick=on_100uints;
                    MOTOR_ON();
                    len_sta=1;
                }
            }
            break;

    }


}


void InitMotor(void)
{
    uint32_t err_code;
    nrf_gpio_cfg_output(MOTOR_PIN);
    MOTOR_OFF();
    // Create security request timer.
    err_code = app_timer_create(&m_motor_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                motor_timeout_handler);
    APP_ERROR_CHECK(err_code);

}

void MotorOn(uint8_t times,uint8_t on100uints,uint8_t off100uints)
{
    uint32_t err_code;
    motorworktimes=times;
    on_100uints=on100uints;
    off_100uints=off100uints;
    len_sta=0;
    if(times)
    {
        err_code      = app_timer_start(m_motor_timer_id, APP_TIMER_TICKS(100) , NULL);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        app_timer_stop(m_motor_timer_id);
    }
}





// uint16_t key_press_500ms_times=0;

/*响应系统消息*/
void task_process_message(  ENUM_SEM rev_sem)
{
    uint8_t change_step;

    static uint8_t rfid_sec=0;
    switch(rev_sem)
    {
			

        case TIME_MINUTE:
            updateDateInMin(true);
            break;
        case END_MOTOR:
            MotorOn(0,0,0);
            break;
        case GET_G_SENSOR:
            change_step=TaskSport();
            if(0!=change_step)
            {
                if(currentIndex==STEP_DIS_MENU)
                    SendSem(STEP_COME);
            }
            ana_step_datas_in_500ms(change_step,StuEeprom.StuPara.step);
            power_charge_statues();
//			if(0 == ReadKey())
//			{
//			LOG("pres_g\r\n");
//			}
			return_to_normal_speed();
            if((IsBleConnect()==false)&&(rfid_sec++>=(RFID_SEND_INTER_SEC-1)))
            {
                rfid_sec=0;
                TaskUpFile();
            }
            check_send_hr_real_time();
            break;
        case SEND_REAL_TIME_DATA:
            send_real_time_datas();
            break;
        case LOW_POWER:
            ShowPowerLow();
            break;
        case SEND_HR:
            save_hr_or_redetect();
            break;
        default:
            break;

    }



}



void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{

    error_code1=error_code;
    if(DISPLAY_HELLO_UI==error_code)
    {
        StuEeprom.StuPara.ask_reset=1;
    }
    StuEeprom.StuPara.reset_time=StuEeprom.StuPara.time;
    StuEeprom.StuPara.ResetReson=error_code;
    EepromAnsy();
//    system_off();
    LOG("fault err:%x-%d-%s\r\n",error_code,line_num,p_file_name);
    NVIC_SystemReset();


}
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    LOG("fault err:%x-%x-%x\r\n",id,pc,info);
    app_error_handler(0,0,0);
}


void HardFault_Handler(void)
{
    LOG("hardfault\r\n");
    app_error_handler(0,0,0);
}

