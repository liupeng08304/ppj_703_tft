#include "alarm.h"
#include "in_flash_manage.h"
#include "SoftwareRTC.h"
#include "sem.h"
#include "sleep.h"
#include "ui_app.h"
#include "protocol.h"
/*检查闹钟是否到达，到达发送闹钟*/
void TaskAlarmCheck(void)
{
    uint8_t i=0,week;
    for(i=0; i<MAX_ALARM; i++)
    {
        week=StuEeprom.StuPara.StuAlarm[i].work_day;

        if( (StuEeprom.StuPara.StuAlarm[i].on_flg==true)&&
            (time.hour==StuEeprom.StuPara.StuAlarm[i].hour)&&
            (time.minutes==StuEeprom.StuPara.StuAlarm[i].min)&&
            (week&(0x01<<time.week))
          )
        {
            if(StuEeprom.StuPara.StuAlarm[i].type<=ALARM_GET_TARGET)
            {
                StuMsg.msg[0]=StuEeprom.StuPara.StuAlarm[i].hour;
                StuMsg.msg[1]=StuEeprom.StuPara.StuAlarm[i].min;
                StuMsg.msg[2]=StuEeprom.StuPara.StuAlarm[i].type;
                StuMsg.len=3;
                StuMsg.type= (MSG_TYPE)(0xb0+StuEeprom.StuPara.StuAlarm[i].type);
                SendSem(NOTICE_COM);
            }
            return;
        }
    }

}

// 检查是否在时间端内
bool in_hour_min(uint16_t timenow,uint16_t timestart,uint16_t timeend)
{

    if(timestart<=timeend)
    {
        if((timenow>=timestart)&&(timenow<=timeend))
            return true;

    }
    else
    {
        if((timenow<=timeend)||(timenow>=timestart))
            return true;

    }
    return false;

}
// 检查是否在久坐提醒的时间端里面
bool check_in_sedentary(void)
{

    uint8_t i=0,week;
    uint16_t timenow;
    uint16_t timestart,timeend;
    timenow=time.hour*60+time.minutes;
    for(i=0; i<6; i++)
    {
        week=StuEeprom.StuPara.StuSendentaryRemain[i].work_day;
        timestart=StuEeprom.StuPara.StuSendentaryRemain[i].s_hour*60+StuEeprom.StuPara.StuSendentaryRemain[i].s_min;
        timeend=StuEeprom.StuPara.StuSendentaryRemain[i].e_hour*60+StuEeprom.StuPara.StuSendentaryRemain[i].e_min;

        if( (StuEeprom.StuPara.StuSendentaryRemain[i].on_flg==true)&&\
            (week&(0x01<<time.week))&&(true==in_hour_min(timenow,timestart,timeend))


          )
        {

            return true;
        }
    }

    return false;
}

//发送久坐提醒通知
void SendSedentaryNotice(void)
{

    StuMsg.type= (MSG_TYPE)(0xb0+SENDENTARY);
    SendSem(NOTICE_COM);

}
#ifdef DEBUF_SEDENTARY
uint16_t bigtimes30[60],in=0;

void TestPrintfSedentary(void)
{
    uint8_t i=0,k,buf[100];
    uint16_t len=0;
    //0---29

    len=0;
    len=sprintf((char*)&buf[len],"big=%d t=%d\r\n",StuEeprom.StuPara.sedentary_big_times,StuEeprom.StuPara.sedentary_min);
    TaskPrintfFile(buf,len);

    k=in;
    for(i=0; i<60; i++)
    {
        len=0;
        len=sprintf((char*)&buf[len],"in=%d k=%d turn:%d big:%d wear:%d\r\n",in,k,i,bigtimes30[k]&0x7fff,(bigtimes30[k]&0x8000)? 1:0);
        TaskPrintfFile(buf,len);
        k++;
        k%=60;
    }

}

#endif



/*检查久坐提醒*/
void TaskSendentaryCheck(bool wear)
{
    static uint8_t static_time=0;
//      static uint8_t send_flg=0;
    if(false==check_in_sedentary()) return;
#ifdef DEBUF_SEDENTARY
    if(wear==true)
        big_times|=0x8000;
    else
        big_times&=0x7fff;
    bigtimes30[in++]=big_times;
    in%=60;
#endif

    if(true!=wear)
    {
        big_times=0;
        return;
    }

    big_times&=0x7fff;
    if(big_times<   StuEeprom.StuPara.sedentary_big_times)
    {


        static_time++;
//          if( (static_time>=StuBandParaHistoryData.sedentary_min)&&(send_flg==0))
        if(static_time>=StuEeprom.StuPara.sedentary_min)
        {
//              send_flg=1;
            static_time=0;
            SendSedentaryNotice();
        }
    }
    else
    {
//          send_flg=0;
        static_time=0;
    }


//      #ifdef DEBUF_SEDENTARY
//      uint8_t buf[100];
//      uint16_t len=0;
//      len=sprintf((char*)&buf[len],"big=%d t=%d tc:%d current big:%d,send flg:%d\r\n",StuBandParaHistoryData.sedentary_big_times,\
//      StuBandParaHistoryData.sedentary_min,\
//      static_time,
//      big_times,send_flg);
//      TaskPrintfFile(buf,len);
//      #endif


    big_times=0;
}




//发送电量低于20%
void SendPower20Notice(void)
{

    StuMsg.type= (MSG_TYPE)(0xb0+POWER_20);
    SendSem(NOTICE_COM);

}


//
void SendMacShowNotice(void)
{

    StuMsg.type= (MSG_TYPE)(0xb0+MAC_SHOW);
    SendSem(NOTICE_COM);

}
