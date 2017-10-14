#include "queueu51822.h"
#include "data_use_api.h"
FLASH_QUEUE stFlashQueue;

unsigned char FlashQueue_EnQueue(FLASH_QUEUE *p_FlashQueue,unsigned char *dat)
{
    uint32_t addr = 0;
    uint32_t *buf = NULL;
    addr = ((p_FlashQueue->head_index) *RECORD_UNIT_LEN)+ FLASH_ADDR_RECORD_DATA;
    FlashQueue_CheckHead(addr,p_FlashQueue);
    buf = (uint32_t *)dat;
    Flash_Write_World((uint32_t *)(addr),buf,RECORD_UNIT_LEN/4);
    p_FlashQueue->head_index++;
    if(p_FlashQueue->head_index >= FLASH_RECORD_MAX_CNT)
    {
        p_FlashQueue->head_index = 0;
    }
    p_FlashQueue->cnt++;
    FlashQueue_SaveParam(p_FlashQueue);
    return 0;
}


unsigned char FlashQueue_IsEmpty(FLASH_QUEUE *p_FlashQueue)
{
    if(p_FlashQueue->cnt == 0)
        return 1;
    else
        return 0;
}
unsigned char FlashQueue_GetTail(FLASH_QUEUE *p_FlashQueue,unsigned char *dat)
{
    uint8_t i;
    uint8_t *p;
    if(FlashQueue_IsEmpty(p_FlashQueue))
    {
        return 1;
    }
    else
    {
        p = (uint8_t*)(p_FlashQueue->tail_index*RECORD_UNIT_LEN+FLASH_ADDR_RECORD_DATA);
        for(i = 0; i < RECORD_UNIT_LEN; i++)
        {
            dat[i] = p[i];
        }
        return 0;
    }
}
void FlashQueue_CheckHead(uint32_t New_Data_Adrr,FLASH_QUEUE* queue)
{
    uint16_t Head_Page_Offset = 0;
    uint16_t Tail_Page_Offset = 0;
    Tail_Page_Offset = (queue->tail_index) / FLASH_RECORD_CNT_PER_PAGE;
    Head_Page_Offset = (New_Data_Adrr - FLASH_ADDR_RECORD_DATA)/PAGE_SIZE;
    if((New_Data_Adrr%PAGE_SIZE) == 0)
    {
        if(Head_Page_Offset == Tail_Page_Offset) //??????????
        {
            if((queue->tail_index == queue->head_index)&&(queue->cnt == 0))
            { }
            else
            {
                queue->tail_index = queue->head_index + FLASH_RECORD_CNT_PER_PAGE;
                queue->cnt = FLASH_RECORD_MAX_CNT - FLASH_RECORD_CNT_PER_PAGE;
                if(queue->tail_index >= FLASH_RECORD_MAX_CNT)
                {
                    queue->tail_index = 0;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                }
            }
        }
        Flash_Erase_Page(New_Data_Adrr/PAGE_SIZE);
        delay_ms(50);
    }
}
uint16_t Flash_FindLastData(void)
{
    uint16_t i;
    uint16_t *dat;
    for(i = 0; i <FLASH_PARAM_MAX; i++)
    {
        dat = (uint16_t *)(FLASH_ADDR_RECORD_PARAM+
                           i*FLASH_ADDR_RECORD_PARAM_LEN +FLASH_ADDR_RECORD_PARAM_LEN-2);
        if(*dat == 0xFFFF)
        {
            break;
        }
        else if(*dat == 0x55AA)
        { }
        else
        {
            i = 0xFFFF;
            break;
        }
    }
    return i;
}


