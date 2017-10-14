#include "sleep.h"

static SleepWrite_fun ReacallSleepWrite;
static SleepErase_fun ReacallSleepErase;
static SleepRead_fun ReacallSleepRead;
static uint16_t start_min,end_min;
static uint8_t sleep_start_flg=0;
STU_SLEEP_FLASH StuSleepFlash;
static uint8_t *pstatic_time=0; /*unit:minute*/


/*
初始化睡眠
参数说明：
start_time_min ：21:25  --->21*60+25
end_time_min ：7:30     --->7*60+30
*/
void SleepInit(uint16_t start_time_min,\
               uint16_t end_time_min,\
               uint8_t*pstatic_min,\
               SleepWrite_fun SleepWrite,\
               SleepErase_fun SleepErase,\
               SleepRead_fun SleepRead
              )
{

    ReacallSleepWrite=SleepWrite;
    ReacallSleepErase=SleepErase;
    ReacallSleepRead=SleepRead;
    pstatic_time=pstatic_min;


    start_min=start_time_min;
    end_min=end_time_min;
    StuSleepFlash.big_times=0;
    StuSleepFlash.wear_flg=UN_WEAR;
    StuSleepFlash.rsv=0;
    StuSleepFlash.tmp=0;
}

/*
uint32_t s_abs(int32_t a)
获取绝对值
*/
uint32_t s_abs(int32_t a)
{
    if(a>0)
        return a;
    else
        return (-a);

}

uint16_t big_times=0;//运动就会自加

void CallByGsensor(int16_t x,int16_t y,int16_t z)
{

#ifdef EBABLE_PRINTF_SLEEP

    uint8_t buf[50],len=0;
#endif
    static  int16_t x_b, y_b, z_b;
    uint32_t acc_dif;
    //  if(0==sleep_start_flg) return;
    acc_dif=s_abs(x_b-x)+s_abs(y_b-y)+s_abs(z_b-z);


#ifdef EBABLE_PRINTF_SLEEP
    if(acc_dif>20)
    {
        len+=sprintf((char*)&buf[len],"acc_dif:%04d\r\n",acc_dif);
        TaskPrintfFile(buf,len);
    }
#endif


    if(acc_dif>MOVE_THROTE)
    {
        if(StuSleepFlash.big_times<0xfffd)
            StuSleepFlash.big_times++;

        if(big_times<0xfffe)
        {
            big_times++;
        }

        if( StuSleepFlash.big_times>4)
            *pstatic_time=0;
    }
    x_b=x;
    y_b=y;
    z_b=z;
}





//获取当前的写位置
static uint32_t GetWriteAddr(uint8_t hour,uint8_t min,uint8_t week)
{
    uint32_t write_addr;
    uint16_t time_now;
    time_now=hour*60+min;
    if(time_now>=start_min)
        write_addr=SLEEP_BEGIN_ADDR+(uint32_t)week*SLEEP_FLASH_SEC_SIZE+(((time_now-start_min)/SLEEP_MINUTES)<<2);
    else
        write_addr=SLEEP_BEGIN_ADDR+(uint32_t)week*SLEEP_FLASH_SEC_SIZE+((((24*60)-start_min+time_now)/SLEEP_MINUTES)<<2);
    return write_addr;
}

void CheckErase(uint32_t write_addr)
{
#define  TMP_BUF_LEN (30/SLEEP_MINUTES)
    uint32_t buf[TMP_BUF_LEN];
    uint8_t i;
    for(i=0; i<(TMP_BUF_LEN); i++)
    {
        buf[i]=ReacallSleepRead((uint32_t*)(write_addr+i*4));
    }

    if( (buf[TMP_BUF_LEN-1]!=0xffffffff)||(buf[TMP_BUF_LEN-2]!=0xffffffff))
    {
        ReacallSleepErase(write_addr/SLEEP_FLASH_SEC_SIZE);
    }

}

