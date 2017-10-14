#include "ui_app.h"
#include "ui_support.h"
#include "wdt.h"


#define NORMAL_ON_TIME  10
#define HR_WAIT_TO_TEST 6
#define HR_WAIT_TIME_OUT 120
#define OLED_TIME_ON    4
#define MSG_ON_TIME  20
#define LOWER_OFF_TIME  20


uint8_t oled_on_time=NORMAL_ON_TIME;
ENUM_INDEX currentIndex=OLED_OFF_MENU;
ENUM_INDEX currentIndex_before_off=TIME_DIS_MENU;
//保存当前的菜单，返回要跳转的菜单
//TIME_DIS_MENU --- K_DIS_MENU  之间
ENUM_INDEX key_press_jump_index(uint32_t enable_index)
{
    uint16_t menu_number,i;

    const STU_ENABLE_INDEX menuindex[]=
    {
        {TIME_DIS_MENU,1},
        {STEP_DIS_MENU,2},
        {HR_DIS_MENU,3},
        {BLOOD_DIS_MENU,7},
        {DISTANCE_DIS_MENU,4},
        {K_DIS_MENU,5},
        {SPORT_TIME_MENU,6},
        {SLEEP_DIS_MENU,8},
        {PAIR_DIS_MENU,9},
    };
    menu_number=sizeof(menuindex)/sizeof(STU_ENABLE_INDEX);

#if(0==(BORD&USE_BLOOD))
    enable_index&=(~(0x000001<<7));
#endif


#if(0==(BORD&USE_HR))
    enable_index&=(~(0x000001<<3));
#endif

    currentIndex_before_off=currentIndex;

    //超找当前菜单的位置

    for(i=0; i<menu_number; i++)
    {
        if(menuindex[i].index==currentIndex)
        {
            break;
        }
    }
    if(i==(menu_number-1))
    {
        return TIME_DIS_MENU;
    }
    i++;
    //找到下一个menu
    for(; i<menu_number; i++)
    {
        if((0x000001<<menuindex[i].position)&enable_index)
            return menuindex[i].index;
    }

    return TIME_DIS_MENU;

}





UiFunction m_Ui_functions[]=
{
    LcdDisplayOff,
    LcdDisplayTime,
    LcdDisplaySteps,
    LcdDisplayHr,
    LcdDisplayBlood,
    LcdDisplayDistance,
    LcdDisplayK,
    LcdDisplaySportTime,
    LcdDisplaySleep,
    LcdDisplayPair,
    LcdDisplayNotice,
    LcdDisplayCharge,
    LcdDisplayAlarm,

};




void task_ui(void)
{
    ENUM_INDEX preindextmp;
    ENUM_SEM sem;
    do
    {
        sem=GetSem();
        if(NO_SEM==sem) return;
        task_process_message(sem);/*响应系统消息*/

        /*充电或者通知都属于通知*/
        if( (CHARGE==sem)||(NOTICE_COM==sem))
        {
            NoticeCloseHr();
            start_sys_time(START_ALL,false);
            if(CHARGE==sem)
                currentIndex=CHARGE_DIS_MENU;
            else if(StuMsg.type<0xb0)
                currentIndex=MSG_DIS_MENU;
            else
                currentIndex=ALARM_DIS_MENU;
            currentIndex=(m_Ui_functions[currentIndex])(INIT_UI);
            return;
        }


        preindextmp = currentIndex;
        currentIndex = (m_Ui_functions[currentIndex])(sem);
        if(preindextmp != currentIndex)
        {
            currentIndex = (m_Ui_functions[currentIndex])(INIT_UI);
        }
    }
    while(1);
}


uint8_t from_oled_on=0;
ENUM_INDEX LcdDisplayOff(ENUM_SEM sem)
{

    if(CHARGE_ING==StuPower.charge_flg)
    {
        SendSem(CHARGE);
        return CHARGE_DIS_MENU;
    }

    if(sem==INIT_UI)/*关闭oled*/
    {
        LOG("off ui init\r\n");
        LCD_Clear(BLACK);
        oled_display_on(false);
        start_sys_time(START_ALL,false);
    }
    else  if( (sem==PRESS_KEY)||(OLED_ON==sem))  /*开oled，切换到time菜单*/
    {

        oled_display_on(true);

        if(OLED_ON==sem)
        {
            from_oled_on=1;
            LOG("off ui auto on to time ui\r\n");
            return TIME_DIS_MENU;
        }
        else if(OLED_SAVE_LAST_UI== StuEeprom.StuPara.oled_sava_last_ui)
        {
            from_oled_on=0;
            if((0x000001<<currentIndex_before_off)& StuEeprom.StuPara.menu_enable_index)
            {
                LOG("off ui to %d ui\r\n",currentIndex_before_off);
                return currentIndex_before_off;
            }
        }
        LOG("off ui  on to time ui\r\n");
        return TIME_DIS_MENU;

    }

    return OLED_OFF_MENU;
}


