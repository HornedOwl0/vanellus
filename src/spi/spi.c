#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>

void SPI_write(uint8_t data){
  SPDR = data;
  while ( !(SPSR&(1<<SPIF)) ){}
}

uint8_t SPI_read(void){
  uint8_t data = SPDR;
  return data;
}

static inline void __attribute__((constructor)) SPI_init(void){
  SPCR = (1<<SPE)|(1<<MSTR)|(0x00<<CPHA)|(0x01<<SPR0); // MODE 0, FOSC/16 - (1MHz)
  DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3);
  return;
}
