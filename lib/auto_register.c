#ifndef F_CPU
	#define F_CPU 128000
#endif
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "utils.h"

#include "auto_register.h"

void auto_reg_init(const struct auto_reg *restrict pR){
  io_mode(pR->D, OUTPUT);
  io_mode(pR->CLK, OUTPUT);
	return;
}

void auto_reg_write(const struct auto_reg *restrict pR, const uint8_t data){
	{
		int8_t i=8;
		while (i-- > 0) {
			io_write(pR->D, !!(data & _BV(i)));
	    io_write(pR->CLK, HIGH);
			_delay_us(1);
	    io_write(pR->CLK, LOW);
		};
  }
	return;
}