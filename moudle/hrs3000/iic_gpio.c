#include "iic_gpio.h"




/******************************************************************************
**
**	I2c Platform Functions
**
*******************************************************************************/
static void HRS_i2c_udelay(unsigned int time)
{
    delay_us(time);
}




/*
*当CLK是高电平的时候，DATA由高变低表示I2C起始
*/
static void HRS_i2c_begin(void)
{

    HRS_I2C_CLK_OUTPUT;


    HRS_I2C_DATA_OUTPUT;
    HRS_i2c_udelay(DELAY_US);//(20);//(40); 	//20
    HRS_I2C_DATA_HIGH;
    HRS_I2C_CLK_HIGH;
    HRS_i2c_udelay(DELAY_US);//(20);//(40); 	//20
    HRS_I2C_DATA_LOW;
    HRS_i2c_udelay(DELAY_US);//(20);		//10
    HRS_I2C_CLK_LOW;
    HRS_i2c_udelay(DELAY_US);//(20);		//10
}

/*
*当CLK是高电平的时候，DATA由低变高表示I2C结束
*/

static void HRS_i2c_end(void)
{
    HRS_I2C_CLK_OUTPUT;
    HRS_I2C_DATA_OUTPUT;

    HRS_i2c_udelay(DELAY_US);		//10
    HRS_I2C_CLK_HIGH;
    HRS_i2c_udelay(DELAY_US);		//10
    HRS_I2C_DATA_HIGH;

}
void HRS_i2c_one_clk(void)
{
    HRS_i2c_udelay(DELAY_US);//(20);		//5
    HRS_I2C_CLK_HIGH;
    HRS_i2c_udelay(DELAY_US);//(40);		//10
    HRS_I2C_CLK_LOW;
    HRS_i2c_udelay(DELAY_US);//(20);		//5
}

/******************************************
	software I2C read byte with ack
*******************************************/
uint8_t HRS_ReadByteAck(void)
{
    int8_t i;
    uint8_t data;

    HRS_I2C_DATA_INPUT;
    data = 0;

    for (i=7; i>=0; i--)
    {
        if (HRS_I2C_GET_BIT())
        {
            data |= (0x01<<i);
        }
        HRS_i2c_one_clk();
    }

    /**send  ack :  data pin set low*/
    HRS_I2C_DATA_OUTPUT;
    HRS_I2C_DATA_LOW;
    HRS_i2c_one_clk();

    return data;
}

/******************************************
	software I2C read byte without ack
*******************************************/
uint8_t HRS_ReadByteNAck(void)
{
    int8_t i;
    uint8_t data;

    HRS_I2C_DATA_INPUT;
    data = 0;

    for (i=7; i>=0; i--)
    {
        if (HRS_I2C_GET_BIT())
        {
            data |= (0x01<<i);
        }
        HRS_i2c_one_clk();
    }

    /*not send ack data pin set high*/
    HRS_I2C_DATA_OUTPUT;
    HRS_I2C_DATA_HIGH;
    HRS_i2c_one_clk();

    return data;
}

void HRS_SendByte(uint8_t sData)
{
    int8_t i;

    for (i=7; i>=0; i--)
    {
        if ((sData>>i)&0x01)
        {
            HRS_I2C_DATA_HIGH;
        }
        else
        {
            HRS_I2C_DATA_LOW;
        }
        HRS_i2c_one_clk();
    }
}

static bool HRS_Chkack(void)
{
    bool result = false;

    HRS_I2C_DATA_INPUT;
    HRS_i2c_udelay(DELAY_US);		//5
    HRS_I2C_CLK_HIGH;
    HRS_i2c_udelay(DELAY_US);		//5

    if(HRS_I2C_GET_BIT())		//Non-ack
    {
        HRS_i2c_udelay(DELAY_US);	//5
        HRS_I2C_CLK_LOW;
        HRS_i2c_udelay(DELAY_US);	//5
        HRS_I2C_DATA_OUTPUT;
        HRS_I2C_DATA_LOW;

        result = false;
    }
    else					//Ack
    {
        HRS_i2c_udelay(DELAY_US);	//5
        HRS_I2C_CLK_LOW;
        HRS_i2c_udelay(DELAY_US);	//5
        HRS_I2C_DATA_OUTPUT;
        HRS_I2C_DATA_LOW;

        result =  true;
    }


    return result;

}
/******************************************
	software I2C restart bit
*******************************************/

void HRS_Restart(void)
{
    HRS_I2C_CLK_OUTPUT;
    HRS_I2C_DATA_OUTPUT;

    HRS_i2c_udelay(DELAY_US);
    HRS_I2C_DATA_HIGH;
    HRS_i2c_udelay(DELAY_US);		//10
    HRS_I2C_CLK_HIGH;
    HRS_i2c_udelay(DELAY_US);
    HRS_I2C_DATA_LOW;
    HRS_i2c_udelay(DELAY_US);		//10
    HRS_I2C_CLK_LOW;
    HRS_i2c_udelay(DELAY_US);		//10
}

static uint8_t iic_addr=HRS3_DEVICE_WRITE_ADDRESS;//HRS3_DEVICE_WRITE_ADDRESS


/******************************************
	 read bytes
*******************************************/

bool HRS_ReadBytes(uint8_t* Data, uint8_t RegAddr)
{
    HRS_i2c_begin();						//start bit
    HRS_SendByte(iic_addr);		//slave address|write bit
    if(false == HRS_Chkack())		//check Ack bit
    {

        HRS_i2c_end();
        return false;
    }

    HRS_SendByte(RegAddr);				//send RegAddr
    if(false == HRS_Chkack())		//check Ack bit
    {

        HRS_i2c_end();
        return false;
    }

    HRS_Restart();						//restart bit

    HRS_SendByte(iic_addr|0x01);		//slave address|read bit
    if(false == HRS_Chkack())
    {

        HRS_i2c_end();
        return false;
    }

    *Data = HRS_ReadByteNAck();

    HRS_i2c_end();						//stop bit
    return true;

}

/******************************************
	 write bytes
*******************************************/

bool HRS_WriteBytes(uint8_t RegAddr, uint8_t Data)
{
    HRS_i2c_begin();						//start bit

    HRS_SendByte(iic_addr);		//slave address|write bit
    if(false == HRS_Chkack())		//check Ack bit
    {

        HRS_i2c_end();
        return false;
    }

    HRS_SendByte(RegAddr);				//send RegAddr
    if(false == HRS_Chkack())		//check Ack bit
    {

        HRS_i2c_end();
        return false;
    }

    HRS_SendByte(Data);					//send parameter
    if(false == HRS_Chkack())
    {

        HRS_i2c_end();
        return false;
    }

    HRS_i2c_end();						//stop bit

    return true;
}



