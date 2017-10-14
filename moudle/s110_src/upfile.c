
#include "upfile.h"
#include "boot.h"

extern void V_FeedWdog(void);
uint8_t rfidsendbuf[32],rfidRev[32];
bool RevOk=false;
void RevRfDataRecall(uint8_t * datain);
static void AskPacket(uint32_t packet);
uint32_t fileOffset,filesize;
uint16_t filecrc;
uint8_t IapFlg[12];
// uint8_t uifileFlg[12];


uint8_t display_version_sec=0;
uint8_t ui_file_flg=0;




uint32_t uifilesize;
uint16_t uifilecrc;







/*
2.4g
*/
#include "ui_app.h"

extern uint16_t wire_work_sec;
void TaskUpFile(void)
{
    ChangeToRfid(true,RevRfDataRecall);
    IntoTestMode();
    ChangeToRfid(false,NULL);
    if(display_version_sec)
    {
        show_test_mode(display_version_sec);
        display_version_sec=0;
    }
}




void TaskPrintfFile(uint8_t *dataout,uint16_t len)
{
    uint16_t left,position,tmp;
    uint8_t buf[10],i=0;
    if(IsBleConnect()==true) return;
    ChangeToRfid(true,RevRfDataRecall);
    IntoTestMode();
    left=len;
    position=0;
    do
    {
        if(left>9)
            buf[0]=9;
        else
            buf[0]=left;

        memcpy(&buf[1],&dataout[position],buf[0]);
        position+=buf[0];
        left-=buf[0];

        RevOk=false;
        rfidsendbuf[0]=buf[0];
        rfidsendbuf[1]=buf[1];
        rfidsendbuf[2]=buf[2];
        for(i=0; i<3; i++)
        {

            RfTx(buf,10);
            tmp=0;
            while(tmp++<200)
            {
                if((true==RevOk)&&((buf[3]+1)==(rfidRev[3])))
                {
                    i=10;
                    break;
                }
            }
        }

    }
    while(left>0);
    ChangeToRfid(false,NULL);
}





void RevRfDataRecall(uint8_t * datain)
{
    if(NRF_RADIO->CRCSTATUS != 1) return;

    if(0==memcmp(&rfidsendbuf[1],&datain[1],2))
    {
        memcpy(rfidRev,datain,30);
        RevOk=true;
    }
}


uint8_t CheckSum(uint8_t *datain,uint32_t len)
{
    uint32_t i,sum=0;
    for(i=0; i<len; i++)
    {
        sum+=datain[i];
    }
    sum=~sum+1;
    return sum;
}


bool SendPack(uint16_t timeout,uint8_t len)
{
    uint16_t tmp=0;
    RevOk=false;
    RfTx(rfidsendbuf,len);/*adv*/
    while( (tmp++<timeout)&&(true!=RevOk));
    return RevOk;
}



bool EraseBvkFlash(uint32_t size)
{
    uint32_t num=0,i,addr,num64;


    if(size&0x800000)
    {

        ui_file_flg=1;
        size&=0x7fffff;
        filesize&=0x7fffff;
		showUpgredePersent(188);
        AskPacket(0);
        LOG("erase: size=%X,crc=%x\r\n",filesize,filecrc);

        // Flash_Sector_Erase(EX_UI_ADDR_PARA,SIZE_4k);


        num64=(size+4096)/(64*1024);
        if((size+4096)%(64*1024))
        {
            num64++;
        }
        addr=EX_UI_ADDR_PARA;


        for(i=0; i<num64; i++)
        {
            Flash_Sector_Erase(addr,SIZE_64k);
            addr+=(64*1024);
            if(0==(i%10))
            {
                AskPacket(0);
                V_FeedWdog();           /*喂狗*/
            }
        }

        LOG("ERASE ALL\r\n");


    }
    else if(size<=APP_BVK_SIZE)
    {
        num=size/PAGE_SIZE;
        if(size%PAGE_SIZE)
        {
            num++;
        }
        addr=APP_BVK_ADDR/PAGE_SIZE;
        for(i=0; i<num; i++)
        {
            Flash_Erase_Page(addr+i);
        }
    }

    return true;

}
void WriteBvkFlash(uint32_t* buf,uint32_t numOfDword)
{

    // uint8_t tmpbuf[256];
    if(0==ui_file_flg)
    {
        Flash_Write_World((uint32_t *)(APP_BVK_ADDR+fileOffset),buf,numOfDword);
    }
    else
    {
		
        SPI_FLASH_WriteCont((uint8_t*)buf,EX_UI_ADDR+fileOffset,numOfDword*4);
        //  SPI_FLASH_ReadCont(tmpbuf,EX_UI_ADDR+fileOffset,numOfDword*4);
        //  if(0!=memcmp(tmpbuf,buf,numOfDword*4))
        //{
        //   LOG("write err\r\n");
        // }
    }
    fileOffset+=(numOfDword*4);
}






