/*
 * ATmega16_DHT11.c
 *
 * http://www.electronicwings.com
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "../lcd/serial.h"
#include "DHT11.h"

void request(unsigned char in_pin)				/* Microcontroller send start pulse/request */
{
	DDRD |= (1<<in_pin);
	PORTD &= ~(1<<in_pin);	/* set to low pin */
	_delay_ms(20);			/* wait for 20ms */
	PORTD |= (1<<in_pin);	/* set to high pin */
}

void response(unsigned char in_pin)				/* receive response from DHT11 */
{
	DDRD &= ~(1<<in_pin);
	while(PIND & (1<<in_pin));
	while((PIND & (1<<in_pin))==0);
	while(PIND & (1<<in_pin));
}

uint8_t receive_data(unsigned char in_pin)			/* receive data */
{	
	int q;
	uint8_t c=0;
    for (q=0; q<8; q++)
	{
		while((PIND & (1<<in_pin)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(PIND & (1<<in_pin))/* if high pulse is greater than 30ms */
        {
            c = (c<<1)|(0x01);	/* then its logic HIGH */
        }
		else			/* otherwise its logic LOW */
        {
            c = (c<<1);
        }
		while(PIND & (1<<in_pin));
	}
	return c;
}

/* get_temp_humid_sample - returns an array containing the temperature and
	retlative humidity samples just taken.
	Note:
		sample[0] contains the integer of relative humidity
		sample[1] contains the decimal of relative humidity
		sample[2] contains the integer of temperature
		sample[3] contains the decimal of temperature
	Returns: sample if the data came back without errors
			 NULL if the data was returned with errors
*/
char* get_temp_humid_sample(unsigned char in_pin)
{
	char *samples = malloc(sizeof(char) * 4);
	char checksum, calc_checksum = 0;
	request(in_pin);
	response(in_pin);
	int i;
	for(i = 0; i < 4; i++)
	{
		samples[i] = receive_data(in_pin);
		calc_checksum += samples[i];
	}
	checksum = receive_data(in_pin);
	if(calc_checksum != checksum)
	{
		return NULL;
	}
	return samples;
}