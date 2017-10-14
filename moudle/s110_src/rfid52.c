

#include "rfid52.h"



ble_anable_fun my_ble_anable;
delay_ms_fun my_rfid_delay_ms_in;
uint32_t timeout_cnt;

uint8_t RfidTxBuf[32];
uint8_t RfidRxBuf[32];

void RfOn(bool Onflg);
static uint8_t Rx_Buf_Temp[32] = {0};
uint8_t RX0[5] = {0x34,0x43,0x10,0x10,0x01};
uint8_t RX1[5] = {0x34,0x43,0x10,0x10,0x01};


//static void RestartRev(void);
uint8_t SB(int8_t inp)
{
    int8_t num=0x00;
    int8_t i;
    for(i = 0; i < 8; i++)
    {
        num |= ((inp >> i) & 0x01) << (7 - i);
    }
    return num;
}
void WaitHfclk(void)
{
    uint32_t CNT_Temp = 0;
    // Wait for the external oscillator to start up.
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        if(CNT_Temp++ >= (16*1000*100))/*100ms*/
            break;
    }
}
void Open_HFCLK(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
//WaitHfclk();
}



RfRevRecall_fun RfRevDataFunction;

void Nrf51Config(uint8_t freq_ch,uint8_t len,uint8_t data_rate)
{

    NRF_RADIO->TXPOWER = (0x04<<0);//
    NRF_RADIO->FREQUENCY = freq_ch&0x3F;//24MHz+2400MHz=2424MHz
//00：1Mbit，01：2Mbit，02：250Kbit，03：1Mbit（ble）
    if((data_rate==1)||(data_rate==16))
        NRF_RADIO->MODE = (00<<0);//1Mbit
    else if(data_rate==2)
        NRF_RADIO->MODE = (01<<0);//2Mbit
    else if(data_rate==250)
        NRF_RADIO->MODE = (2<<0);//250Kbit
    else
        NRF_RADIO->MODE = (01<<0);//1Mbit(ble)
    NRF_RADIO->PREFIX0 =(SB(RX1[0])<< 8) |(SB(RX0[0])<< 0);
    NRF_RADIO->BASE0 =
        (SB(RX0[1])<<24)|(SB(RX0[2])<<16)|(SB(RX0[3])<<8)|(SB(RX0[4])<<0);
    NRF_RADIO->BASE1 =
        (SB(RX1[1])<<24)|(SB(RX1[2])<<16)|(SB(RX1[3])<<8)|(SB(RX1[4])<<0);
    NRF_RADIO->TXADDRESS = 0x00UL; //
    NRF_RADIO->RXADDRESSES = (1<<1); //

    NRF_RADIO->PCNF0 = (3<<16)| //S1 :3bits
                       (0<<8) | //S0 :0
                       (6<<0); //Length :6bits

    NRF_RADIO->PCNF1 = (0<<25)| //效验位（0 关，1 开）
                       (1<<24)| //数据大小端（高低字节哪个先发 0 低字节，1 高字节）1 为数组的最高位先发
                       (4<<16)| //通道 0~7 地址高字长度（ nrf24L01+高低字节 5 字节：4 个高字+1 个低字）
                       0| //(len<<8) | //数据字节长度（255~1）32 字节 QxC
                       (len<<0); //硬件传输字节长度（255~1）32 字节 QxC
// CRC 校验长度配置
    NRF_RADIO->CRCCNF = 1; // 校验长度 1 个 char
    if(data_rate==16)
        NRF_RADIO->CRCCNF = 2; // 校验长度 1 个 char
    if ((NRF_RADIO->CRCCNF & 0x03)== 2 )
    {
//NRF_RADIO->CRCINIT = 0xaaaaUL; // 校验初始值
        NRF_RADIO->CRCINIT = 0xffffUL; // 校验初始值
        NRF_RADIO->CRCPOLY = 0x11021UL; // CRC poly: x^16+x^12^x^5+1
    }
    else if ((NRF_RADIO->CRCCNF & 0x03) == 1 )
    {
        NRF_RADIO->CRCINIT = 0xFFUL; // 校验初始值
        NRF_RADIO->CRCPOLY = 0x107UL; // CRC poly: x^8+x^2^x^1+1
    }
    NRF_RADIO->SHORTS = 0x08;
//接收/发射寄存器是 NRF_RADIO->PACKETPTR
}
/*freq_ch 20 ~ 60
**
**
*/


