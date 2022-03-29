/*************************************************************
*       at328-0.c - Demonstrate simple I/O functions of ATmega328
*
*       Program loops turning PC0 on and off as fast as possible.
*
* The program should generate code in the loop consisting of
*   LOOP:   SBI  PORTC,0        (2 cycles)
*           CBI  PORTC,0        (2 cycles)
*           RJMP LOOP           (2 cycles)
*
* PC0 will be low for 4 / XTAL freq
* PC0 will be high for 2 / XTAL freq
* A 9.8304MHz clock gives a loop period of about 600 nanoseconds.
*
* Revision History
* Date     Author      Description
* 09/14/12 A. Weber    Initial Release
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <avr/delay.h>

int main(void)
{
    DDRC |= 1 << DDC0;          // Set PORTC bit 0 for output
    DDRC |= 1 << DDC1;          // Set PORTC bit 1 for output
    DDRC |= 1 << DDC2;          // Set PORTC bit 2 for output
    DDRC |= 1 << DDC3;          // Set PORTC bit 3 for output
    DDRC |= 1 << DDC4;          // Set PORTC bit 4 for output
    DDRC |= 1 << DDC5;          // Set PORTC bit 5 for output

    DDRC |= 1 << DDB0;          // Set PORTB bit 0 for output
    DDRC |= 1 << DDB1;          // Set PORTB bit 1 for output
    DDRC |= 1 << DDB2;          // Set PORTB bit 2 for output
    DDRC |= 1 << DDB3;          // Set PORTB bit 3 for output
    DDRC |= 1 << DDB4;          // Set PORTB bit 4 for output
    DDRC |= 1 << DDB5;          // Set PORTB bit 5 for output
    DDRC |= 1 << DDB7;          // Set PORTB bit 7 for output

    DDRD |= 1 << DDD0;          // Set PORTD bit 0 for output
    DDRD |= 1 << DDD1;          // Set PORTD bit 1 for output
    DDRD |= 1 << DDD2;          // Set PORTD bit 2 for output
    DDRD |= 1 << DDD3;          // Set PORTD bit 3 for output
    DDRD |= 1 << DDD4;          // Set PORTD bit 4 for output
    DDRD |= 1 << DDD5;          // Set PORTD bit 5 for output
    DDRD |= 1 << DDD6;          // Set PORTD bit 6 for output
    DDRD |= 1 << DDD7;          // Set PORTD bit 7 for output

    while (1) {
    
	PORTC |= 1 << PC0;      // Set PC0 to a 1
	PORTC &= ~(1 << PC0);   // Set PC0 to a 0
    PORTC |= 1 << PC2;      // Set PC2 to a 1
	PORTC &= ~(1 << PC2);   // Set PC2 to a 0
    PORTC |= 1 << PC4;      // Set PC4 to a 1
	PORTC &= ~(1 << PC4);   // Set PC4 to a 0
    PORTD |= 1 << PD0;      // Set PD0 to a 1
	PORTD &= ~(1 << PD0);   // Set PD0 to a 0

    

    }

    return 0;   /* never reached */
}