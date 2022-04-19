#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"
#include "lcd.h"

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
      /* Should never get here */
      default:
        ch = 0xFF;
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
  serial_out(0x51)

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

/*
  lcd_screen - Takes a "sz"x20 char array and prints the contents
  to the screen.
*/
void lcd_screen(char *str_arr[], unsigned char sz)
{
  int i;
  for(i = 0; i < sz; i++)
  {
    lcd_moveto(i, 0);
    lcd_stringout(str_arr[i]);

  }
}

void lcd_clear()
{
    serial_out(0xfe);              // Clear the screen
    serial_out(0x51);
}