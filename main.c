#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <util/delay.h>
#include <util/atomic.h>

#define ACM_ALL_REQUESTS
#include "hibiscus/hibiscus.h"
#include "lib/macros.h"

void SETCLK(void) __attribute__((naked)) __attribute__((section(".init3")));

int main(void){
	cli(); _delay_ms(5); /* Begin Setup - no interrupts */
  
  ACM_init();

  SET(DDRD, PD5);
  SET(DDRB, PB0);
  CLR(PORTD, PD5);

  wdt_enable(WDTO_2S);

  static char buf[16] = {0};

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    _delay_ms(100);
    SET(PINB, PB0);
    if ( ACM_available() ){
      ACM_gets(buf, ARRAY_SIZE(buf));
      ACM_puts(buf);
    }
  }

	return 0;
}

void SETCLK(void){
  #if (F_CPU==16000000UL)
    clock_prescale_set(clock_div_1); // 16/1 = 16 MHz
  #elif (F_CPU==8000000UL)
    clock_prescale_set(clock_div_2); // 16/2 = 8 MHz
  #elif (F_CPU==4000000UL)
    clock_prescale_set(clock_div_4); // 16/4 = 4 MHz
  #elif (F_CPU==2000000UL)
    clock_prescale_set(clock_div_8); // 16/8 = 2 MHz
  #elif (F_CPU==1000000UL)
    clock_prescale_set(clock_div_16); // 16/16 = 1 MHz
  #endif
  return;
}