void InitUiFileCrc(void)
{
    uifilecrc=0xffff;
    uifilesize=0;


    SPI_FLASH_ReadCont(IapFlg,EX_UI_ADDR_PARA,12);
    if( (IapFlg[0]==0xaa)&&(IapFlg[1]==0x55))
    {
        uifilesize=IapFlg[2];
        uifilesize<<=8;
        uifilesize|=IapFlg[3];
        uifilesize<<=8;
        uifilesize|=IapFlg[4];
        uifilesize<<=8;
        uifilesize|=IapFlg[5];

        uifilecrc=IapFlg[8];
        uifilecrc<<=8;
        uifilecrc|=IapFlg[9];
    }
}

void WriteFlgBootSys(void)
{

    if(0==ui_file_flg)
    {
       LOG("to boot1\r\n");
        boot_to_new_appilacation(IapFlg,12);
		   LOG("to boot2\r\n");
        StuEeprom.StuPara.ask_version=2;
        EepromAnsy();
		   LOG("to boot3\r\n");
        delay_ms(2000);
        CallErr(DISPLAY_HELLO_UI+1,0,(const uint8_t *)"rfid success Askreset");
    }
    else
    {

        SPI_FLASH_WriteCont(IapFlg,EX_UI_ADDR_PARA,12);
        InitUiFileCrc();
    }

}

bool RfWritePacket(uint8_t*rev_buf,uint32_t maxbuf,uint32_t *rev_lenth,uint32_t *packet )
{
    uint8_t write_all,ask_flg=0;
    write_all=false;
    memcpy(&rev_buf[rev_lenth[0]],&rfidRev[3],16);
    rev_lenth[0]+=16;
    if((fileOffset+rev_lenth[0])<filesize)
    {
        ask_flg=1;

    }
    if(rev_lenth[0]>=maxbuf)
    {

        WriteBvkFlash((uint32_t*)rev_buf,maxbuf/4);
        rev_lenth[0]-=maxbuf;
    }
    if((fileOffset+rev_lenth[0])>=filesize)
    {
        if(rev_lenth[0])
        {

            WriteBvkFlash((uint32_t*)rev_buf,rev_lenth[0]/4);
            rev_lenth[0]=0;
        }
        write_all=true;
    }
    if(ask_flg)
    {
//         delay_us(100);
        packet[0]++;
        AskPacket(packet[0]);
    }

    return write_all;
}
void RfRevFileRecall(uint8_t *revbuf)
{
    if(NRF_RADIO->CRCSTATUS != 1) return;
    if(RevOk==true) return;
    if((revbuf[0]==UPFILE_DATA_24)||\
            (revbuf[0]==UPFILE_END_24))
    {
        if(revbuf[19]==CheckSum(revbuf,19))
        {
            memcpy(rfidRev,revbuf,30);
            RevOk=true;
        }
    }
}
static void AskPacket(uint32_t packet)
{
    RevOk=false;
    if(0==packet)
    {
        rfidsendbuf[6]=UPFILE_DATA_24;
    }



    rfidsendbuf[7]=packet>>8;
    rfidsendbuf[8]=packet;
    rfidsendbuf[9]=CheckSum(rfidsendbuf,9);
    rfidsendbuf[10]=packet>>16;
    RfTx(rfidsendbuf,20);/*adv*/

//              if(0==(packet%256))
//      {
//      LOG("P:%x 7:%x  8:%x  10:%x \r\n",packet,rfidsendbuf[7],rfidsendbuf[8],rfidsendbuf[10]);
//     }


}

