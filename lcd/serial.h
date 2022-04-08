#ifndef SERIAL_H
#define SERIAL_H
    #define FOSC 7372800		// Clock frequency
    #define BAUD 9600              // Baud rate used by the LCD
    #define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
    void serial_init(void);
    void serial_outs(char *);
    void serial_out(char);
    char serial_in(void);
#endif