#ifndef FRAMEBUF_H
#define FRAMEBUF_H

void framebuf_drawpx(uint8_t x, uint8_t y, int8_t val);

void framebuf_drawspr(uint8_t x, uint8_t y, uint8_t *spr);

void framebuf_drawbig(uint8_t x, uint8_t y, const __memx uint8_t *spr);

#define FRAMEBUF_LARGE ( 1 )
#define FRAMEBUF_SMALL ( 0 )

void framebuf_putc(uint8_t x, uint8_t y, int8_t size, char c);

void framebuf_puts(uint8_t x, uint8_t y, int8_t size, const __memx char *s);

void framebuf_fill(int8_t val);

void framebuf_fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t val);

void framebuf_drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, int8_t val);

void framebuf_drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t val);

uint8_t* framebuf_ptr(void);

uint16_t framebuf_size(void);

#endif /* FRAMEBUF_H */
