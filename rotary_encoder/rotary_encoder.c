/*********************************************************************
*
*       Adapted from the EE109 Rotary Encoder lab
*       
*       Gives you access to:
*       - 'rot_new_state' = current state of encoder
*       - 'rot_up' = flag indicating if state went "up" or "down"
*       - 'rot_changed' = flag indicating if state has changed
*
*********************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "rotary_encoder.h"

void rot_encoder_init(void)
{
    unsigned char bits, a, b;

    PORTC |= (1 << ROT1 | 1 << ROT2); // Enable pull-ups on ROT1, ROT2

    init_pcinterrupt();          // Initialize pin change intterupt for rotary encoder

    // Determine the intial state
    bits = PINC;
    a = bits & (1 << ROT1);
    b = bits & (1 << ROT2);

    if (!b && !a)
	rot_old_state = 0;
    else if (!b && a)
	rot_old_state = 1;
    else if (b && !a)
	rot_old_state = 2;
    else
	rot_old_state = 3;

    rot_new_state = rot_old_state;
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

    if (rot_old_state == 0) {
	if (a) {
	    rot_new_state = 1;
	    rot_up = 1;
	}
	else if (b) {
	    rot_new_state = 2;
	    rot_up = 0;
	}
    }
    else if (rot_old_state == 1) {
	if (!a) {
	    rot_new_state = 0;
	    rot_up = 0;
	}
	else if (b) {
	    rot_new_state = 3;
	    rot_up = 1;
	}
    }
    else if (rot_old_state == 2) {
	if (a) {
	    rot_new_state = 3;
	    rot_up = 0;
	}
	else if (!b) {
	    rot_new_state = 0;
	    rot_up = 1;
	}
    }
    else {   // old_state = 3
	if (!a) {
	    rot_new_state = 2;
	    rot_up = 1;
	}
	else if (!b) {
	    rot_new_state = 1;
	    rot_up = 0;
	}
    }

    if (rot_new_state != rot_old_state)
	{
		rot_changed = 1;
		rot_old_state = rot_new_state;
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
void init_pcinterrupt()
{
	PCICR |= (1 << PCIE1);  // Enable PCINT on Port C
	PCMSK1 |= (1 << ROT1 | 1 << ROT2); // Interrupt on ROT1, ROT2
	sei();                  // Enable interrupts
}

ISR(PCINT1_vect)
{
    check_encoder();
}
