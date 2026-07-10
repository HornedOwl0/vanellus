#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>
#include <avr/io.h>

#include "scheduler.h"
	
static volatile uint16_t elapsed;

ISR(TIMER0_COMPA_vect){
	elapsed += 1;
	return;
}

uint16_t atomic_get_ms(void){
	uint16_t cache;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		 cache = elapsed;
	}
	return cache;
}