static STU_SLEEP_FLASH ReadSleepItem(uint32_t addr)
{
    uint32_t tmp;
    STU_SLEEP_FLASH StuSleepFlash_tmp;
    tmp=ReacallSleepRead((uint32_t*)addr);
    StuSleepFlash_tmp=(*((STU_SLEEP_FLASH*)&tmp));
    return StuSleepFlash_tmp;

}

void erase_sleep_flash(void)
{
    uint32_t page_num= SLEEP_BEGIN_ADDR/SLEEP_FLASH_SEC_SIZE,i;

    for(i=0; i<7; i++)
        ReacallSleepErase(page_num++);
}


static void DeleteWearFlg(uint16_t delete_minutes,uint32_t addr_now)
{
    STU_SLEEP_FLASH StuSleepFlash_tmp;
    uint32_t i,items_start_addr;
    items_start_addr=   addr_now-((delete_minutes/SLEEP_MINUTES)<<2);
    for(i=0; i<(delete_minutes/SLEEP_MINUTES); i++)
    {
        StuSleepFlash_tmp=ReadSleepItem(items_start_addr+i*4);
        StuSleepFlash_tmp.wear_flg=UN_WEAR;
        ReacallSleepWrite((uint32_t*)(items_start_addr+i*4),(uint32_t*)&StuSleepFlash_tmp,1);
    }

}



extern uint16_t irvalue;



//睡眠监测函数 1min调用一次
void TaskSleep(uint8_t hour,uint8_t min,uint8_t week_in,uint8_t wearflg)
{
    uint16_t time_now;
    uint32_t write_addr;
    uint8_t week;
    uint8_t cmpmin;

    cmpmin=UN_WEAR_MIN_BEFORE12;
    week=week_in;
    if(hour<=12)
    {
        cmpmin=UN_WEAR_MIN;
        if(week>0)
            week--;
        else
            week=6;
    }

    //wait sleep start
    if(0==sleep_start_flg)
    {
        time_now=hour*60+min;
        if((time_now>=start_min)||(time_now<end_min))
        {
            sleep_start_flg=1;
            write_addr=GetWriteAddr(hour,min,week);
            CheckErase(write_addr);
            if(hour>12)
                *pstatic_time=cmpmin+10;

        }
    }

    if(1==sleep_start_flg)
    {

        //collect sleep data wait sleep end
        if(*pstatic_time==cmpmin)
        {
            //删除之前的佩戴记录
            write_addr=GetWriteAddr(hour,min,week);
            DeleteWearFlg(*pstatic_time,write_addr);
        }

        if(*pstatic_time>=cmpmin)/*认为没有佩戴*/
        {
            StuSleepFlash.wear_flg=UN_WEAR;
        }
        else
        {
            //可以根据佩戴标志来发送是否佩戴
            if(wearflg)
                StuSleepFlash.wear_flg=WEAR;
            else
                StuSleepFlash.wear_flg=UN_WEAR;
        }


        if(0==(min%SLEEP_MINUTES))
        {

            //保存一次一次记录

            write_addr=GetWriteAddr(hour,min,week);

            StuSleepFlash.tmp=irvalue;

            ReacallSleepWrite((uint32_t*)write_addr,(uint32_t*)&StuSleepFlash,1);


            time_now=hour*60+min;
            //检查是否结束睡眠
            if(
                ((StuSleepFlash.big_times>=WAKE_THROSHORT)&&\
                 (time_now<(10*60))&&(time_now>=end_min)
                )||((time_now<(12*60))&&(time_now>=(10*60)) )
            )
            {
                sleep_start_flg=0;
            }
            StuSleepFlash.big_times=0;
            StuSleepFlash.wear_flg=UN_WEAR;
            StuSleepFlash.rsv=0;
            StuSleepFlash.tmp=0;
        }

    }


    if(*pstatic_time<0xfd)
        pstatic_time[0]++;

}


