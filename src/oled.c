#include <stdint.h>
#include <stddef.h>

#include <util/twi.h>

#include "hedera.h"
#include "macros.h"

void oled_writecmd(const __memx uint8_t *buf, uint16_t len){
  TWI_start(60, TW_WRITE);
  TWI_write(0x00);
  TWI_writebuf(buf, len);
  TWI_stop();

  return;
}

void oled_writescr(const __memx uint8_t *buf, uint16_t len){
  TWI_start(60, TW_WRITE);
  TWI_write(0x40);
  TWI_writebuf(buf, len);
  TWI_stop();

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

  oled_writecmd(ssd1306_startup, ARRAY_SIZE(ssd1306_startup));
  return;
}
