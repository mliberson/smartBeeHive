#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

void lcd_init(void);
void lcd_moveto(unsigned char, unsigned char);
void lcd_stringout(char *);
void lcd_clear(void);
void lcd_reset(void);

void lcd_init()
{
    _delay_ms(250);             // Wait 500msec for the LCD to start up
    _delay_ms(250);
    serial_out(0xfe);              // Clear the screen
    serial_out(0x51);
}

/*
  moveto - Move the cursor to the row and column given by the arguments.
  Row is 0 or 1, column is 0 - 15.
*/
void lcd_moveto(unsigned char row, unsigned char col)
{
    char ch;
    switch(row) 
    {
      case 0:
        ch = 0x00;
        break;
      case 1:
        ch = 0x40;
        break;
      case 2:
        ch = 0x14;
        break;
      case 3:
        ch = 0x54;
        break;
      default:
        return;
    }
    ch += col;
    
    serial_out(0xfe);              // Set the cursor position
    serial_out(0x45);
    serial_out(ch);
    _delay_ms(1000);             // Wait 500msec for the LCD to start up
}

void lcd_clear(void)
{
  serial_out(0xfe);
  serial_out(0x51);
}

void lcd_reset(void)
{
  lcd_clear();
  lcd_moveto(0,0);
}


/*
  lcd_stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void lcd_stringout(char *str)
{
    serial_outs(str);              // Output the string
}