/*时间菜单*/
ENUM_INDEX LcdDisplayTime(ENUM_SEM sem)
{
    RTC_UTCTimeStruct tim1;
    static uint8_t type=2;
    if(INIT_UI==sem)
    {
        LOG("time ui init\r\n");
        type++;
		type%=5;

        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
        GetTimeAndSec(&tim1);
        lcd_show_time(type,tim1,\
                      StuPower.lever|0x80,\
                      IsBleConnect());
        oled_on_time=NORMAL_ON_TIME+20;
    }
    else if(OLED_ON==sem)
    {
        oled_on_time=NORMAL_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            if(from_oled_on==0)
                currentIndex_before_off=currentIndex;

            LOG("time ui to off ui\r\n");
            return OLED_OFF_MENU;
        }
        if(oled_on_time&0x01)
        {
            GetTimeAndSec(&tim1);
            lcd_show_time(type,tim1,\
                          StuPower.lever,\
                          IsBleConnect());
        }
    }
    else if(PRESS_KEY==sem)
    {

        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);

    }
    else if(OLED_OFF==sem)
    {
        if(from_oled_on==0)
            currentIndex_before_off=currentIndex;

        LOG("time ui auto off ui key\r\n");
        return OLED_OFF_MENU;
    }

    return TIME_DIS_MENU;
}


ENUM_INDEX LcdDisplaySteps(ENUM_SEM sem)
{
    static uint8_t cnt=0;
//    uint8_t len=0;
    char buf[10];
    cnt = cnt;
    buf[0] = buf[0];
    if(INIT_UI==sem)
    {
        cnt=sprintf(buf,"%d",StuEeprom.StuPara.step);
        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
        lcd_show_step(StuEeprom.StuPara.step,StuEeprom.StuPara.step_goal,0xff);
        oled_on_time=40;//NORMAL_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {

        lcd_show_step(StuEeprom.StuPara.step,StuEeprom.StuPara.step_goal,oled_on_time);
        oled_on_time--;
        if(oled_on_time==0)
        {
            currentIndex_before_off=currentIndex;
            return OLED_OFF_MENU;
        }

    }
    else if(PRESS_KEY==sem)
    {
        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
    }
    else if(OLED_OFF==sem)
    {
        currentIndex_before_off=currentIndex;
        return OLED_OFF_MENU;
    }

    return STEP_DIS_MENU;

}


ENUM_INDEX LcdDisplayHr(ENUM_SEM sem)
{

#define WAIT_START 0
#define WAIT_WEAR 1
#define WAIT_GET  2
#define WAIT_END  3
    static uint8_t hr=0,hrstate=WAIT_START,frash=0;
    STU_HR stuhr;

    if(INIT_UI==sem)
    {

        start_sys_time(START_TIME,true);
        hr=0;
        hrstate=WAIT_START;
        LCD_Clear(BLACK);
        lcd_show_HR(hr,2);
        oled_on_time=HR_WAIT_TO_TEST;
		LOG("wait start\r\n");
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(frash) frash--;
        if(oled_on_time==0)
        {
            if(WAIT_START==hrstate)//3
            {
                StartHeatRate(false);
                hrstate=WAIT_WEAR;
                oled_on_time=HR_WAIT_TIME_OUT;
				LOG("wait wear\r\n");
            }
            else
            {

                if(WAIT_END==hrstate)
                    SendSem(SEND_HR);
                else
                    StartHeatRate(false);
                currentIndex_before_off=currentIndex;
                return OLED_OFF_MENU;
            }

        }
        if(WAIT_WEAR==hrstate)
        {
			LOG("check wear\r\n");
            if(true==HrCheckWear(0))
            {
				LOG("wait get\r\n");
                hrstate=WAIT_GET;
                StartHeatRate(true);
            }
        }else  if(WAIT_GET==hrstate)
        {
            if(get_hr==GetHrState(&stuhr))
            {
				LOG("get hr \r\n");
                MotorOn(1,1,1);
                oled_on_time=HR_WAIT_TIME_OUT;
                hrstate=WAIT_END;
            }
            lcd_show_HR(hr,oled_on_time&0x01);

        }else if(WAIT_END==hrstate)
        {
            GetHrState(&stuhr);
            if((hr!=stuhr.hr_fielt)&&(frash==0))
            {
                frash=6;
                hr=stuhr.hr_fielt;
                LCD_Clear(BLACK);
                lcd_show_HR(hr,2);
            }
            lcd_show_HR(hr,oled_on_time&0x01);
        }
    }else if(PRESS_KEY==sem)
    {
        if(WAIT_END==hrstate)
            SendSem(SEND_HR);
        else
            StartHeatRate(false);

        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
    }

    return HR_DIS_MENU;
}
ENUM_INDEX LcdDisplayBlood(ENUM_SEM sem)
{



    return HR_DIS_MENU;
}

