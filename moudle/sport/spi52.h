#ifndef SPI52_H__
#define SPI52_H__

#include "nrf_drv_spi.h"
#include "app_error.h"
#include "string.h"
#include "bsp.h"

#define MODE_LCD_SPI 10
bool spi_send(uint8_t spi_num,uint8_t* in,uint8_t inlen,uint8_t *out,uint8_t out_len);
void EnableSpi(bool en_fg,uint8_t mode,uint8_t mi,uint8_t mo,uint8_t sck);


#endif
