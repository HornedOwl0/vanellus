#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>

#define ACM_ALL_REQUESTS
#include "hibiscus/hibiscus.h"
#include "lib/macros.h"

int main(void){
  sei(); // ACN init -- Allow Interrupts

  SET(DDRD, PD5);
  SET(DDRB, PB0);
  CLR(PORTD, PD5);

  _delay_ms(20);
  ACM_init();
  _delay_ms(100);

	cli(); /* Begin Setup - no interrupts */

  wdt_enable(WDTO_2S);

  static char buf[16] = {0};

	sei(); /* End Setup - all interrupts */

  _delay_ms(1000);

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
