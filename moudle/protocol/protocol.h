#ifndef protocol_data_h___
#define  protocol_data_h___

#include "s132config.h"
#include "ble_fifo.h"




typedef enum
{
    NONE=0,
    MAC_SHOW=16,
    POWER_20=17,
    SENDENTARY=18,
    NEW_CALL=0x99,//
    NEW_MSG,//
    GENERIC_NOTIFICATION,
    NEW_GMSG,
    WHATSAPP,//
    FACEBOOK,
    TWITTER,
    SKYPE,
    QQ,//
    WEICHAR,//
    SNAPCHAT,


} MSG_TYPE;



typedef struct
{
    MSG_TYPE type;
    uint8_t len;
    uint8_t msg[16];
} STU_MSG;
extern STU_MSG StuMsg;

extern void task_and_ble_data(void);

extern void SendMsg(MSG_TYPE msgtype,uint8_t msglen,uint8_t*msgbuf);



extern void check_send_hr_real_time(void);
extern void send_real_time_datas(void);


extern void return_to_normal_speed(void);
extern void SetHighSpeed(void);

#endif


