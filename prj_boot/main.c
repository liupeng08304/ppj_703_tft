#include "nrf_log.h"
#include "rfid.h"
#include "utility.h"
#include "s132config.h"
#include "app_timer.h"
#include "boot.h"
#include "in_flash_manage.h"




uint32_t size;
uint16_t crc;
uint16_t version_boot;

bool IsNeedUpdata(void)
{
	//upflg aa 55 size0 size1 size2 size 3 ver0 ver1 crc0 crc1
	//uint8_t buf[1024];
	uint8_t* pdata;
	pdata=(uint8_t*)(	BOOT_PARA_CUSTOM_ADDR);

	size=(uint32_t)pdata[2]<<24;
	size|=(uint32_t)pdata[3]<<16;
	size|=(uint32_t)pdata[4]<<8;
	size|=(uint32_t)pdata[5];
	version_boot=(uint16_t)pdata[6]<<8;
	version_boot|=(uint16_t)pdata[7];
	crc=(uint16_t)pdata[8]<<8;
	crc|=(uint16_t)pdata[9];
	if((pdata[0]!=0xaa)||(pdata[1]!=0x55))
	{ 
		   return false;
	}
	  return true;
} 

void WriteToFlash(void)
{

	uint32_t readtimes,i,pageaddr;
	uint32_t *pdatasrc;
	pdatasrc=(uint32_t *)APP_BVK_ADDR;
	pageaddr=APP_ADDR;
	readtimes=size/PAGE_SIZE;
	
	if(size%PAGE_SIZE)
		readtimes++;
	
	for(i=0;i<readtimes;i++)
		flash_nvic_page_erase(pageaddr+i*PAGE_SIZE);
	
	readtimes=size/4;
	if(size%4)
		readtimes++;
	for(i=0;i<readtimes;i++)
		flash_nvic_word_write((uint32_t *)(APP_ADDR+4*i),pdatasrc[i]);
	
	
	
	
} 
uint16_t CalcCrc16(uint16_t crc,const uint8_t* pchMsg, uint32_t wDataLen)
{
		uint32_t i;
		uint8_t j;
		uint16_t c;
    for (i=0; i<wDataLen; i++)
    {
        c = *(pchMsg+i) & 0x00FF;
        crc^=c;
        for (j=0; j<8; j++)
        {
             if (crc & 0x0001)
             {
                crc >>= 1;
                crc ^= 0xA001;
             }
             else
             { 
                crc >>= 1;
             }
        }
   }
    crc = (crc>>8) + (crc<<8);
    return(crc);
}
/*ok return 1 else return 0*/
uint8_t checkok(void)
{ 
	uint16_t tmpcrc=0xffff,outcrc;
	outcrc=CalcCrc16(tmpcrc,(const uint8_t *)APP_ADDR,size);
	if(outcrc==crc)
	{ 
		return 1;
	} 
	return 0;
}

void CleanUpflg(void)
{ 
uint8_t buf[12];
memset(buf,0,12);
boot_to_new_appilacation(buf,12);
} 

int main(void)
{
	uint8_t i=0;
	
		/*使用自带的boot升级*/
		if(true==IsNeedUpdata())
		{

			i = 0;
			do
			{ 
			WriteToFlash();
			i++;
			}while((checkok()==0)&&(i<3));
			if(i<3)
			{
			CleanUpflg();
			}
			
		}
	
//    uint32_t err_code;
    nrf_bootloader_app_start(APP_ADDR);
}



