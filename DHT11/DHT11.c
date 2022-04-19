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

void request()				/* Microcontroller send start pulse/request */
{
	DDRD |= (1<<DHT11_PIN);
	PORTD &= ~(1<<DHT11_PIN);	/* set to low pin */
	_delay_ms(20);			/* wait for 20ms */
	PORTD |= (1<<DHT11_PIN);	/* set to high pin */
}

void response()				/* receive response from DHT11 */
{
	DDRD &= ~(1<<DHT11_PIN);
	while(PIND & (1<<DHT11_PIN));
	while((PIND & (1<<DHT11_PIN))==0);
	while(PIND & (1<<DHT11_PIN));
}

uint8_t receive_data()			/* receive data */
{	
	int q;
	uint8_t c=0;
    for (q=0; q<8; q++)
	{
		while((PIND & (1<<DHT11_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(PIND & (1<<DHT11_PIN))/* if high pulse is greater than 30ms */
        {
            c = (c<<1)|(0x01);	/* then its logic HIGH */
        }
		else			/* otherwise its logic LOW */
        {
            c = (c<<1);
        }
		while(PIND & (1<<DHT11_PIN));
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
char* get_temp_humid_sample()
{
	char *samples = malloc(sizeof(char) * 4);
	char checksum, calc_checksum = 0;
	request();
	response();
	int i;
	for(i = 0; i < 4; i++)
	{
		samples[i] = receive_data();
		calc_checksum += samples[i];
	}
	checksum = receive_data();
	if(calc_checksum != checksum)
	{
		return NULL;
	}
	return samples;
}

/* commented this out to get rid of a "multiple definitions of main" warning when
*	compiling top.c
*/
// int main(void)
// {	
// 	char data[5];
// 	serial_init();
	
//     while(1)
// 	{	
// 		Request();		/* send start pulse */
// 		Response();		/* receive response */
// 		I_RH=Receive_data();	/* store first eight bit in I_RH */
// 		D_RH=Receive_data();	/* store next eight bit in D_RH */
// 		I_Temp=Receive_data();	/* store next eight bit in I_Temp */
// 		D_Temp=Receive_data();	/* store next eight bit in D_Temp */
// 		CheckSum=Receive_data();/* store next eight bit in CheckSum */
//         serial_outs("poop");
		
// 		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
// 		{
// 			serial_outs("Error");
// 		}
		
// 		else
// 		{	
// 			itoa(I_RH,data,10);
// 			serial_outs(data);
// 			serial_outs(".");
			
// 			itoa(D_RH,data,10);
// 			serial_outs(data);
// 			serial_outs("%");

// 			itoa(I_Temp,data,10);
// 			serial_outs(data);
// 			serial_outs(".");
			
// 			itoa(D_Temp,data,10);
// 			serial_outs(data);
// 			serial_outs("C ");
			
// 			itoa(CheckSum,data,10);
// 			serial_outs(data);
// 			serial_outs(" ");
// 		}
				
// 	_delay_ms(2000);
// 	}	
// }