/* Add Header Files */

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"
#include "HX711.h"
/* Add Function Prototypes */

//set stream pointer

/* Add Variables */
unsigned int  current_weight_128;
unsigned int weight;
char buff[20];

/* Add Constants here */

/* Add function Definations here */


int main(void)
{
    /// Setup
    unsigned int calibration_128 = 120;
    serial_init(MYUBRR);
    sei();
    HX711_init(128);
    _delay_ms(500);

    /// Main Loop
    while(1)
    {   
        current_weight_128 = HX711_read_average(10);
        current_weight_128 = current_weight_128/calibration_128;
   
        sprintf(buff,"%u", current_weight_128);
        sci_outs(buff);
        sci_outs(" , ");
        _delay_ms(500);

    }
}