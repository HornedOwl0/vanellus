#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "font.h"

uint8_t framebuf[FRAMEBUF_SIZE] = {0};

int8_t framebuf_drawpx(uint8_t x, uint8_t y, int8_t val){
  if(x>127){ return -1; }
  if(y>31){ return 1; }
  uint16_t pos = ( x+((y&0xF8)<<4) );
  if (val)
    framebuf[pos] |= (1<<(y&0x07));
  else
    framebuf[pos] &= ~(1<<(y&0x07));
  return 0;
}

int8_t framebuf_drawsprite(uint8_t x, uint8_t y, const __memx uint8_t* spr){
  for (int8_t i=0; i<8; i++){
    for (int8_t j=0; j<8; j++){
      if (int8_t err = framebuf_drawpx(x+i, y+j, (spr[i]&(1<<j)))){ return err; }
    }
  }
  return 0;
}

int8_t framebuf_putc(uint8_t x, uint8_t y, char c){
  int8_t idx = c-32;
  if ( idx<0 ){ return 1; }
  if (int8_t err = framebuf_drawsprite(x, y, ascii_font[idx])){ return err; }
  return 0;
}

int8_t framebuf_puts(uint8_t x, uint8_t y, const __memx char *s){
  if ( s!=NULL ){
    uint8_t offset = 0;
    while (*s){
      if (int8_t err = framebuf_putc(x+((offset++)<<3), y, *(s++))){ return err; }
    }
  }
  return 0;
}
