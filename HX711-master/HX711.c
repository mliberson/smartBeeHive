#define __USE_C99_MATH

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "HX711.h"

/* Code derived from user bogde's library:
        https://github.com/bogde/HX711
*/

/* HX711_init - Initialize weight sensor with appropriate gain */
void HX711_init(uint8_t gain)
{
    PD_SCK_SET_OUTPUT;
    DOUT_SET_INPUT;

    HX711_set_gain(gain);
    HX711_tare(10);
    HX711_set_scale(5*1024/gain);
}

/* HX711_is_ready - return whether the sensor has finished with its measurement */
int HX711_is_ready(void)
{
    return (DOUT_INPUT & (1 << DOUT_PIN)) == 0;
}

/* HX711_set_gain - adjust gain of weight sensor */
void HX711_set_gain(uint8_t gain)
{
	switch (gain)
	{
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
	}

	PD_SCK_SET_LOW;
	HX711_read();
}

/* HX711_read - get a new measurment using the defined
    communication protocol
*/
uint32_t HX711_read(void)
{
	// wait for the chip to become ready
	while (!HX711_is_ready());

    uint32_t count; 
    unsigned char i;
 
    DOUT_SET_HIGH;
    
    //_delay_us(1);
 
    PD_SCK_SET_LOW;
    //_delay_us(1);
 
    count=0; 
    while(DOUT_READ); 
    for(i=0;i<24;i++)
    { 
        PD_SCK_SET_HIGH; 
        //_delay_us(1);
        count=count<<1; 
        PD_SCK_SET_LOW; 
        //_delay_us(1);
        if(DOUT_READ)
            count++; 
    } 
    PD_SCK_SET_HIGH; 
    //_delay_us(1);
    PD_SCK_SET_LOW; 
    //_delay_us(1);
    count ^= 0x800000;
    return(count);
}

/* HX711_read_average - get the average measurement value
    Note: this is useful because the sensor is very
        sensitive and doesn't always give great readings
*/
uint32_t HX711_read_average(uint8_t times)
{
	uint32_t sum = 0;
    uint8_t i;
	for (i = 0; i < times; i++)
	{
		sum += HX711_read();
		// TODO: See if yield will work | yield();
	}
	return sum / times;
}

/* HX711_get_value - get the smoothed sensor value */
uint32_t HX711_get_value(uint8_t times)
{
	return HX711_read_average(times) - OFFSET;
}

/* HX711_get_units - scales the sensor value to desired units */
uint32_t HX711_get_units(uint8_t times)
{
	return HX711_get_value(times) / SCALE;
}

/* HX711_tare - allows user to tare (zero) the weight sensor */
void HX711_tare(uint8_t times)
{
	uint32_t sum = HX711_read_average(times);
	HX711_set_offset(sum);
}

/* HX711_set_scale - allows user to set the desired scale */
void HX711_set_scale(uint32_t scale)
{
	SCALE = scale;
}

/* HX711_get_scale - helper function that returns scale */
uint32_t HX711_get_scale(void)
{
	return SCALE;
}

/* HX711_set_offset - allows user to set offset */
void HX711_set_offset(uint32_t offset)
{
    OFFSET = offset;
}

/* HX711_get_offset - helper function that returns offset */
uint32_t HX711_get_offset(void)
{
	return OFFSET;
}

/* HX711_power_down - turns off the weight sensor */
void HX711_power_down(void)
{
	PD_SCK_SET_LOW;
	PD_SCK_SET_HIGH;
	_delay_us(70);
}

/* HX711_power_up - turns on the weight sensor */
void HX711_power_up(void)
{
	PD_SCK_SET_LOW;
}

/* shiftIn - get the raw data from the sensor */
uint8_t shiftIn(void)
{
    uint8_t value = 0;

    uint8_t i;
    for (i = 0; i < 8; ++i)
    {
        PD_SCK_SET_HIGH;
        value |= DOUT_READ << (7 - i);
        PD_SCK_SET_LOW;
    }
    return value;
}

/* get_weight_sample - returns the smoother, scaled weight value */
uint8_t get_weight_sample()
{
    uint32_t weight = HX711_get_units(10);

    return (uint8_t)(weight & 0xff);

}