#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>

void SPI_write(uint8_t data){
  SPDR = data;
  while ( !(SPSR&(1<<SPIF)) ){}
}

static inline void __attribute__((constructor)) SPI_init(void){
  SPCR = (1<<SPE)|(1<<MSTR)|(0x00<<CPHA)|(0x01<<SPR0); // MODE 0, FOSC/16 - (1MHz)
  return;
}