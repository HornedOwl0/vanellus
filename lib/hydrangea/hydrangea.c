#include <stdint.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "hydrangea.h"
	
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
		#else // Presume 1MHz
			#define PRESCALER (0x2)
		#endif /* Freq 8MHz or 16MHz */ 
	#endif /* PRESCALER */
#else
	#error "(Hydrangea) F_CPU is undefined/unsupported for scheduler"
#endif

static volatile uint16_t volatile_tick_ms_ctu;
ISR(TIMER0_COMPA_vect){
	volatile_tick_ms_ctu++;
}

void task_run(task_t arr[], uint8_t count){
  if (arr!=NULL){
    uint16_t now_ms = volatile_tick_ms_ctu;
    for (int8_t i=0; i<count; i++){
      if(now_ms - arr[i].last_ms >= arr[i].interval_ms){
        arr[i].last_ms = now_ms;
        arr[i].run();
      }
    }
  }
  return;
}

uint16_t hydrangea_tick(void){
  uint16_t buf;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		buf = volatile_tick_ms_ctu;
	}
  return buf;
}

static inline void __attribute__((constructor)) hydrangea_init(){
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
