#ifndef FRAMEBUF_H
#define FRAMEBUF_H

int8_t framebuf_drawpx(uint8_t x, uint8_t y, int8_t val);

int8_t framebuf_drawsprite(uint8_t x, uint8_t y, const __memx uint8_t* spr);

int8_t framebuf_putc(uint8_t x, uint8_t y, char c);

int8_t framebuf_puts(uint8_t x, uint8_t y, const __memx char *s);

uint8_t* framebuf_ptr(void);

uint16_t framebuf_size(void);

#endif /* FRAMEBUF_H */
