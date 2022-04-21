#ifndef EEPROM_H
#define EEPROM_H
    #define EEPROM_ADDR     0xA0
    #define READ_LEN        40
    extern char rdata[READ_LEN];
    extern char err_str[80];
    char write_eeprom(char*, int);
    char* read_eeprom(int, unsigned char);
    uint8_t wrprom(uint8_t*, uint16_t, uint16_t);
    uint8_t rdprom(uint8_t*, uint16_t, uint16_t);
#endif