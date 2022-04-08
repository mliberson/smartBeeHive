#ifndef LCD_H
#define LCD_H
    void lcd_init(void);
    void lcd_moveto(unsigned char, unsigned char);
    void lcd_stringout(char *);
    void lcd_screen(char **str_arr, unsigned char sz);
#endif