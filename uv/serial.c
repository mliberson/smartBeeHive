#include <avr/io.h>
#include <util/delay.h>

void serial_init(unsigned short);
void sci_outs(char *);
void serial_out(char);
char serial_in(void);

void serial_init(unsigned short ubrr)
{
    UBRR0 = ubrr;               // set baud
    UCSR0B |= (1 << TXEN0);     // turn on Tx
    UCSR0B |= (1 << RXEN0);     // turn on Rx
    UCSR0C |= (3 << UCSZ00);    // async, no parity, one stop, 8 data
}

// sends out a null-byte terminated string
void sci_outs(char *s)
{
    char ch;
    while ((ch = *s++) != '\0')
    {
        serial_out(ch);
    }
}

// sends out a single character on serial port
void serial_out(char ch)
{
    while ((UCSR0A & (1 << UDRE0)) == 0);
    UDR0 = ch;
}

// reads in a byte from USART0
char serial_in()
{
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}