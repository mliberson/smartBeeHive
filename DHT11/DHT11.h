#ifndef DHT11_H
#define DHT11_H
    #define DHT11_PIN 2
    #define FOSC 7372800		    // Clock frequency
    #define BAUD 9600               // Baud rate used by the LCD
    #define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
    void request(void);
    void response(void);
    uint8_t receive_data(void);
    char* get_temp_humid_sample(void);
#endif