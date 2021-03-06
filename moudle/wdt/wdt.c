#include "wdt.h"


#ifdef ENLABLE_DOG

static bool B_ReadWdtRunStatues(void)
{
    if(NRF_WDT->RUNSTATUS)
        return true;
    else
        return false;
}
#endif

void V_InitWdt(uint32_t TimeOutMs)
{
#ifdef ENLABLE_DOG
    NRF_WDT->TASKS_START=false;
    NRF_WDT->CRV=32768*TimeOutMs/1000-1;
    NRF_WDT->CONFIG=0;
    NRF_WDT->CONFIG|=0x01;//Keep the watchdog running while the CPU is sleeping.
    NRF_WDT->CONFIG|=0x08;//Keep the watchdog running while the CPU is halted by the debugger.
    //NRF_WDT->CONFIG&=0xf7;//Pause watchdog while the CPU is halted by the debugger.
    //NRF_WDT->INTENSET=WDT_INTENSET_TIMEOUT_Set;//???????
    NRF_WDT->TASKS_START=true;
    while(false==B_ReadWdtRunStatues);
    V_FeedWdog();
#endif
}




void V_FeedWdog(void)
{
#ifdef ENLABLE_DOG
    NRF_WDT->RREN=1;
    NRF_WDT->RR[0]=0x6E524635;
    NRF_WDT->RREN=0;
#endif
}


