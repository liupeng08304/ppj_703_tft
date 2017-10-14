#ifndef UP24_FILE_H__
#define UP24_FILE_H__

#include "string.h"
#include "stdbool.h"
#include "stdint.h"
#include "rfid52.h"
#include "stdbool.h"
#include "in_flash_manage.h"
#include "utility.h"
#include "sem.h"
#include "s132config.h"
#include "ex_flash.h"

#define DISPLAY_HELLO_UI  0xbbbbbbbb
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);
#define CallErr  app_error_handler



#define UPFILE_START_24    0x40
#define UPFILE_DATA_24     0x41
#define UPFILE_END_24      0x42
#define POWER_OFF_24       0x43
#define VERSION_DISPLAY_24       0x44
#define RFID_WRITE_MAC       0x45

extern uint16_t uifilecrc;
/*
2.4g
*/
void TaskUpFile(void);
void IntoTestMode(void);

void TaskBleUpfile(void);
void InitBleUpdata(uint32_t size,uint16_t crc,bool Isble);
void BleFileIn(uint16_t packet,uint8_t*datain,uint8_t len);

void InitUiFileCrc(void);


#endif


