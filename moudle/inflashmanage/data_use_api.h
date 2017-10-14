#ifndef DATA_USE_API_H____
#define DATA_USE_API_H____

#include "stdint.h"
#include "queueu51822.h"
#include "SoftwareRTC.h"
#include "wdt.h"
#include "ui_app.h"
//	time.year=2017;
//		time.month=1;
//		time.day=1;
//		time.hour=0;
//		time.minutes=0;
//	time.seconds=0;
//
//sec=convert_time_to_Second(time);	//0x1ffb0300
//
//		time.year=2099;
//		time.month=1;
//		time.day=1;
//		time.hour=0;
//		time.minutes=0;
//	time.seconds=0;
//	  uint32_t sec,sec1,sec2;
//		time.year=2017;
//		time.month=1;
//		time.day=9;
//		time.hour=12;
//		time.minutes=22;
//		time.seconds=22;
//	sec=convert_time_to_Second(time);	//0x1ffb0300
//	sec1=(sec/DAY-1)*DAY;
//	sec2=(sec/DAY-7)*DAY;
//

//ConvertToUTCTime(&time, sec1 );
//	ConvertToUTCTime(&time, sec2 );
//
//sec=convert_time_to_Second(time);	//0xba37e000
#define SEC_2017  0x1ffb0300
#define SEC_2099 0xba37e000


bool data_use_packet_data(uint8_t type ,uint32_t sec,uint8_t* datain,uint8_t *pdata,uint32_t cnt);


//目前只有记步和心率
uint32_t  usr_api_read(uint32_t cnt_start,uint8_t type,uint32_t sec_start ,uint8_t *pdata);

//写入用户数据
//step :type+sec+step(4byte)+step_sec(2byte)+distance_cm(2byte)+calories(2byte)
//HR:  type+sec+ hr(1byte)
void usr_api_write(uint8_t type,uint8_t* datain,uint32_t sec);

bool ble_send_current_step(uint8_t *buf);
void ble_send_history_step(uint32_t sec,uint8_t *buf);
#endif



