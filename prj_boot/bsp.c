

#include "bsp.h"
#include "s110config.h"
#include "app_button.h"
#include "nrf_soc.h"
#include "in_flash_manage.h"
#include "SoftwareRTC.h"
#include "Battery_ADC.h"
#include "wdt.h"
#include "protocol.h"
#include "ui_app.h"
#include "pedometer.h"
#include "ui_app.h"
#include "app_timer.h"
#include "upfile.h"
#include "alarm.h"
#include "app_oled_driver.h"
#include "lis3dh_driver.h"
#include "sleep.h"
#include "hr_app.h"
#include "queueu51822.h"
//USE_GO_UI

//#ifdef VERSION_IFIT_NO_HR
//const uint8_t version[8] __attribute__ ((at(0x18000+1024)))= {2,0,10,'L','H',0xdd,0,10};
//#else
//const uint8_t version[8] __attribute__ ((at(0x18000+1024)))= {2,1,10,'L','H',0xdd,1,10};
//#endif

 


#if(BORD==BORD_EE_00)
const uint8_t version[8] __attribute__ ((at(0x18000+1024)))= {2,0,23,'L','H',0xee,0,25};
#elif(BORD==BORD_EE_01)
const uint8_t version[8] __attribute__ ((at(0x18000+1024)))= {2,1,24,'L','H',0xee,1,25};
#elif(BORD==BORD_CC_00)
const uint8_t version[8] __attribute__ ((at(0x18000+1024)))= {2,0,23,'L','H',0xcc,0,25};
#elif(BORD==BORD_CC_01)
const uint8_t version[8] __attribute__ ((at(0x18000+1024)))= {2,1,24,'L','H',0xcc,1,25};
#endif







/*
修改记录:
cc,1,20   解决分离器件，佩戴监测的问题，佩戴监测的值，固定为1600,
					(出现之前差值大的原因是测量佩戴监测前，运放电压没有稳定)

cc,1,21   佩戴监测的值，固定为2000, 2017_05_12
					蓝牙速度调整
					7点到19点oled高亮，其他时间低亮
				

*/

uint8_t g_sensor_ok=2;

uint32_t error_code1;


#ifdef PRINTF_ANCS
#define FIFO_LEN  32
uint8_t buffifo[FIFO_LEN];
uint16_t fifolen=0;
void simple_uart_put(uint8_t cr)
{
    uint16_t counter=16*100;
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        counter--;
        if(counter==0)
        {
            NRF_UART0->EVENTS_TXDRDY = 0;
            return;
        }
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}




void UartEnabe(bool flg)
{
    if(true==flg)
    {
        NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
    } else
    {
        NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);

    }
    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;
}


void UartInit(void)
{
    nrf_gpio_cfg_output(TX_PIN);
    nrf_gpio_pin_set(TX_PIN);

    NRF_UART0->PSELTXD = TX_PIN;
    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
    UartEnabe(true);

     uint8_t buf_printf[100];
    UsartTx(buf_printf,sprintf((char*)buf_printf,"band start.......\r\n"));
    usart_send_buf();
}


void UsartTx(uint8_t *buf,uint16_t len)
{

    if((fifolen+len)<FIFO_LEN)
    {
        memcpy(&buffifo[fifolen],buf,len);
        fifolen+=len;
    }


}


void UsartPrintfHex(uint8_t *buf,uint16_t len)
{
	uint16_t i;
	if((fifolen+len*2)<FIFO_LEN)
	{
	for(i=0; i<len; i++)
	fifolen+=sprintf((char*)&buffifo[fifolen],"%02x",buf[i]);
	}

}


void usart_send_buf(void)
{
    uint16_t i;

    if(fifolen)
    {
        for(i=0; i<fifolen; i++)
            simple_uart_put(buffifo[i]);

        fifolen=0;
    }
}



#endif




