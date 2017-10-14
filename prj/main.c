#include "nrf_log.h"
#include "rfid.h"
#include "utility.h"
#include "s132config.h"
#include "app_timer.h"
#include "boot.h"

#include "upfile.h"

#include "lis3dh_driver.h"

#include "lcd.h"
#include "protocol.h"
#include "wdt.h"
#include "ui_app.h"
#include "pedometer_klx.h"
#include "sleep.h"

 
#define VERSION_NUMBER 1
const uint8_t version[8] __attribute__ ((at(0x23000+1024)))= {2,1,VERSION_NUMBER,'Y','H',0xe3,1,VERSION_NUMBER};

void init_all(void);


int main(void)
{
	init_all();
    while(1)
    {
        power_manage();         /*key  time BLE */
        V_FeedWdog();           /*喂狗*/
        task_and_ble_data();    /*处理蓝牙协议栈数据*/
        task_ui();
        TaskBleUpfile();
    }
}


void init_all(void)
{
//     uint32_t err_code,sec;
#ifdef DEBUG_PRINTF
    SEGGER_RTT_Init();
    LOG("start%s\r\n",__TIME__);
#endif

    boot_to_new_appilacation(0,0);
    close_all_gpio();
    InitEEprom();
	StuEeprom.StuPara.step_goal=10000;

	NRF_POWER->DCDCEN=1;

    InitExFlash();
    InitUiFileCrc();
    InitAdc();
    LCD_Init();
	

	InitPower();
    V_InitWdt(30000);
	app_timer_init();
    RtcTimeInit();
    init_ui_time();
    InitHeatRate();
    InitMotor();
	Open32768();
    SleepInit(StuEeprom.StuPara.sleep_start_min,\
              StuEeprom.StuPara.sleep_end_min,\
              &StuEeprom.StuPara.static_min,\
              Flash_Write_World,\
              Flash_Erase_Page,\
              Flash_ReadData);
    g_sensor_ok=((false==AccSensorConfig()) ? 0:2);/*初始化加速度传感器*/
    SetAccPowerOff(true);/*关闭加速度传感器*/
    ManagePowerOn();/*等待开机*/
    init_acc_time(); /*初始化加速度传感器的采集定时器*/
    start_acc_time(true);/*启动加速度传感器的采集定时器*/
    SetAccPowerOff(false);/*启动加速度传感器*/

	
	InitRfid(s132_config_enable,delay_ms,1000);
	InitStep();
	ChangeToRfid(false,NULL);	/*启动蓝牙*/
	HrCheckWear(0);
}
