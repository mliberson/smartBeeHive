#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "..\adc\adc.h"
#include "..\uv\uv.h"
#include "..\lcd\serial.h"
#include "..\lcd\lcd.h"

int main () 
{
    serial_init();        // Initialize the SCI port
    lcd_init();           // Initialize the LCD

    lcd_stringout("lcd working");
    _delay_ms(500);
    lcd_reset();

    unsigned char sample;
    char buff[21];

    while (1) {                 // Loop forever
        sample = get_uv_sample();
        sprintf(buff, "UV Index:%11u", sample);
        lcd_stringout(buff);
        _delay_ms(500);
        lcd_reset();
    }
    return 0;
}