bool IsRightBoard(char* boardbuf)
{
    bool OkFlg=true;
    if(0!=memcmp((char*)&version[3],boardbuf+3,3))
        OkFlg=false;
    return OkFlg;
}

bool GetFileCrc(uint16_t crcin,uint16_t* crcout,uint8_t *buf,uint32_t len)
{
    uint16_t tmpcrc=0xffff,crc_tmp;
    uint32_t num,last,j,addr;


// 	ui_file_flg=1;


    if(0==ui_file_flg)
    {
        crcout[0]=CalcCrc16(tmpcrc,(const uint8_t *)APP_BVK_ADDR,filesize);
        if(crcin==crcout[0])
            return true;
        return false;
    }


// 	filesize=2750712;


    last=len;
    num=filesize/len;
    if(filesize%len)
    {
        num++;
        last=filesize%len;
    }
    addr=EX_UI_ADDR;///EX_UI_ADDR;
    for(j=0; j<num; j++)
    {
        if(j==(num-1))
        {
            len=last;
        }
        SPI_FLASH_ReadCont(buf,addr,len);
        addr+=len;
        crc_tmp=CalcCrc16(tmpcrc,(const uint8_t *)buf,len);
        tmpcrc=((crc_tmp>>8)+(crc_tmp<<8));
    }
    crcout[0]=crc_tmp;
    if(crcin==crcout[0])
        return true;

    LOG("IN:%x ,out:%x\r\n",crcin,crc_tmp);
    return false;


}

