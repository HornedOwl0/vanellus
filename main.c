#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "hibiscus/hibiscus.h"
#include "lib/macros.h"

static struct {
  const volatile uint8_t *pin;
  const uint8_t mask;
  const int16_t weight;
} sensor_lst[] = {
  {&PINF, PF7, -1}, // OUT5
  {&PINF, PF6, -1}, // OUT4
  {&PINB, PB1, 0}, // OUT3
  {&PINF, PF5, 1}, // OUT2
  {&PINF, PF4, 1}, // OUT1
};

void SETCLK(void) __attribute__((naked)) __attribute__((section(".init3")));

int main(void){
	cli(); _delay_ms(5); /* Begin Setup - no interrupts */
  
  ACM_init();

  CLRBM(DDRF, (1<<PF7)|(1<<PF6)|(1<<PF5)|(1<<PF4) ); // OUT5/4 - OUT2/1
  CLR(DDRB, PB1); // OUT3

  SET(DDRD, PD5);
  SET(DDRB, PB0);
  CLR(PORTD, PD5);

  /* TC1 Setup - set registers */
  TCCR1A = (1<<COM1A1)|(1<<COM1B1); // Non-Inverted Mode;
  TCCR1B = (1<<WGM13); // Phase and Frequency Correct ICR1 - Mode 8;
  /* 1MHz frequency */
  ICR1 = 256; // set period

  OCR1A = 255U;
  OCR1B = 255U;

  DDRB |= (1<<PB5)|(1<<PB6);

  TCCR1B |= (0x01<<CS10); // Push the clock prescaler for timer startup
  /* end TC1 setup*/

  wdt_enable(WDTO_2S);

  static char buf[16];
  static int16_t result;
  (void)result;

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    _delay_ms(100);
    SET(PINB, PB0);
    ACM_puts("\r\n");

    memset(buf, '\0', ARRAY_SIZE(buf));

    for (int8_t i=0; i<ARRAY_SIZE(sensor_lst); i++){
      buf[i] = '0' + GET(*(sensor_lst[i].pin), sensor_lst[i].mask);
    }

    ACM_puts(buf);
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
