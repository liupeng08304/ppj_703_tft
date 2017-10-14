#include "protocol.h"
#include "in_flash_manage.h"
#include "SoftwareRTC.h"
#include "utility.h"
#include "Battery_ADC.h"
#include "sem.h"
#include "ui_app.h"
#include "alarm.h"
#include "sleep.h"
#include "bsp.h"
#include "upfile.h"
#include "hr_app.h"
#include "wdt.h"
#include "data_use_api.h"
#include "s132config.h"
STU_MSG StuMsg;



#define REAL_TIME_MAX  5  //(min)
uint16_t report_real_time_data=0;


void send_real_time_datas(void)
{
    uint8_t buf[20],len=0;
    STU_HR stuhr;

    if(report_real_time_data==0)  return;
    if(is_ble_tx_enable()!=true)
    {
        report_real_time_data=0;
        return;
    }


    buf[len++]=0xa9;
    buf[len++]=StuEeprom.StuPara.step>>24;
    buf[len++]=StuEeprom.StuPara.step>>16;
    buf[len++]=StuEeprom.StuPara.step>>8;
    buf[len++]=StuEeprom.StuPara.step>>0;


    buf[len++]=(StuEeprom.StuPara.step_sec/60)>>8;
    buf[len++]=(StuEeprom.StuPara.step_sec/60)>>0;

    buf[len++]=(StuEeprom.StuPara.distance_cm/10000)>>8;
    buf[len++]=(StuEeprom.StuPara.distance_cm/10000)>>0;

    buf[len++]=(StuEeprom.StuPara.calories)>>8;
    buf[len++]=(StuEeprom.StuPara.calories)>>0;
    GetHrState(&stuhr);
    buf[len++]=stuhr.hr_fielt;
    BleWriteData(buf,len);

}

//500ms扫描一次
void check_send_hr_real_time(void)
{
    static uint8_t tick=0;
    static uint16_t stepb,hrb;
    STU_HR stuhr;
    if(report_real_time_data==0) return;
    tick++;
    if(120==tick)
    {
        tick=0;
        report_real_time_data--;
    }
    GetHrState(&stuhr);
    if(tick%8==0)
    {

        if((stepb!=StuEeprom.StuPara.step)||(stuhr.hr_fielt!=hrb))
        {
            stepb=StuEeprom.StuPara.step;
            hrb=stuhr.hr_fielt;
            if(0!=stuhr.hr_fielt)
                SendSem(SEND_REAL_TIME_DATA);
        }

    }

}



void SendMsg(MSG_TYPE msgtype,uint8_t msglen,uint8_t*msgbuf)
{
    uint8_t i;
    StuMsg.type=msgtype;
    StuMsg.len=msglen;
    for(i=0; i<msglen; i++)
    {
        StuMsg.msg[i]=msgbuf[i];
    }
    SendSem(NOTICE_COM);
}


bool SetTime(uint8_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t min,uint8_t sec,uint8_t*ansy)
{
    uint32_t tmp;
    RTC_UTCTimeStruct timetmp;
    if( (year>99)||(month>12)||(day>31)||(hour>23)||(min>59)||(sec>60))
        return false;

    if((month==0)||(day==0)||(year<16))
        return false;

    timetmp.year=year+2000;
    timetmp.month=month;
    timetmp.day=day;
    timetmp.hour=hour;
    timetmp.minutes=min;
    timetmp.seconds=sec;
    tmp=convert_time_to_Second(timetmp);

    ConvertToUTCTime(&time,tmp);



//     if(abs(tmp-StuEeprom.StuPara.time)>60)
    {
        StuEeprom.StuPara.time=tmp;
//         ansy[0]=1;
//         ConvertToUTCTime(&time,StuEeprom.StuPara.time);
    }
    return true;
}

bool  SetBandPara(uint8_t weight,uint8_t hight,uint8_t age,uint8_t female,uint8_t stride,uint8_t *ansy_flg)
{

    if((weight<30)||(weight>150)) return false;
    if((hight<100)||(hight>200)) return false;
    if((age<5)||(age>99)) return false;
    if (female>1) return false;
    if((stride<30)||(stride>150)) return false;



    if((StuEeprom.StuPara.weight!=weight)||\
            (StuEeprom.StuPara.hight!=hight)||\
            (StuEeprom.StuPara.age!=age)||\
            (StuEeprom.StuPara.female!=female)||\
            (	StuEeprom.StuPara.stride!=stride)
      )
    {
        StuEeprom.StuPara.weight=weight;
        StuEeprom.StuPara.hight=hight;
        StuEeprom.StuPara.age=age;
        StuEeprom.StuPara.female=female;
        StuEeprom.StuPara.stride=stride;
        ansy_flg[0]=1;

    }
    return true;
}


