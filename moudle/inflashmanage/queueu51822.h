#ifndef __QUEUE51822_H__
#define __QUEUE51822_H__

#include "in_flash_manage.h"
#include "utility.h"
#include "string.h"

//#define PAGE_SIZE 1024



#define FLASH_ADDR_RECORD_PARAM 		   FLASH_QUEUE_ADDR//(122*1024)
#define FLASH_ADDR_RECORD_PARAM_ZIZE 	 (1*PAGE_SIZE)
#define FLASH_ADDR_RECORD_PARAM_LEN 	 8
#define FLASH_PARAM_MAX 				       (FLASH_ADDR_RECORD_PARAM_ZIZE/FLASH_ADDR_RECORD_PARAM_LEN)


#define FLASH_ADDR_RECORD_DATA		 (FLASH_ADDR_RECORD_PARAM+FLASH_ADDR_RECORD_PARAM_ZIZE)
#define FLASH_RECORD_MAX_SIZE 		  (FLASH_QUEUE_SIZE-FLASH_ADDR_RECORD_PARAM_ZIZE)//(24*PAGE_SIZE)
#define RECORD_UNIT_LEN 16/*4的倍数*/
#define FLASH_RECORD_MAX_CNT 		(FLASH_RECORD_MAX_SIZE/RECORD_UNIT_LEN)
#define FLASH_RECORD_CNT_PER_PAGE 	(PAGE_SIZE/RECORD_UNIT_LEN)

typedef struct
{
    unsigned short cnt;
    unsigned short tail_index;
    unsigned short head_index;
    unsigned short param_addr_index;
} FLASH_QUEUE;

void FlashQueue_SaveParam(FLASH_QUEUE *p_FlashQueue);
unsigned char FlashQueue_GetParam(FLASH_QUEUE *p_FlashQueue);
unsigned char FlashQueue_IsFull(FLASH_QUEUE *p_FlashQueue);
unsigned char FlashQueue_IsEmpty(FLASH_QUEUE *p_FlashQueue);
unsigned char FlashQueue_EnQueue(FLASH_QUEUE *p_FlashQueue,unsigned char *dat);
unsigned char FlashQueue_GetTail(FLASH_QUEUE *p_FlashQueue,unsigned char *dat);
unsigned char FlashQueue_DelTail(FLASH_QUEUE *p_FlashQueue);
unsigned char FlashQueue_MultiDel(FLASH_QUEUE *p_FlashQueue,unsigned short del_cnt);
unsigned char FlashQueue_DeQueue(FLASH_QUEUE *p_FlashQueue,unsigned char *dat);
void FlashQueue_CheckHead(unsigned int New_Data_Adrr,FLASH_QUEUE* queue);





//初始化队列，开机调用一次
void InitQueue(void);
//写数据到队列中
void WriteDataToQueue(uint8_t*datain,uint16_t write_cnt);
//从队列中读
uint32_t  ReadDaraFromQueue(uint8_t*dataout,uint16_t max_cnt);
//从队列中删除队列数，删除成功删除的队列数
uint16_t DeleteDaraFromQueue(uint16_t delete_cnt);

//删除队列
void DeleteQueue(void);


#endif