ENUM_INDEX LcdDisplayDistance(ENUM_SEM sem)
{

  static  uint32_t display_value;

   
	
    if((STEP_COME==sem)||(INIT_UI==sem))
    {
        display_value=StuEeprom.StuPara.distance_cm/1000;
        if(UNIT_MILE==StuEeprom.StuPara.distance_unit)
        {
            display_value=display_value*6214/1000;
        }
    }
    if(INIT_UI==sem)
    {
        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
        lcd_show_distance(display_value,0xff);
        oled_on_time=NORMAL_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            currentIndex_before_off=currentIndex;
            return OLED_OFF_MENU;
        }
        lcd_show_distance(display_value,oled_on_time);
    }
    else if(PRESS_KEY==sem)
    {
        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
    }
    else if(OLED_OFF==sem)
    {
        currentIndex_before_off=currentIndex;
        return OLED_OFF_MENU;
    }

    return DISTANCE_DIS_MENU;
}


/*卡路里*/
ENUM_INDEX LcdDisplayK(ENUM_SEM sem)
{
//    static uint8_t cnt=0;
//    uint8_t len=0;
//    char buf[10];

    if(INIT_UI==sem)
    {
        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
        lcd_show_K(StuEeprom.StuPara.calories,2);
        oled_on_time=NORMAL_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            currentIndex_before_off=currentIndex;
            return OLED_OFF_MENU;
        }
        lcd_show_K(StuEeprom.StuPara.calories,oled_on_time&0x01);
    }
    else if(PRESS_KEY==sem)
    {
        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
    }
    else if(OLED_OFF==sem)
    {
        currentIndex_before_off=currentIndex;
        return OLED_OFF_MENU;
    }

    return K_DIS_MENU;
}
/*运动时间*/
ENUM_INDEX LcdDisplaySportTime(ENUM_SEM sem)
{
// 	static volatile ENUM_INDEX index111;
//    static uint8_t cnt=0;
//    uint8_t len=0;
//    char buf[10];
    if(INIT_UI==sem)
    {
        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
        lcd_show_sport_time(StuEeprom.StuPara.step_sec/60,2);
        oled_on_time=NORMAL_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            currentIndex_before_off=currentIndex;
            return OLED_OFF_MENU;
        }
        lcd_show_sport_time(StuEeprom.StuPara.step_sec/60,oled_on_time&0x01);
    }
    else if(PRESS_KEY==sem)
    {
        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
// 			return index111;
    }
    else if(OLED_OFF==sem)
    {
        currentIndex_before_off=currentIndex;
        return OLED_OFF_MENU;
    }

    return SPORT_TIME_MENU;
}

//闹钟通知菜单
ENUM_INDEX LcdDisplayAlarm(ENUM_SEM sem)
{

    if(INIT_UI==sem)
    {
        oled_display_on(true);
        LCD_Clear(BLACK);
        if(StuMsg.type!=(0xb0+MAC_SHOW))
        {
            MotorOn(3,5,5);
        }
        if(StuMsg.type==(0xb0+MAC_SHOW))
        {
            lcd_show_battery(40,0);
        }
        else if(StuMsg.type==(0xb0+POWER_20))
        {
            lcd_show_battery(20,0);
        }
        else          if(StuMsg.type==(0xb0+SENDENTARY))
        {
            lcd_show_battery(30,oled_on_time&0x03);
        }
        else
        {
            lcd_show_alarm(StuMsg.msg[0],StuMsg.msg[1],(StuEeprom.StuPara.time_format==TIME_12HOUR)?true:false,StuMsg.msg[2]);
        }


        oled_on_time=MSG_ON_TIME;
        if(StuMsg.type==(0xb0+MAC_SHOW))
        {
            oled_on_time=60;
        }
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            return OLED_OFF_MENU;
        }
    }
    else if(PRESS_KEY==sem)
    {
        start_sys_time(START_TIME,false);
        return TIME_DIS_MENU;
    }

    return ALARM_DIS_MENU;
}




