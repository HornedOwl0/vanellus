#ifndef HIBISCUS_H
#define HIBISCUS_H

#include <stdint.h>

#define VENDOR_HEX (0x1209)
#define PRODUCT_HEX (0x0007)

/* Writes a string (up to 64B), raw into the FIFO and sends it immediately, long (64B+ strings) will wait */
void ACM_puts(const char __memx *str);

/* Writes a char, raw into the FIFO and sends it immediately */
void ACM_putc(const char c);

/* Returns the number of chars available for reading in the RX FIFO */
uint8_t ACM_available(void);

/* Returns a single char from the RX FIFO */
char ACM_getc(void);

/* Reads n (up to 64B) chars from the FIFO into into the provided buffer */
void ACM_gets(char *ptr, uint8_t n);

#endif /* HIBISCUS_H */
