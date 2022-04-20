/* Add Header Files */

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"
#include "HX711.h"
#include "lcd.h"
/* Add Function Prototypes */

//set stream pointer

/* Add Variables */
uint8_t  current_weight_128;
char buff[20];

int main(void)
{
    /// Setup
    serial_init(MYUBRR);
    lcd_init();
    sei();
    HX711_init(128);
    _delay_ms(500);

    /// Main Loop
    while(1)
    {   
        current_weight_128 = get_weight_sample();
        //current_weight_128 = current_weight_128/calibration_128;
   
        sprintf(buff,"%u", current_weight_128);
        lcd_reset();
        lcd_stringout(buff);
        _delay_ms(500);

    }
}