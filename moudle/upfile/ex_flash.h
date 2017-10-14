#ifndef LE25U40CQH__H__
#define LE25U40CQH__H__
#include "nrf_gpio.h"
#include "spi52.h"
#include "stdint.h"
#include "stdbool.h"

#define WRITE_ENABLE 0x06
#define ReadRigesterStutes   0x05//
#define WRITE_DISABLE 0x04


#define SIZE_4k 0xd7
#define SIZE_64k 0xd8
#define SIZE_FLASH 0xc7

#define WRITE_PAGE      0x02
#define READ_FLASH_FAST      0x0b
#define ID_READ    			0x9f
#define Dummy_Byte 0xff
#define POWER_DOWN 0xb9
#define NORMAL_POWER 0xab



/*

*/
#define SIZE64K	0x10000
#define SIZE4K  0x01000



#define EX_UI_ADDR_PARA  0x00000000  //aa 55 size size szie crc crc
#define EX_UI_ADDR  (EX_UI_ADDR_PARA+4096)
#define EX_APP_BVK_SIZE (3*1024*1024)//2.7m










extern bool SpiOkFlg;


//uint32_t  Flash_ReadID(void);
void InitExFlash(void);
void Flash_Sector_Erase(uint32_t Addr,uint8_t  size);
void SPI_FLASH_WriteCont(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
void SPI_FLASH_ReadCont(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);






#endif


