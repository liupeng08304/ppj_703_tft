#ifndef SLEEP_H___
#define SLEEP_H___


#include "stdint.h"
#include "stdbool.h"
#include "bsp.h"
#include"bsp.h"
#include "in_flash_manage.h"

#define UN_WEAR  0
#define WEAR     1

#pragma pack(1)
typedef struct
{
		uint32_t big_times:16;
		uint32_t wear_flg:1;
		uint32_t rsv:1;
		uint32_t tmp:14;
} STU_SLEEP_FLASH;
#pragma pack()


extern STU_SLEEP_FLASH StuSleepFlash;

typedef bool (*SleepWrite_fun)(uint32_t *addr,uint32_t *Data_Buf,uint32_t len); // Flash_Write_World
typedef bool (*SleepErase_fun)(uint32_t Page_Num); // Flash_Erase_Page
typedef uint32_t (*SleepRead_fun)(uint32_t *addr); // Flash_ReadData


#define RE_START_THROSHORT_BEFORE12  500

#define MOVE_THROTE  100

#define UN_WEAR_MIN   					120
#define UN_WEAR_MIN_BEFORE12   	40



#define SLEEP_FLASH_SEC_SIZE  PAGE_SIZE
#define SLEEP_MINUTES   5  				/*˯���жϵ�ʱ����*/



#define WAKE_THROSHORT   (500)//(400)
#define LIGHT_SLEEP_THROSHORT (500)//(300)
#define DEEP_SLEEP_THROSHORT	(3)//(10) 3


/*
��ʼ��˯��
����˵����
start_time_min ��21:25  --->21*60+25
end_time_min ��7:30   	--->7*60+30
*/
void SleepInit(uint16_t start_time_min,uint16_t end_time_min,\
               uint8_t*pstatic_min,\
               SleepWrite_fun SleepWrite,\
               SleepErase_fun SleepErase,\
               SleepRead_fun SleepRead);




/*��ʱ1min����                              week:0--6*/
void TaskSleep(uint8_t hour,uint8_t min,uint8_t week,uint8_t wearflg);

/*gsensor���� */
void CallByGsensor(int16_t x,int16_t y,int16_t z);



#define AWAKE  				0x00
#define LIGHT_SLEEP 	0x01
#define DEEP_SLEEP 		0x02
#define empty_item 		0x03
typedef struct
{
    uint8_t s_hour;
    uint8_t s_min;
    uint8_t e_hour;
    uint8_t e_min;
    uint16_t deep_sleep_min;
    uint16_t light_sleep_min;
    uint16_t wake_min;
    uint8_t sleep_unit;
    uint8_t pdata[20*60/SLEEP_MINUTES/4];/*ÿ����λ��ʾһ����Ԫ��״̬*/
} STU_SLEEP_DT;


//��ȡ˯�ߵ�����
bool ReadSleepDatas(uint8_t week,STU_SLEEP_DT *StuSleepDt);

void erase_sleep_flash(void);

void TestPrintfSleep(void);

extern  uint16_t big_times;//�˶��ͻ��Լ�,

void TestPrintfaddr(uint32_t addrin,uint16_t size);
#endif


