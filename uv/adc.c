/**************************************
 * Adapted from USC EE 109 Lab 6 Code
 *************************************/

#include <avr/io.h>
#include "adc.h"

#define ADC_MASK    0b1111
#define ADC_PRSC    0b111   // Divide by 128

void adc_init(void)
{
    ADMUX |= (1 << REFS0);          // Set high voltage reference = VCC
    ADMUX |= (1 << ADLAR);          // 8-bit result (could do 10-bit by setting this to 0)
    ADCSRA |= (ADC_PRSC << ADPS0);  // prescalar = 128
    ADCSRA |= (1 << ADEN);          // Enable the ADC
}

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