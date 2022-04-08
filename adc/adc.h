#ifndef _ADC_H
#define _ADC_H
    #define ADC_MASK    0b1111
    #define ADC_PRSC    0b111   // Divide by 128
    void adc_init(void);
    unsigned char adc_sample (unsigned char);
    // ISR(ADC_vect);
#endif