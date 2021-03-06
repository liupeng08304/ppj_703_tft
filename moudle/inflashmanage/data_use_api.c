#include "data_use_api.h"


//type+sec+data(11)


//写入用户数据
//step :type+sec+step(4byte)+step_sec(2byte)+distance_cm(2byte)+calories(2byte)
//HR:  type+sec+ hr(1byte)
void usr_api_write(uint8_t type,uint8_t* datain,uint32_t sec)
{
    uint8_t len=0;
    static uint8_t buf[RECORD_UNIT_LEN];


    if((sec<SEC_2017)||(sec>SEC_2099)) return;
    buf[len++]=type;
    buf[len++]=sec>>24;
    buf[len++]=sec>>16;
    buf[len++]=sec>>8;
    buf[len++]=sec;
    memcpy(&buf[len],datain,11);
    WriteDataToQueue(buf,1);
}

//读取用户数据,pdata为0，只读取数据个数，否则读一条发送一条


bool ble_send_step(uint8_t *buf,uint8_t *datain,uint8_t index,uint32_t sec);
//发送当前的记步数据
bool ble_send_current_step(uint8_t *buf);
bool ble_send_hr(uint8_t *buf,uint8_t *datain,uint32_t index,uint32_t sec);
bool ble_send_wait_to_success(uint8_t *datain,uint16_t len);

//打包用户数据,是否发送在于用户
bool data_use_packet_data(uint8_t type ,uint32_t sec,uint8_t* datain,uint8_t *pdata,uint32_t cnt)
{

    if(type==0xa8)//心率数据
    {
        return ble_send_hr(pdata,datain,cnt,sec);
    } else 	if(type==0x92)//记步数据
    {
        return  ble_send_step(pdata,datain,cnt,sec);
    }

    return false;

}




bool ble_send_hr(uint8_t *buf,uint8_t *datain,uint32_t index,uint32_t sec)
{
    RTC_UTCTimeStruct time_tmp;
    bool rev=true;
    uint32_t position;
    static uint32_t len=0;
    position=index%3;

//	if(index==3)
//	{
//position=6;
// }
    if(1==position)
    {
        len=0;
        buf[len++]=0xa8;
        buf[len++]=index/3;
    }
    ConvertToUTCTime(&time_tmp,sec);
    buf[len++]=time_tmp.year-2000;
    buf[len++]=time_tmp.month;
    buf[len++]=time_tmp.day;
    buf[len++]=time_tmp.hour;
    buf[len++]=time_tmp.minutes;
    buf[len++]=datain[0];
    if(0==position)
    {
        rev=ble_send_wait_to_success(buf,len);
        memset(buf,0,20);
        len=0;
    }

    return rev;

}

//发送当前的记步数据
bool ble_send_current_step(uint8_t *buf)
{
    uint8_t len=0;
    buf[len++]=0x92;
    buf[len++]=0;


    buf[len++]=time.year-2000;
    buf[len++]=time.month;
    buf[len++]=time.day;

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

    return ble_send_wait_to_success(buf,len);
}


void ble_send_history_step(uint32_t sec,uint8_t *buf)
{
    uint16_t len,i,head=0;
    RTC_UTCTimeStruct time_tmp;
    if(StuEeprom.StuPara.time>sec)
    {
        head=1;
    }
    for(i=0; i<HISTORY_DATA_ITEM; i++)
    {
        if(StuEeprom.StuPara.StuHistory[i].time>=sec)
        {

            len=0;
            buf[len++]=0x92;
            buf[len++]=head+i;
            ConvertToUTCTime(&time_tmp,StuEeprom.StuPara.StuHistory[i].time);
            buf[len++]=time_tmp.year-2000;
            buf[len++]=time_tmp.month;
            buf[len++]=time_tmp.day;

            buf[len++]=StuEeprom.StuPara.StuHistory[i].step>>24;
            buf[len++]=StuEeprom.StuPara.StuHistory[i].step>>16;
            buf[len++]=StuEeprom.StuPara.StuHistory[i].step>>8;
            buf[len++]=StuEeprom.StuPara.StuHistory[i].step>>0;


            buf[len++]=StuEeprom.StuPara.StuHistory[i].step_min>>8;
            buf[len++]=StuEeprom.StuPara.StuHistory[i].step_min>>0;

            buf[len++]=StuEeprom.StuPara.StuHistory[i].distance>>8;
            buf[len++]=StuEeprom.StuPara.StuHistory[i].distance>>0;

            buf[len++]=(StuEeprom.StuPara.StuHistory[i].calories)>>8;
            buf[len++]=(StuEeprom.StuPara.StuHistory[i].calories)>>0;
            if(true!=ble_send_wait_to_success(buf,len))
                return;

        }
    }
}

bool ble_send_step(uint8_t *buf,uint8_t *datain,uint8_t index,uint32_t sec)
{
    RTC_UTCTimeStruct time_tmp;
    uint8_t len=0;
    ConvertToUTCTime(&time_tmp,sec);
    buf[len++]=0x92;
    buf[len++]=index;
    buf[len++]=time_tmp.year-2000;
    buf[len++]=time_tmp.month;
    buf[len++]=time_tmp.day;

    memcpy(&buf[len],datain,10);
    len+=10;
    return ble_send_wait_to_success(buf,len);
}

bool ble_send_wait_to_success(uint8_t *datain,uint16_t len)
{
    uint32_t ticktime;
    uint8_t err_times=0;


    while(true!=send_ble_data(datain,len))
    {
        ticktime=NRF_RTC1->COUNTER;
        while(get_time_escape_ms(NRF_RTC1->COUNTER,ticktime)<1000)
        {
            power_manage();/*key  time BLE */
            V_FeedWdog();/*喂狗*/
            task_ui();
            if(is_ble_tx_enable()!=true) return false;
        }
        if(err_times++>5)
        {
            return false;
        }
    }
    err_times=0;
    return true;
}