bool SetBandParaAlarm_new(uint8_t* pdata,uint8_t *ansy_flg)
{

    uint8_t turn;
    uint8_t i;
    uint8_t* pdatanow;
    STU_ALARM stu_alarm_tmp;

    turn=pdata[0];

    for(i=0; i<MAX_ALARM/2; i++)
    {
        pdatanow=pdata+4*i+1;
        stu_alarm_tmp.type=pdatanow[0];
        if(pdatanow[0]>ALARM_RUN)
            return false;
        stu_alarm_tmp.work_day=pdatanow[1]&0x7f;
        if(pdatanow[1]&0x80)
            stu_alarm_tmp.on_flg=true;
        else
            stu_alarm_tmp.on_flg=false;

        if(pdatanow[2]>23)
            return false;
        if(pdatanow[3]>59)
            return false;
        stu_alarm_tmp.hour=pdatanow[2];
        stu_alarm_tmp.min=pdatanow[3];

        if( (stu_alarm_tmp.type!=StuEeprom.StuPara.StuAlarm[i+4*turn].type)||\
                (stu_alarm_tmp.on_flg!=StuEeprom.StuPara.StuAlarm[i+4*turn].on_flg)||\
                (stu_alarm_tmp.work_day!=StuEeprom.StuPara.StuAlarm[i+4*turn].work_day)||\
                (stu_alarm_tmp.hour!=StuEeprom.StuPara.StuAlarm[i+4*turn].hour)||\
                (stu_alarm_tmp.min!=StuEeprom.StuPara.StuAlarm[i+4*turn].min)
          )
        {
            ansy_flg[0]=1;
            StuEeprom.StuPara.StuAlarm[i+4*turn]=stu_alarm_tmp;
        }
    }



    return true;

}

//uint32_t s_hour:5;
//uint32_t s_min:6;
//uint32_t e_hour:5;
//uint32_t e_min:6;
//uint32_t work_day:7;   /*0到6位对应 星期一到星期日*/
////uint32_t ver:3;
//StuSendentaryRemain[i+4*3]
// 0,1   week   sh sm  eh em
bool SetBandParaSendentary(uint8_t* pdata,uint8_t *ansy_flg)
{

    uint8_t turn;
    uint8_t i;
    uint8_t* pdatanow;
    STU_SEDENTARY_REAMND stu_sendary_tmp;

    turn=pdata[0];

    for(i=0; i<3; i++)
    {
        pdatanow=pdata+5*i;

        stu_sendary_tmp.work_day=pdatanow[1]&0x7f;
        if(pdatanow[1]&0x80)
            stu_sendary_tmp.on_flg=true;
        else
            stu_sendary_tmp.on_flg=false;

        if(pdatanow[2]>23)
            return false;
        if(pdatanow[3]>59)
            return false;
        if(pdatanow[4]>23)
            return false;
        if(pdatanow[5]>59)
            return false;
        stu_sendary_tmp.s_hour=pdatanow[2];
        stu_sendary_tmp.s_min=pdatanow[3];
        stu_sendary_tmp.e_hour=pdatanow[4];
        stu_sendary_tmp.e_min=pdatanow[5];

        if( (stu_sendary_tmp.on_flg!=StuEeprom.StuPara.StuSendentaryRemain[i+3*turn].on_flg)||\
                (stu_sendary_tmp.work_day!=StuEeprom.StuPara.StuSendentaryRemain[i+3*turn].work_day)||\
                (stu_sendary_tmp.s_hour!=StuEeprom.StuPara.StuSendentaryRemain[i+3*turn].s_hour)||\
                (stu_sendary_tmp.s_min!=StuEeprom.StuPara.StuSendentaryRemain[i+3*turn].s_min)||\
                (stu_sendary_tmp.e_hour!=StuEeprom.StuPara.StuSendentaryRemain[i+3*turn].e_hour)||\
                (stu_sendary_tmp.e_min!=StuEeprom.StuPara.StuSendentaryRemain[i+3*turn].e_min)
          )
        {
            ansy_flg[0]=1;
            StuEeprom.StuPara.StuSendentaryRemain[i+3*turn]=stu_sendary_tmp;
        }
    }



    return true;

}


bool SetSendentarypara(uint8_t* pdata,uint8_t *ansy_flg)
{
    uint8_t min;
    uint16_t big;
    min=pdata[0];
    big=pdata[1];
    big*=5;
    if(pdata[1]>200)
        return false;

    if(pdata[0]>120)
        return false;

    if((StuEeprom.StuPara.sedentary_min!=min)||(big!= StuEeprom.StuPara.sedentary_big_times))
    {
        ansy_flg[0]=1;
        StuEeprom.StuPara.sedentary_min=min;
        StuEeprom.StuPara.sedentary_big_times=big;
    }

    return true;
}


bool SetBandParaAlarm(uint8_t* pdata,uint8_t *ansy)
{

    uint8_t turn;
    uint8_t i;
    uint8_t* pdatanow;
    STU_ALARM stu_alarm_tmp;

    turn=pdata[0]-1;
    for(i=0; i<5; i++)
    {
        pdatanow=pdata+3*i+1;
        stu_alarm_tmp.type=ALARM_ALARM;

        if(pdatanow[0]==1)
        {
            stu_alarm_tmp.work_day=0x7f;
            stu_alarm_tmp.on_flg=true;
        } else
        {
            stu_alarm_tmp.work_day=0;
            stu_alarm_tmp.on_flg=false;
        }

        if(pdatanow[1]>23)
            return false;
        if(pdatanow[2]>59)
            return false;

        stu_alarm_tmp.hour=pdatanow[1];
        stu_alarm_tmp.min=pdatanow[2];

        if( (stu_alarm_tmp.type!=StuEeprom.StuPara.StuAlarm[i+5*turn].type)||\
                (stu_alarm_tmp.on_flg!=StuEeprom.StuPara.StuAlarm[i+5*turn].on_flg)||\
                (stu_alarm_tmp.work_day!=StuEeprom.StuPara.StuAlarm[i+5*turn].work_day)||\
                (stu_alarm_tmp.hour!=StuEeprom.StuPara.StuAlarm[i+5*turn].hour)||\
                (stu_alarm_tmp.min!=StuEeprom.StuPara.StuAlarm[i+5*turn].min)
          )
        {
            ansy[0]=1;
            StuEeprom.StuPara.StuAlarm[i+5*turn]=stu_alarm_tmp;
        }
    }
    return true;

}




