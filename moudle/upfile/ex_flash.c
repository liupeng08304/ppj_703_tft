#include "ex_flash.h"
#include "bsp.h"
#include "utility.h"
#define TYPE_W25X40 0x001330ef
#define TYPE_PN25X40 0x001340e0
uint8_t ex_flash_ok=1;
uint32_t flashid;
bool SpiOkFlg=false;
#define DELAY_US_N  5

uint32_t Flash_ReadID(void);




/*
*********************************************************************************************************
*   函 数 名:void InitExFlash()
*   功能说明:
*
*       作    者 ：liupeng
*   形    参：
*       版    本：version 1.0
*   返 回 值: 无
*********************************************************************************************************
*/

uint8_t SpiSend(NRF_SPI_Type *spi,uint8_t datasend)
{
    uint8_t out=0;
    SpiOkFlg=spi_send(0,&datasend,1,&out,1);
    return out;

}


void WriteEn(bool Write_en)
{
    FLASH_CS_LOW();
    SpiSend(NRF_SPI0,(true==Write_en) ? WRITE_ENABLE:WRITE_DISABLE);
    FLASH_CS_HIGH();
}


void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t value;
    while(1)
    {
        FLASH_CS_LOW();
		delay_us(DELAY_US_N*2);
        /* Send "Read Status Register" instruction */
        SpiSend(NRF_SPI0,ReadRigesterStutes);
        value=SpiSend(NRF_SPI0,Dummy_Byte);
        FLASH_CS_HIGH();
        delay_us(DELAY_US_N*2);
        if(0==(value&0x01))
            return;


    }




}





static void StartFlashSpi(bool strtFlg)
{
	static uint8_t stab=0xff;
	
	if(stab==strtFlg) return;
	
	
	stab=strtFlg;
    uint8_t i;
    if(true == strtFlg)
    {
        EnableSpi(true,0,SPI_MISO_PIN,SPI_MOSI_PIN,SPI_SCK_PIN);
        for(i=0; i<10; i++)
        {
            FLASH_CS_LOW();
            delay_us(DELAY_US_N);
            SpiSend(NRF_SPI0,NORMAL_POWER);
            FLASH_CS_HIGH();
            delay_us(DELAY_US_N);
            if(SpiOkFlg==true)
            {
                return;
            }
            else
            {
                delay_us(1000);
            }
        }

    }
    else
    {

        for(i=0; i<10; i++)
        {
            FLASH_CS_LOW();
            delay_us(DELAY_US_N);
            SpiSend(NRF_SPI0,POWER_DOWN);
            FLASH_CS_HIGH();
            delay_us(DELAY_US_N);
            if(SpiOkFlg==true)
            {
                EnableSpi(false,0,SPI_MISO_PIN,SPI_MOSI_PIN,SPI_SCK_PIN);
                return;
            }
            else
            {
                delay_us(1000);
            }
        }
        EnableSpi(false,0,SPI_MISO_PIN,SPI_MOSI_PIN,SPI_SCK_PIN);

    }
}


/*
*********************************************************************************************************
*   函 数 名:void InitExFlash()
*   功能说明:
*
*       作    者 ：liupeng
*   形    参：
*       版    本：version 1.0
*   返 回 值: 无
*********************************************************************************************************
*/
//CONFIG_NFCT_PINS_AS_GPIOS 
void InitExFlash(void)
{
    uint8_t i;
    nrf_gpio_cfg_output(SPI_CS_FLASH);
    FLASH_CS_HIGH();
    StartFlashSpi(true);
    for(i=0; i<10; i++)
    {
        FLASH_CS_LOW();
        delay_us(DELAY_US_N);
        SpiSend(NRF_SPI0,ID_READ);                       //JEDEC-ID命令返回三个字节
        flashid = SpiSend(NRF_SPI0,Dummy_Byte);                     //BF
        flashid |= SpiSend(NRF_SPI0,Dummy_Byte)<<8;                 //25
        flashid |= SpiSend(NRF_SPI0,Dummy_Byte)<<16;    //              //4A
        FLASH_CS_HIGH();
         delay_ms(100);                                                                               //TYPE_W25X40                       pn25
        if((0x00130662 == flashid)||(0x00132020 == flashid)||(0x001330ef == flashid)||(0x001340e0 == flashid)||(0x1640ef == flashid))
        {
            ex_flash_ok=1;
            StartFlashSpi(false);
            return;
        }
    }
    ex_flash_ok=0;
 StartFlashSpi(false);
    return;
}

