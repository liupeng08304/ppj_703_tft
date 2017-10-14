#ifndef UI_TIME_H___
#define UI_TIME_H___

#include"stdint.h"
#include"stdbool.h"

#define CHARGE_TIME 0x00000001
#define MSG_TIME		0x00000002
#define START_TIME  0x00000004
#define START_ALL   0xffffffff

void start_sys_time(uint32_t who_start,bool is_start);
void init_ui_time(void);





#endif


