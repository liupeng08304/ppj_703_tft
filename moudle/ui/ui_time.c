#include "ui_time.h"
#include "app_timer.h"
#include "s132config.h"
#include "bsp.h"
void sys_time_event_handler(void * val)
{

    SendSem(REFLASH_UI);


}

APP_TIMER_DEF(m_sys_time_id);

volatile uint32_t sys_time_flg=0;
void init_ui_time(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_sys_time_id,
                                APP_TIMER_MODE_REPEATED,
                                sys_time_event_handler);
    APP_ERROR_CHECK(err_code);
    sys_time_flg=0;

}

void start_sys_time(uint32_t who_start,bool is_start)
{
    uint32_t err_code,sys_time_flgb;
    sys_time_flgb=sys_time_flg;

    if(true==is_start)
    {
        sys_time_flg|=who_start;
        if( (sys_time_flg)&&(0==sys_time_flgb))
        {
            err_code = app_timer_start(m_sys_time_id,  APP_TIMER_TICKS(500) , NULL);
            APP_ERROR_CHECK(err_code);
        }

    } else
    {
        sys_time_flg&=(~who_start);
        if( (sys_time_flg==0)&&(sys_time_flgb))
        {
            err_code = app_timer_stop(m_sys_time_id);
            APP_ERROR_CHECK(err_code);
        }
    }



}
