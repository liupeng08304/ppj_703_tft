#ifndef __IIC_H__
#define __IIC_H__
#include "stdbool.h"
#include "stdint.h"
#include "bsp.h"
#include "utility.h"
#include "nrf_gpio.h"

#define HRS3_DEVICE_WRITE_ADDRESS	 	0x88
#define HRS3_DEVICE_READ_ADDRESS 		0x89


#define	 HRS_I2C_CLK_OUTPUT			nrf_gpio_cfg_output(HRS_SCL_PIN)
#define	 HRS_I2C_CLK_HIGH			nrf_gpio_pin_set(HRS_SCL_PIN)
#define	 HRS_I2C_CLK_LOW			nrf_gpio_pin_clear(HRS_SCL_PIN)





#define	 HRS_I2C_DATA_OUTPUT		nrf_gpio_cfg_output(HRS_SDA_PIN)
#define  HRS_I2C_DATA_INPUT			nrf_gpio_cfg_input(HRS_SDA_PIN,NRF_GPIO_PIN_NOPULL)
#define	 HRS_I2C_DATA_HIGH			nrf_gpio_pin_set(HRS_SDA_PIN)
#define	 HRS_I2C_DATA_LOW			nrf_gpio_pin_clear(HRS_SDA_PIN)


#define HRS_I2C_GET_BIT()  nrf_gpio_pin_read(HRS_SDA_PIN)


#define DELAY_US  1
bool HRS_ReadBytes(uint8_t* Data, uint8_t RegAddr);
bool HRS_WriteBytes(uint8_t RegAddr, uint8_t Data);
#endif