//通知短信电话
ENUM_INDEX LcdDisplayNotice(ENUM_SEM sem)
{
    if(INIT_UI==sem)
    {
        MotorOn(3,5,5);
        start_sys_time(START_TIME,true);
        oled_display_on(true);
        LCD_Clear(BLACK);
        LcdShowNotice(StuMsg.type-0x99,StuMsg.msg,StuMsg.len);
        oled_on_time=MSG_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            return OLED_OFF_MENU;
        }
    }
    else if(PRESS_KEY==sem)
    {
        start_sys_time(START_TIME,false);
        return TIME_DIS_MENU;
    }

    return MSG_DIS_MENU;
}

//充电菜单
ENUM_INDEX LcdDisplayCharge(ENUM_SEM sem)
{
    uint8_t lever;
    if(StuPower.lever==0)
        lever=1;
    else
        lever=StuPower.lever;
		
    if(INIT_UI==sem)
    {
        oled_display_on(true);
        LCD_Clear(BLACK);
        if(oled_on_time&0x01)
            lcd_show_battery_charging(lever-1);
        else
            lcd_show_battery_charging(lever);
        start_sys_time(START_TIME,true);
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time&0x01)
            lcd_show_battery_charging(lever-1);
        else
            lcd_show_battery_charging(lever);

    }
    if( (PRESS_KEY==sem)||(CHARGE_ING!=StuPower.charge_flg))
    {
        return TIME_DIS_MENU;
    }

    return CHARGE_DIS_MENU;
}

ENUM_INDEX LcdDisplaySleep(ENUM_SEM sem)
{


    if(INIT_UI==sem)
    {
        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
        lcd_show_sleep(StuEeprom.StuPara.step_sec/60,0xff);
        oled_on_time=NORMAL_ON_TIME;
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            currentIndex_before_off=currentIndex;
            return OLED_OFF_MENU;
        }
        lcd_show_sleep(StuEeprom.StuPara.step_sec/60,oled_on_time);
    }
    else if(PRESS_KEY==sem)
    {
        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
    }
    else if(OLED_OFF==sem)
    {
        currentIndex_before_off=currentIndex;
        return OLED_OFF_MENU;
    }

    return SLEEP_DIS_MENU;
}
ENUM_INDEX LcdDisplayPair(ENUM_SEM sem)
{


    if(INIT_UI==sem)
    {

        LCD_Clear(BLACK);
        start_sys_time(START_TIME,true);
		oled_on_time=NORMAL_ON_TIME;
        lcd_show_pair(0xff);
        
    }
    else if(REFLASH_UI==sem)
    {
        oled_on_time--;
        if(oled_on_time==0)
        {
            currentIndex_before_off=currentIndex;
            return OLED_OFF_MENU;
        }
        lcd_show_pair(oled_on_time);
    }
    else if(PRESS_KEY==sem)
    {
        return key_press_jump_index(StuEeprom.StuPara.menu_enable_index);
    }
    else if(OLED_OFF==sem)
    {
        currentIndex_before_off=currentIndex;
        return OLED_OFF_MENU;
    }

    return PAIR_DIS_MENU;


}

