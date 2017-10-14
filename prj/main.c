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
        V_FeedWdog();           /*ι��*/
        task_and_ble_data();    /*��������Э��ջ����*/
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
    g_sensor_ok=((false==AccSensorConfig()) ? 0:2);/*��ʼ�����ٶȴ�����*/
    SetAccPowerOff(true);/*�رռ��ٶȴ�����*/
    ManagePowerOn();/*�ȴ�����*/
    init_acc_time(); /*��ʼ�����ٶȴ������Ĳɼ���ʱ��*/
    start_acc_time(true);/*�������ٶȴ������Ĳɼ���ʱ��*/
    SetAccPowerOff(false);/*�������ٶȴ�����*/

	
	InitRfid(s132_config_enable,delay_ms,1000);
	InitStep();
	ChangeToRfid(false,NULL);	/*��������*/
	HrCheckWear(0);
}