void Nrf51Config_FUN(uint8_t freq_ch,uint8_t len,uint8_t data_rate,RfRevRecall_fun RevdataCall)//无线配置，准备和 nrf24L01+通讯
{
    RfOn(false);
    my_rfid_delay_ms_in(1);
    Nrf51Config(freq_ch,len,data_rate);
    NRF_RADIO->SHORTS = 0x0;
    RfRevDataFunction=RevdataCall;
    NRF_RADIO->INTENSET=(RADIO_INTENSET_END_Enabled<<RADIO_INTENSET_END_Pos)|(RADIO_INTENSET_RSSIEND_Enabled<<RADIO_INTENSET_RSSIEND_Pos);
    NVIC_EnableIRQ(RADIO_IRQn);
// NVIC_SetPriority(GPIOTE_IRQn, 1);
    NVIC_SetPriority(RADIO_IRQn, 1);
    NRF_RADIO->PACKETPTR = (uint32_t)Rx_Buf_Temp;
//NRF_RADIO->SHORTS = 0x18;/*disable-->rxen address--startrssi*/
// NRF_RADIO->SHORTS = 0x10;/*disable-->rxen address--startrssi*/
    NRF_RADIO->SHORTS = 0x30;/*end even --->start task address even --->rssi*/
    RfOn(true);
    my_rfid_delay_ms_in(1);
}
uint8_t Myrssi=0;
void SetRssi(int32_t rssi)
{
    if(rssi<0)
    {
        rssi=-rssi;
    }
    if(rssi<=127)
    {
        Myrssi=rssi;
    }
}



uint8_t GetRssi(void)
{
    return Myrssi;
}
bool trs_end=true;
void RADIO_IRQHandler(void)
{
    int rssitmp;
    if(NRF_RADIO->EVENTS_END)
    {
        if(true == trs_end)
        {
//成功收到数据
            RfRevDataFunction((uint8_t*)&Rx_Buf_Temp[2]);
        }
        trs_end=true;
        NRF_RADIO->EVENTS_END=0;
    } else if(NRF_RADIO->EVENTS_RSSIEND)
    {
        NRF_RADIO->EVENTS_RSSIEND=0;
        rssitmp=NRF_RADIO->RSSISAMPLE;
        SetRssi(rssitmp);
    }
}


void RfOn(bool Onflg)
{
//static u8 is_open=3;
    uint16_t counter=0;
    if(true==Onflg)
    {
        Open_HFCLK();
        my_rfid_delay_ms_in(3); //
        NRF_RADIO->EVENTS_READY = 0U;
        NRF_RADIO->TASKS_RXEN = 1U;
        counter=0;
        while(NRF_RADIO->EVENTS_READY == 0U)
        {
            if(counter++>16*2000)
            {
                break;
            }
        }
        NRF_RADIO->TASKS_START = 1U;
    } else
    {
        if(false!=Onflg)
            Onflg=false;

        NRF_CLOCK->TASKS_HFCLKSTOP = 1;
        NRF_RADIO->EVENTS_DISABLED = 0U;
        NRF_RADIO->TASKS_DISABLE = 1U;
        counter = 0;
        while(NRF_RADIO->EVENTS_DISABLED == 0U)
        {
            if(counter++>16*2000)
            {
                break;
            }
        }
        NRF_RADIO->EVENTS_END=0;
    }
}

