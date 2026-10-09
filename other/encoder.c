#define F_CPU 16000000UL

#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "../lib/macros.h"

static volatile uint8_t encoder_ctu = 0;
static volatile uint8_t prev_PINB = 0;

#define PINB_RISING(p) ( (PINB&(1<<p)) && !(prev_PINB&(1<<p)) )
#define PINB_FALLING(p) ( !(PINB&(1<<p)) && (prev_PINB&(1<<p)) )

ISR(PCINT0_vect){
	if ( PINB_RISING(PB1) ){
		if ( GET(PINB, PB2) )
			encoder_ctu++;
		else
			encoder_ctu--;
	}
	if ( PINB_FALLING(PB3) )
		SET(PINB, PB4);
	prev_PINB = PINB;
}

int main(void){
	cli(); /* Begin Setup - no interrupts */
	
	SET(PCICR, PCIE0); 
	PCMSK0 = (1<<PCINT1)|(1<<PCINT3);
	
	SET(DDRB, PB0);
	SET(DDRB, PB4);

	wdt_enable(WDTO_2S);
	
	sei(); /* End Setup - all interrupts */
	for(;;){
	    wdt_reset();
	    _delay_ms(500);
	    TOG(PORTB, PB0);
  	}
	return 0;
}
