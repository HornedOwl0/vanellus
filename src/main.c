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
#include "framebuf.h"

/* Global Static Allocation */

static int8_t ctu = -128;

/* Non-Task Function Prototypes */

extern void oled_writecmd(const __memx uint8_t *buf, uint16_t len);

extern void oled_writescr(const __memx uint8_t *buf, uint16_t len);

/* Task Function Prototypes */

void blink(void);
void display(void);
void update_counter(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  TASK_DEF(&blink, 100, 35),
  TASK_DEF(&display, 35, 65),
  TASK_DEF(&update_counter, 85, 115),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  {
    gpio_t LED_STATIC = {&PIND, &DDRD, &PORTD, PD5};
    gpio_set(&LED_STATIC);
  }

  framebuf_puts(24, 0, FRAMEBUF_SMALL, (const __memx char*)"HornedOwl_");

  wdt_enable(WDTO_2S);

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    task_run(task_main, ARRAY_SIZE(task_main));
  }

	return 0;
}

void blink(void){
  static const __flash gpio_t LED_BLINK = {&PINB, &DDRB, &PORTB, PB0};

  gpio_output(&LED_BLINK);
  gpio_tog(&LED_BLINK);
  return;
}

void display(void){
  char buf[8] = {0};
  
  framebuf_fill_rect(0, 8, 128, 24, 0); // Clr
  itoa(ctu, buf, 10); // Get
  framebuf_puts(28+((4-strlen(buf))<<3), 16, FRAMEBUF_LARGE, buf); // Put

  oled_writescr(framebuf_ptr(), framebuf_size());
  return;
}

void update_counter(void){
  ctu++;
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

  if (!TWI_start(60, TW_WRITE)){
    TWI_write(0x00);
    for (int8_t i=0; i<ARRAY_SIZE(ssd1306_startup); i++) {
      TWI_write(ssd1306_startup[i]);
    }
  }
  TWI_stop();

  return;
}
