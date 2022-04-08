#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "uv.h"
#include "..\adc\adc.h"

unsigned char get_uv_sample()
{
    return adc_sample(UV_CHAN);        // get sample
}