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

#define UV_CHAN 5 // A5

int main (void)
{

    adc_init();  // Initialize ADC
    serial_init(MYUBRR);

    unsigned char uv_sample;
    
    while(1)
    {
        uv_sample = adc_sample(UV_CHAN);        // get sample
        char buff[20];
        snprintf(buff, 20, "%d", uv_sample);
        sci_outs(buff);                      // output sample to computer
        _delay_ms(500);
    }

    return 0;
}