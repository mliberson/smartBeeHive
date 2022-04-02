#ifndef _DHT_H_
#define _DHT_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DHT_OK				0
#define DHT_ERROR_CHECKSUM	-1
#define DHT_ERROR_TIMEOUT	-2

#define MIN_INTERVAL 2000 /**< min interval value */

struct dht
{
  int humidity;
	int temperature;
  int (*read)(struct dht *this);
  void (*init_timer1)(struct dht *this,int m);
  volatile unsigned tm;
};

#endif