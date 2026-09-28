#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "hibiscus/hibiscus.h"
#include "lib/macros.h"

int main(void){
	cli(); /* Begin Setup - no interrupts */
  
  _delay_ms(20);

  SET(DDRD, PD5);
	SET(DDRB, PB0);
  SET(PORTB, PB0);
  CLR(PORTD, PD5);

  ACM_init();

  wdt_enable(WDTO_2S);

  static char buf[128] = {0};

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
