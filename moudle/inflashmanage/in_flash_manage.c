#include "in_flash_manage.h"
#include "utility.h"
#include "SoftwareRTC.h"
#include "sleep.h"
#include "nrf_nvic.h"

uint16_t CalcCrc16(uint16_t crc,const uint8_t* pchMsg, uint32_t wDataLen)
{
    uint32_t i;
    uint8_t j;
    uint16_t c;
    for (i=0; i<wDataLen; i++)
    {
        c = *(pchMsg+i) & 0x00FF;
        crc^=c;
        for (j=0; j<8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    crc = (crc>>8) + (crc<<8);
    return(crc);
}


/*************************************************************************************************
* function name: void EnableAllGPIOEInt(bool enble_flg)
* function describe :
*
* author: liupeng
*
* version: 1.0
* return describe: enable or disbable gpiote irq
**************************************************************************************************/
void EnableAllGPIOEInt(bool enble_flg)
{
    if(true==enble_flg)
        sd_nvic_EnableIRQ((IRQn_Type) 6);
    else
        sd_nvic_DisableIRQ((IRQn_Type) 6);
}


bool u32equel(uint32_t *addr,uint32_t *Data_Buf,uint32_t len)
{

    uint32_t i;
    for(i=0; i<len; i++)
    {
        if(addr[i]!=Data_Buf[i])
            return false;
    }
    return true;
}

bool CheckAddr(uint32_t addr)
{
    if( ((addr>=APP_BVK_ADDR)&&(addr<=BOOT_ADDR))||\
            ((addr>=BOOT_PARA_ADDR)&&(addr<=(BOOT_PARA_ADDR+BOOT__PARA_SIZE)))
      )
    {
        return true;

    }
    return false;


}
/*************************************************************************************************
* function name: uint8_t Flash_Write_World(uint32_t *addr,uint32_t *Data_Buf,uint32_t len)
* function describe :  write data to inflash with softdevice
*
* author: liupeng
*
* version: 1.0
* return describe:
**************************************************************************************************/
bool Flash_Write_World(uint32_t *addr,uint32_t *Data_Buf,uint32_t len)
{
    uint8_t flash_write_flag;
    uint8_t CNT = 0;
    uint32_t delay_msv;
    delay_msv=1+len/16;
    volatile uint32_t Temp_Data = 0;

    if(true==CheckAddr((uint32_t)addr))
    {
        EnableAllGPIOEInt(false);
        do
        {
            flash_write_flag = sd_flash_write(addr,Data_Buf,len);
            CNT++;
            if((true==u32equel(addr,Data_Buf,len))&&(flash_write_flag == NRF_SUCCESS))
            {
                EnableAllGPIOEInt(true);
                delay_ms(2);
                return true;
            }
            delay_ms(delay_msv);
        } while(CNT < MAX_FLASH_WRITE_CNT);
        EnableAllGPIOEInt(true);
        delay_ms(2);
    }
    return false;
}


uint32_t Flash_ReadData(uint32_t *addr)
{

    return *addr;
}

/*************************************************************************************************
* function name: uint8_t Flash_Erase_Page(uint32_t Page_Num)
* function describe :  erase inflash with softdevice
*
* author: liupeng
*
* version: 1.0
* return describe:
**************************************************************************************************/
bool Flash_Erase_Page(uint32_t Page_Num)
{
    uint8_t flash_erase_flag;
    uint8_t cnt;
    uint32_t addr;
    cnt = 0;
    flash_erase_flag = 0xFF;
    addr=Page_Num*PAGE_SIZE;


    if(true==CheckAddr((uint32_t)addr))
    {
        EnableAllGPIOEInt(false);
        do
        {
            flash_erase_flag = sd_flash_page_erase(Page_Num);
            cnt++;
            if(flash_erase_flag == NRF_SUCCESS)
            {
                cnt = MAX_FLASH_ERASE_CNT;
                EnableAllGPIOEInt(true);
                delay_ms(2);
                return true;
            }
            delay_ms(2);
        } while(cnt < MAX_FLASH_ERASE_CNT);
        EnableAllGPIOEInt(true);
    }
    return false;
}






STU_EEPROM StuEeprom  __attribute__ ((at(EEPROM_START_RAM)));
#define POSITION_ERASE ((PAGE_SIZE/EEPROM_SIZE)-1)
static uint16_t WritePosition=POSITION_ERASE;
static uint16_t BvkWritePosition=POSITION_ERASE;



/*从内部flash读数据*/
void InflashRead(uint8_t *buf,uint32_t addr,uint16_t len)
{
    uint16_t i;
    const uint8_t *pdata;
    pdata=(uint8_t*)addr;
    for(i=0; i<len; i++)
        buf[i]=pdata[i];

}


/*查找上次写到的位置*/
bool EEpromFindPosition(uint32_t startaddr,uint16_t *position)
{
    uint8_t pdata[4];

    uint16_t times,i;
    times=POSITION_ERASE+1;
    for(i=0; i<times; i++)
    {
        InflashRead(pdata,startaddr+i*EEPROM_SIZE,4);
        if((0x55==pdata[1])&&(0xaa==pdata[0]))
        {
            position[0]=i;
            return true;
        }
    }
    return false;
}


/*计算数据的校验码是否正确*/
bool IsVarityOk(uint32_t startaddr,uint16_t position)
{
    uint8_t buf[EEPROM_SIZE];
    uint8_t varity;
    uint32_t j;
    varity=0;

    InflashRead(buf,startaddr+position*EEPROM_SIZE,EEPROM_SIZE);
    for(j=0; j<EEPROM_SIZE; j++)
    {
        varity^=buf[j];
    }

    if(0==varity)
        return true;
    else
        return false;
}
/*写数据到eeprom中*/
void EEpromWrite(bool bvk_flg)
{
    uint16_t i;
//uint8_t buf[20];
    uint32_t addr,tmp32;
    uint16_t* WritePositiontmp;
    uint8_t *pdes;
    pdes=(uint8_t*)&StuEeprom;
    WritePositiontmp=&WritePosition;
    addr=EEPROM_ADDR;
    if(true == bvk_flg)
    {
        addr=EEPROM_ADDR+PAGE_SIZE;
        WritePositiontmp=&BvkWritePosition;
    }

    if(WritePositiontmp[0]>=POSITION_ERASE)
    {
        Flash_Erase_Page(addr/PAGE_SIZE);
        WritePositiontmp[0]=0;
    } else
    {
//buf[0]=0;
//buf[1]=0;
//SPI_FLASH_WriteCont(buf,addr+((uint32_t)WritePositiontmp[0])*EEPROM_SIZE,2,false);
        tmp32=0;
        Flash_Write_World((uint32_t *)(((uint32_t)WritePositiontmp[0])*EEPROM_SIZE+addr),&tmp32,1);

        WritePositiontmp[0]++;
    }
    pdes[EEPROM_SIZE-1]=0;
    for(i=0; i<(EEPROM_SIZE-1); i++)
    {
        pdes[EEPROM_SIZE-1]^=pdes[i];
    }

    Flash_Write_World((uint32_t *)(((uint32_t)WritePositiontmp[0])*EEPROM_SIZE+addr),(uint32_t *)pdes,EEPROM_SIZE/4);

}


/*初始化eeprom*/
bool InitEEprom(void)
{
    uint8_t *pdes;


    bool findflg,bvkfindflg;
    pdes=(uint8_t*)&StuEeprom;



    /*step 1*/
    findflg=EEpromFindPosition(EEPROM_ADDR,&WritePosition);
    bvkfindflg=EEpromFindPosition(EEPROM_ADDR+PAGE_SIZE,&BvkWritePosition);

    /*step 2*/
    if(findflg==true)
        findflg=IsVarityOk(EEPROM_ADDR,WritePosition);
    if(bvkfindflg==true)
        findflg=IsVarityOk(EEPROM_ADDR+PAGE_SIZE,BvkWritePosition);

    /*step 3*/
    if((findflg==false)&&(bvkfindflg==false))
    {
        WritePosition=POSITION_ERASE;
        BvkWritePosition=POSITION_ERASE;
        EEpromSetParaToFactory();
        return false;

    } else if((findflg==false)&&(bvkfindflg==true))
    {

        InflashRead(pdes,EEPROM_ADDR+BvkWritePosition*EEPROM_SIZE+PAGE_SIZE,EEPROM_SIZE);
        WritePosition=POSITION_ERASE;
        EEpromWrite(false);
    } else if((findflg==true)&&(bvkfindflg==false))
    {
        InflashRead(pdes,EEPROM_ADDR+WritePosition*EEPROM_SIZE,EEPROM_SIZE);
        BvkWritePosition=POSITION_ERASE;
        EEpromWrite(true);
    } else
    {
        InflashRead(pdes,EEPROM_ADDR+WritePosition*EEPROM_SIZE,EEPROM_SIZE);
    }

    return true;
}



/*同步数据到eeprom*/
void EepromAnsy(void)
{
    EEpromWrite(false);
    EEpromWrite(true);
}


/*数据恢复出厂设置*/
void EEpromSetParaToFactory(void)
{
    uint16_t i;
    uint8_t *pdes;
    pdes=(uint8_t*)&StuEeprom;
//memset(pdes,0,EEPROM_SIZE);
    for(i=0; i<EEPROM_SIZE; i++)
    {
        pdes[i]=0;
    }
    pdes[0]=0xaa;
    pdes[1]=0x55;

    time.year=2016;
    time.month=9;
    time.day=1;
    time.hour=12;
    time.minutes=0;
    time.seconds=0;
    StuEeprom.StuPara.time=convert_time_to_Second(time);

    StuEeprom.StuPara.step_goal=10000;

    StuEeprom.StuPara.weight=60;
    StuEeprom.StuPara.hight=170;
    StuEeprom.StuPara.age=25;
    StuEeprom.StuPara.female=1;
    StuEeprom.StuPara.stride=75;
    StuEeprom.StuPara.oled_auto_on=OLED_AUTO_ON;
    StuEeprom.StuPara.sleep_start_min=21*60;
    StuEeprom.StuPara.sleep_end_min=7*60;

    StuEeprom.StuPara.oled_sava_last_ui=OLED_SAVE_LAST_UI;
    StuEeprom.StuPara.menu_enable_index=0xffffffff;

    StuEeprom.StuPara.sedentary_min=45;
    StuEeprom.StuPara.sedentary_big_times=(25*10);


    StuEeprom.StuPara.StuSendentaryRemain[0].on_flg=0;
    StuEeprom.StuPara.StuSendentaryRemain[0].work_day=0x7f;
    StuEeprom.StuPara.StuSendentaryRemain[0].s_hour=8;
    StuEeprom.StuPara.StuSendentaryRemain[0].s_min=0;
    StuEeprom.StuPara.StuSendentaryRemain[0].e_hour=12;
    StuEeprom.StuPara.StuSendentaryRemain[0].e_min=0;

    StuEeprom.StuPara.StuSendentaryRemain[1].on_flg=0;
    StuEeprom.StuPara.StuSendentaryRemain[1].work_day=0x7f;
    StuEeprom.StuPara.StuSendentaryRemain[1].s_hour=14;
    StuEeprom.StuPara.StuSendentaryRemain[1].s_min=0;
    StuEeprom.StuPara.StuSendentaryRemain[1].e_hour=18;
    StuEeprom.StuPara.StuSendentaryRemain[1].e_min=0;

    StuEeprom.StuPara.weight=60;
    StuEeprom.StuPara.hight=170;
    StuEeprom.StuPara.age=25;
    StuEeprom.StuPara.female=1;
    StuEeprom.StuPara.stride=75;
    StuEeprom.StuPara.oled_auto_on=OLED_AUTO_ON;
    StuEeprom.StuPara.sleep_start_min=21*60;
    StuEeprom.StuPara.sleep_end_min=7*60;

    StuEeprom.StuPara.oled_sava_last_ui=OLED_SAVE_LAST_UI;
    StuEeprom.StuPara.menu_enable_index=0xffffffff;

    StuEeprom.StuPara.sedentary_min=45;
    StuEeprom.StuPara.sedentary_big_times=(25*10);


    StuEeprom.StuPara.StuSendentaryRemain[0].on_flg=0;
    StuEeprom.StuPara.StuSendentaryRemain[0].work_day=0x7f;
    StuEeprom.StuPara.StuSendentaryRemain[0].s_hour=8;
    StuEeprom.StuPara.StuSendentaryRemain[0].s_min=0;
    StuEeprom.StuPara.StuSendentaryRemain[0].e_hour=12;
    StuEeprom.StuPara.StuSendentaryRemain[0].e_min=0;

    StuEeprom.StuPara.StuSendentaryRemain[1].on_flg=0;
    StuEeprom.StuPara.StuSendentaryRemain[1].work_day=0x7f;
    StuEeprom.StuPara.StuSendentaryRemain[1].s_hour=14;
    StuEeprom.StuPara.StuSendentaryRemain[1].s_min=0;
    StuEeprom.StuPara.StuSendentaryRemain[1].e_hour=18;
    StuEeprom.StuPara.StuSendentaryRemain[1].e_min=0;

    EepromAnsy();


}





/*读取记录的条数*/
uint16_t read_history_items(uint32_t time_sec)
{
    uint16_t item=1,i;
    for(i=0; i<HISTORY_DATA_ITEM; i++)
    {
        if(StuEeprom.StuPara.StuHistory[i].time>=time_sec)
        {
            item++;
        }
    }
    return item;
}










/*每天需要写一次的历史数据*/
/*参数是要写入的日期*/
void write_para_history(uint16_t year,uint8_t month ,uint8_t day)
{
    uint16_t i;
    uint32_t time_in_sec;
    RTC_UTCTimeStruct timetmpin;
    RTC_UTCTimeStruct timetmp[HISTORY_DATA_ITEM];

    /*把入口的时间转化为 sec*/
    timetmpin.year=year;
    timetmpin.month=month;
    timetmpin.day=day;
    timetmpin.hour=0;
    timetmpin.minutes=0;
    timetmpin.seconds=0;
    time_in_sec=convert_time_to_Second(timetmpin);

    /*把记录的时间转化为年月日*/
    for(i=0; i<HISTORY_DATA_ITEM; i++)
    {
        if(StuEeprom.StuPara.StuHistory[i].time==0)
        {
            timetmp[i].year=0;
            timetmp[i].month=0;
            timetmp[i].day=0;
        } else
        {
            ConvertToUTCTime(&timetmp[i],StuEeprom.StuPara.StuHistory[i].time);
        }
    }

    /*查看日期是否有重复的，有重复的替代重复的*/
    for(i=0; i<HISTORY_DATA_ITEM; i++)
    {
        if(StuEeprom.StuPara.StuHistory[i].time==time_in_sec)
        {
            StuEeprom.StuPara.StuHistory[i].time=time_in_sec;
            StuEeprom.StuPara.StuHistory[i].step=StuEeprom.StuPara.step;
            StuEeprom.StuPara.StuHistory[i].step_min=StuEeprom.StuPara.step_sec/60;
            StuEeprom.StuPara.StuHistory[i].distance=StuEeprom.StuPara.distance_cm/10000;
            StuEeprom.StuPara.StuHistory[i].calories=StuEeprom.StuPara.calories;

            return;
        }

    }


    /*没有重复的，看是否有空闲的，有空闲的就写如空闲的位置*/
    for(i=0; i<HISTORY_DATA_ITEM; i++)
    {
        if(StuEeprom.StuPara.StuHistory[i].time==0)
        {
            StuEeprom.StuPara.StuHistory[i].time=time_in_sec;
            StuEeprom.StuPara.StuHistory[i].step=StuEeprom.StuPara.step;
            StuEeprom.StuPara.StuHistory[i].step_min=StuEeprom.StuPara.step_sec/60;
            StuEeprom.StuPara.StuHistory[i].distance=StuEeprom.StuPara.distance_cm/10000;
            StuEeprom.StuPara.StuHistory[i].calories=StuEeprom.StuPara.calories;
            return;
        }

    }

    /*如果没有空闲的，先左移动，然后写入最后一个位置*/
    for(i=0; i<(HISTORY_DATA_ITEM-1); i++)
    {
        StuEeprom.StuPara.StuHistory[i]=StuEeprom.StuPara.StuHistory[i+1];
    }
    StuEeprom.StuPara.StuHistory[i].time=time_in_sec;
    StuEeprom.StuPara.StuHistory[i].step=StuEeprom.StuPara.step;
    StuEeprom.StuPara.StuHistory[i].step_min=StuEeprom.StuPara.step_sec/60;
    StuEeprom.StuPara.StuHistory[i].distance=StuEeprom.StuPara.distance_cm/10000;
    StuEeprom.StuPara.StuHistory[i].calories=StuEeprom.StuPara.calories;




}


#define PROX_MAX  2000  //2000
//读写接近阈值
uint32_t ReadProx(void)
{
    uint32_t prox_value;

#if(BORD&USE7028)
    prox_value=PROX_MAX;
#else
    prox_value=PROX_MAX;
#endif
    return prox_value;
}