void SendAlarm(uint8_t *pdata)
{

    uint8_t i,len,j;

    for(j=0; j<2; j++)
    {
        len=0;
        pdata[len++]=0x96;
        pdata[len++]=0x0d;
        pdata[len++]=j;
        for(i=0; i<MAX_ALARM/2; i++)
        {
            pdata[len++]=StuEeprom.StuPara.StuAlarm[i+4*j].type;
            pdata[len]=StuEeprom.StuPara.StuAlarm[i+4*j].work_day;
            if(true==StuEeprom.StuPara.StuAlarm[i+4*j].on_flg)
            {
                pdata[len]|=0x80;
            }
            len++;
            pdata[len++]=StuEeprom.StuPara.StuAlarm[i+4*j].hour;
            pdata[len++]=StuEeprom.StuPara.StuAlarm[i+4*j].min;

        }
        BleWriteData(pdata,len);
    }

}





void SendItems(uint8_t *pdata,bool now)
{


    if(true==now)
    {
        ble_send_current_step(pdata);
    } else
    {
        ble_send_current_step(pdata);
        ble_send_history_step((StuEeprom.StuPara.time/DAY-(6))*DAY,pdata);
    }
}


//(StuEeprom.StuPara.time/DAY-(0))*DAY;

void SendSleepIndex(uint8_t *pdata,uint32_t sec)///############################
{
    uint16_t items,len;
    uint32_t data_sec,i;
    uint8_t weeknow;
    STU_SLEEP_DT StuSleepDt;
    RTC_UTCTimeStruct timer_tmp;
    uint32_t time_tick;
    weeknow=time.week;

    //数据存储的星期是凌晨前的，所以需要比当前天反推一天
    if(weeknow)
    {
        weeknow--;
    }
    else
    {
        weeknow=6;
    }


    i=0;
    weeknow=(weeknow+1)%7;//第一条是七天前的数据


    items=0;
    for(; i<7; i++)
    {
        data_sec=(StuEeprom.StuPara.time/DAY-(6-i))*DAY;
        if(data_sec>sec)
        {
            if(true==ReadSleepDatas(weeknow,&StuSleepDt))
            {
                len=0;
                pdata[len++]=0x93;
                pdata[len++]=items;
                items++;
                time_tick=StuEeprom.StuPara.time-DAY*(7-i);
                ConvertToUTCTime(&timer_tmp,time_tick);
                //year month day hour min  year month day hour min

                if(StuSleepDt.s_hour<12)
                {
                    time_tick+=DAY;
                    ConvertToUTCTime(&timer_tmp,time_tick);
                    time_tick-=DAY;
                }
                pdata[len++]=timer_tmp.year-2000;
                pdata[len++]=timer_tmp.month;
                pdata[len++]=timer_tmp.day;
                pdata[len++]=StuSleepDt.s_hour;
                pdata[len++]=StuSleepDt.s_min;

                if(StuSleepDt.e_hour<12)
                {
                    time_tick+=DAY;
                    ConvertToUTCTime(&timer_tmp,time_tick);
                }

                pdata[len++]=timer_tmp.year-2000;
                pdata[len++]=timer_tmp.month;
                pdata[len++]=timer_tmp.day;
                pdata[len++]=StuSleepDt.e_hour;
                pdata[len++]=StuSleepDt.e_min;

                pdata[len++]=StuSleepDt.deep_sleep_min>>8;
                pdata[len++]=StuSleepDt.deep_sleep_min;

                pdata[len++]=StuSleepDt.light_sleep_min>>8;
                pdata[len++]=StuSleepDt.light_sleep_min;

                pdata[len++]=StuSleepDt.wake_min>>8;
                pdata[len++]=StuSleepDt.wake_min;

                BleWriteData(pdata,len);

            }
        }


        weeknow++;
        weeknow%=7;
    }

    if(0==items)
    {
        len=0;
        pdata[len++]=0x93;
        pdata[len++]=items;
        memset(&pdata[len],0,16);
        len+=16;
        BleWriteData(pdata,len);
    }
}



//uint8_t hr_items_today=0;
//uint8_t hr_items_7day=0;
uint8_t SendSleepRecord(uint8_t *pdata,uint32_t sec)///############################
{
    uint32_t data_sec,i;
    uint16_t len,items;
    uint8_t weeknow,frams,j;
    STU_SLEEP_DT StuSleepDt;


    weeknow=time.week;

    //数据存储的星期是凌晨前的，所以需要比当前天反推一天
    if(weeknow)
    {
        weeknow--;
    }
    else
    {
        weeknow=6;
    }


    weeknow=(weeknow+1)%7;//第一条是七天前的数据
    i=0;


    items=0;
    for(; i<7; i++)
    {
        data_sec=(StuEeprom.StuPara.time/DAY-(6-i))*DAY;
        if(data_sec>sec)
        {
            if(true==ReadSleepDatas(weeknow,&StuSleepDt))
            {
                uint8_t bytes,framlen;
                bytes=StuSleepDt.sleep_unit/4;
                if(StuSleepDt.sleep_unit%4)
                {
                    bytes++;
                }
                frams=bytes/16;
                if(bytes%16)
                {
                    frams++;
                }
                for(j=0; j<frams; j++)
                {
                    len=0;
                    pdata[len++]=0x94;
                    pdata[len++]=items;
                    pdata[len++]=j;
                    framlen=16;
                    if((j==(frams-1))&&(bytes%16))
                    {
                        framlen=bytes%16;
                    }
                    pdata[len++]=framlen;
                    memcpy(&pdata[len],&StuSleepDt.pdata[j*16],framlen);
                    len+=framlen;
                    BleWriteData(pdata,len);
                }
                items++;
            }
        }
        weeknow++;
        weeknow%=7;
    }

    if(0==items)
    {
        len=0;
        pdata[len++]=0x94;
        pdata[len++]=items;
        memset(&pdata[len],0,17);
        len+=17;
        BleWriteData(pdata,len);
    }

    return items;
}



