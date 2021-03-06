#ifndef INFLASH_MANAGE_H__
#define INFLASH_MANAGE_H__

#include "stdint.h"
#include "nrf52.h"
#include "nrf52_bitfields.h"
#include "nrf_soc.h"
#include "bsp.h"

#define MAX_FLASH_WRITE_CNT  100
#define MAX_FLASH_ERASE_CNT  100

/*===========================================================================================
flash:
start		end		size		left		item
0			140k		140k		372k		softdevice
140k			280k		140k		232		application
280k			420k		140k		92k		bvk
420k			448k		28k		64k		sleep
448k			468k		20k		44k		hr
468k			476k		8k		36k		eeprom(para+7days)
;;;32k

508k			512k		4k		0k	    bootloader
//=========================================================================================
*/

#define PAGE_SIZE  4096UL

#define APP_ADDR (140*1024)
#define APP_SIZE (140*1024)


#define APP_BVK_ADDR (APP_ADDR+APP_SIZE)
#define APP_BVK_SIZE  (140*1024)

#define SLEEP_BEGIN_ADDR  (APP_BVK_ADDR+APP_BVK_SIZE)
#define SLEEP_ADDR_SIZE		(28*1024)


#define FLASH_QUEUE_ADDR  	 (SLEEP_BEGIN_ADDR+SLEEP_ADDR_SIZE)    //心率数据开始地址
#define FLASH_QUEUE_SIZE			(20*1024)


#define EEPROM_ADDR  (FLASH_QUEUE_ADDR+FLASH_QUEUE_SIZE)
#define EEPROM_ADDR_SIZE   0x00000800


#define BOOT_ADDR ((512-4)*1024)
#define BOOT_SIZE (4*1024)



#define BOOT_PARA_ADDR (0x10001014)
#define BOOT__PARA_SIZE  (16)

#define BOOT_PARA_CUSTOM_ADDR (0x10001080)




#define UNIT_KM  0
#define UNIT_MILE (1+UNIT_KM)

#define TIME_24HOUTR  0
#define TIME_12HOUR   (1+TIME_24HOUTR)







/*eeprom 数据*/
#define EEPROM_SIZE  512ul
#define EEPROM_START_RAM (0x20008000-EEPROM_SIZE)


#define ANCS_MSG  			0x00000001
#define ANCS_PHONE 			0x00000002
#define ANCS_WEICHAT  		0x00000004
#define ANCS_QQ 			0x00000008
#define ANCS_WHATSAPP		0x00000010
#define ANCS_FACEBOOK		0x00000020
#define ANCS_TWITTER		0x00000040
#define ANCS_SKYPE			0x00000080
#define ANCS_SNAPCHAT		0x00000100
#define MAX_ALARM   8

#pragma pack(1)

typedef struct
{
    uint32_t time;
    uint32_t step;
    uint16_t step_min;/*min 2byte*/
    uint16_t distance;/*0.1km   m/10  2byte*/
    uint16_t calories;/**/
} STU_HISTORY;

#define OLED_AUTO_ON  0x00
#define OLED_AUTO_OFF  (1+OLED_AUTO_ON)

#define OLED_UN_SAVE_LAST_UI  0x00
#define OLED_SAVE_LAST_UI  (1+OLED_UN_SAVE_LAST_UI)
typedef struct
{
    uint16_t type:4;
    uint16_t on_flg:1;
    uint16_t hour:5;
    uint16_t min:6;
    uint8_t work_day;   /*0到6位对应 星期一到星期日*/
} STU_ALARM;


typedef struct
{
    uint32_t s_hour:5;
    uint32_t s_min:6;
    uint32_t e_hour:5;
    uint32_t e_min:6;
    uint32_t work_day:7;   /*0到6位对应 星期一到星期日*/
    uint32_t on_flg:1;
    uint32_t ver:2;
} STU_SEDENTARY_REAMND;



typedef struct
{
    uint32_t time;
    uint32_t step;
    uint32_t step_sec;/**/
    uint32_t distance_cm;/**/
    uint16_t calories;/**/
    uint8_t ask_power_off;/*为真表示手机设置关机*/
    uint8_t updata_affirm;/*升级成功后，按键确认*/
    uint8_t ask_reset;/*主动重启，不要开机界面*/
    uint32_t reset_time;
    uint32_t ResetReson;
    uint8_t ask_version;/*请求开机一直显示版本一直到有按键按下 1外部版本 2内部版本60s关机  3外部版本60s关机
	4内部版本*/
    uint8_t ask_dfu;/*请求进入DFU升级*/
    uint8_t distance_unit:2;
    uint8_t time_format:2;
    uint8_t hr_inter:2;/*0 1 2 3 */
    uint8_t resev:2;
    uint8_t static_min;


    uint8_t weight;/*30--150kg*/
    uint8_t hight;/*100ms--200ms*/
    uint8_t age;/*5--99*/
    uint8_t female;/*female 1  male--0*/
    uint8_t stride;/*30--150cm*/
    STU_HISTORY StuHistory[HISTORY_DATA_ITEM];
    STU_ALARM 	StuAlarm[MAX_ALARM+2];
    uint8_t oled_auto_on:1 ;
    uint8_t oled_sava_last_ui:1;
    uint8_t rsv:6;
    uint32_t sleep_start_min:12;
    uint32_t sleep_end_min:12;
    uint32_t rsv32:8;
    uint32_t ancs_notice;/*ancs通知*/
    uint32_t menu_enable_index;
    STU_SEDENTARY_REAMND StuSendentaryRemain[6];
    uint8_t sedentary_min;
    uint16_t sedentary_big_times;

    uint16_t step_goal;

} STU_PARA;

typedef struct
{
    uint8_t head[2];
    STU_PARA StuPara;
    uint8_t empty[EEPROM_SIZE-3-sizeof(STU_PARA)];
    uint8_t varity;
} STU_EEPROM;
#pragma pack()





extern STU_EEPROM StuEeprom;





bool Flash_Write_World(uint32_t *addr,uint32_t *Data_Buf,uint32_t len);
bool Flash_Erase_Page(uint32_t Page_Num);
uint32_t Flash_ReadData(uint32_t *addr);

void EEpromSetParaToFactory(void);
void EepromAnsy(void);
bool InitEEprom(void);











/*每天需要写一次的历史数据*/
/*参数是要写入的日期*/
void write_para_history(uint16_t year,uint8_t month ,uint8_t day);
uint16_t read_history_items(uint32_t time_sec);

uint16_t CalcCrc16(uint16_t crc,const uint8_t* pchMsg, uint32_t wDataLen);


uint32_t ReadProx(void);
#endif



