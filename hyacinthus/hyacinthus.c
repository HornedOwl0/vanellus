#include <stdint.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "hyacinthus.h"
	
static volatile uint16_t volatile_tick_ms_ctu;

ISR(TIMER0_COMPA_vect){
	volatile_tick_ms_ctu++;
}

static void TC0_init(void);

int main(){
  cli();
  
  _delay_ms(5);
  init();
  TC0_init();

  sei();
  for(;;){
    uint16_t now_ms = volatile_tick_ms_ctu;
		for (int8_t i=0; i<task_count; i++){
			if(now_ms - task_lst[i].last_ms >= task_lst[i].interval_ms){
				task_lst[i].last_ms = now_ms;
				task_lst[i].run();
			}
    }
  }
}

static inline void TC0_init(void){
  /* OCR0A value to compare to and clear */
  OCR0A = PRELOAD;
  /* Clear Timer on Compare Match (CTC) mode WGM02:0 = 2 */
  TCCR0A = (2<<WGM00); 
  /* Clear OCR0A match flag */
  TIFR0 = (1<<OCF0A);
  /* Enable OCR0A match interrupt */
  TIMSK0 = (1<<OCIE0A);
  /* CLK = F_CPU / 64 (or 8 @ 1MHz) */
  TCCR0B = (PRESCALER<<CS00); // Push the clock for timer startup
	return;
}

inline uint16_t atomic_get_ms(void){
  uint16_t buf;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		 buf = volatile_tick_ms_ctu;
	}
  return buf;
}
