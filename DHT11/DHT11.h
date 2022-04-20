#ifndef DHT11_H
#define DHT11_H
    #define FOSC 7372800		    // Clock frequency
    #define BAUD 9600               // Baud rate used by the LCD
    #define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
    void request(unsigned char);
    void response(unsigned char);
    uint8_t receive_data(unsigned char);
    char* get_temp_humid_sample(unsigned char);
#endif