//(StuEeprom.StuPara.time/DAY-(0))*DAY;
void  GetSleepRecord(uint8_t *indexrecords,uint32_t sec)
{
    uint32_t data_sec,i;
    uint16_t items,items1;
    uint8_t weeknow,frams;
    STU_SLEEP_DT StuSleepDt;



    weeknow=time.week;

    //数据存储的星期是凌晨前的，所以需要比当前天反推一天
    if(weeknow)
    {
        weeknow--;
    }
    else
    {
        weeknow=6;
    }

    i=0;
    weeknow=(weeknow+1)%7;//第一条是七天前的数据


    items=0;
    items1=0;
    for(; i<7; i++)
    {
        data_sec=(StuEeprom.StuPara.time/DAY-(6-i))*DAY;
        if(data_sec>sec)
        {
            if(true==ReadSleepDatas(weeknow,&StuSleepDt))
            {
                uint8_t bytes;
                bytes=StuSleepDt.sleep_unit/4;
                if(StuSleepDt.sleep_unit%4)
                {
                    bytes++;
                }
                frams=bytes/16;
                if(bytes%16)
                {
                    frams++;
                }
                items1+=frams;
                items++;
            }
        }
        weeknow++;
        weeknow%=7;

    }

    indexrecords[0]=items;
    indexrecords[1]=items1;

}











#define SEND_NONE  	0x00
#define SEND_T_92_STEP  (SEND_NONE+1)
#define SEND_T_93_SLEEP_RECORD  (SEND_T_92_STEP+1)
#define SEND_T_94_SLEEP_RDETAIL (SEND_T_93_SLEEP_RECORD+1)
#define SEND_T_A8_HR  (SEND_T_94_SLEEP_RDETAIL+1)


uint8_t send_point_data=0;
uint32_t point_time_sec;
bool ana_time_point(uint8_t *pdata,uint8_t *items)
{
    RTC_UTCTimeStruct timetmp;
    uint32_t tmp32,tmp32_1;
    items[0]=0;
    if( (pdata[1]>99)||\
            (pdata[2]>12)|| (pdata[2]==0)||\
            (pdata[3]>31)|| (pdata[3]==0)||\
            (pdata[4]>23)||\
            (pdata[5]>59))
    {

        return false;
    }

    timetmp.year=pdata[1]+2000;
    timetmp.month=pdata[2];
    timetmp.day=pdata[3];
    timetmp.hour=pdata[4];
    timetmp.minutes=pdata[5];
    timetmp.seconds=0;
    if(pdata[6]==0x92)//记步数据
    {
        timetmp.hour=0;
        timetmp.minutes=0;
    }
    point_time_sec=convert_time_to_Second(timetmp);



    if(pdata[6]==0x92)//记步数据
    {
        send_point_data=SEND_T_92_STEP;
        items[0]=read_history_items(point_time_sec);
        if(StuEeprom.StuPara.time<=point_time_sec)
        {
            items[0]--;
        }
    } else if(pdata[6]==0x93)//d
    {
        point_time_sec-=DAY;
        GetSleepRecord(&pdata[18],point_time_sec);
        items[0]=pdata[18];
        if(items[0])
            send_point_data=SEND_T_93_SLEEP_RECORD;
    }
    else if(pdata[6]==0x94)//
    {
        point_time_sec-=DAY;
        GetSleepRecord(&pdata[18],point_time_sec);
        items[0]=pdata[19];
        if(items[0])
            send_point_data=SEND_T_94_SLEEP_RDETAIL;
    } else if(pdata[6]==0xa8)//
    {
        send_point_data=SEND_T_A8_HR;
        tmp32_1=usr_api_read(0,0xa8,point_time_sec ,0);
        tmp32=tmp32_1/3;
        if(tmp32_1%3)
            tmp32++;
        if(tmp32>0xfe)
            tmp32=0xfe;

        items[0]=tmp32;
    } else
    {
        return false;
    }


    return  true;

}



void send_time_point(uint8_t *datain)
{
    uint32_t tmp32,tmp32_1;
    if(SEND_NONE==send_point_data) return;





    if(SEND_T_A8_HR==send_point_data)//发送HR
    {
        memset(datain,0,20);
        tmp32_1=usr_api_read(0,0xa8,point_time_sec ,0);
        tmp32=tmp32_1/3;
        if(tmp32_1%3)
            tmp32++;
        if(tmp32>0xfe)
        {
//			 tmp32=tmp32-0x00fe*3;
            tmp32=(tmp32-0x00fe)*3;
        }
        else
        {
            tmp32=0;
        }
        usr_api_read(tmp32,0xa8,point_time_sec ,datain);
        if(datain[0]!=0)
        {
            BleWriteData(datain,20);
        }
    } else if(SEND_T_92_STEP==send_point_data)//发送记步数据
    {
        ble_send_current_step(datain);
        ble_send_history_step(point_time_sec,datain);

    } else if(SEND_T_93_SLEEP_RECORD==send_point_data)
    {

        SendSleepIndex(datain,point_time_sec);
    } else if(SEND_T_94_SLEEP_RDETAIL==send_point_data)
    {
        SendSleepRecord(datain,point_time_sec);

    }


    send_point_data=SEND_NONE;
}