uint8_t FlashQueue_GetParam(FLASH_QUEUE *p_FlashQueue)
{
    uint16_t i;
// uint16_t buf[4];
    uint32_t addr;
    i = Flash_FindLastData();
    if((i == 0xFFFF)||(i == 0)||(p_FlashQueue->cnt > FLASH_RECORD_MAX_CNT)) //?? 4????,?????
    {
        Flash_Erase_Page(FLASH_ADDR_RECORD_PARAM/PAGE_SIZE);
        delay_ms(40);
        p_FlashQueue->param_addr_index = 0;
        p_FlashQueue->cnt = 0;
        p_FlashQueue->head_index = 0;
        p_FlashQueue->tail_index = 0;
        return 1;
    }
    p_FlashQueue->param_addr_index = i;
    addr = FLASH_ADDR_RECORD_PARAM + (i-1)*FLASH_ADDR_RECORD_PARAM_LEN;
    p_FlashQueue->cnt = (*(uint16_t *)addr);
    p_FlashQueue->tail_index= (*(uint16_t *)(addr+2));
    p_FlashQueue->head_index= (*(uint16_t *)(addr+4));
    if(p_FlashQueue->head_index == (p_FlashQueue->tail_index +
                                    p_FlashQueue->cnt)%FLASH_RECORD_MAX_CNT)
    {
        return 0;
    }
    else
    {
        Flash_Erase_Page(FLASH_ADDR_RECORD_PARAM/PAGE_SIZE);
        delay_ms(40);
        p_FlashQueue->param_addr_index = 0;
        p_FlashQueue->cnt = 0;
        p_FlashQueue->head_index = 0;
        p_FlashQueue->tail_index = 0;
        return 1;
    }
}
unsigned char FlashQueue_DelTail(FLASH_QUEUE *p_FlashQueue)
{
    if((p_FlashQueue->cnt == 0)&&(p_FlashQueue->head_index == p_FlashQueue->tail_index))
    {
        return 1;
    }
    else
    {
        p_FlashQueue->tail_index ++;
        p_FlashQueue->cnt--;
        if(p_FlashQueue->tail_index >= FLASH_RECORD_MAX_CNT)
        {
            p_FlashQueue->tail_index = 0;
        }
    }
    FlashQueue_SaveParam(p_FlashQueue);
    return 0;
}

void FlashQueue_SaveParam(FLASH_QUEUE *p_FlashQueue)
{
    uint32_t addr;
    static uint8_t buf[FLASH_ADDR_RECORD_PARAM_LEN];
    static uint32_t buf32[FLASH_ADDR_RECORD_PARAM_LEN/4];
    if(p_FlashQueue->param_addr_index >= FLASH_PARAM_MAX)
    {
        Flash_Erase_Page(FLASH_ADDR_RECORD_PARAM/PAGE_SIZE);
        delay_ms(40);
        p_FlashQueue->param_addr_index = 0;
    }
    addr = FLASH_ADDR_RECORD_PARAM + (p_FlashQueue->param_addr_index)*FLASH_ADDR_RECORD_PARAM_LEN; //??????

    buf[0] = (p_FlashQueue->cnt);
    buf[1] = (p_FlashQueue->cnt)>>8;
    buf[2] = (p_FlashQueue->tail_index);
    buf[3] = (p_FlashQueue->tail_index)>>8;
    buf[4] = (p_FlashQueue->head_index);
    buf[5] = (p_FlashQueue->head_index)>>8;
    buf[FLASH_ADDR_RECORD_PARAM_LEN-2] = 0xAA;
    buf[FLASH_ADDR_RECORD_PARAM_LEN-1] = 0x55;
    memcpy((uint8_t*)buf32,buf,FLASH_ADDR_RECORD_PARAM_LEN);
    Flash_Write_World((uint32_t *)addr,(uint32_t *)buf32,FLASH_ADDR_RECORD_PARAM_LEN/4);
    p_FlashQueue->param_addr_index++;
}


