#include "pedometer_klx.h"


STU_G_SENSOR StuGSendor;
ENUM_MOTION EnumMotion=motion_static;

static void Filter(uint16_t  * dataPtr, uint16_t length, uint16_t * outPtr)
{
    uint8_t  i;
    uint32_t temp;

    temp = 0;
    for (i=length-1;; i--)
    {
        temp += (uint32_t)dataPtr[i];
        if (i!=0)
        {
            dataPtr[i] = dataPtr[i-1];
        }
        else break;
    }
    *outPtr = (uint16_t) (temp / length);
}



/*********************************
256--1g     10---0.2s
**********************************/
void InitCntStep(uint16_t amp_thres, uint8_t interval_thres)/*20  ,10*/
{
    uint8_t  i;
    StuGSendor.amp_thres= amp_thres;
    StuGSendor.interval_thres= interval_thres;
    for (i=0; i<FILTER_POINT; i++) StuGSendor.buffer[i] = 0;
}




void InitStep(void)
{
    InitCntStep(300,3);/**/
    EnumMotion=motion_static;
}

#define FINDING_THROUGH 0
#define FINDING_PEAK   (FINDING_THROUGH+1)
static uint8_t sta_step=FINDING_THROUGH;

uint16_t step_counter(uint16_t g_sensorin,uint8_t*err)
{

    uint16_t  presentValue=0xaaaa,amp;
    uint8_t step=0,escape;
    static uint16_t trougt_value;
    static uint16_t buf[3];

    static uint32_t cnt=0,last_step_cnt=0;
    StuGSendor.buffer[0]=g_sensorin;
    Filter(StuGSendor.buffer, FILTER_POINT, &presentValue);
    if(presentValue==buf[0]) return 0;
    buf[2] = buf[1];
    buf[1] = buf[0];
    buf[0]=presentValue;
    cnt++;
    switch(sta_step)
    {
    case FINDING_THROUGH:
        if((buf[1]<buf[2])&&(buf[1]<buf[0]))
        {
            sta_step=FINDING_PEAK;
            trougt_value=buf[1];
            step=5;
        }
        break;
    case FINDING_PEAK:

        if(buf[0]<trougt_value)
        {
            trougt_value=buf[1];
            step=5;
        }
        if( (buf[1]>buf[2])&&(buf[1]>buf[0]))
        {
            amp=buf[1]-trougt_value;
            if(amp>10000)
            {
                sta_step=FINDING_THROUGH;
                break;
            }
            escape=cnt-last_step_cnt;

            if((amp>StuGSendor.amp_thres)&&(escape>StuGSendor.interval_thres))
            {
                last_step_cnt=cnt;
                sta_step=FINDING_THROUGH;
                step=10;
                err[0]=0;
            } else
            {
                err[0]++;
            }
        }
        break;

    }
    step/=10;
    return step;
}


uint8_t SportsProcess(uint8_t get_data_item,uint32_t *AccSensorData)
{
    uint16_t steps=0;
    uint8_t i;
    uint8_t returnstep=0;
    static uint8_t sta=0,err=0;
    static uint8_t fifo_step=0;
    static uint8_t err_cnt=0;
    for(i=0; i<get_data_item; i++)
    {

        steps=step_counter(AccSensorData[i]/10,&err);
        if(steps)
        {
            err_cnt=0;
            fifo_step+=steps;
        } else
        {
            err_cnt++;
        }

        switch(sta)
        {
        case 0://开始记步
            if((err_cnt>=ERROR_CNTS)||(err>=IN_STEP_ERROR_CNTS))
            {
                fifo_step=0;
            }
            if(fifo_step>=BEGIN_STEP)
            {
                EnumMotion=motion_other;
                returnstep+=fifo_step;
                fifo_step=0;
                err=0;
                sta++;
            }
            break;
        case 1:
            if((err_cnt>=ERROR_CNTS)||(err>=IN_STEP_ERROR_CNTS))
            {
                fifo_step=0;
                returnstep=0;
                sta=0;
                sta_step=FINDING_THROUGH;
                EnumMotion=motion_static;
            } else
            {
                returnstep+=steps;
            }
            break;
        }

    }

    return returnstep;
}