/*发送完成后，自动进入接收模式*/
void RfTxOnly(uint8_t* data,uint8_t len,bool first)
{
    uint16_t counter=0;
    static uint8_t pid=0;
//NRF_CLOCK->TASKS_HFCLKSTART = 1;
//PRINTF(("tt=%d\r\n",(NRF_RTC1->COUNTER)*1000/TICK_TO_MS));
    if(len<31)
    {
        pid = (pid+1)&0x03; //包序号，循环累加，避免重复
        Rx_Buf_Temp[0] = len;//Len 域 d
        Rx_Buf_Temp[1] = (pid<<1);//s1
        memcpy(Rx_Buf_Temp+2,data,len);
    } else
    {
        memcpy(Rx_Buf_Temp,data,len);
    }
    if(first==true)
    {
        // 关闭
        NRF_RADIO->SHORTS = 0x04;/*disable --- tx*/
        NRF_RADIO->EVENTS_DISABLED = 0U;//无线电已关闭指示灯，关灯
        NRF_RADIO->TASKS_DISABLE = 1U; // 关闭无线电
        counter=0;
        while(NRF_RADIO->EVENTS_DISABLED == 0U)//等待无线电设备关闭
        {
            if(counter++>16*2000)
            {
                break;
            }
        }
        //切换到 tx
        counter=0;
        NRF_RADIO->EVENTS_READY = 0U; //收发模式转换完成 标志位
        //NRF_RADIO->TASKS_TXEN = 1U; //启动接收模式
        while(NRF_RADIO->EVENTS_READY == 0U) //等待收发模式转换完成(指示灯)
        {
            if(counter++>16*2000)
            {
                break;
            }
        }
    }
// 启动数据传输
    trs_end=false;
    counter=0;
    NRF_RADIO->EVENTS_END = 0U;//传输完成指示灯 ，关灯
    NRF_RADIO->TASKS_START = 1U;//开始传输
    while(false == trs_end) //等待传输结束
    {
        if(counter++>16*2000)
        {
            break;
        }
    }
}
/*发送完成后，自动进入接收模式*/
void RfTx(uint8_t* data,uint8_t len)
{
    uint16_t counter;
    RfTxOnly(data,len,true);
    NRF_RADIO->SHORTS = 0x30;/**/
// NRF_RADIO->PACKETPTR = (uint32_t)Rx_Buf_Temp;
    NRF_RADIO->EVENTS_DISABLED = 0U;//无线电已关闭指示灯，关灯
    NRF_RADIO->TASKS_DISABLE = 1U; // 关闭无线电
    counter=0;
    while(NRF_RADIO->EVENTS_DISABLED == 0U)//等待无线电设备关闭
    {
        if(counter++>16*2000)
        {
            break;
        }
    }
    NRF_RADIO->EVENTS_READY = 0U; //收发模式转换完成 标志位
    NRF_RADIO->TASKS_RXEN = 1U; //启动接收模式
    counter=0;
    while(NRF_RADIO->EVENTS_READY == 0U) //等待收发模式转换完成(指示灯)
    {
        if(counter++>16*2000)
        {
            break;
        }
    }
    NRF_RADIO->TASKS_START = 1U; // 开始传输
    return ;
}
void Open32768(void)
{
    uint32_t CNT_Temp = 0;
    NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    // Wait for the external oscillator to start up.
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        if(CNT_Temp++ >= (16*1000*100))/*100ms*/
            break;
    }
}

void InitRfid(ble_anable_fun ble_anable,delay_ms_fun rfid_delay_ms_in,uint32_t timeoutcnt)
{
    my_ble_anable=ble_anable;
    my_rfid_delay_ms_in=rfid_delay_ms_in;
    timeout_cnt=timeoutcnt;
}


void ChangeToRfid(bool flg ,RfRevRecall_fun RfRevRecall)
{

    static uint8_t change=1;
    if(true==flg)
    {
        if(change != 2)
        {
            change=2;
            my_ble_anable(false);
            Open32768();
            Nrf51Config_FUN(40,10,1,RfRevRecall);
        }
    } else if(change != 3)
    {
        change=3;
        NVIC_DisableIRQ(RADIO_IRQn);
        RfOn(false);
        my_ble_anable(true);
    }
}







