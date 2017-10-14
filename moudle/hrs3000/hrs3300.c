#include <stdio.h>
#include <stdbool.h>
//#include "SEGGER_RTT.h"
//#include "hrs3300_i2c.h"
//#include "app_timer.h"
#include "nrf_log.h"

//////////////////////////////
#include "hrs3300.h"
//#include "hrs3300_alg.h"
#include "hrs3300_reg_init.h"

#include "hr_app.h"
#include "iic_gpio.h"
#include "lis3dh_driver.h"
//////////////////////////////
// #define GSENSER_DATA

// #ifdef GSENSER_DATA
// #include "lis3dh_drv.h"
// #endif

// hrs3300 customer config
const uint8_t  hrs3300_bp_timeout_grade = 0;  // max 15
const uint8_t  hrs3300_agc_init_stage = 0x04;  // init AGC state  
const uint8_t  hrs3300_bp_power_grade = 1;//0   1-->power half
const uint8_t  hrs3300_accurate_first_shot = 0;
const uint8_t  hrs3300_up_factor = 3;
const uint8_t  hrs3300_up_shift = 2;
const uint16_t hrs3300_AMP_LTH = 120;
const uint16_t hrs3300_hr_AMP_LTH = 150;
const uint16_t hrs3300_hr_PVAR_LTH = 10;
// hrs3300 customer config end

//20161117 added by ericy for "low power in no_touch state"
static bool hrs3300_power_up_flg = 0 ;
uint8_t reg_0x7f ;
uint8_t reg_0x80 ;
uint8_t reg_0x81 ;
uint8_t reg_0x82 ;
//20161117 added by ericy for "low power in no_touch state"


// #define Hrs3300_write_reg  HRS_WriteBytes
// #define Hrs3300_read_reg()  bool HRS_ReadBytes(uint8_t* Data, uint8_t RegAddr)


bool Hrs3300_write_reg(uint8_t addr, uint8_t data) 
{
	bool revvalue;
	revvalue=HRS_WriteBytes(addr,data);
	if(revvalue==true)
	return 1;
	else
	return 0;  	
}

uint8_t Hrs3300_read_reg(uint8_t addr) 
{
    uint8_t data_buf;	
	HRS_ReadBytes(&data_buf,addr);
	  return data_buf;  	
}


uint16_t Hrs3300_read_hrs(void)
{
	uint8_t  databuf[3];
	uint16_t data;

	databuf[0] = Hrs3300_read_reg(0x09);	// addr09, bit
  databuf[1] = Hrs3300_read_reg(0x0a);	// addr0a, bit
  databuf[2] = Hrs3300_read_reg(0x0f);	// addr0f, bit
	
	data = ((databuf[0]<<8)|((databuf[1]&0x0F)<<4)|(databuf[2]&0x0F));

	return data;
}

uint16_t Hrs3300_read_als(void)
{
	uint8_t  databuf[3];
	uint16_t data;

	databuf[0] = Hrs3300_read_reg(0x08);	// addr09, bit [10:3]
  databuf[1] = Hrs3300_read_reg(0x0d);	// addr0a, bit [17:11]
  databuf[2] = Hrs3300_read_reg(0x0e);	// addr0f, bit [2:0]
	
	data = ((databuf[0]<<3)|((databuf[1]&0x3F)<<11)|(databuf[2]&0x07));
	
	if (data > 32767) data = 32767;  // prevent overflow of other function

	return data;
}



bool Hrs3300_chip_init()
{
	int i =0 ;
// 	uint8_t id =0;
// 	LOGRTT(">>> hrs3300 init \r\n");
	for(i = 0; i < INIT_ARRAY_SIZE;i++)
	{
	    if ( Hrs3300_write_reg( init_register_array[i][0],
                                init_register_array[i][1]) != 0 )
	    {
	       goto RTN;
	    }
  	}	
	
		
		//20161117 added by ericy for "low power in no_touch state"		
		if(hrs3300_power_up_flg == 0){
		  reg_0x7f=Hrs3300_read_reg(0x7f) ;
		  reg_0x80=Hrs3300_read_reg(0x80) ;
		  reg_0x81=Hrs3300_read_reg(0x81) ;
		  reg_0x82=Hrs3300_read_reg(0x82) ;		
			hrs3300_power_up_flg =  1; 
		}
		//20161117 added by ericy for "low power in no_touch state"
		

		

	  return true;
RTN:
	  return false;		
}

void Hrs3300_chip_enable()
{	
  Hrs3300_write_reg( 0x16, 0x78 );
  Hrs3300_write_reg( 0x01, 0xd0 );	
	Hrs3300_write_reg( 0x0c, 0x2e );
	
	return ;	
}