#include "ui_app.h"
#include "wdt.h"
void TestPrintfSleep(void)
{
    uint8_t buf[100];
    uint8_t len;


    uint32_t addr,i;
    uint16_t end_time_item;
    STU_SLEEP_FLASH StuSleepFlash_tmp;
//    uint8_t state,byte,bit;


    uint8_t hour,min,week;



    for(week=0; week<7; week++)
    {
        hour=start_min/60;
        min=start_min%60;
        V_FeedWdog();/*喂狗*/
        addr= GetWriteAddr(start_min/60,start_min%60,week);
        end_time_item=(24*60-start_min+10*60)/SLEEP_MINUTES;

        //超次数 或者没有数据的时候结束
        for(i=0; i<end_time_item; i++)
        {
            StuSleepFlash_tmp=ReadSleepItem(addr+i*4);
            len=0;
            len=sprintf((char*)&buf[len],"week:%d %02d:%02d b:%05d w:%d dec:%d\r\n",week,hour,min,StuSleepFlash_tmp.big_times,StuSleepFlash_tmp.wear_flg,StuSleepFlash_tmp.tmp);
            TaskPrintfFile(buf,len);
            min+=5;
            if(min>=60)
            {
                min=0;
                hour++;
                if(hour>=24)
                    hour=0;
            }
        }
    }

}


void TestPrintfaddr(uint32_t addrin,uint16_t size)
{
    uint8_t buf[100];
    uint8_t len;
    uint16_t i;
    uint8_t *pdata;
#include "queueu51822.h"
    FLASH_QUEUE p_FlashQueue;
#include "SoftwareRTC.h"
    RTC_UTCTimeStruct timetmp;
    uint32_t tmp32;
//#define FLASH_QUEUE_ADDR       (APP_ADDR+APP_SIZE)    //心率数据开始地址
//#define FLASH_QUEUE_SIZE          (27*1024)
//#define FLASH_ADDR_RECORD_PARAM_LEN    8
//#define FLASH_PARAM_MAX
    if(0!=addrin)
    {
        pdata=(uint8_t*)addrin;
        len=sprintf((char*)&buf[0],"data start\r\n");
        TaskPrintfFile(buf,len);
        i=0;
        while(i<size)
        {
            len=sprintf((char*)&buf[0],"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",\
                        pdata[0],pdata[1],pdata[2],pdata[3],pdata[4],pdata[5],pdata[6],pdata[7],\
                        pdata[8],pdata[9],pdata[10],pdata[11],pdata[12],pdata[13],pdata[14],pdata[15]);
            TaskPrintfFile(buf,len);
            V_FeedWdog();/*喂狗*/
            pdata+=16;
            i+=16;
        }
        return ;
    }
    pdata=(uint8_t*)FLASH_QUEUE_ADDR;
    len=sprintf((char*)&buf[0],"para start\r\n");
    TaskPrintfFile(buf,len);
    i=0;
    while(i<1024)
    {
        if(pdata[6]!=0xaa)
        {

            len=sprintf((char*)&buf[0],"%02x%02x%02x%02x%02x%02x%02x%02x\r\n",pdata[0],pdata[1],pdata[2],pdata[3],pdata[4],pdata[5],pdata[6],pdata[7]);
        }
        else
        {
            p_FlashQueue.cnt = (*(uint16_t *)pdata);
            p_FlashQueue.tail_index= (*(uint16_t *)(pdata+2));
            p_FlashQueue.head_index= (*(uint16_t *)(pdata+4));
            len=sprintf((char*)&buf[0],"cnt:%dtail:%dhead:%d\r\n",\
                        p_FlashQueue.cnt,\
                        p_FlashQueue.tail_index,\
                        p_FlashQueue.head_index
                       );

        }
        TaskPrintfFile(buf,len);
        V_FeedWdog();/*喂狗*/
        pdata+=8;
        i+=8;
    }


    len=sprintf((char*)&buf[0],"data start\r\n");
    TaskPrintfFile(buf,len);
    i=0;
    while(i<(26*1024))
    {
        if(pdata[0]==0xa8)
        {

            tmp32=pdata[1];
            tmp32=(tmp32<<8)+pdata[2];
            tmp32=(tmp32<<8)+pdata[3];
            tmp32=(tmp32<<8)+pdata[4];
            ConvertToUTCTime(&timetmp,tmp32);
            len=sprintf((char*)&buf[0],"%d %d:%d:%d %d-%d-%d %d\r\n",i/16,timetmp.year,timetmp.month,\
                        timetmp.day,timetmp.hour,timetmp.minutes,timetmp.seconds,pdata[5]);
        }
        else
        {
            len=sprintf((char*)&buf[0],"%d%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",i/16,\
                        pdata[0],pdata[1],pdata[2],pdata[3],pdata[4],pdata[5],pdata[6],pdata[7],\
                        pdata[8],pdata[9],pdata[10],pdata[11],pdata[12],pdata[13],pdata[14],pdata[15]);
        }
        TaskPrintfFile(buf,len);
        V_FeedWdog();/*喂狗*/
        pdata+=16;
        i+=16;
    }


}






