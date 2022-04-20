#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "..\i2c\i2c.h"
#include "eeprom.h"
#include "..\lcd\lcd.h"

char rdata[24], err_str[80];

char write_eeprom(char* str, int data_addr)
{
    char status;
    
    status = wrprom((uint8_t *) str, 21, data_addr);
    return status;
}

char* read_eeprom(int data_addr)
{
    char status;

    memset(rdata, 0, 24);
    status = rdprom((unsigned char*) rdata, 24, data_addr);
    if(status == 0)
    {
        return rdata;
    }
    sprintf(err_str, "Status = 0x%2.2x", status);
    return err_str;
}

uint8_t wrprom(uint8_t *p, uint16_t n, uint16_t a)
{
    uint16_t maxw;              // Maximum bytes to write in the page
    uint8_t status;
    uint8_t adata[2];           // Array to hold the address

    while (n > 0) {
        adata[0] = a >> 8;      // Put EEPROM address in adata buffer,
        adata[1] = a & 0xff;    // MSB first, LSB second
        // We can write up to the next 64 byte boundary,
        // but no more than is left to write
        maxw = 64 - (a % 64);   // Max for this page
        if (n < maxw)
            maxw = n;           // Number left to write in page
        status = i2c_io(EEPROM_ADDR, adata, 2, p, maxw, NULL, 0);
        if (status != 0)
            return(status);
        _delay_ms(5);           // Wait 5ms for EEPROM to write
        p += maxw;              // Increment array address
        a += maxw;              // Increment address
        n -= maxw;              // Decrement byte count
    }
    return(0);
}

uint8_t rdprom(uint8_t *p, uint16_t n, uint16_t a)
{
    uint8_t status;
    uint8_t adata[2];           // Array to hold the address

    adata[0] = a >> 8;          // Put EEPROM address in adata buffer,
    adata[1] = a & 0xff;        // MSB first, LSB second

    status = i2c_io(EEPROM_ADDR, adata, 2, NULL, 0, p, n);
    return(status);
}