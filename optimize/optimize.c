#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "eeprom.h"
#include "..\data.h"
#include "optimize.h"

/*optimize100 - returns the optimal conditions among 100 adjacent elements by calling optimize10 10 times
    Parameters:
        data_addr -> address of the first Data struct to be retrieved from eeprom
*/
Data optimize100(int data_addr) 
{
    
    unsigned char i;
    for(i=0; i<10; i++)
    {
        dataArray100[i] = optimize10(data_addr);
        data_addr += 10*READ_LEN;
    }

    int maxIdx = maxFinder100();

    return dataArray100[maxIdx];

}

/* optimize10 - returns the optimal conditions among 10 adjacent elements
    Parameters:
        data_addr -> address of the first Data struct to be retrieved from eeprom
*/
Data optimize10(int data_addr)
{
    char eeprom_data[40];

    unsigned char i;
    for(i = 0; i < 10; i++) 
    {
        eeprom_data = read_eeprom(data_addr);
        dataArray10[i] = convert(eeprom_data);
        data_addr += READ_LEN;
    }

    for(i = 0; i < 9; i++)
    {
        dataArray10[i] = average(dataArray10[i],dataArray10[i+1]);
    }

    int maxIdx = maxFinder10();

    return dataArray10[maxIdx];
}

/* average - takes average of Data struct member variables except for weight, 
             calculates difference between new and old weights
    Parameters:
        d1 -> older of the two adjacent Data structs
        d2 -> newer of the two adjacent Data structs
*/
Data average(Data d1, Data d2) 
{
    Data ave;

    ave.uv = (d1.uv+d2.uv)/2;
    ave.temp_in_int = (d1.temp_in_int+d2.temp_in_int)/2;
    ave.temp_in_dec = (d1.temp_in_dec+d2.temp_in_dec)/2;
    ave.temp_out_int = (d1.temp_out_int+d2.temp_out_int)/2;
    ave.demp_out_dec = (d1.demp_out_dec+d2.demp_out_dec)/2;
    ave.hum_in_int = (d1.hum_in_int+d2.hum_in_int)/2;
    ave.hum_in_dec = (d1.hump_in_dec+d2.hum_in_dec)/2;
    ave.hum_out_int = (d1.hum_out_int+d2.hum_out_int)/2;
    ave.hum_out_dec = (d1.hum_out_dec+d2.hum_out_dec)/2;
    ave.weight = d2.weight-d1.weight;

    return ave;
}

Data convert(char* eeprom_data)
{
    Data retD[10];
    unsigned char i,j;
 
    retD.uv = 100*(eeprom_data[0] + '0') + 10*(eeprom_data[1] + '0') + (eeprom_data[2] + '0');
    retD.temp_in_int = 100*(eeprom_data[4] + '0') + 10*(eeprom_data[5] + '0') + (eeprom_data[6] + '0');
    retD.temp_in_dec = 100*(eeprom_data[8] + '0') + 10*(eeprom_data[9] + '0') + (eeprom_data[10] + '0');
    retD.temp_out_int = 100*(eeprom_data[12] + '0') + 10*(eeprom_data[13] + '0') + (eeprom_data[14] + '0');
    retD.temp_out_dec = 100*(eeprom_data[16] + '0') + 10*(eeprom_data[17] + '0') + (eeprom_data[18] + '0');
    retD.hum_in_int = 100*(eeprom_data[20] + '0') + 10*(eeprom_data[21] + '0') + (eeprom_data[22] + '0');
    retD.hum_in_dec = 100*(eeprom_data[24] + '0') + 10*(eeprom_data[25] + '0') + (eeprom_data[26] + '0');
    retD.hum_out_int = 100*(eeprom_data[28] + '0') + 10*(eeprom_data[29] + '0') + (eeprom_data[30] + '0');
    retD.hum_out_dec = 100*(eeprom_data[32] + '0') + 10*(eeprom_data[33] + '0') + (eeprom_data[34] + '0');
    retD.weight = 100*(eeprom_data[36] + '0') + 10*(eeprom_data[37] + '0') + (eeprom_data[38] + '0');
    
}

unsigned char maxFinder100() 
{
    unsigned char maxIdx = 0;

    unsigned char maxDelta = dataArray10[0].weight;

    unsigned char i;
    for(i=1; i<10; i++)
    {
        if(dataArray10[i].weight > maxDelta)
        {
            maxDelta = dataArray10[i].weight;
            maxIdx = i;
        }
    }

    return maxIdx;
}

unsigned char maxFinder10() 
{
    unsigned char maxIdx = 0;

    unsigned char maxDelta = dataArray10[0].weight;

    unsigned char i;
    for(i=1; i<9; i++)
    {
        if(dataArray10[i].weight > maxDelta)
        {
            maxDelta = dataArray10[i].weight;
            maxIdx = i;
        }
    }

    return maxIdx;
}