//void TestSleep(void)
//{
//  static uint8_t hour=20, min=50,flg=0;
//
////    while(1)
//  {
//          StuSleepFlash.rsv=min;
//      StuSleepFlash.wear_flg=~StuSleepFlash.wear_flg;
//          TaskSleep(hour,min,3);
//          min++;
//          if(min>=60)
//          {
//              min=0;
//              hour++;
//              if(hour>=24)
//              {
//                  hour=0;
//                  flg=1;
//              }
//              if( (hour==9)&&(1==flg))
//              {
//                  flg=0;
//                  return;
//              }
//
//          }
//  }

//}


//typedef struct
//{
//uint8_t s_hour;
//uint8_t s_min;
//uint8_t e_hour;
//uint8_t e_min;
//uint8_t deep_sleep_min;
//uint8_t light_sleep_min;
//uint8_t wake_min;
//uint8_t sleep_unit;
//uint8_t pdata[20*60/SLEEP_MINUTES/4];/*每两个位表示一个单元的状态*/
//}STU_SLEEP_DT;



void InitSleepPara(STU_SLEEP_DT *StuSleepDt)
{
    uint8_t i;
    StuSleepDt->s_hour=0xff;
    StuSleepDt->sleep_unit=0;
    StuSleepDt->deep_sleep_min=0;
    StuSleepDt->light_sleep_min=0;
    StuSleepDt->wake_min=0;
    for(i=0; i<(20*60/SLEEP_MINUTES/4); i++)
    {
        StuSleepDt->pdata[i]=0;
    }
}


bool IsDeepSleep(STU_SLEEP_FLASH *CurrentStuSleepDt,STU_SLEEP_FLASH*beforeStuSleepDt)
{

    if( ((CurrentStuSleepDt[0].big_times<=DEEP_SLEEP_THROSHORT)&&(CurrentStuSleepDt[0].wear_flg==WEAR))&&\
        ((CurrentStuSleepDt[1].big_times<=DEEP_SLEEP_THROSHORT)&&(CurrentStuSleepDt[1].wear_flg==WEAR))&&\
        ((CurrentStuSleepDt[2].big_times<=DEEP_SLEEP_THROSHORT)&&(CurrentStuSleepDt[2].wear_flg==WEAR))
      )
        return true;


    if( ((beforeStuSleepDt[0].big_times<=DEEP_SLEEP_THROSHORT)&&(beforeStuSleepDt[0].wear_flg==WEAR))&&\
        ((CurrentStuSleepDt[1].big_times<=DEEP_SLEEP_THROSHORT)&&(CurrentStuSleepDt[1].wear_flg==WEAR))&&\
        ((CurrentStuSleepDt[0].big_times<=DEEP_SLEEP_THROSHORT)&&(CurrentStuSleepDt[0].wear_flg==WEAR))
      )
        return true;

    if( ((beforeStuSleepDt[0].big_times<=DEEP_SLEEP_THROSHORT)&&(beforeStuSleepDt[0].wear_flg==WEAR))&&\
        ((beforeStuSleepDt[1].big_times<=DEEP_SLEEP_THROSHORT)&&(beforeStuSleepDt[1].wear_flg==WEAR))&&\
        ((CurrentStuSleepDt[0].big_times<=DEEP_SLEEP_THROSHORT)&&(CurrentStuSleepDt[0].wear_flg==WEAR))
      )
        return true;

    return false;
}