void WriteDataToQueue(uint8_t*datain,uint16_t write_cnt)
{
    static  uint32_t buf[RECORD_UNIT_LEN/4];
    uint16_t i;
    for(i=0; i<write_cnt; i++)
    {
        memcpy((uint8_t*)buf,datain+RECORD_UNIT_LEN*i,RECORD_UNIT_LEN);
        FlashQueue_EnQueue(&stFlashQueue,(uint8_t*)buf);//write one data
    }
}

void DeleteQueue(void)
{

    Flash_Erase_Page(FLASH_ADDR_RECORD_PARAM/PAGE_SIZE);
    delay_ms(40);
    stFlashQueue.param_addr_index = 0;
    stFlashQueue.cnt = 0;
    stFlashQueue.head_index = 0;
    stFlashQueue.tail_index = 0;
}
void InitQueue(void)
{
    stFlashQueue.cnt=0;
    FlashQueue_GetParam(&stFlashQueue);
}

//�Ӷ����ж�
uint32_t  ReadDaraFromQueue(uint8_t*dataout,uint16_t max_cnt)
{
    uint8_t *pdataout;
    FLASH_QUEUE flash_queue_temp;
    uint32_t cnt=0,i;
    memcpy(&flash_queue_temp,&stFlashQueue,sizeof(stFlashQueue));
    cnt=0;
    pdataout=dataout;
    for(i=0; i<max_cnt; i++)
    {

        if(0==FlashQueue_GetTail(&flash_queue_temp,pdataout))//read one data
        {
            flash_queue_temp.cnt--;
            flash_queue_temp.tail_index++;
            if(flash_queue_temp.tail_index >= FLASH_RECORD_MAX_CNT)
                flash_queue_temp.tail_index = 0;
            pdataout+=RECORD_UNIT_LEN;
            cnt++;
        }
        else
        {
            return cnt;
        }
    }
    return cnt;


}




//�Ӷ�����ɾ����������ɾ���ɹ�ɾ���Ķ�����
uint16_t DeleteDaraFromQueue(uint16_t delete_cnt)
{
    uint16_t i,cnt=0;
    for(i=0; i<delete_cnt; i++)
    {
        if(FlashQueue_IsEmpty(&stFlashQueue))//no data
            return cnt;
        FlashQueue_DelTail(&stFlashQueue);//delete one data
        cnt++;
    }

    return cnt;
}



//��ȡ�û�����
uint32_t  usr_api_read(uint32_t cnt_start,uint8_t type,uint32_t sec_start ,uint8_t *pdata)

{
    uint8_t pdataout[RECORD_UNIT_LEN];
    FLASH_QUEUE flash_queue_temp;
    uint32_t cnt=0,i;
    uint8_t type_t;
    uint32_t sec_t;

    memcpy(&flash_queue_temp,&stFlashQueue,sizeof(stFlashQueue));
    cnt=0;

//      flash_queue_temp.tail_index+=cnt_start;
//      flash_queue_temp.cnt-=cnt_start;
    for(i=0; i<FLASH_RECORD_MAX_CNT; i++)
    {
        if(0==FlashQueue_GetTail(&flash_queue_temp,pdataout))//read one data
        {
            flash_queue_temp.cnt--;
            flash_queue_temp.tail_index++;
            if(flash_queue_temp.tail_index >=FLASH_RECORD_MAX_CNT)
            {
                flash_queue_temp.tail_index = 0;
            }

            type_t=pdataout[0];
            sec_t=pdataout[1];
            sec_t<<=8;
            sec_t|=pdataout[2];
            sec_t<<=8;
            sec_t|=pdataout[3];
            sec_t<<=8;
            sec_t|=pdataout[4];

            if( (sec_t>sec_start)   &&((sec_t>SEC_2017)&&(sec_t<SEC_2099)&&(type==type_t)))
            {
                cnt++;
                if((pdata)&&(cnt>cnt_start))
                {
                    data_use_packet_data(type_t,sec_t,&pdataout[5],pdata,cnt-cnt_start);
                }
            }
        }
        else
        {
            return cnt;
        }
    }
    return cnt;
}