void show_test_mode(uint8_t sec)
{
/////////////////////////////////////////////////////////////////////

#define PRESS_SEC   (3*2)
    uint16_t display_value,on_time;
    uint8_t buf[20],len=1;

    ENUM_SEM sem;
    oled_display_on(true);
    LCD_Clear(BLACK);

    StartHeatRate(false);

    len=sprintf((char*)buf,"vol:%d %01d",(StuPower.persent>=100)? 99:StuPower.persent,ex_flash_ok+g_sensor_ok+hr_ok);
    Printfdata(0,0,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);

    len=sprintf((char*)buf,"%02x-%02x-%02x",version[5],version[6],version[7]);

	 Printfdata(0,32,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
	 

    start_sys_time(START_TIME,true);
    on_time=sec;
    on_time*=10;
	SendSem(REFLASH_UI);
    while(1)
    {
        sem=GetSem();
        if((REFLASH_UI==sem))
        {
            on_time--;
            if(0==on_time)
            {
                LcdDisplayOff(INIT_UI);
                return ;
            }
            {
                if(!(on_time%4))
				{
                    HrCheckWear(&display_value);
				}
                if(display_value>9999)
                    display_value=9999;
                len=sprintf((char*)buf,"%04d %04d",display_value,ReadProx());
				Printfdata(0,64,12,32,buf,len,(uint8_t*)&ascii_1232[0][0],64);
            }
        } else if(PRESS_KEY==sem)
        {
            MotorOn(1,5,5);
        }

        power_manage();/*key  time BLE */
        V_FeedWdog();/*喂狗*/
    }



}



void ShowPowerLow(void)
{
    uint8_t time_on=LOWER_OFF_TIME;
    MotorOn(3,5,5);
    oled_display_on(true);
    LCD_Clear(BLACK);
    start_sys_time(START_TIME,true);
    lcd_show_battery_charging(0);
    while(time_on)
    {
        if(GetSem()==REFLASH_UI)
        {
            time_on--;
        }
        power_manage();
    }
    CallErr(DISPLAY_HELLO_UI,0,(const uint8_t *)"LowerPower");

}


bool wait_key_to_power_off(void)
{
    static uint8_t sta=0;
    static uint32_t timertc;
   uint32_t num_version=0;
    if(( (StuEeprom.StuPara.ask_version==2)||(StuEeprom.StuPara.ask_version==3))&&(sta!=0))/*60s后开机*/
    {
        if(get_time_escape_ms(NRF_RTC1->COUNTER,timertc)>60000)
        {
            LcdDisplayOff(INIT_UI);
            return true;
        }
    }



    switch(sta)
    {
    case 0:
        timertc=NRF_RTC1->COUNTER;
		oled_display_on(true);
		LCD_Clear(BLACK);

         if((StuEeprom.StuPara.ask_version==1)||(StuEeprom.StuPara.ask_version==3))
            oled_show_version();
        else 
		if((StuEeprom.StuPara.ask_version==2)||(StuEeprom.StuPara.ask_version==4))
        {
            num_version=version[7];
            if(StuPower.lever==0)
            {
                num_version+=80000;
            } else
            {
                num_version+=10000*StuPower.lever;
            }
             showUpgredePersent(num_version);
        }
        sta++;
        break;
    case 1:/*等待按键释放*/
			
		   if(get_time_escape_ms(NRF_RTC1->COUNTER,timertc)>20000)
        {
            LcdDisplayOff(INIT_UI);
            return true;
        }
		
//        if(ReadKey())/*释放了*/
//        {
//            delay_ms(50);
//            if(ReadKey())/*释放了*/
//            {
//                sta++;
//            }
//        }
        break;
    case 2:/*等待按下*/
        if(!ReadKey())
        {
            delay_ms(50);
            if(!ReadKey())
            {
                LcdDisplayOff(INIT_UI);
                return  true;
            }
        }
        break;
    }

    return false;
}
void display_in_sleep_mode(uint8_t return_low_power_mode)
{

    static uint8_t sta=0,time_reflash;
    ENUM_SEM sem;
    if( (return_low_power_mode)&&(sta))
    {
        LcdDisplayOff(INIT_UI);
        sta=0;
        return;
    }

    do {
        sem=GetSem();
        if(TIME_MINUTE==sem)
        {
            updateDateInMin(false);
        }

        if(0==sta)
        {
            if(charger_status()==CHARGE_ING)
            {
                start_sys_time(START_TIME,true);
                time_reflash=6;
				oled_display_on(true);
				LCD_Clear(BLACK);
				lcd_show_battery_charging(0);
                sta++;
            }
        } else
        {
			
            if(sem==REFLASH_UI)
            {
                time_reflash--;
                if(1==time_reflash)
                {
                    oled_display_on(true);
                } else if(0==time_reflash)
                {
                    time_reflash=6;
					oled_display_on(false);
                }
            }
            if(charger_status()!=CHARGE_ING)
            {
                LcdDisplayOff(INIT_UI);
                sta=0;
            }
        }

    } while(NO_SEM!=sem);

}