void CallErr(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    error_code1=error_code;
    if(DISPLAY_HELLO_UI==error_code)
    {
        StuEeprom.StuPara.ask_reset=1;
    }
    StuEeprom.StuPara.reset_time=StuEeprom.StuPara.time;
    StuEeprom.StuPara.ResetReson=error_code;
    EepromAnsy();
    system_off();
}
//static uint32_t escapebuf[20],ies=0;
void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if((KEY_PIN==pin_no)&&(button_action==APP_BUTTON_PUSH))
    {
        SendSem(PRESS_KEY);
    }

}


void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {KEY_PIN, APP_BUTTON_ACTIVE_LOW,NRF_GPIO_PIN_NOPULL,button_event_handler}
    };

    delay_ms(500);
    nrf_gpio_cfg_input(KEY_PIN,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_pin_set(KEY_IC_POWER_PIN);
		

    app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]),APP_TIMER_TICKS(50,APP_TIMER_PRESCALER));
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}










app_timer_id_t m_rtc_time_id;


void rtc_event_handler(void * val)
{

//	static uint8_t sec=0;
    uint8_t min;
//	if(sec++&0x01)
	{
    StuEeprom.StuPara.time+=10;
		min=(StuEeprom.StuPara.time % 3600UL) / 60UL;
		if(min!=time.minutes)
		SendSem(TIME_MINUTE);
	}

}




void RtcTimeInit(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_rtc_time_id,
                                APP_TIMER_MODE_REPEATED,
                                rtc_event_handler);
    APP_ERROR_CHECK(err_code);



    err_code = app_timer_start(m_rtc_time_id,  APP_TIMER_TICKS(10000, APP_TIMER_PRESCALER) , NULL);
    APP_ERROR_CHECK(err_code);
}





void sleep_mode_enter(void)
{
    uint32_t err_code;

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}
void save_last_day(void)
{
    uint32_t timelastday;
    RTC_UTCTimeStruct time_tmp;
    timelastday=StuEeprom.StuPara.time-600;
    ConvertToUTCTime(&time_tmp,timelastday);
    write_para_history(time_tmp.year,time_tmp.month,time_tmp.day);

    StuEeprom.StuPara.step=0;
    StuEeprom.StuPara.step_sec=0;
    StuEeprom.StuPara.distance_cm=0;
    StuEeprom.StuPara.calories=0;
    StuEeprom.StuPara.ask_reset=0;

//    StuEeprom.StuPara.time+=10;
    EepromAnsy();
    CallErr(DISPLAY_HELLO_UI+1,0,(const uint8_t *)"save_last_day and off");
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
        } else
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
    StuEeprom.StuPara.distance_cm=total_step*StuBandParaHistoryData.hight*75/175;
    StuEeprom.StuPara.calories=(total_step*StuBandParaHistoryData.weight*StuBandParaHistoryData.hight/170)>>10;
	 }
		calc_step_time(change_step,total_step);

}

void updateDateInMin(bool save_flg)
{
 bool wear_flg;
//	static uint8_t wear_min=0;
    /*1min调用一次*/
    ConvertToUTCTime(&time,StuEeprom.StuPara.time);
	  TaskCheckWearAndTestHr();
		if(true==save_flg)
		{
		#ifdef ENABLE_SLEEP

		#endif
		
			TaskGetPower();
			TaskAlarmCheck();
		
//			wear_flg=HrCheckWear(0);
			 wear_flg=ReadWearFlg();
			

			TaskSendentaryCheck(wear_flg);
//			if(true==wear_flg)
//				wear_min=20;
//			if(wear_min)
//			wear_min--;
			
		  TaskSleep(time.hour,time.minutes,time.week,(true==wear_flg)? WEAR:UN_WEAR);
		}
	
    if(time.minutes%10)  return;
    /*10min调用一次*/
    if(true==save_flg)
        EepromAnsy();/*保存一次现场数据*/
    /*凌晨清零 并且重启手环*/
    if((time.minutes==0)&&(time.hour==0))
    {
        save_last_day();
    }
}






