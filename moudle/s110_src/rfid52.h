#ifndef RFID_H__
#define RFID_H__




#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "nrf52.h"
#include "nrf52_bitfields.h"





typedef void (*RfRevRecall_fun)(uint8_t* datain);
typedef void (*ble_anable_fun)(bool en_ble);
typedef void (*delay_ms_fun)(uint32_t delayms);





void ChangeToRfid(bool flg ,RfRevRecall_fun RfRevRecall);


//InitRfid(s110_config_enable,delay_ms,16*2000);
void InitRfid(ble_anable_fun ble_anable,delay_ms_fun rfid_delay_ms_in,uint32_t timeoutcnt);


/*����ʱ�����һ��*/
void Open32768(void);

/*������ɺ��Զ��������*/
void RfTx(uint8_t* data,uint8_t len);

/*��ʼ��rfid,��ʼ�����Ѿ���  40 10 1  void xxxfunction(uint8_t *datain)*/
//void RevRfDataRecall(uint8_t * datain)
//{
//	if(NRF_RADIO->CRCSTATUS != 1) return;
//	if(0==memcmp(&rfidsendbuf[1],&datain[1],4))
//	{
//	memcpy(rfidRev,datain,30);
//	RevOk=true;
//	}
//}

void Nrf51Config_FUN(uint8_t freq_ch,uint8_t len,uint8_t data_rate,RfRevRecall_fun RevdataCall);

/*�������߹ر�rfid*/
void RfOn(bool Onflg);
#endif



