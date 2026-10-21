#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "helianthus.h"

static volatile int8_t ctu = 0;
static volatile struct ADC_mnt *ptr = NULL;

ISR(ADC_vect){
  if (ctu < 0){ ctu = 0; goto read; }
  if (ptr != NULL){ ptr->val = ADC; } // old pointer, store the reading
  if (ctu >= ADC_count){ ctu = 0; }
  
  read:
  ptr = &ADC_lst[ctu++]; // new pointer, start the reading
  if (ptr != NULL){
    ADMUX = ( ADMUX & 0xE0 )|( (ptr->ADCn)&0x07 ); // 3 low bits into MUX2:0 
    if ( (ptr->ADCn)&0x08 ){ // 4th High bit into MUX5
      ADCSRB |= (1<<MUX5);
    } else {
      ADCSRB &= ~(1<<MUX5);
    }
    ADCSRA |= (1<<ADSC); // start conversion
  }
}

uint16_t ADC_force(uint8_t muxbits){
  ADCSRA &= ~(1<<ADIE); // stop ADIE interrupts
  
  uint16_t ADCr = 0;
  ctu = -1; // avoid storing next reading
  // Take the reading
  ADMUX = ( ADMUX & 0xE0 )|( (muxbits)&0x1F ); // 3 low bits into MUX2:0 
  if ( (muxbits)&0x20 ){ // 6th High bit into MUX5
    ADCSRB |= (1<<MUX5);
  } else {
    ADCSRB &= ~(1<<MUX5);
  }
  _delay_us(165); // datasheet says wait at least 125us
  ADCSRA |= (1<<ADSC); // start conversion
	while( !!(ADCSRA & (1<<ADSC)) ){};
  ADCr = ADC;
    
  ADCSRA |= (1<<ADIE)|(1<<ADSC); // enable interrupts and restart conversions

  return ADCr;
}

static inline void __attribute__((constructor)) ADC_init(){
  if (ADC_lst != NULL){
    ADCSRA = (1<<ADEN)|(0x7<<ADPS0)|(1<<ADIE);
    ADCSRB = 0x00;
    
    ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR); // External AREF
    ADMUX = ( ADMUX & 0xE0 )|(0x1F); /* Clear MUX bits, set initial reading to internal 0V (GND) */
    ADCSRA |= (1<<ADSC);
	  while( !!(ADCSRA & (1<<ADSC)) ){};
  }
  return;
}