/*响应系统消息*/
void task_process_message(	ENUM_SEM rev_sem)
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

        if(!ReadKey())
        {
					
            key_press_500ms_times++;
						if((key_press_500ms_times==20)&&(IsBleConnect()==false))
						{
						SendMacShowNotice();
						}
					
        } else
        {
            key_press_500ms_times=0;
        }
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






static app_timer_id_t            m_motor_timer_id;
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
            } else
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
        err_code      = app_timer_start(m_motor_timer_id, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER) , NULL);
        APP_ERROR_CHECK(err_code);
    } else
    {
        app_timer_stop(m_motor_timer_id);
    }
}



void read_time(void)
{
    ConvertToUTCTime(&time,StuEeprom.StuPara.time);
}


void LowerPower(void)
{
    power_manage();
    V_FeedWdog();
//	sleep_mode_enter();
}

void ManagePowerOn(void)
{
	uint8_t ansy=1;
    /*电压要大于3.5v  关机标志，充电 开机*/


    /*开机对电压的要求*/
    while(GetVol()<(VoltageTable[7]+40))//3650   (3615关机，开机3620 改为3650)
    {
        LowerPower();
        flash_oled_in_sleep_mode(0);

    }
    flash_oled_in_sleep_mode(1);

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
        } else
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
				ShowStartTime();
    }

    
		
		 if(StuEeprom.StuPara.ask_power_off)
		 {
        para_history_factory_set();
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


#include"app_timer.h"
#include"s110config.h"
#include"bsp.h"
void acc_time_event_handler(void * val)
{


    SendSem(GET_G_SENSOR);


}
app_timer_id_t m_acc_time_id;
void init_acc_time(void)
{

    uint32_t err_code;
    err_code = app_timer_create(&m_acc_time_id,
                                APP_TIMER_MODE_REPEATED,
                                acc_time_event_handler);
   APP_ERROR_CHECK(err_code);

}

void start_acc_time(bool start_flg)
{
    uint32_t err_code;
    if(true==start_flg)
    {
        err_code = app_timer_start(m_acc_time_id,  APP_TIMER_TICKS(500, APP_TIMER_PRESCALER) , NULL);
        APP_ERROR_CHECK(err_code);
    } else
    {
        err_code = app_timer_stop(m_acc_time_id);
        APP_ERROR_CHECK(err_code);
    }

}




/******************************************************************************
*******************************
Function: void TaskSport(void)
Description:
Input:
Output:
Return:
Others:
*******************************************************************************
**************************************/
uint8_t  TaskSport(void)
{
    uint8_t get_data_item;
    uint8_t i,add_step;
    int16_t xt,yt,zt;
		AxesRaw_t axes[32];
    static uint32_t AccSensorData[32];

    if(g_sensor_ok==0) return 0;
    get_data_item=GetData(axes);
    for(i = 0; i < get_data_item; i++)
    {
        xt=((int16_t)axes[i].AXIS_X)>>4;
        yt=((int16_t)axes[i].AXIS_Y)>>4;
        zt=((int16_t)axes[i].AXIS_Z)>>4;
        AccSensorData[i] = xt*xt+yt*yt+zt*zt;
        AccSensorData[i] /= 100;

        CallByGsensor(xt,yt,zt);
        if(OLED_AUTO_ON==StuBandParaHistoryData.oled_auto_on)
        {
					
//					            //水平  0 			0 				-1024
//            //下垂  1024    0     		0
//            //侧放  0				-1024			0
//            AccForLcdOnOff(xt,yt,zt,AccSensorData[i]);
					
					
#ifdef PCB_DREAM
            AccForLcdOnOff(-xt,-yt,zt,AccSensorData[i]);
#elif defined PCN_VF103_OLED_ON
            AccForLcdOnOff(xt,yt,zt,AccSensorData[i]);
#endif
        }

    }
		

		add_step=SportsProcess(get_data_item,AccSensorData);
		if(add_step)
		{
			StuEeprom.StuPara.step+=add_step;
		}
    return add_step;

}




