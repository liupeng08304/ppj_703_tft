
#ifndef BSP_H__
#define BSP_H__

#include <stdint.h>
#include <stdbool.h>
#include "sem.h"
#include "SoftwareRTC.h"
#include "lis3dh_driver.h"

#define USE_HR  0x01
#define USE_BLOOD 0x02




#define BORD  (USE_HR) 


#define SPI_MISO_PIN   20
#define SPI_MOSI_PIN    15
#define SPI_SCK_PIN    17
#define SPI_CS_FLASH   21
#define SPI_CS_3DH      18


#define ACC_CS_HIGH() nrf_gpio_pin_set(SPI_CS_3DH)
#define ACC_CS_LOW() nrf_gpio_pin_clear(SPI_CS_3DH)
#define FLASH_CS_HIGH() nrf_gpio_pin_set(SPI_CS_FLASH)
#define FLASH_CS_LOW() nrf_gpio_pin_clear(SPI_CS_FLASH)


#define LCD_SCLK_GPIO_Pin           29
#define LCD_SDIN_GPIO_Pin           8
#define LCD_RST_GPIO_Pin            6
#define LCD_RS_GPIO_Pin             2
#define LCD_CS_GPIO_Pin             11
#define LCD_LED_GPIO_Pin            12
/* 26-ADC0
** 27-ADC1
** 01-ADC2
** 02-ADC3  
** 03-Adc4 ... 06-ADC7
**
*/

#define BAT_ADC           		4
#define BAT_CHANNEL             NRF_SAADC_INPUT_AIN2

#define CHARGER_CONNECTED_PIN 	3
#define CHARGER_CHARGING_PIN  	9








#define RFID_SEND_INTER_SEC   (5*2)









#define ENABLE_SLEEP
#define HISTORY_DATA_ITEM       			7
#define V_A                                 4190//3374
#define V_B                                 574//481
#define ENABLE_BLE


// OLED Control
//#define OLED_SPI_MOSI                0   // OLED_SDA_PIN 
//#define OLED_SPI_SCK                 1   // OLED_SCL_PIN 
//#define OLED_SPI_MISO                5   // OLED_DC_PIN 
//#define OLED_RESET                   6   // OLED Reset
//#define OLED_SPI_CS                  7   // OLED_CS_PIN 




// #define RESET_PIN 16


#define KEY_IC_POWER_PIN 13
#define KEY_PIN 14
#define ReadKey()  nrf_gpio_pin_read(KEY_PIN)



#define MOTOR_PIN 10
#define MOTOR_ON()   nrf_gpio_pin_set(MOTOR_PIN)
#define MOTOR_OFF()  nrf_gpio_pin_clear(MOTOR_PIN)


#define HRS_POWER          25
#define HR_PWR_OFF() nrf_gpio_pin_set(HRS_POWER)
#define HR_PWR_ON() nrf_gpio_pin_clear(HRS_POWER)

#define HRS_SCL_PIN  	28
#define HRS_SDA_PIN     26
#define HRS_REV        	27





#define TX_PIN  13
#define RX_PIN  14

extern const uint8_t version[8];
extern uint8_t g_sensor_ok;
extern uint8_t ex_flash_ok;

void task_process_message(ENUM_SEM rev_sem);/////////////////////////////////////
void buttons_init(void);
void RtcTimeInit(void);
void GetTimeAndSec(  RTC_UTCTimeStruct *time1);


void InitMotor(void);
void MotorOn(uint8_t times,uint8_t on100uints,uint8_t off100uints);
void ManagePowerOn(void);
void init_acc_time(void);
void start_acc_time(bool start_flg);
void read_time(void);
void AccForLcdOnOff(int16_t xvalue,int16_t yvalue,int16_t zvalue,uint32_t total);
void updateDateInMin(bool save_flg);
uint8_t TaskSport(void);
 AxesRaw_t get_acc_data(void);


#endif