//判断睡眠开始
bool IsStartSleep(STU_SLEEP_FLASH*StuSleepFlash_tmp,STU_SLEEP_FLASH*StuSleepFlash_tmpb)
{
    uint8_t i=0,j=0;
    for(i=0; i<3; i++)
    {
        if((StuSleepFlash_tmp[i].wear_flg==WEAR)&&(StuSleepFlash_tmp[i].big_times<=DEEP_SLEEP_THROSHORT))
            j++;

        if(StuSleepFlash_tmp[i].big_times>=LIGHT_SLEEP_THROSHORT)
            j=0;
    }
    for(i=0; i<2; i++)
    {
        if((StuSleepFlash_tmpb[i].wear_flg==WEAR)&&(StuSleepFlash_tmpb[i].big_times<=DEEP_SLEEP_THROSHORT))
            j++;

        if(StuSleepFlash_tmpb[i].big_times>=LIGHT_SLEEP_THROSHORT)
            j=0;
    }

    if(j>2)
    {
        return true;
    }
    return false;
}

//获取睡眠的数据  end_min
bool ReadSleepDatas(uint8_t week,STU_SLEEP_DT *StuSleepDt)
{
    uint32_t addr,i,sleepi,time24,starti;
    uint16_t end_time_item,wear_tims=0,unwear_time_in_sleep;
    STU_SLEEP_FLASH StuSleepFlash_tmp[3];
    uint8_t state,byte,bit,err=0,errff=0;
    STU_SLEEP_FLASH StuSleepFlash_tmpb[2];

    InitSleepPara(StuSleepDt);
    sleepi=1000;


    StuSleepFlash_tmpb[0].wear_flg=UN_WEAR;
    StuSleepFlash_tmpb[1].wear_flg=UN_WEAR;
    addr= GetWriteAddr(start_min/60,start_min%60,week);
    end_time_item=(24*60-start_min+10*60)/SLEEP_MINUTES;
    time24=(24*60-start_min)/SLEEP_MINUTES;

    //超次数 或者没有数据的时候结束
    for(i=0; i<end_time_item; i++)
    {
        StuSleepFlash_tmp[0]=ReadSleepItem(addr+i*4);
        StuSleepFlash_tmp[1]=ReadSleepItem(addr+i*4+4);
        StuSleepFlash_tmp[2]=ReadSleepItem(addr+i*4+8);


        if((i<time24)&&(StuSleepFlash_tmp[0].big_times>=RE_START_THROSHORT_BEFORE12))
        {
            InitSleepPara(StuSleepDt);
        }


        //睡眠
        if(StuSleepFlash_tmp[0].wear_flg==WEAR)
        {

            wear_tims++;
            if(true==IsDeepSleep(StuSleepFlash_tmp,StuSleepFlash_tmpb))
            {
                state=DEEP_SLEEP;
                if(StuSleepDt->sleep_unit)
                    StuSleepDt->deep_sleep_min+=SLEEP_MINUTES;

            }
            else      if(StuSleepFlash_tmp[0].big_times<=LIGHT_SLEEP_THROSHORT)
            {
                if(StuSleepDt->sleep_unit)
                    StuSleepDt->light_sleep_min+=SLEEP_MINUTES;
                state=LIGHT_SLEEP;
            }
            else
            {
                state=AWAKE;

            }

        }
        else
        {
            state=empty_item;
//                   if(StuSleepDt->s_hour>=6)
            unwear_time_in_sleep++;
        }




        if(0==StuSleepDt->sleep_unit)
        {
            /*获取开始时间*/
            if(true==IsStartSleep(StuSleepFlash_tmp,StuSleepFlash_tmpb))
            {

                //逆推回去
                while(i!=0)
                {
                    StuSleepFlash_tmp[0]=ReadSleepItem(addr+i*4);
                    if( (StuSleepFlash_tmp[0].big_times>LIGHT_SLEEP_THROSHORT)||\
                        (StuSleepFlash_tmp[0].wear_flg!=WEAR)
                      )
                    {
                        break;
                    }
                    i--;
                }

                InitSleepPara(StuSleepDt);
                sleepi=i;
//                              sleepunit=StuSleepDt->sleep_unit;

                StuSleepDt->s_hour=(start_min+i*SLEEP_MINUTES)/60;
                StuSleepDt->s_min=(start_min+i*SLEEP_MINUTES)%60;
                if(StuSleepDt->s_hour>=24)
                {
                    StuSleepDt->s_hour-=24;
                }
                StuSleepDt->deep_sleep_min+=SLEEP_MINUTES;
                byte=StuSleepDt->sleep_unit/4;
                bit=StuSleepDt->sleep_unit%4;
                StuSleepDt->pdata[byte]|=(state<<(2*(bit)));
                StuSleepDt->sleep_unit++;
                starti=i;
                unwear_time_in_sleep=0;
            }
        }
        else
        {
            byte=StuSleepDt->sleep_unit/4;
            bit=StuSleepDt->sleep_unit%4;
            StuSleepDt->pdata[byte]|=(state<<(2*(bit)));
            StuSleepDt->sleep_unit++;


            if(StuSleepFlash.rsv!=0)
            {
                err++;
                if(err>5)
                {
                    StuSleepDt->sleep_unit=0;
                    return false;
                }
            }

            if(0xffff==StuSleepFlash_tmp[0].big_times)
            {
                errff++;
                if(errff>5)
                    break;
            }

        }



        if((DEEP_SLEEP==state)||(LIGHT_SLEEP==state))
        {
            sleepi=i;
        }


        StuSleepFlash_tmpb[1]=StuSleepFlash_tmpb[0];
        StuSleepFlash_tmpb[0]=StuSleepFlash_tmp[0];
    }

    /*佩戴时间小于4h   或者第一条睡眠是6点以后 或者睡眠时长小于2h,第一条睡眠后未佩戴大于2h*/
    if( (wear_tims<(240/SLEEP_MINUTES))||\
        ( (StuSleepDt->s_hour>=6)&&(StuSleepDt->s_hour<10))||\
        ((StuSleepDt->deep_sleep_min+StuSleepDt->light_sleep_min)<120)||\
        (unwear_time_in_sleep>(120/SLEEP_MINUTES))
      )
    {
        InitSleepPara(StuSleepDt);
    }


    //获取结束时间
    if(StuSleepDt->sleep_unit)
    {
//       i=sleepi+1;
        i=sleepi;
        if(StuSleepDt->light_sleep_min)
            StuSleepDt->light_sleep_min-=SLEEP_MINUTES;

        StuSleepDt->sleep_unit=(uint8_t)(i-starti);
        if(((i-starti)*SLEEP_MINUTES)>(StuSleepDt->light_sleep_min+StuSleepDt->deep_sleep_min))
            StuSleepDt->wake_min=(i-starti)*SLEEP_MINUTES-StuSleepDt->deep_sleep_min-StuSleepDt->light_sleep_min;
        StuSleepDt->e_hour=(start_min+i*SLEEP_MINUTES)/60;
        StuSleepDt->e_min=(start_min+i*SLEEP_MINUTES)%60;

        if(0==StuSleepDt->wake_min)
        {
            byte=StuSleepDt->sleep_unit/4;
            if((StuSleepDt->pdata[byte]&0xc0)==0)
            {
                StuSleepDt->pdata[byte]|=0x40;
            }

            if((StuSleepDt->pdata[byte]&0x30)==0)
            {
                StuSleepDt->pdata[byte]|=0x10;
            }

            if((StuSleepDt->pdata[byte]&0x03)==0)
            {
                StuSleepDt->pdata[byte]|=0x04;
            }
        }

        if(StuSleepDt->e_hour>=24)
        {
            StuSleepDt->e_hour-=24;
        }
        return true;
    }
    return false;

}