void RfRevFileSendResult(uint16_t realcrc)
{
    uint8_t i ;
    RevOk=false;

    rfidsendbuf[6]=UPFILE_END_24;
    rfidsendbuf[7]=realcrc>>8;
    rfidsendbuf[8]=realcrc;
    rfidsendbuf[9]=CheckSum(rfidsendbuf,9);
    for(i=0; i<3; i++)
    {
        RfTx(rfidsendbuf,20);/*adv*/
        delay_us(300);
    }
    SemEmpty();
    SendSem(INIT_UI);
    LOG("end:%d\r\n",realcrc);
}
void TaskP24UpdataFile(uint8_t fre)
{
#define MAX_REV_BUF_LEN 256
    uint32_t packet=0,revpacket,rev_lenth;
    uint32_t ticktime=0;
    uint16_t crcout;

    bool crc_ok=false;

    uint8_t rev_buf[MAX_REV_BUF_LEN+16];


    uint8_t tickout50ms=0;
    uint16_t tickout1000ms=0;

#define TIME_OUT_50_MS    (4/2)
#define TIME_OUT_1000_MS   (3000/2)




#define STA_ASK_DATA 0x00
#define STA_SEND_RESULT 0x01

    uint8_t sta=STA_ASK_DATA;
    Nrf51Config_FUN(fre,20,16,RfRevFileRecall);
    ticktime=NRF_RTC1->COUNTER;
    rev_lenth=0;
    AskPacket(packet);
    while(1)
    {
        if(RevOk==true)
        {

            if(rfidRev[0]==UPFILE_DATA_24)
            {
                tickout50ms=0;
                tickout1000ms=0;
                revpacket=rfidRev[1];
                revpacket=(revpacket<<8)+rfidRev[2];
                if((revpacket&0xffff)==(packet&0xffff))
                {
                    

                    if(ui_file_flg==0)
                    {
                        if((packet==64)&&\
                                (true!=IsRightBoard((char*)&rfidRev[3])))
                        {
                            RfRevFileSendResult(1);
                            return;
                        }
                    }

                    if(true==(RfWritePacket(rev_buf,MAX_REV_BUF_LEN,&rev_lenth,&packet)))
                    {
                        sta=STA_SEND_RESULT;
                        crc_ok=GetFileCrc(filecrc,&crcout,rev_buf,256);
                        if(true==crc_ok)
                        {

                            RfRevFileSendResult(0);
                            showUpgredePersent(100);
                            IapFlg[0]=0xaa;
                            IapFlg[1]=0x55;
                            IapFlg[2]=filesize>>24;
                            IapFlg[3]=filesize>>16;
                            IapFlg[4]=filesize>>8;
                            IapFlg[5]=filesize;
                            IapFlg[6]=0;//version
                            IapFlg[7]=0;//version
                            IapFlg[8]=filecrc>>8;
                            IapFlg[9]=filecrc>>0;
                        }
                        else
                        {
                            RfRevFileSendResult(1);
                            showUpgredePersent(133);
							return;
                        }
                    }else
					{
					showUpgredePersent(fileOffset*100/filesize);
					}
					
					
					

                }
                else
                {
                    LOG("p:%x ap:%x 10:%x s8:%x s7:%x\r\n",packet,revpacket,rfidsendbuf[10],rfidsendbuf[8],rfidsendbuf[7]);
                }
            }
            else if(rfidRev[0]==UPFILE_END_24)
            {
                if(true==crc_ok)
                {

                    WriteFlgBootSys();
                }

                tickout50ms=0;
                tickout1000ms=0;
                LOG("rev UPFILE_END_24\r\n");
                SemEmpty();
                SendSem(INIT_UI);
                return ;
            }
            RevOk=false;
        }
        V_FeedWdog();
        if(get_time_escape_ms(NRF_RTC1->COUNTER,ticktime)<2)
        {
            continue;
        }
        ticktime=NRF_RTC1->COUNTER;

        tickout50ms++;
        if(tickout50ms>TIME_OUT_50_MS)
        {
            tickout50ms=0;
            if(sta==STA_ASK_DATA)
            {
                AskPacket(packet);
            }
            else if(sta==STA_SEND_RESULT)
            {
                if(true==crc_ok)
                    RfRevFileSendResult(0);
                else
                    RfRevFileSendResult(1);
            }
        }
        if(tickout1000ms++>TIME_OUT_1000_MS)
        {
            if(true==crc_ok)
            {

                WriteFlgBootSys();

            }
            LOG("time out tickout1000ms\r\n");
            LOG("p:%x ap:%x 10:%x s8:%x s7:%x\r\n",packet,revpacket,rfidsendbuf[10],rfidsendbuf[8],rfidsendbuf[7]);;
            delay_ms(500);
            delay_ms(500);
            SemEmpty();
            SendSem(INIT_UI);
            return;
        }
    }
}



