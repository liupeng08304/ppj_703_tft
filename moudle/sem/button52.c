#include "button52.h"


#define BUTTON                  7
/* Scale range. */
#define RANGE                   50

#define THRESHOLD_BUTTON        400

/*lint -e19 -save */
NRF_CSENSE_BUTTON_DEF(m_button, (BUTTON, THRESHOLD_BUTTON));


extern uint16_t csense_value;

void nrf_csense_handler(nrf_csense_evt_t * p_evt)
{
    switch (p_evt->nrf_csense_evt_type)
    {
        case NRF_CSENSE_BTN_EVT_PRESSED:
			  if (p_evt->p_instance == (&m_button))			
	  LOG("press adc:%d\r\n",csense_value);
            break;
        case NRF_CSENSE_BTN_EVT_RELEASED:
			  if (p_evt->p_instance == (&m_button))
					  LOG("release adc:%d\r\n",csense_value);
//		
//            if (p_evt->p_instance == (&m_button))
//            {
//                uint16_t * btn_cnt = ((uint16_t *)p_evt->p_instance->p_context);
//                (*btn_cnt)++;
//                LOG("Button touched %03d times.\r\n", (*btn_cnt));
//            }
            break;
        case NRF_CSENSE_SLIDER_EVT_PRESSED:
        case NRF_CSENSE_SLIDER_EVT_RELEASED:
            break;
        case NRF_CSENSE_SLIDER_EVT_DRAGGED:
            break;
        default:
            LOG("Unknown event.\r\n");
            break;
    }
}

void csense_start(void)
{
    ret_code_t err_code;

    static uint16_t touched_counter = 0;

    err_code = nrf_csense_init(nrf_csense_handler, APP_TIMER_TICKS(50));
    APP_ERROR_CHECK(err_code);

    nrf_csense_instance_context_set(&m_button, (void*)&touched_counter);

    err_code = nrf_csense_add(&m_button);
    APP_ERROR_CHECK(err_code);


}


