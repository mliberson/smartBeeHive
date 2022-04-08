#ifndef ROTARY_H
#define ROTARY_H
    #define ROT1    PC5     // first pin of rotary encoder
    #define ROT2    PC1     // second pin of rotary encoder
    volatile unsigned char rot_new_state, rot_old_state;
    volatile unsigned char rot_changed;
    unsigned char rot_up;		// Count to display
    void rot_encoder_init(void);
    void check_encoder(void);
    void init_pcinterrupt(void);
#endif