void IntoTestMode(void)
{
    uint32_t size,returnvalue;
    uint8_t i,mac_data[6];

//  	LOG("sendrf\r\n");
    get_mac(mac_data);
    returnvalue=0;
    rfidsendbuf[0]=0x2a;
    rfidsendbuf[1]=mac_data[4];
    rfidsendbuf[2]=mac_data[5];
    rfidsendbuf[3]=version[5];
    rfidsendbuf[4]=version[6];
    rfidsendbuf[5]=version[7];
    rfidsendbuf[6]=0;
    rfidsendbuf[7]=uifilecrc>>8;
    rfidsendbuf[8]=uifilecrc;
    rfidsendbuf[9]=CheckSum(rfidsendbuf,9);
    if(false == SendPack(1600,10)) return;
    if(rfidRev[9]!=CheckSum(rfidRev,9)) return;

//      LOG("get cmd:%02x\r\n",*rfidRev);

    switch(rfidRev[0])
    {

    case UPFILE_START_24:
        size=rfidRev[3];
        size=(size<<8)+rfidRev[4];
        size=(size<<8)+rfidRev[5];
        filecrc=rfidRev[6];
        filecrc=(filecrc<<8)+rfidRev[7];
        filesize=size;
        fileOffset=0;
        ui_file_flg=0;
        if((filesize&0x800000)&&(uifilecrc==filecrc))
        {
            LOG("ui file same to before ,return\r\n");
            break;
        }
        if(false==EraseBvkFlash(size))
        {
            break;
        }
        TaskP24UpdataFile(rfidRev[8]);
        SendSem(INIT_UI);
        break;
    case POWER_OFF_24 :

        if(rfidRev[6]==0x00)/*关机等待60s*/
        {
            StuEeprom.StuPara.ask_power_off=1;
            StuEeprom.StuPara.ask_version=2;
        }
        else      if(rfidRev[6]==0x01)  /*直接关机*/
        {
            StuEeprom.StuPara.ask_power_off=1;
            StuEeprom.StuPara.ask_version=0;
        }
        else        if(rfidRev[6]==0x02)/*显示内部版本号，直到触摸*/
        {
            StuEeprom.StuPara.ask_power_off=1;
            StuEeprom.StuPara.ask_version=4;
        }
        else   if(rfidRev[6]==0x03) /*关机等待60s usb开机*/
        {
            StuEeprom.StuPara.ask_version=2;
            StuEeprom.StuPara.ask_power_off=2;
        }
        returnvalue=1;
        break;
    case VERSION_DISPLAY_24:/*显示30s版本号*/
        display_version_sec=rfidRev[6];
        break;
    case RFID_WRITE_MAC:
        returnvalue=rfidRev[3];
        returnvalue=(returnvalue<<8)+rfidRev[4];
        returnvalue=(returnvalue<<8)+rfidRev[5];
        returnvalue=(returnvalue<<8)+rfidRev[6];
        break;
    default:
        break;
    }

    if(returnvalue)
    {

        rfidsendbuf[0]=rfidRev[0];
        rfidsendbuf[1]=mac_data[4];
        rfidsendbuf[2]=mac_data[5];
        rfidsendbuf[3]=returnvalue>>24;
        rfidsendbuf[4]=returnvalue>>16;
        rfidsendbuf[5]=returnvalue>>8;
        rfidsendbuf[6]=returnvalue>>0;
        rfidsendbuf[9]=CheckSum(&rfidsendbuf[0],9);

        for(i=0; i<3; i++)
        {
            RfTx(rfidsendbuf,10);
            delay_us(300);
        }
        if(rfidRev[0]==RFID_WRITE_MAC)
        {
//                  if((DisplayMac(returnvalue)==true)&&(ReadSN(0)!=returnvalue))
//                  {
//                  WriteSN(0,returnvalue);
//                  }
        }
        if(rfidRev[0]==POWER_OFF_24)
        {
            CallErr(DISPLAY_HELLO_UI+1,0,(const uint8_t *)"rfidreset");
        }
    }


}



//蓝牙升级支持
uint16_t ble_ask_packet=0;
bool startble=false;

void InitBleUpdata(uint32_t size,uint16_t crc,bool Isble)
{
    filesize=size;
    filecrc=crc;
//    file_addr=EX_APP_BVK_ADDR;
//    if(size>EX_APP_BVK_SIZE)
//    {
//        file_addr=EX_FRONT_ADDR;
//    }

    if((0==ble_ask_packet)||(fileOffset==0))
    {
		showUpgredePersent(188);
        ble_ask_packet=0;
        fileOffset=0;
        EraseBvkFlash(filesize);
    }
    startble=Isble;
}

uint8_t ble_data[20];
uint8_t ble_len=0;

void BleFileIn(uint16_t packet,uint8_t*datain,uint8_t len)
{
//void WriteBvkFlash(uint32_t* buf,uint32_t numOfDword)
//WriteBvkFlash
    if(ble_ask_packet==packet)
    {
        ble_len=len;
        memcpy(ble_data,datain,ble_len);
    }
}


bool BleIsUpfile(void)
{
    return startble;
}

