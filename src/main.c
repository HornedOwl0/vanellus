#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>
#include <util/twi.h>

#include <hibiscus.h> // this is special and should always be included

#include "hydrangea.h"
#include "hyacinthus.h"
#include "hedera.h"

#include "macros.h"
#include "config.h"

/* Non-Task Function Prototypes */

extern uint8_t framebuf[FRAMEBUF_SIZE];

extern int8_t framebuf_drawpx(uint8_t x, uint8_t y, int8_t val);

extern int8_t framebuf_drawsprite(uint8_t x, uint8_t y, const __memx uint8_t* spr);

extern int8_t framebuf_putc(uint8_t x, uint8_t y, char c);

extern int8_t framebuf_puts(uint8_t x, uint8_t y, const __memx char *s);

extern void oled_writecmd(const __memx uint8_t *buf, uint16_t len);

extern void oled_writescr(const __memx uint8_t *buf, uint16_t len);

/* Task Function Prototypes */

void blink(void);
void display(void);
void display_white(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  TASK_DEF(&blink, 100, 35),
  TASK_DEF(&display, 200, 65),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  framebuf_drawpx(0, 0, 1);
  framebuf_drawpx(127, 0, 1);
  framebuf_drawpx(0, 31, 1);
  framebuf_drawpx(127, 31, 1);

  framebuf_puts(16, 12, "Hello World!");

  TWI_start(60, TW_WRITE);
  if (TWI_stop()){
    gpio_t LED_STATIC = {&PIND, &DDRD, &PORTD, PD5};
    gpio_output(&LED_STATIC);
    gpio_clr(&LED_STATIC);
  }

  wdt_enable(WDTO_2S);

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    task_run(task_main, ARRAY_SIZE(task_main));
  }

	return 0;
}

void blink(void){
  static const __flash gpio_t LED_STATIC = {&PORTD, &DDRD, &PORTD, PD5};
  static const __flash gpio_t LED_BLINK = {&PINB, &DDRB, &PORTB, PB0};

  gpio_output(&LED_STATIC);
  gpio_set(&LED_STATIC);

  gpio_output(&LED_BLINK);
  gpio_tog(&LED_BLINK);
  return;
}

void display(void){
  oled_writescr(framebuf, FRAMEBUF_SIZE);
  return;
}
