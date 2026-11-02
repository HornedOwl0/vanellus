#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "framebuf.h"
#include "config.h"
#include "font.h"

static uint8_t framebuf[FRAMEBUF_SIZE] = {0};

int8_t framebuf_drawpx(uint8_t x, uint8_t y, int8_t val){
  if(x>=SIZE_X){ return -1; }
  if(y>=SIZE_Y){ return 1; }
  uint16_t pos = ( x+((y&0xF8)<<4) );
  if (val)
    framebuf[pos] |= (1<<(y&0x07));
  else
    framebuf[pos] &= ~(1<<(y&0x07));
  return 0;
}

int8_t framebuf_drawsprite(uint8_t x, uint8_t y, const __memx uint8_t* spr){
  if(x>=SIZE_X){ return -1; }
  if(y>=SIZE_Y){ return 1; }
  for (int8_t i=0; i<8; i++){
    for (int8_t j=0; j<8; j++){
      if (int8_t err = framebuf_drawpx(x+i, y+j, (spr[i]&(1<<j)))){ return err; }
    }
  }
  return 0;
}

int8_t framebuf_drawsprite_big(uint8_t x, uint8_t y, const __memx uint8_t* spr){ // 4x the size, 4x the fun!
  if(x>=SIZE_X){ return -1; }
  if(y>=SIZE_Y){ return 1; }
  for (int8_t i=0; i<8; i++){
    for (int8_t j=0; j<8; j++){
      for (int8_t k=0; k<4; k++){
        if (int8_t err = framebuf_drawpx(x+(k>>1)+(i<<1), y+(k&1)+(j<<1), (spr[i]&(1<<j)))){ return err; }
      }
    }
  }
  return 0;
}

int8_t framebuf_putc(uint8_t x, uint8_t y, int8_t size, char c){
  int8_t idx = c-32;
  if ( idx<0 ){ return 1; }
  if ( size ) {
    if (int8_t err = framebuf_drawsprite_big(x, y, ascii_font[idx])){ return err; }
  } else {
    if (int8_t err = framebuf_drawsprite(x, y, ascii_font[idx])){ return err; }
  }
  return 0;
}

int8_t framebuf_puts(uint8_t x, uint8_t y, int8_t size, const __memx char *s){
  if ( s!=NULL ){
    uint8_t offset = 0;
    charchk:
    while (*s){
      if (*s=='\n'){ y += (8<<(!!size)); s++; goto charchk; }
      if (*s=='\r'){ offset = 0; s++; goto charchk; }
      if (int8_t err = framebuf_putc(x+(((offset++)<<3)<<(!!size)), y, size, *(s++))){ return err; }
    }
  }
  return 0;
}

inline int8_t framebuf_fill(int8_t val){
  memset(framebuf, ((val)?(0xFF):(0x00)), FRAMEBUF_SIZE);
  return 0;
}

inline int8_t framebuf_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t val){
  val = ((val)?(0xFF):(0x00));
  for (int8_t i=0; i<w; i++){
    for (int8_t j=0; j<h; j++){
      if (int8_t err = framebuf_drawpx(x+i, y+j, val)){ return err; }
    }
  }
  return 0;
}

inline uint8_t* framebuf_ptr(void){
  return framebuf;
}

inline uint16_t framebuf_size(void){
  return FRAMEBUF_SIZE;
}
