#include "dht.h"

void dht::init_timer1(unsigned short m)
{
  tm = 0;
  TCCR1B |= (1 << WGM12);
  TIMSK1 |= (1 << OCIE1A);
  OCR1A = m;
  TCCR1B |= (1 << CS12);
}

int dht::read(void)
{
	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (int i=0; i< 5; i++) bits[i] = 0;

	  // REQUEST SAMPLE
	DDRD |= 1 << DDD2;
  PORTD &= ~(1 << PD2); 
	_delay_ms(18);
	PORTD |= 1 << PD2;
	_delay_us(40);
	DDRD &= ~(1 << DDD2);

	// ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = 10000;
	while((PORTD & PD2) == 0)
		if (loopCnt-- == 0) return DHT_ERROR_TIMEOUT;

	loopCnt = 10000;
	while((PORTD & PD2))
		if (loopCnt-- == 0) return DHT_ERROR_TIMEOUT;

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	for (int i=0; i<40; i++)
	{
		loopCnt = 10000;
		while((PORTD & PD2) == 0)
			if (loopCnt-- == 0) return DHT_ERROR_TIMEOUT;

		init_timer1(1152);

		loopCnt = 10000;
		while((PORTD & PD2))
			if (loopCnt-- == 0) return DHT_ERROR_TIMEOUT;

		if (tm >= 1) bits[idx] |= (1 << cnt);
		if (cnt == 0)   // next byte?
		{
			cnt = 7;    // restart at MSB
			idx++;      // next byte!
		}
		else cnt--;
	}

	// WRITE TO RIGHT VARS
        // as bits[1] and bits[3] are allways zero they are omitted in formulas.
	humidity    = bits[0]; 
	temperature = bits[2]; 

	uint8_t sum = bits[0] + bits[2];  

	if (bits[4] != sum) return DHT_ERROR_CHECKSUM;
	return DHT_OK;
}
ISR(TIMER1_COMPA_vect)
{
  // increments every 40ms
  tm++;
}