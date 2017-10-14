#include "spi52.h"




static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);  /**< SPI instance. */
static const nrf_drv_spi_t spi_1 = NRF_DRV_SPI_INSTANCE(1);  /**< SPI instance. */

static uint8_t spi_xfer_done = true;
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
//     NRF_LOG_INFO("Transfer completed.\r\n");
//     if (m_rx_buf[0] != 0)
//     {
//         NRF_LOG_INFO(" Received: \r\n");
//         NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
//     }
}


void EnableSpi(bool en_fg,uint8_t mode,uint8_t mi,uint8_t mo,uint8_t sck)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    nrf_drv_spi_t *pspi;

    if(mode==MODE_LCD_SPI)
    {
        mode=0;
        pspi=(nrf_drv_spi_t *)&spi_1;
    }else
    {
        pspi=(nrf_drv_spi_t *)&spi;
    }

    if(true==en_fg)
    {
        spi_config.mode=(nrf_drv_spi_mode_t)mode;
        spi_config.miso_pin = mi;
        spi_config.mosi_pin = mo;
        spi_config.sck_pin  = sck;
        APP_ERROR_CHECK(nrf_drv_spi_init(pspi, &spi_config, spi_event_handler, NULL));
    }
    else
    {
        nrf_drv_spi_uninit(pspi);
    }

}




bool spi_send(uint8_t spi_num,uint8_t* in,uint8_t inlen,uint8_t *out,uint8_t out_len)
{
    uint32_t delay_us;
	    nrf_drv_spi_t *pspi;

    if(spi_num==MODE_LCD_SPI)
    {
        pspi=(nrf_drv_spi_t *)&spi_1;

    }
    else
    {
        pspi=(nrf_drv_spi_t *)&spi;

    }
    delay_us=2000*inlen;
    if(spi_xfer_done ==false)
        return false;
    spi_xfer_done=false;
	
    nrf_drv_spi_transfer(pspi, in, inlen, out, out_len);
    while(((delay_us--)>2)&&(spi_xfer_done==false));

    if(delay_us>10)
        return true;
    return false;
}







