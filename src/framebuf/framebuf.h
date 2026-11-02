#ifndef FRAMEBUF_H
#define FRAMEBUF_H

int8_t framebuf_drawpx(uint8_t x, uint8_t y, int8_t val);

int8_t framebuf_drawsprite(uint8_t x, uint8_t y, const __memx uint8_t* spr);

#define FRAMEBUF_LARGE ( 1 )
#define FRAMEBUF_SMALL ( 0 )

int8_t framebuf_putc(uint8_t x, uint8_t y, int8_t size, char c);

int8_t framebuf_puts(uint8_t x, uint8_t y, int8_t size, const __memx char *s);

int8_t framebuf_fill(int8_t val);

int8_t framebuf_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t val);

uint8_t* framebuf_ptr(void);

uint16_t framebuf_size(void);

#endif /* FRAMEBUF_H */
