/*************************************************************
*       at328-6.c - Demonstrate interface to a serial LCD display
*
*       This program will print a message on an LCD display
*       using a serial interface.  It is designed to work with a
*       Matrix Orbital LK204-25 using an RS-232 interface.
*
* Revision History
* Date     Author      Description
* 11/07/07 A. Weber    First Release
* 02/26/08 A. Weber    Code cleanups
* 03/03/08 A. Weber    More code cleanups
* 04/22/08 A. Weber    Added "one" variable to make warning go away
* 04/11/11 A. Weber    Adapted for ATmega168
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"
#include "lcd.h"

char str1[] = "1234567890";
char str2[] = ">> USC EE459L <<";
char str3[] = ">> at328-6.c <<<";
char str4[] = "-- April 11, 2011 --";

#define FOSC 7372800		// Clock frequency
#define BAUD 9600              // Baud rate used by the LCD
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

int main(void) {

    serial_init(MYUBRR);                 // Initialize the SCI port
    
    lcd_init();                 // Initialize the LCD

    lcd_moveto(0,0);
    lcd_stringout(str1);
    lcd_moveto(1,1);
    lcd_stringout(str2);
    lcd_moveto(2,2);
    lcd_stringout(str3);
    lcd_moveto(3,3);
    lcd_stringout(str4);

    while (1) {                 // Loop forever
    }

    return 0;   /* never reached */
}