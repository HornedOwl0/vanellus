#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>

#include <util/delay.h>
#include <util/twi.h>

#include "hedera.h"
#include "framebuf.h"

#define OLED_ADDR ( 60 )

void OLED_refresh(void){
  if ( !TWI_available() ){ return; }

  if ( !TWI_writebuf(OLED_ADDR, (uint8_t[]){0x40}, 1, TWI_START, TWI_NOSTOP) ){ // noerr
    while ( !TWI_available() ){}
    TWI_writebuf(OLED_ADDR, framebuf_ptr(), framebuf_size(), TWI_NOSTART, TWI_STOP);
  }
  return;
}

void __attribute__((constructor)) OLED_init(void){
  static const __flash uint8_t cmd[] = { // SSD1306
    0x00, // Send Cmd
    0xAE, // Display off
    /* Stripped Startup */
    0xA8, 31,     // Set Multiplex Ratio -- 32-1 
    0x8D, 0x14,   // Charge Pump On
    0xDA, 0x02,   // Alternative Arrangement
    0xDB, 0x00,   // Set VCOMH deselect Level (LOWEST)
    0x81, 56,     // Set Contrast
    /* Set Dimensions */
    0x20, 0x00,   // Vertical Addressing
    0x21, 0, 127, // Set Start and End Columns
    0x22, 0, 3,   // Set Start and End Pages
    /* Exiting */
    // 0xA0, 0xC0,   // Normal Orientation
    0xA1, 0xC8,   // Flip Orientation
    0xAF, // Display On
  };

  _delay_ms(35);
  
  if (!TWI_start(OLED_ADDR, TW_WRITE)){
    for (int8_t i=0; i<sizeof(cmd); i++) {
      TWI_write(cmd[i]);
    }
    TWI_stop();
  }

  return;
}
