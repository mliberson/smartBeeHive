#include "..\eeprom\eeprom.h"
#include "..\data.h"

#ifndef OPTIMIZE_H
#define OPTIMIZE_H
    extern Data optimal_conditions;
    Data dataArray10[10];
    Data dataArray100[10];
    Data optimize10(int);
    Data optimize100(int);
    Data average(Data,Data);
    unsigned char maxFinder10();
    unsigned char maxFinder100();
#endif