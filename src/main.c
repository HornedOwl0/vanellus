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
#include "framebuf/framebuf.h"

/* Non-Task Function Prototypes */

extern void oled_writecmd(const __memx uint8_t *buf, uint16_t len);

extern void oled_writescr(const __memx uint8_t *buf, uint16_t len);

/* Task Function Prototypes */

void blink(void);
void display(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  TASK_DEF(&blink, 100, 35),
  TASK_DEF(&display, 10, 65),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  framebuf_puts(0, 8, "OwO~ \\^w^/:3|*");
  framebuf_puts(0, 16, "HornedOwl_ +-");

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
  oled_writescr(framebuf_ptr(), framebuf_size());
  return;
}

void oled_writecmd(const __memx uint8_t *buf, uint16_t len){
  if (TWI_available()){
    uint8_t ctrl[1] = {0x00};
    TWI_writebuf(60, ctrl, ARRAY_SIZE(ctrl), TWI_START, TWI_NOSTOP);
    while (!TWI_available()){}
    TWI_writebuf(60, buf, len, TWI_NOSTART, TWI_STOP);
  }

  return;
}

void oled_writescr(const __memx uint8_t *buf, uint16_t len){
  if (TWI_available()){
    uint8_t ctrl[1] = {0x40};
    TWI_writebuf(60, ctrl, ARRAY_SIZE(ctrl), TWI_START, TWI_NOSTOP);
    while (!TWI_available()){}
    TWI_writebuf(60, buf, len, TWI_NOSTART, TWI_STOP);
  }

  return;
}

void __attribute__((constructor)) oled_init(void){
  static const __flash uint8_t ssd1306_startup[] = {
    0xAE, // Display off
    /* Stripped Startup */
    0xA8, 31,     // Set Multiplex Ratio -- 32-1 
    0x8D, 0x14,   // Charge Pump On
    0xDA, 0x02,   // Alternative Arrangement
    /* Set Dimensions */
    0x20, 0x00,   // Vertical Addressing
    0x21, 0, 127, // Set Start and End Columns
    0x22, 0, 3,   // Set Start and End Pages
    /* Exiting */
    // 0xA0, 0xC0,   // Normal Orientation
    0xA1, 0xC8,   // Flip Orientation
    0xAF, // Display On
  };

  TWI_start(60, TW_WRITE);
  TWI_write(0x00);
  for (int8_t i=0; i<ARRAY_SIZE(ssd1306_startup); i++) {
    TWI_write(ssd1306_startup[i]);
  }
  TWI_stop();

  return;
}
