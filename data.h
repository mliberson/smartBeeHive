#ifndef DATA_H
#define DATA_H
    struct Data {
        unsigned char uv;               // UV index
        unsigned char temp_in_int;      // Internal tempmerature integer
        unsigned char temp_in_dec;      // Internal temperature decimal
        unsigned char temp_out_int;     // External temperature integer
        unsigned char temp_out_dec;     // External temperature decimal
        unsigned char hum_in_int;       // Internal fumidity integer
        unsigned char hum_in_dec;       // Internal humidity decimal
        unsigned char hum_out_int;      // External humidity integer
        unsigned char hum_out_dec;      // External humidity decimal
        unsigned char weight;           // Weight
    };
#endif