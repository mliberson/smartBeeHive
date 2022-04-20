#ifndef I2C_H
#define I2C_H
extern void i2c_init(void);
extern uint8_t i2c_io(uint8_t, uint8_t *, uint16_t,
		      uint8_t *, uint16_t, uint8_t *, uint16_t);
#endif