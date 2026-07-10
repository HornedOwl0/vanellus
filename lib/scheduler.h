#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdint.h>
#include <avr/io.h>

#ifdef F_CPU 
	#ifndef PRELOAD
		#if ( F_CPU == 16000000UL )
			#define PRELOAD ( 249U )
		#else
			#define PRELOAD ( 124U )
		#endif /* Freq 16 MHz */
	#endif /* PRELOAD */
	
	#ifndef PRESCALER
		#if ( ( F_CPU == 8000000UL ) || ( F_CPU == 16000000UL ) )
			#define PRESCALER (0x3)
		#else 
			#define PRESCALER (0x2)
		#endif /* Freq 8MHz or 16MHz */ 
	#endif /* PRESCALER */
#else
	#error "F_CPU is undefined for scheduler"
#endif

struct scheduler_task {
	void (*run)(void);
	uint16_t last_ms;
	const uint16_t interval_ms;
};

static inline void scheduler_init(void){
	/* Disable global interrupts - Enter Atomic Section */
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		/* Reset registers */
		TCCR0A = 0;
		TCCR0B = 0;
		/* OCR0A value to compare to and clear */
		OCR0A = PRELOAD;
		/* Clear Timer on Compare Match (CTC) mode WGM02:0 = 2 */
		TCCR0A |= (1<<WGM01); 
		/* Clear OCR0A match flag */
		TIFR0 = (1<<OCF0A);
		/* Enable OCR0A match interrupt */
		TIMSK0 = (1<<OCIE0A);
		/* CLK = F_CPU / 64 (or 8 @ 1MHz) */
		TCCR0B |= (PRESCALER<<CS00); // Push the clock for timer startup
	}
	/* Exit Atomic Section - Enable global interrupts */
	return;
}

uint16_t atomic_get_ms(void);

#endif /* SCHEDULER_H */