/*******************************************************************************
* Function Name  :
* Description    : Sector Erases 4K 40ms(type) 150ms(max)   64K 80ms(type) 250ms(max)
* Input          : Dst: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/


void Flash_Sector_Erase(uint32_t Addr,uint8_t  size)
{
    uint8_t A1, A2, A3;
    uint8_t buf[10],len=0;
    if(ex_flash_ok==0) return;
    StartFlashSpi(true);
    A1 = (Addr >> 16) & 0xFF;
    A2 = (Addr >> 8 ) & 0xFF;
    A3 = (Addr      ) & 0xFF;
    WriteEn(true);
    FLASH_CS_LOW();

    if(((0x000000ef==(flashid&0x000000ff))||(TYPE_PN25X40==flashid))&&(SIZE_4k==size))
    {
        buf[len++]=0x20;
    }
    else
    {
        buf[len++]=size;
    }

    if(SIZE_FLASH!=size)
    {
        buf[len++]=A1;
        buf[len++]=A2;
        buf[len++]=A3;
    }
    spi_send(0,&buf[0],len,0,0);
    FLASH_CS_HIGH();
    SPI_FLASH_WaitForWriteEnd();
    WriteEn(false);
	StartFlashSpi(false);


}

// #define WAKE_TIME  2
// uint8_t back_to_sleep=WAKE_TIME;
// //call per 500 ms
// void task_ex_flash_power_manage(bool force_sleep)
// {
// 	if(true==force_sleep)
// 	{
// 	StartFlashSpi(false);
// 	back_to_sleep=0;
// 	return;
// 	}
// 	
// 	if(back_to_sleep==0) return;
// 	back_to_sleep--;
// 	if(0==back_to_sleep)
// 	{
// 	StartFlashSpi(false);
// 	}
// }
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadCout
* Description    : Read more than one byte from the flash
* Input          : - ReadAddr: FLASH's internal address to Read from.
*                  - pBuffer: pointer to the buffer  containing the data to be
*                    read from the FLASH.
*                  - NumByteToWrite: number of bytes to read from the FLASH,
*                    must be equal or less than the capability of FLASH
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_ReadCont(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
//    uint16_t i=0;
	#define BUF_LEN_READ  (128)
    uint32_t left,len,position;
    uint8_t buf[BUF_LEN_READ];
    uint8_t a1,a2, a3;
    if(ex_flash_ok==0) return;

    StartFlashSpi(true);
    a1 = (ReadAddr >> 16) & 0xFF;
    a2 = (ReadAddr >> 8 ) & 0xFF;
    a3 = (ReadAddr      ) & 0xFF;

    FLASH_CS_LOW();
    buf[0]=READ_FLASH_FAST;
    buf[1]=a1;
    buf[2]=a2;
    buf[3]=a3;
    buf[4]=Dummy_Byte;
    spi_send(0,&buf[0],5,0,0);


    position=0;
    left=NumByteToRead;
	 memset(buf,Dummy_Byte,BUF_LEN_READ);
    while(left)
    {
        if(left>BUF_LEN_READ)
            len=BUF_LEN_READ;
        else
            len=left;
    
        spi_send(0,&buf[0],len,&pBuffer[position],len);
        left-=len;
        position+=len;
    }

  StartFlashSpi(false);
    FLASH_CS_HIGH();


}

/*******************************************************************************
//函    数：S25FL032p
//功    能：从指定地址WriteAddr开始，读取NumByteToWrite个数据到*pBuffer中 ,最大256Byte
//          超出部分将被忽略

*******************************************************************************/
void SPI_FLASH_WriteCont(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{


    uint8_t buf[5];
//    uint32_t len=0,i
	uint32_t i=0;

//	i = i;
    uint8_t a1,a2,a3;
    if(ex_flash_ok==0) return;


    StartFlashSpi(true);

L1:

    a1 = (WriteAddr >> 16) & 0xFF;
    a2 = (WriteAddr >> 8 ) & 0xFF;
    a3 = (WriteAddr      ) & 0xFF;
    WriteEn(true);
    FLASH_CS_LOW();
    delay_us(DELAY_US_N);

    //  SpiSend(NRF_SPI0,WRITE_PAGE);
    //  SpiSend(NRF_SPI0,a1);
    //  SpiSend(NRF_SPI0,a2);
    //  SpiSend(NRF_SPI0,a3);

    buf[0]=WRITE_PAGE;
    buf[1]=a1;
    buf[2]=a2;
    buf[3]=a3;
    spi_send(0,&buf[0],4,0,0);



    while (NumByteToWrite--)
    {

        SpiSend(NRF_SPI0,pBuffer[i]);
        i++;
        WriteAddr++;
        if((WriteAddr%256==0)&&(NumByteToWrite>0))
        {
            FLASH_CS_HIGH();
            SPI_FLASH_WaitForWriteEnd();
            goto L1;
        }

    }
    FLASH_CS_HIGH();
    delay_us(DELAY_US_N);
    SPI_FLASH_WaitForWriteEnd();
    WriteEn(false);
	StartFlashSpi(false);


}



// uint8_t buf[4096];
// void TestFlash(void)
// {


//     uint32_t addr=0,i;
//     delay_ms(2000);
//     SPI_FLASH_ReadCont(buf,addr,4096);
//     Flash_Sector_Erase(addr,SIZE_4k);
//     SPI_FLASH_ReadCont(buf,addr,4096);
//     for(i=0; i<4096; i++)
//         buf[i]=i;
//     SPI_FLASH_WriteCont(buf,addr,4096);
//     SPI_FLASH_ReadCont(buf,addr,4096);

// }






