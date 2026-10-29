#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include "hyacinthus.h"

void gpio_output( const __memx gpio_t *pX ){
  if (pX!=NULL)
    *((volatile uint8_t*)(pX->ddrx)) |= (1<<(pX->ref_n));
  return;
}

void gpio_input( const __memx gpio_t *pX ){
  if (pX!=NULL)
    *((volatile uint8_t*)(pX->ddrx)) &= ~(1<<(pX->ref_n));
  return;
}

void gpio_set( const __memx gpio_t *pX ){
  if (pX!=NULL)
    *((volatile uint8_t*)(pX->portx)) |= (1<<(pX->ref_n));
  return;
}

void gpio_clr( const __memx gpio_t *pX ){
  if (pX!=NULL)
    *((volatile uint8_t*)(pX->portx)) &= ~(1<<(pX->ref_n));
  return;
}

void gpio_tog( const __memx gpio_t *pX ){
  if (pX!=NULL)
    *((volatile uint8_t*)(pX->pinx)) |= (1<<(pX->ref_n));
  return;
}

uint8_t gpio_get( const __memx gpio_t *pX ){
  if (pX!=NULL)
    return ( ( *((volatile uint8_t*)(pX->pinx)) )&( 1<<(pX->ref_n) ) );
  return 0;
}

