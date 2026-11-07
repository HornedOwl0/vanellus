#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <avr/io.h>

#include "framebuf.h"
#include "config.h"
#include "font.h"

static uint8_t framebuf[FRAMEBUF_SIZE] = {0};

void framebuf_drawpx(uint8_t x, uint8_t y, int8_t val){
  if( x >= SIZE_X || y >= SIZE_Y ){ return; }
  uint16_t pos = ( x+((y&0xF8)<<4) );
  if (val)
    framebuf[pos] |= (1<<(y&0x07));
  else
    framebuf[pos] &= ~(1<<(y&0x07));
  return;
}

void framebuf_drawspr(uint8_t x, uint8_t y, uint8_t *spr){
  if( x >= SIZE_X || y >= SIZE_Y ){ return; }

  uint8_t width = (((uint16_t)x+8)>SIZE_X)?(SIZE_X-x):(8);
  uint8_t page = (y>>3);
  uint8_t shift = (y&0x07);

  uint16_t pos = (x+(page<<7));
  uint8_t mask = ~(0xFF<<shift);
  for (int8_t i=0; i<width; i++){
    framebuf[pos+i] &= mask;
    framebuf[pos+i] |= spr[i]<<shift;
  }
  if (shift){ // if there are leftovers, run it again
    if ((++page) >= (SIZE_Y>>3)){ return; }

    pos = (x+(page<<7));
    mask = ~(0xFF>>(8-shift));
    for (int8_t i=0; i<width; i++){
      framebuf[pos+i] &= mask;
      framebuf[pos+i] |= spr[i]>>(8-shift);
    }
  }

  return;
}

void framebuf_drawbig(uint8_t x, uint8_t y, const __memx uint8_t *spr){ // 4x the size, 4x the fun!
  if( x >= SIZE_X || y >= SIZE_Y ){ return; }
  
  uint8_t spr_big[8];
  
  for(uint8_t i=0; i<4; i++){
    for(uint8_t j=0; j<8; j++){
      uint8_t nibble = ((spr[((i&0x02)<<1)+(j>>1)]>>((i&0x01)<<2)));
      uint8_t extspr = 0;
      if (nibble&0x08){ extspr |= (0xC0); }
      if (nibble&0x04){ extspr |= (0x30); }
      if (nibble&0x02){ extspr |= (0x0C); }
      if (nibble&0x01){ extspr |= (0x03); }
      spr_big[j] = extspr;
    }
    framebuf_drawspr(x+((i&0x02)<<2), y+((i&0x01)<<3), spr_big);
  }

  return;
}

void framebuf_putc(uint8_t x, uint8_t y, int8_t size, char c){
  int8_t idx = c-32;
  if ( idx<0 ){ return; }

  uint8_t fspr[8] = {0};
  for (int8_t i=0; i<8; i++) {
    fspr[i] = ascii_font[idx][i];
  }

  if ( size )
    framebuf_drawbig(x, y, fspr);
  else
    framebuf_drawspr(x, y, fspr);
  return;
}

void framebuf_puts(uint8_t x, uint8_t y, int8_t size, const __memx char *s){
  if ( s!=NULL ){
    uint8_t offset = 0;
    charchk:
    while (*s){
      switch (*s){
        case '\n':
          y += (8<<(!!size)); s++; 
          goto charchk; 
        case '\r':
          offset = 0; s++; 
          goto charchk;
        default:
          framebuf_putc(x+(((offset++)<<3)<<(!!size)), y, size, *(s++));
          break;
      }
    }
  }
  return;
}

inline void framebuf_fill(int8_t val){
  memset(framebuf, ((val)?(0xFF):(0x00)), FRAMEBUF_SIZE);
  return;
}

void framebuf_fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t val){
  if ( w<1 || h<1 ){ return; }
  for (uint8_t i=0; i<(w-1); i++){
    for (uint8_t j=0; j<(h-1); j++){
      framebuf_drawpx(x+i, y+j, val);
    }
  }
  return;
}

#define ABS(x) ( ((x)<0) ? -(x) : (x) )

void framebuf_drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, int8_t val){
  int16_t dx = ABS(x1 - x0);
  int16_t xi = ((x0<x1)?(+1):(-1));
  int16_t dy = -ABS(y1 - y0);
  int16_t yi = ((y0<y1)?(+1):(-1));

  int16_t p = (dy+dx); // error/decision parameter

  for(;;){
    framebuf_drawpx(x0, y0, val);
    if ((p<<1) >= dy){
      if (x0 == x1){ break; }
      p += dy;
      x0 += xi;
    }
    if ((p<<1) <= dx){
      if (y0 == y1){ break; }
      p += dx;
      y0 += yi;
    }
  }
  return;
}

void framebuf_drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t val){
  if ( w<1 || h<1 ){ return; }
  w -= 1; h -= 1;
  framebuf_drawline(x, y, x+w, y, val);
  framebuf_drawline(x, y, x, y+h, val);
  framebuf_drawline(x+w, y+h, x, y+h, val);
  framebuf_drawline(x+w, y+h, x+w, y, val);
}

inline uint8_t* framebuf_ptr(void){
  return framebuf;
}

inline uint16_t framebuf_size(void){
  return FRAMEBUF_SIZE;
}
