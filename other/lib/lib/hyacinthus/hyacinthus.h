#ifndef HYACINTHUS_H
#define HYACINTHUS_H

typedef struct{
  volatile uint8_t *pinx;
  volatile uint8_t *ddrx;
  volatile uint8_t *portx;
  uint8_t ref_n;
} gpio_t;

void gpio_output( const __memx gpio_t *pX );

void gpio_input( const __memx gpio_t *pX );

void gpio_set( const __memx gpio_t *pX );

void gpio_clr( const __memx gpio_t *pX );

void gpio_tog( const __memx gpio_t *pX );

uint8_t gpio_get( const __memx gpio_t *pX );

#endif /* HYACINTHUS_H */
