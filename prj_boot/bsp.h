
#ifndef BSP_H__
#define BSP_H__

#include <stdint.h>
#include <stdbool.h>
#include "sem.h"

extern const uint8_t version[8];

#define RFID_SEND_INTER_SEC   (5*2)

#define USE_HR  0x01
#define USE7028 0x02
#define USE_BLOOD 0x04


#define BORD_CC_00  (0)
#define BORD_CC_01  (USE_HR)
#define BORD_EE_01  (USE_HR|USE7028|USE_BLOOD)
#define BORD_EE_00  (USE7028)




#define BORD BORD_CC_01 

/*
#define BORD BORD_CC_01   cc带心率
#define BORD BORD_CC_00   cc不带心率
BORD_EE_01
BORD_EE_00



#define BORD_CC_01  (USE_HR|USE_BLOOD)cc带心率+血压
*/


#define VERSION_IFIT



//#define PRINTF_STEP_STOP_REASON

//#define PRINTF_ANCS
//#define EBABLE_PRINTF_2P4
//#define EBABLE_PRINTF_SLEEP

//#define DEBUF_SEDENTARY

//#define USE_GO_UI



#ifdef 	VERSION_IFIT
//===================================================================================================================

//#define USE_GO_UI


#ifdef USE_GO_UI
#if(0==(BORD&USE_HR))
#define DEVICE_NAME				 "GO"
#define DEVICE_NAME_D				"GO-D"
#else
#define DEVICE_NAME				 "GO HR"
#define DEVICE_NAME_D				"GO HR-D"
#endif
#else
#if(0==(BORD&USE_HR))
#define DEVICE_NAME				 "Fitpolo"
#define DEVICE_NAME_D				"Fitpolo-D"
#else
#define DEVICE_NAME				 "FitpoloHR"
#define DEVICE_NAME_D				"FitpoloHR-D"
#endif
#endif
#define ENABLE_SLEEP
#define	PCN_VF103_OLED_ON		/*使用硬件PCBvf103自动亮屏幕使用这个宏定义*/
#define	BLE_CHANGE_NAME                /*长按改变关播名字*/
#define HISTORY_DATA_ITEM		7
#define V_A									4000//3374
#define V_B									566//481
#define OLED_X_SIZE					64
#define OLED_Y_SIZE					48
#define X_OFFSET						32
#define Y_OFFSET						2
#define TRUN_180_EN
#define	ENABLE_BLE

//#define REVERSE_OLED  //OLED 鍙嶆樉
//===================================================================================================================

#endif


#define DISPLAY_HELLO_UI  0xbbbbbbbb



// OLED Control
#define OLED_SPI_MOSI                0   // OLED_SDA_PIN 
#define OLED_SPI_SCK                 1   // OLED_SCL_PIN 
#define OLED_SPI_MISO                5   // OLED_DC_PIN 
#define OLED_RESET                   6   // OLED Reset
#define OLED_SPI_CS                  7   // OLED_CS_PIN 




#define RESET_PIN 16


#define KEY_IC_POWER_PIN 10
#define KEY_PIN 9
#define ReadKey()  nrf_gpio_pin_read(KEY_PIN)



#define MOTOR_PIN 11
#define MOTOR_ON()   nrf_gpio_pin_set(MOTOR_PIN)
#define MOTOR_OFF()  nrf_gpio_pin_clear(MOTOR_PIN)



/*  SPI0 */
#define SPI_PSELSCK0              21   /*!< GPIO pin number for SPI clock (note that setting this to 31 will only work for loopback purposes as it not connected to a pin) */
#define SPI_PSELMOSI0             22   /*!< GPIO pin number for Master Out Slave In    */
#define SPI_PSELMISO0             23   /*!< GPIO pin number for Master In Slave Out    */



#define SPI_PSELSS0_G_SENSOR      25   /*!< GPIO pin number for Slave Select           */
#define SPI0_INT_PIN_G_SENSOR  		28 // SPI INT signal.
#define ACC_CS_HIGH() nrf_gpio_pin_set(SPI_PSELSS0_G_SENSOR)
#define ACC_CS_LOW() nrf_gpio_pin_clear(SPI_PSELSS0_G_SENSOR)


#define  FLASH_CS_PIN 				24
#define FLASH_CS_HIGH() nrf_gpio_pin_set(FLASH_CS_PIN)
#define FLASH_CS_LOW() nrf_gpio_pin_clear(FLASH_CS_PIN)


#define BAT_ADC           2
#define BAT_CHANNEL 			ADC_CONFIG_PSEL_AnalogInput3

#define CHARGER_CONNECTED_PIN 30
#define CHARGER_CHARGING_PIN  29
#define BATTERY_ADC_RESOLUTION  (ADC_CONFIG_RES_10bit)

#define HR_OUT_ADC      3
#define HR_CHANNEL 			ADC_CONFIG_PSEL_AnalogInput4
#define SKIN_CHECK_ADC  4
#define SKIN_CHANNEL 			ADC_CONFIG_PSEL_AnalogInput5
#define HR_LED_PIN			8
#define HR_PWR_PIN			12

#define HR_LED_ON() nrf_gpio_pin_set(HR_LED_PIN)
#define HR_LED_OFF() nrf_gpio_pin_clear(HR_LED_PIN)

#define HR_PWR_OFF() nrf_gpio_pin_set(HR_PWR_PIN)
#define HR_PWR_ON() nrf_gpio_pin_clear(HR_PWR_PIN)



#define TX_PIN  13
#define RX_PIN  14





extern const uint8_t version[8];

extern uint8_t g_sensor_ok;
extern uint8_t ex_flash_ok;



void CallErr(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);

void task_process_message(ENUM_SEM rev_sem);

void buttons_init(void);
void RtcTimeInit(void);

void InitMotor(void);
void MotorOn(uint8_t times,uint8_t on100uints,uint8_t off100uints);

void ManagePowerOn(void);

void init_acc_time(void);
void start_acc_time(bool start_flg);

void read_time(void);

void AccForLcdOnOff(int16_t xvalue,int16_t yvalue,int16_t zvalue,uint32_t total);


void updateDateInMin(bool save_flg);

uint8_t TaskSport(void);


#ifdef PRINTF_ANCS
void UsartTx(uint8_t *buf,uint16_t len);
void UartInit(void);
void usart_send_buf(void);
void UsartPrintfHex(uint8_t *buf,uint16_t len);
#endif



#endif





