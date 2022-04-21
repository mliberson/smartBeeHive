#ifndef OPTIMIZE_H
#define OPTIMIZE_H
    struct Data dataArray10[10];
    struct Data dataArray100[10];
    struct Data optimize10(int);
    struct Data optimize100(int);
    struct Data average(struct Data,struct Data);
    struct Data convert(char *);
    unsigned char maxFinder10();
    unsigned char maxFinder100();
#endif