#include <avr/interrupt.h>
#include <stdint.h>
#include <avr/io.h>

#include "scheduler.h"
	
volatile uint16_t __tick_ms_ctu;

ISR(TIMER0_COMPA_vect){
	__tick_ms_ctu++;
}