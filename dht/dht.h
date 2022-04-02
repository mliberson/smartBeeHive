#ifndef _DHT_DHT_H_
#define _DHT_DHT_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DHT_OK				0
#define DHT_ERROR_CHECKSUM	-1
#define DHT_ERROR_TIMEOUT	-2

#define MIN_INTERVAL 2000 /**< min interval value */

extern class dht
{
public:
  int read(void);
	int humidity;
	int temperature;
  volatile unsigned tm = 0;
private:
  void init_timer1(unsigned short m);
};
#endif