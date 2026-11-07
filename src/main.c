#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdfix.h>
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

static int8_t relh = 0;
static _Accum temp = 0.0k;

/* Non-Task Function Prototypes */

/* Task Function Prototypes */

void blink(void);
void display(void);
void update_counter(void);
void temp_sensor(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  // TASK_DEF(&blink, 1000, 35),
  TASK_DEF(&display, 85, 65),
  TASK_DEF(&update_counter, 100, 115),
  TASK_DEF(&temp_sensor, 165, 135),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  {
    gpio_t LED_STATIC = {&PIND, &DDRD, &PORTD, PD5};
    gpio_output(&LED_STATIC);
    gpio_set(&LED_STATIC);
    gpio_t LED_BLINK = {&PINB, &DDRB, &PORTB, PB0};
    gpio_output(&LED_BLINK);
    gpio_set(&LED_BLINK);
  }

  wdt_enable(WDTO_120MS);

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
  if ( TWI_available() ){
    if ( !TWI_start(60, TW_WRITE) ){ // noerr
      TWI_write(0x40);
      TWI_writebuf(60, framebuf_ptr(), framebuf_size(), TWI_NOSTART, TWI_STOP);
    }
  }
  return;
}

void update_counter(void){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    char pbuf[16] = {0}; // Print Buf
    char tbuf[8] = {0}; // Temp Buf

    framebuf_fill(0);

    framebuf_puts(24, 0, FRAMEBUF_SMALL, (const __memx char*)"Temp");
    framebuf_drawrect(0, 12, 82, 20, 1);
    
    itoa(temp, pbuf, 10); // Get
    strcat(pbuf, (const __memx char*)".");
    itoa((temp-(int16_t)temp)*10, tbuf, 10); // Get
    strcat(pbuf, tbuf);
    framebuf_puts(1+((5-strlen(pbuf))<<3), 13, FRAMEBUF_LARGE, pbuf); // Put
    
    framebuf_drawrect(84, 12, 44, 20, 1);
    itoa(relh, pbuf, 10); // Get

    framebuf_puts(90, 0, FRAMEBUF_SMALL, (const __memx char*)"RelH"); // Put
    framebuf_puts(90+((2-strlen(pbuf))<<3), 13, FRAMEBUF_LARGE, pbuf); // Put
  }
  
  return;
}

void temp_sensor(void){
  uint8_t aht[7] = {0};
  static const __flash uint8_t cmd[] = {
    0xAC, 0x33, 0x00,
  };
  
  if ( !TWI_readinto(0x38, aht, ARRAY_SIZE(aht), TWI_NOSTOP) ){
    while ( !TWI_available() ){}

    if ( aht[0]&0x80 ){ return; } // Check Status

    TWI_writebuf(0x38, cmd, ARRAY_SIZE(cmd), TWI_START, TWI_STOP);

    // CRC8 
    // Polynomial: X8+X5+X4+1 
    // Poly: 1 0011 0001 -> 0x31 (Truncated)
    
    uint8_t crc = 0xFF;
    for (int8_t i=0; i<ARRAY_SIZE(aht); i++){
      crc ^= aht[i];
      for (int8_t b=8; b>0; --b){
        if (crc&0x80)
          crc = (crc<<1)^(0x31);
        else
          crc = (crc<<1);
      }
    }

    uint32_t relh_raw = ((uint32_t)aht[1])<<12|((uint32_t)aht[2])<<4|((uint32_t)aht[3]&0xF0)>>4;
    relh = (relh_raw*100)>>20;

    uint32_t temp_raw = ((uint32_t)aht[3]&0x0F)<<16|((uint32_t)aht[4])<<8|((uint32_t)aht[5]<<0);
    temp = ((_Accum)(temp_raw>>(20-ACCUM_IBIT))>>ACCUM_IBIT)*200-50;
  }

  return;
}

void __attribute__((constructor)) oled_init(void){
  static const __flash uint8_t cmd[] = { // SSD1306
    0x00, // Send Cmd
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
    for (int8_t i=0; i<ARRAY_SIZE(cmd); i++) {
      TWI_write(cmd[i]);
    }
    TWI_stop();
  }

  return;
}
