#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "hibiscus/hibiscus.h"
#include "hedera/hedera.h"
#include "lib/macros.h"

#define SERVO_BRAD(x) ( SERVO_MIN_US + (x*(SERVO_RANGE>>7)) )

void SETCLK(void) __attribute__((naked)) __attribute__((section(".init3")));

struct mntlst servo_OCRA_mnt[4] = {
  {&PORTE, PE6, SERVO_BRAD(96)},
};

int main(void){
	cli(); _delay_ms(5); /* Begin Setup - no interrupts */
  
  ACM_init();
  servo_init();
  SET(DDRE, PE6);

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
  #elif (F_CPU==1000000UL)
    clock_prescale_set(clock_div_16); // 16/16 = 1 MHz
  #else 
    #error "(SETCLK) F_CPU wont divide! Try 16/8/1MHz"
  #endif
  return;
}
