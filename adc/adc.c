/**************************************
 * Adapted from USC EE 109 Lab 6 Code
 *************************************/

#include <avr/io.h>
#include "adc.h"

/* adc_init - Initializes the adc module */
void adc_init(void)
{
    ADMUX |= (1 << REFS0);          // Set high voltage reference = VCC
    ADMUX |= (1 << ADLAR);          // 8-bit result (could do 10-bit by setting this to 0)
    ADCSRA |= (ADC_PRSC << ADPS0);  // prescalar = 128
    ADCSRA |= (1 << ADEN);          // Enable the ADC
}

/* adc_sample - allows the user to poll for an adc value from a specified channel */
unsigned char adc_sample(unsigned char channel)
{
    ADMUX &= ~ADC_MASK;                         // Clear out mux bits
    ADMUX |= ((channel & ADC_MASK) << MUX0);    // Declare which channel to read from
    // ADCSRA |= (1 << ADIE);                      If we want to use interrupts
    ADCSRA |= (1 << ADSC);                      // Start conversion
    while (ADCSRA & (1 << ADSC));               // Wait for conversion to finish
    return ADCH;                                // Return result
}

/* Could use ISR if conversions are using up too much time */
// ISR(ADC_vect)
// {
//     // Set global variable to ADCH
// }