void BleAskPacket(uint16_t packet)
{
    uint8_t data[20];

    data[0]=0xa6;
    data[1]=packet>>8;
    data[2]=packet;
    BleWriteData(data,3);

}
void BleSendResult(uint8_t result)
{
    uint8_t data[20];

    data[0]=0xa7;
    data[1]=result;
    BleWriteData(data,2);
    fileOffset=0;
    SemEmpty();
    SendSem(INIT_UI);
    startble=false;
//      ChangeToHightSpeed(false);




//   oled_show_result(result);
//  delay_ms(4000);

}

void TaskBleUpfile(void)
{

#define TIME_OUT_NO_DATA        (8000/500)
#define TIME_OUT_ASK_PACKET     (1000/500)
    uint32_t ticktime;
    uint16_t time_out_no_data=0;
    uint8_t cmp_version=0;
    uint16_t i;
    uint16_t crcout;
    uint8_t buf256[256+17];
    uint16_t buf256_len=0;
    bool crc_ok=false;

    if(false==startble)  return;



	
    ticktime=NRF_RTC1->COUNTER;
    BleAskPacket(ble_ask_packet);

    while(1)
    {
        power_manage();/*key  time BLE */
        V_FeedWdog();/*喂狗*/

        task_and_ble_data();/*处理蓝牙协议栈数据*/
        while(ble_len)//如果有数据过来,存储数据
        {
            ble_ask_packet++;
            time_out_no_data=0;
            ticktime=NRF_RTC1->COUNTER;
            memcpy(&buf256[buf256_len],ble_data,ble_len);
            buf256_len+=ble_len;
            ble_len=0;
            if(buf256_len>=256)
            {
                WriteBvkFlash((uint32_t*)buf256,256/4);
                if(buf256_len>256)
                {
                    for(i=0; i<(buf256_len-256); i++)
                    {
                        buf256[i]=buf256[256+i];
                    }
                }
                buf256_len-=256;
            }
            if((fileOffset>(1024))&&(0==cmp_version))
            {
                cmp_version=1;

                if(true!=IsRightBoard((char*)(APP_BVK_ADDR+1024)))
                {
                    BleSendResult(3);
                    startble=false;
                    return;
                }
            }

            showUpgredePersent(fileOffset*100/filesize);
            if((fileOffset+buf256_len)>=filesize)//升级完成
            {
                if(buf256_len)
                {
                    //                WriteBvkFlash(buf256,256);
                    WriteBvkFlash((uint32_t*)buf256,256/4);
                    buf256_len=0;
                }
                crc_ok=GetFileCrc(filecrc,&crcout,buf256,256);
                if(true==crc_ok)
                {
                    BleSendResult(0);
					showUpgredePersent(100);
					LOG("crc ok\r\n");
                    IapFlg[0]=0xaa;
                    IapFlg[1]=0x55;
                    IapFlg[2]=filesize>>24;
                    IapFlg[3]=filesize>>16;
                    IapFlg[4]=filesize>>8;
                    IapFlg[5]=filesize;
                    IapFlg[6]=0;//version
                    IapFlg[7]=0;//version
                    IapFlg[8]=filecrc>>8;
                    IapFlg[9]=filecrc>>0;
//                     TaskUpFile();
					delay_ms(2000);
					ChangeToRfid(true,RevRfDataRecall);
					IntoTestMode();
					delay_ms(3000);
                    WriteFlgBootSys();
					return;
                }
                else
                {
					LOG("crc err\r\n");
					showUpgredePersent(133);
                    BleSendResult(2);
					return;

                }
            }
        }

		
        if(get_time_escape_ms(NRF_RTC1->COUNTER,ticktime)>=500)
        {
            ticktime=NRF_RTC1->COUNTER;

            if(time_out_no_data++>=TIME_OUT_NO_DATA)
            {
                BleSendResult(1);
                startble=false;
                return;
            }
			LOG("time out:%d\r\n",time_out_no_data);

        }

    }

}

