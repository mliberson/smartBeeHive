#ifndef EEPROM_H
#define EEPROM_H
    #define EEPROM_ADDR     0xA0
    extern char rdata[24];
    extern char err_str[80];
    char write_eeprom(char*, int);
    char* read_eeprom(int);
    uint8_t wrprom(uint8_t*, uint16_t, uint16_t);
    uint8_t rdprom(uint8_t*, uint16_t, uint16_t);
#endif