#define KEEP_HIGH_SPEED  10
volatile uint8_t isHightSpeed=0;

void return_to_normal_speed(void)
{
    if(isHightSpeed)
    {
        isHightSpeed--;
        if(0==isHightSpeed)
            ChangeToHightSpeed(false);
    }
}


void SetHighSpeed(void)
{
    if(0==isHightSpeed)
    {
        isHightSpeed=KEEP_HIGH_SPEED;
        ChangeToHightSpeed(true);
        delay_ms(100);
    } else
    {
        isHightSpeed=KEEP_HIGH_SPEED;
    }

}

/*解析蓝牙收到的数据*/
void task_and_ble_data(void)
{

    uint8_t pdata[20];
    uint8_t len;//,i;
    uint8_t cmd;
    uint32_t tmp32,tmp32_1;
    uint16_t tmp16;
    uint8_t ansy_flg=0;

    send_time_point(pdata);
    if(false==BleQueueOut(pdata,&len))
        return;
    SetHighSpeed();
    cmd=pdata[0];
    switch(cmd)
    {
    case 0x11:/*设置时间*/
        if(7==len)
        {
            if(true== SetTime(pdata[1],pdata[2],pdata[3],pdata[4],pdata[5],pdata[6],&ansy_flg))
            {
                SendSem(TIME_MINUTE);
                pdata[0]=0x96;
            }
            else
                pdata[0]=0x95;
            pdata[1]=cmd;
            BleWriteData(pdata,2);
        }
        break;
    case 0x12:/*设置参数，身高，体重*/
        pdata[0]=0x95;

        if( (5==len)||(6==len))
        {
            if(pdata[4]<2)
            {
                if(true==SetBandPara(pdata[1],pdata[2],pdata[3],pdata[4],pdata[5],&ansy_flg))
                {
                    pdata[0]=0x96;
                }
            }
            else
            {
                if(true==SetBandPara(pdata[1],pdata[2],pdata[3],1,pdata[4],&ansy_flg))
                {
                    pdata[0]=0x96;
                }
            }

        }

        pdata[1]=cmd;
        BleWriteData(pdata,2);
        if(1==ansy_flg)
            EepromAnsy();
        break;
    case 0x13:/*设置闹钟*/
        if((17==len)&&(pdata[1]<3)&&(pdata[1]>=1)&&(true==SetBandParaAlarm(&pdata[1],&ansy_flg))) /*老的闹钟协议*/
        {
            pdata[0]=0x96;
        } else
        {
            pdata[0]=0x95;
        }
        pdata[1]=cmd;
        BleWriteData(pdata,2);
        if(1==ansy_flg)
            EepromAnsy();
        break;
    case 0x26:/*新的闹钟协议*/
        if((18==len)&&(pdata[1]<2)&&(true==SetBandParaAlarm_new(&pdata[1],&ansy_flg)))
        {
            pdata[0]=0x96;
        } else
        {
            pdata[0]=0x95;
        }
        pdata[1]=cmd;
        BleWriteData(pdata,2);
        if(1==ansy_flg)
            EepromAnsy();
        break;
    case 0x14:/*睡眠开始结束*/
        pdata[2]=0x96;
        pdata[3]=cmd;
        BleWriteData(&pdata[2],2);
        if( (pdata[0]==0x14)&&(pdata[1]==0x15))
        {
            StuEeprom.StuPara.ask_power_off=1;
            StuEeprom.StuPara.ask_version=1;
            delay_ms(3000);
            CallErr(DISPLAY_HELLO_UI,0,(const uint8_t *)"0x15Askreset");
        }
        break;
    case 0x15:/*出厂设置+关机*/
        StuEeprom.StuPara.ask_power_off=1;
        StuEeprom.StuPara.ask_version=1;

        pdata[0]=0x96;
        pdata[1]=cmd;
        BleWriteData(pdata,2);

        EepromAnsy();
        delay_ms(3000);
        CallErr(DISPLAY_HELLO_UI,0,(const uint8_t *)"0x15Askreset");
        break;
    case 0x16:/*数据请求*/
        /*
        	0x00: request memory status
        	0x01: request step data
        	0x02: request sleep index
        	0x03: request sleep record
        	0x04:  SN read
        	0x05: 读取睡眠次数
        	0x06:firmware version
        	0x07:request BT mac address(LSB)
        	0x08:request module name
        	0x09:内部版本
        	0x0a:出厂设置+时间清零
        	0x0b:出厂设置
        	0x0c:关机+出厂设置
        */
        if(pdata[1]==0x00)/**/
        {
            GetSleepRecord(&pdata[4],(StuEeprom.StuPara.time/DAY-(6))*DAY);
            if(pdata[5]>15)
                pdata[5]=15;
            pdata[0]=0x91;
            pdata[1]=read_history_items((StuEeprom.StuPara.time/DAY-(6))*DAY);
            pdata[2]=(pdata[4]<<4)|(pdata[5]&0x0f);
            pdata[3]=StuPower.persent;
            BleWriteData(pdata,4);
        } else if(pdata[1]==1)
        {
            SendItems(pdata,false);
        } else if(pdata[1]==2)
        {
            SendSleepIndex(pdata,(StuEeprom.StuPara.time/DAY-(6))*DAY);
        } else if(pdata[1]==3)
        {
            SendSleepRecord(pdata,(StuEeprom.StuPara.time/DAY-(6))*DAY);
        }
        else if(pdata[1]==4)
        {
//             SendSn(pdata);
        } else if(pdata[1]==6)
        {
            pdata[0]=0x90;
            pdata[1]=version[0];
            pdata[2]=version[1];
            pdata[3]=version[2];
            BleWriteData(pdata,4);
        } else if(pdata[1]==7)
        {
            pdata[0]=0xa2;
            get_mac(pdata+1);
            BleWriteData(pdata,7);
        } else if(pdata[1]==8)
        {
            pdata[0]=0xa3;
            pdata[1]=12;
//             for(i=0; i<12; i++)
//             {
//                 pdata[2+i]=moudle_name[i];
//             }
            BleWriteData(pdata,2+12);
        } else if(pdata[1]==9)
        {
            pdata[0]=0x96;
            pdata[1]=0x9;
            pdata[2]=version[5];
            pdata[3]=version[6];
            pdata[4]=version[7];
            BleWriteData(pdata,5);
        } else if(pdata[1]==10)
        {
            pdata[0]=0x96;
            pdata[1]=10;
            BleWriteData(pdata,2);
// 					para_history_factory_set();
            EEpromSetParaToFactory();
        } else if(pdata[1]==11)
        {
            RTC_UTCTimeStruct timetmp;
            timetmp=time;
            pdata[0]=0x96;
            pdata[1]=11;
            BleWriteData(pdata,2);
//             para_history_factory_set();
            EEpromSetParaToFactory();
            time=timetmp;
            StuEeprom.StuPara.time=convert_time_to_Second(time);
        } else if(pdata[1]==0x0c)
        {
            pdata[0]=0x96;
            pdata[1]=0x0c;
            BleWriteData(pdata,2);
            StuEeprom.StuPara.ask_power_off=1;
            CallErr(DISPLAY_HELLO_UI,0,(const uint8_t *)"blereset");
        } else if(pdata[1]==0x0d) /*读闹钟*/
        {
            SendAlarm(pdata);
        } else if(pdata[1]==0x0e)
        {
            pdata[0]=0x96;
            pdata[1]=0x0e;
            BleWriteData(pdata,2);
            StuEeprom.StuPara.ask_power_off=2;
            StuEeprom.StuPara.ask_version=3;
            CallErr(DISPLAY_HELLO_UI,0,(const uint8_t *)"blereset");
        } else if(pdata[1]==0x0f)
        {
            pdata[0]=0x96;
            pdata[1]=0x0f;
            BleWriteData(pdata,2);
            delay_ms(500);
            AskPair();
        } else if(pdata[1]==0x10)//
        {
            if(6==len)
            {
                tmp32=pdata[2];
                tmp32=(tmp32<<8)+pdata[3];
                tmp32=(tmp32<<8)+pdata[4];
                tmp32=(tmp32<<8)+pdata[5];

                pdata[0]=0x96;
                pdata[1]=0x10;
                BleWriteData(pdata,2);
                if( StuEeprom.StuPara.ancs_notice!=tmp32)
                {
                    StuEeprom.StuPara.ancs_notice=tmp32;
                    EepromAnsy();
                }
            } else
            {
                pdata[0]=0x95;
                pdata[1]=0x10;
                BleWriteData(pdata,2);
            }
        } else if(pdata[1]==0x11)//
        {
            if(2==len)
            {
                pdata[0]=0xa5;
                pdata[1]=0x11;
                pdata[2]=StuEeprom.StuPara.ancs_notice>>24;
                pdata[3]=StuEeprom.StuPara.ancs_notice>>16;
                pdata[4]=StuEeprom.StuPara.ancs_notice>>8;
                pdata[5]=StuEeprom.StuPara.ancs_notice>>0;
                BleWriteData(pdata,6);
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x11;
                BleWriteData(pdata,2);
            }
        } else if(pdata[1]==0x12)//总的数据
        {
            if(2==len)
            {

                pdata[0]=0xa5;
                pdata[1]=0x12;
                GetSleepRecord(&pdata[2],(StuEeprom.StuPara.time/DAY-6)*DAY );


                tmp32_1=usr_api_read(0,0xa8,(StuEeprom.StuPara.time/DAY-6)*DAY ,0);
                tmp32=tmp32_1/3;
                if(tmp32_1%3)
                    tmp32++;
                if(tmp32>0xfe)
                    tmp32=0xfe;
                pdata[4]=tmp32;
                pdata[5]=0;
                BleWriteData(pdata,6);


            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x12;
                BleWriteData(pdata,2);
            }
        } else if(pdata[1]==0x13)//读取当天的数据
        {
            if(2==len)
            {
                ChangeToHightSpeed(true);
                pdata[0]=0xa5;
                pdata[1]=0x13;
                GetSleepRecord(&pdata[2],(StuEeprom.StuPara.time/DAY-0)*DAY-DAY/4);
                tmp32_1=usr_api_read(0,0xa8,(StuEeprom.StuPara.time/DAY-0)*DAY ,0);
                tmp32=tmp32_1/3;
                if(tmp32_1%3)
                    tmp32++;
                if(tmp32>0xfe)
                    tmp32=0xfe;
                pdata[4]=tmp32;

                pdata[5]=0;
                BleWriteData(pdata,6);
                SendItems(pdata,true);
                SendSleepIndex(pdata,(StuEeprom.StuPara.time/DAY-(0))*DAY-DAY/4);
                SendSleepRecord(pdata,(StuEeprom.StuPara.time/DAY-(0))*DAY-DAY/4);
                send_point_data=SEND_T_A8_HR;
                point_time_sec=(StuEeprom.StuPara.time/DAY-0)*DAY;
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x13;
                BleWriteData(pdata,2);
            }
        } else if(pdata[1]==0x14)//设置开始时间结束时间
        {
            if((6==len)&&(pdata[2]>=18)&&(pdata[2]<=23)&&(pdata[3]<60)&&\
                    (pdata[4]<=10)&&(pdata[5]<60))
            {
                uint16_t starttime,endtime;
                starttime=((uint16_t)pdata[2])*60+pdata[3];
                endtime=((uint16_t)pdata[4])*60+pdata[5];
                pdata[0]=0xa5;
                pdata[1]=0x14;
                BleWriteData(pdata,2);
                if((StuEeprom.StuPara.sleep_start_min!=starttime)||(StuEeprom.StuPara.sleep_end_min!=endtime))
                {
                    StuEeprom.StuPara.sleep_start_min=starttime;
                    StuEeprom.StuPara.sleep_end_min=endtime;
                    EepromAnsy();
                }


            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x14;
                BleWriteData(pdata,2);
            }
        } else if(pdata[1]==0x15)//读取睡眠开始时间结束时间
        {
            if(2==len)
            {
                pdata[0]=0xa5;
                pdata[1]=0x15;
                pdata[2]=StuEeprom.StuPara.sleep_start_min/60;
                pdata[3]=StuEeprom.StuPara.sleep_start_min%60;
                pdata[4]=StuEeprom.StuPara.sleep_end_min/60;
                pdata[5]=StuEeprom.StuPara.sleep_end_min%60;
                BleWriteData(pdata,6);
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x15;
                BleWriteData(pdata,2);
            }

        } else if(pdata[1]==0x16)//解除ancs，断开蓝牙连接
        {
            if(2==len)
            {
                pdata[0]=0xa5;
                pdata[1]=0x16;
                BleWriteData(pdata,2);
                delay_ms(1000);
                EraseBond();
                TaskUpFile();
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x16;
                BleWriteData(pdata,2);
            }

        }
        else if(pdata[1]==0x17)//配置心率自动监测间隔
        {
            if( (4==len)&&(pdata[2]<4))
            {
                StuEeprom.StuPara.hr_inter=pdata[2];
                pdata[0]=0xa5;
                pdata[1]=0x17;
                BleWriteData(pdata,2);
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x17;
                BleWriteData(pdata,2);
            }

        }	else if(pdata[1]==0x18)//读取所有心率数
        {
            if(2==len)
            {
                ChangeToHightSpeed(true);
                pdata[0]=0xa5;
                pdata[1]=0x18;
                BleWriteData(pdata,2);
                send_point_data=SEND_T_A8_HR;
                point_time_sec=(StuEeprom.StuPara.time/DAY-6)*DAY;
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x18;
                BleWriteData(pdata,2);
            }

        }	 else if(pdata[1]==0x19)//设置菜单
        {
            if((6==len)&&(pdata[5]&0x01))
            {
                tmp32=pdata[2];
                tmp32=(tmp32<<8)+pdata[3];
                tmp32=(tmp32<<8)+pdata[4];
                tmp32=(tmp32<<8)+pdata[5];

                pdata[0]=0xa5;
                pdata[1]=0x19;
                BleWriteData(pdata,2);
                tmp32<<=1;
                if( StuEeprom.StuPara.menu_enable_index!=tmp32)
                {
                    StuEeprom.StuPara.menu_enable_index=tmp32;
                    EepromAnsy();
                }
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x19;
                BleWriteData(pdata,2);
            }
        } else if(pdata[1]==0x20)//设置自动上报实时数据
        {
            if(2==len)
            {
                report_real_time_data=REAL_TIME_MAX;
                pdata[0]=0xa5;
                pdata[1]=0x20;
                BleWriteData(pdata,2);
            } else
            {
                pdata[0]=0xa4;
                pdata[1]=0x20;
                BleWriteData(pdata,2);
            }
        }
        break;
    case 0x17:  /**/
        pdata[0]=0x95;
        if( (5==len)&&(pdata[1]<3)&&(pdata[2]<21)&&(pdata[3]<51)&&(pdata[4]<51))
        {
            MotorOn(pdata[2],pdata[3],pdata[4]);
            pdata[0]=0x96;
        }
        pdata[1]=0x17;
        BleWriteData(pdata,2);
        break;
    case 0x20:
//         if(len==17)
//         {
//             SetSn(&pdata[1],&ansy_flg);
//             pdata[0]=0x96;
//             pdata[1]=0x20;
//             BleWriteData(pdata,2);
//             if(1==ansy_flg)
//                 ansy_para_history();

//         } else
//         {
//             pdata[0]=0x95;
//             pdata[1]=0x20;
//             BleWriteData(pdata,2);
//         }
        break;
    case 0x22:/*dfu开始升级命令*/
//        if(pdata[1]==0x01)
//        {

//            pdata[0]=0xa1;
//            pdata[1]=0x22;
//            BleWriteData(pdata,2);
//            StuEeprom.StuPara.ask_dfu=1;
//            EepromAnsy();
//            ShowUpdating();
//            delay_ms(3000);
//            CallErr(NO_HELLO_UI+1,0,(const uint8_t *)"0x2001dfureset");
//        }
        break;
    case 0x23:/*距离显示单位设置 0公制  1英制*/
        pdata[0]=0x95;
        if(pdata[1]<2)
        {
            StuEeprom.StuPara.distance_unit=pdata[1];
            pdata[0]=0x96;
        }
        pdata[1]=0x23;
        BleWriteData(pdata,2);
        break;
    case 0x24:/*24H--0   12H---1*/
        pdata[0]=0x95;
        if(pdata[1]<2)
        {
            StuEeprom.StuPara.time_format=pdata[1];
            pdata[0]=0x96;
        }
        pdata[1]=0x24;
        BleWriteData(pdata,2);
        break;
    case 0x25:/*自动亮屏幕*/
        if((2==len)&&(pdata[1]<2))
        {
            if(pdata[1]!=StuEeprom.StuPara.oled_auto_on)
            {
                StuEeprom.StuPara.oled_auto_on=pdata[1];
                ansy_flg=1;
            }
            pdata[0]=0x96;
        } else
        {
            pdata[0]=0x95;
        }
        pdata[1]=cmd;
        BleWriteData(pdata,2);

        if(1==ansy_flg)
        {
            EepromAnsy();
        }
        break;
    case 0x27:/*保持上次的屏幕*/
        if((2==len)&&(pdata[1]<2))
        {
            if(pdata[1]!=StuEeprom.StuPara.oled_sava_last_ui)
            {
                StuEeprom.StuPara.oled_sava_last_ui=pdata[1];
                ansy_flg=1;
            }
            pdata[0]=0x96;
        } else
        {
            pdata[0]=0x95;
        }
        pdata[1]=cmd;
        BleWriteData(pdata,2);
        if(1==ansy_flg)
            EepromAnsy();
        break;
    case 0x28://开始升级
        if(len==7)
        {
//            tmp32=pdata[1];
//            tmp32=(tmp32<<8)+pdata[2];
//            tmp32=(tmp32<<8)+pdata[3];
//            tmp32=(tmp32<<8)+pdata[4];
//            tmp16=pdata[5];
//            tmp16=(tmp16<<8)+pdata[6];
            /*28 crc crc len len len len*/
            tmp32=pdata[3];
            tmp32=(tmp32<<8)+pdata[4];
            tmp32=(tmp32<<8)+pdata[5];
            tmp32=(tmp32<<8)+pdata[6];
            tmp16=pdata[1];
            tmp16=(tmp16<<8)+pdata[2];
            InitBleUpdata(tmp32,tmp16,true);

            pdata[0]=0x96;
            pdata[1]=0x28;
            BleWriteData(pdata,2);
        } else
        {
            pdata[0]=0x95;
            pdata[1]=0x28;
            BleWriteData(pdata,2);
        }
        break;
    case 0x29://升级数据
        if(len==20)
        {
            tmp16=pdata[1];
            tmp16=(tmp16<<8)+pdata[2];
            BleFileIn(tmp16,&pdata[3],17);
            return ;
        } else
        {
            pdata[0]=0x95;
            pdata[1]=0x29;
            BleWriteData(pdata,2);


        }
        break;
    case 0x2a:/*设置久坐提醒*/
        if((17==len)&&(pdata[1]<2)&&(true==SetBandParaSendentary(&pdata[1],&ansy_flg)))
        {
            pdata[0]=0x96;
        } else
        {
            pdata[0]=0x95;
        }
        pdata[1]=0x2a;
        BleWriteData(pdata,2);
        if(1==ansy_flg)
            EepromAnsy();
        break;
    case 0x2b:/*设置久坐提醒参数*/
        if((3==len)&&(true==SetSendentarypara(&pdata[1],&ansy_flg)))
        {
            pdata[0]=0x96;
        } else
        {
            pdata[0]=0x95;
        }
        pdata[1]=0x2b;
        BleWriteData(pdata,2);
        if(1==ansy_flg)
            EepromAnsy();
        break;
    case 0x2c://获取时间节点数据
        if(7!=len) break;
        len=pdata[6];
        if(true==ana_time_point(pdata,&pdata[19]))
        {
            pdata[0]=0xaa;
            pdata[1]=len;
            pdata[2]=pdata[19];
            BleWriteData(pdata,3);
        }
        break;
    case 0x99://call
    case 0x9a://msg
    case 0x9b://
    case 0x9c://
    case 0x9d://
    case 0x9e://
    case 0x9f://
    case 0xa0://
        if((pdata[1]<2)&&(pdata[2]==0)&&(pdata[3]<17))
        {
            SendMsg((MSG_TYPE)pdata[0],pdata[3],&pdata[4]);
            pdata[0]=0x96;
            pdata[1]=cmd;
            BleWriteData(pdata,2);
        }
        break;
    case 0xff:
        if((0xaa== pdata[1])&&(0xaa== pdata[2])&&(len==3))
        {
            TaskUpFile();
            TestPrintfSleep();
        }
        if((0xbb== pdata[1])&&(0xbb== pdata[2])&&(len==9))//ff bb bb addr addr addr addr size size
        {
            TaskUpFile();

            tmp32=pdata[3];
            tmp32=(tmp32<<8)+pdata[4];
            tmp32=(tmp32<<8)+pdata[5];
            tmp32=(tmp32<<8)+pdata[6];
            tmp16=pdata[7];
            tmp16=(tmp16<<8)+pdata[8];

            // TestPrintfSleep();

            TestPrintfaddr(tmp32,tmp16);

        }
#ifdef DEBUF_SEDENTARY
        if((0xcc== pdata[1])&&(0xcc== pdata[2])&&(len==3))
        {
            TaskUpFile();
            TestPrintfSedentary();
        }
#endif

        break;
    default:
        break;

    }





}






