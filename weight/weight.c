#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "adc.h"
#include "serial.h"

#define FOSC 7372800		// Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

// #include "lcd.h"

#define PD_SCK 0    // 14 = PB0
#define DOUT 7      // 13 = PD7

int main (void)
{

    adc_init();  // Initialize ADC
    serial_init(MYUBRR);
    DDRB |= (1 << PD_SCK);

    unsigned int val = 0, i;
    char buff[20];
    
    while(1)
    {
        val = 0;
        PORTB &= ~(1 << PD_SCK);
        while (PIND & (1 << DOUT));
        _delay_us(1);
        // Get reading
        for(i = 0; i < 24; i++)
        {
            PORTB |= (1 << PD_SCK);
            _delay_us(1);
            PORTB &= ~(1 << PD_SCK);
            if (PIND & (1 << DOUT))
            {
                val |= (1 << (23 - i));
            }
            _delay_us(1);
        }
        // set gain
        for(i = 0; i < 2; i++)
        {
            PORTB |= (1 << PD_SCK);
            _delay_us(1);
            PORTB &= ~(1 << PD_SCK);
            _delay_us(1);
        }
        snprintf(buff, 20, " %u ", val);
        serial_outs(buff);
        _delay_ms(3000);
    }

    return 0;
}