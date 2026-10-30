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
#include <util/twi.h>

#include <hibiscus.h> // this is special and should always be included

#include "hydrangea.h"
#include "hyacinthus.h"
#include "hedera.h"

#include "macros.h"

static uint8_t framebuf[512] = {0};
void framebuf_drawpixel(const uint8_t x, const uint8_t y){
  return;
}
/* Non-Task Function Prototypes */

/* Task Function Prototypes */

void blink(void);
void display(void);
void display_white(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  TASK_DEF(&blink, 100, 35),
  TASK_DEF(&display, 1000, 65),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  memset(framebuf, 0xFF, ARRAY_SIZE(framebuf));

  wdt_enable(WDTO_120MS);

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    task_run(task_main, ARRAY_SIZE(task_main));
  }

	return 0;
}

void blink(void){
  static const __flash gpio_t LED_STATIC = {&PIND, &DDRD, &PORTD, PD5};
  gpio_output(&LED_STATIC);
  gpio_set(&LED_STATIC);

  static const __flash gpio_t LED_BLINK = {&PINB, &DDRB, &PORTB, PB0};
  gpio_output(&LED_BLINK);
  gpio_tog(&LED_BLINK);
  return;
}

void display(void){
  TWI_start(60, TW_WRITE);
  TWI_write(0x40);
  TWI_writebuf(framebuf, ARRAY_SIZE(framebuf));
  TWI_stop();

  return;
}

void __attribute__((constructor)) display_init(void){
  static const __flash uint8_t ssd1306_startup[] = {
    0xAE, // Display off
    /* Stripped Startup */
    0xA8, 31,     // Set Multiplex Ratio -- 32-1 
    0x8D, 0x14,   // Charge Pump On
    0xDA, 0x02,   // Alternative Arrangement
    /* Set Dimensions */
    0x20, 0x01,   // Vertical Addressing
    0x21, 0, 127, // Set Start and End Columns
    0x22, 0, 3,   // Set Start and End Pages
    /* Exiting */
    0xAF, // Display On
  };

  TWI_init(400000UL);
  TWI_start(60, TW_WRITE);
  TWI_write(0x00);
  TWI_writebuf(ssd1306_startup, ARRAY_SIZE(ssd1306_startup));
  TWI_stop();

  return;
}

