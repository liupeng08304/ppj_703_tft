#include"ble_fifo.h"



#define BLE_BUF_SIZE 10
static bool BleQueueflg=false;
static uint8_t BleIn=0,BleOut=0;
static uint8_t BleBuf[BLE_BUF_SIZE][21];

bool BleQueueIn(uint8_t*datain,uint8_t len)
{

		memcpy(&BleBuf[BleIn][1],datain,len);
		BleBuf[BleIn][0]=len;
		BleIn++;
		BleIn%=BLE_BUF_SIZE;
		BleQueueflg=true;
		return true;


}

void BleQueueAllOut(void)
{
	BleOut=0;
	BleIn=0;
	BleQueueflg = false; 

}
bool BleQueueOut(uint8_t*dataout,uint8_t* len)
{

	if(BleQueueflg!=true) 
	{
		return false;
	}
	else
	{
		*len=BleBuf[BleOut][0];
		memcpy(dataout,&BleBuf[BleOut][1],*len);
		BleOut++;
		BleOut %= BLE_BUF_SIZE; 
		if(BleOut==BleIn)
		{
		BleQueueflg = false; 
		}
		return true;
	}
}




//#define TX_BUF_LEN  150
//uint8_t tx_buf[TX_BUF_LEN],txlen=0;



//bool TxBleDataIn(uint8_t *buf,uint8_t len)
//{
//  uint8_t i;
//	if((len+txlen+1)>TX_BUF_LEN) return false;
//	tx_buf[txlen]=len;
//	for(i=0;i<len;i++)
//	{
//	tx_buf[txlen+i+1]=buf[i];
//	}
//	txlen+=(len+1);
//		
//return true;
//}

//uint8_t TxBleLen(void)
//{
//return txlen;
//}


//uint8_t  *TxBufDataOut(uint8_t *len)
//{

//	 uint8_t i,getlen;
//	uint8_t *pdata;
//	if(txlen==0)
//	{
//	len[0]=0;
//	return pdata;
//	}

//	len[0]=tx_buf[0];
//	getlen=tx_buf[0]+1;
//	pdata=&tx_buf[1];
//	txlen-=getlen;
//	if(txlen)
//	{
//		for(i=0;i<txlen;i++)
//		{
//			tx_buf[i]=tx_buf[getlen+i];
//		}
//	}
//	
//	return pdata;
//}

