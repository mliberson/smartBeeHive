/**************************************
 * Adapted from USC EE 109 Lab 6 Code
 *************************************/

#ifndef _ADC_H
#define _ADC_H

void adc_init(void);
unsigned char adc_sample (unsigned char);
// ISR(ADC_vect);

#endif