void Hrs3300_chip_disable()
{
	Hrs3300_write_reg( 0x01, 0x08 );
	Hrs3300_write_reg( 0x02, 0x80 );
	Hrs3300_write_reg( 0x0c, 0x4e );
	
	Hrs3300_write_reg( 0x16, 0x88 );
	
	Hrs3300_write_reg( 0x0c, 0x22 );
	Hrs3300_write_reg( 0x01, 0xf0 );
	Hrs3300_write_reg( 0x0c, 0x02 );

	Hrs3300_write_reg( 0x0c, 0x22 );
	Hrs3300_write_reg( 0x01, 0xf0 );
	Hrs3300_write_reg( 0x0c, 0x02 );
	
	Hrs3300_write_reg( 0x0c, 0x22 );
	Hrs3300_write_reg( 0x01, 0xf0 );
	Hrs3300_write_reg( 0x0c, 0x02 );
	
	Hrs3300_write_reg( 0x0c, 0x22 );
	Hrs3300_write_reg( 0x01, 0xf0 );
	Hrs3300_write_reg( 0x0c, 0x02 );
	
	return ;	
}

// #define BP_CUSTDOWN_ALG_LIB
void heart_rate_meas_timeout_handler(uint8_t * p_context,uint8_t *wear_flg,uint8_t* end,uint8_t*high,uint8_t*low)
{
int16_t new_raw_data,  als_raw_data;
	  hrs3300_results_t alg_results;
	AxesRaw_t axes;
	uint8_t n;
#ifdef USE_LOW_RAM_HR		
	hrs3300_bp_results_t	bp_alg_results ;	
#endif	
	  static uint16_t timer_index =0;
	new_raw_data=Hrs3300_read_hrs();
	als_raw_data=Hrs3300_read_als();
//  	LOGRTT("%d %d\r\n",new_raw_data,als_raw_data);
#ifdef USE_LOW_RAM_HR	

	  Hrs3300_alg_send_data( new_raw_data,  als_raw_data, 0, 0, 0,0);

#else
	
	
// 	  Hrs3300_alg_send_data( new_raw_data,  als_raw_data, 0, 0, 0,0);
	axes=get_acc_data();
	Hrs3300_alg_send_data( new_raw_data,als_raw_data, axes.AXIS_X, axes.AXIS_Y, axes.AXIS_Z, 0); 
#endif
	
	  timer_index ++;
    if (timer_index >= 25)  {    // get result per second
			  timer_index =0;

		*p_context=0;
		alg_results = Hrs3300_alg_get_results();
		//LOGRTT("st:%d\r\n",alg_results);
		if (alg_results.alg_status == MSG_NO_TOUCH)
		{
			*wear_flg=0;
				LOG("MSG_NO_TOUCH\r\n");
		}
		else if (alg_results.alg_status == MSG_PPG_LEN_TOO_SHORT)
		{
			*wear_flg=1;
			//LOGRTT("MSG_PPG_LEN_TOO_SHORT\r\n");
		}
		else
		{
#ifdef USE_LOW_RAM_HR					
        bp_alg_results = Hrs3300_alg_get_bp_results();  
        if (bp_alg_results.sbp!= 0)
		{
			*end=GET_BP;
			*high=bp_alg_results.sbp;
			*low=bp_alg_results.dbp;
			
			LOGRTT("bp value:%d LOW:%d hr:%d\r\n",bp_alg_results.sbp,bp_alg_results.dbp,alg_results.hr_result);
			*p_context=	alg_results.hr_result;	
        }else
		{
			LOGRTT("hr value:%d\r\n",alg_results.hr_result);
			*p_context=	alg_results.hr_result;		
		}
#else
			LOG("hr value:%d\r\n",alg_results.hr_result);
            *p_context=	alg_results.hr_result;		
#endif			
	
		}
	}
}



#ifndef USE_LOW_RAM_HR

void blood_presure_meas_timeout_handler(uint8_t* end,uint8_t*high,uint8_t*low)
{

	hrs3300_bp_results_t bp_alg_results;
	static uint16_t timer_index =0;


	  Hrs3300_bp_alg_send_data(Hrs3300_read_hrs());

	  timer_index ++;
	if (timer_index >= 50)  {    // get result per second
		timer_index =0;
		bp_alg_results = Hrs3300_alg_get_bp_results();
			
		if (bp_alg_results.object_flg == 1){ 

		}
		if (bp_alg_results.bp_alg_status == MSG_BP_NO_TOUCH)
		{

		}
	        else if (bp_alg_results.bp_alg_status == MSG_PPG_LEN_TOO_SHORT)
		{

		}	          
		else if (bp_alg_results.bp_alg_status == MSG_BP_ALG_TIMEOUT)
		{
			*end=TIME_OUT_BP;
			if (bp_alg_results.sbp != 0)
			{
			}
			else
			{

			}
			LOG("bp time out:hr %d  bph:%d bpl:%d\r\n", bp_alg_results.hr_result,bp_alg_results.sbp,bp_alg_results.dbp);	
		
		}
		else if (bp_alg_results.bp_alg_status == MSG_BP_READY)
		{
			*end=GET_BP;
			*high=bp_alg_results.sbp;
			*low=bp_alg_results.dbp;
			LOG("bp:hr %d  bph:%d bpl:%d\r\n", bp_alg_results.hr_result,bp_alg_results.sbp,bp_alg_results.dbp);	
		}


	}

}

#endif

