/*
 * ATmega16_DHT11.c
 *
 * http://www.electronicwings.com
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "serial.h"
#define DHT11_PIN 2

#define FOSC 7372800		// Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;

void Request()				/* Microcontroller send start pulse/request */
{
	DDRD |= (1<<DHT11_PIN);
	PORTD &= ~(1<<DHT11_PIN);	/* set to low pin */
	_delay_ms(20);			/* wait for 20ms */
	PORTD |= (1<<DHT11_PIN);	/* set to high pin */
}

void Response()				/* receive response from DHT11 */
{
	DDRD &= ~(1<<DHT11_PIN);
	while(PIND & (1<<DHT11_PIN));
	while((PIND & (1<<DHT11_PIN))==0);
	while(PIND & (1<<DHT11_PIN));
}

uint8_t Receive_data()			/* receive data */
{	
	int q;
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

int main(void)
{	
	char data[5];
	serial_init(MYUBRR);
	
    while(1)
	{	
		Request();		/* send start pulse */
		Response();		/* receive response */
		I_RH=Receive_data();	/* store first eight bit in I_RH */
		D_RH=Receive_data();	/* store next eight bit in D_RH */
		I_Temp=Receive_data();	/* store next eight bit in I_Temp */
		D_Temp=Receive_data();	/* store next eight bit in D_Temp */
		CheckSum=Receive_data();/* store next eight bit in CheckSum */
        sci_outs("poop");
		
		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
		{
			sci_outs("Error");
		}
		
		else
		{	
			itoa(I_RH,data,10);
			sci_outs(data);
			sci_outs(".");
			
			itoa(D_RH,data,10);
			sci_outs(data);
			sci_outs("%");

			itoa(I_Temp,data,10);
			sci_outs(data);
			sci_outs(".");
			
			itoa(D_Temp,data,10);
			sci_outs(data);
			sci_outs("C ");
			
			itoa(CheckSum,data,10);
			sci_outs(data);
			sci_outs(" ");
		}
				
	_delay_ms(2000);
	}	
}