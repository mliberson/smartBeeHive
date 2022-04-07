/*********************************************************************
*
*       Adapted from the EE109 Rotary Encoder lab
*
*********************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#define ROT1    PC5     // first pin of rotary encoder
#define ROT2    PC1     // second pin of rotary encoder

void check_encoder(void);
void init_pcinterrupt(void);

volatile unsigned char new_state, old_state;
volatile unsigned char changed;
volatile int count = 0;		// Count to display

int main(void) {
    unsigned char bits, a, b;

    PORTC |= (1 << ROT1 | 1 << ROT2); // Enable pull-ups on ROT1, ROT2

    init_pcinterupt();          // Initialize pin change intterupt for rotary encoder
    lcd_init();                 // Initialize the LCD

    // Determine the intial state
    bits = PINC;
    a = bits & (1 << ROT1);
    b = bits & (1 << ROT2);

    if (!b && !a)
	old_state = 0;
    else if (!b && a)
	old_state = 1;
    else if (b && !a)
	old_state = 2;
    else
	old_state = 3;

    new_state = old_state;

    while (1)
    {
        if (changed)
        {
	        changed = 0;
            // `count` tells you current count
            // `new_state` tells you current state
        }
    }
}

/*
  check_encoder - Checks the encoder inputs to determine which
  state we go to next and adjusts the count value.

  Called from the ISR in interrupt mode, or directly from the main
  loop in polling mode.
*/
void check_encoder(void)
{
    unsigned char bits, a, b;

    bits = PINC;		// Read the two encoder input at the same time
    a = bits & (1 << ROT1);
    b = bits & (1 << ROT2);

    if (old_state == 0) {
	if (a) {
	    new_state = 1;
	    count++;
	}
	else if (b) {
	    new_state = 2;
	    count--;
	}
    }
    else if (old_state == 1) {
	if (!a) {
	    new_state = 0;
	    count--;
	}
	else if (b) {
	    new_state = 3;
	    count++;
	}
    }
    else if (old_state == 2) {
	if (a) {
	    new_state = 3;
	    count--;
	}
	else if (!b) {
	    new_state = 0;
	    count++;
	}
    }
    else {   // old_state = 3
	if (!a) {
	    new_state = 2;
	    count++;
	}
	else if (!b) {
	    new_state = 1;
	    count--;
	}
    }

    if (new_state != old_state) {
	changed = 1;
	old_state = new_state;
    }
}

/*
    init_pcintterupt - initializes the pin change interrupt for the desired port

    This function may change depending on which port (B, C, D?) you intend to use
    Currently setup for port C
    For port B: use PCIE0 and PCMSK0
    For port C: use PCIE1 and PCMSK1
    For port D: use PCIE2 and PCMSK2
*/
void init_pcinterrupt(void)
{
	PCICR |= (1 << PCIE1);  // Enable PCINT on Port C
	PCMSK1 |= (1 << ROT1 | 1 << ROT2); // Interrupt on ROT1, ROT2
	sei();                  // Enable interrupts
}

ISR(PCINT1_vect)
{
    check_encoder();
}
