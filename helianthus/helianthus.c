#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "helianthus.h"

ISR(ADC_vect){
  static volatile int8_t ctu = 0;
  static volatile struct ADC_mnt *ptr = NULL;

  if (ptr != NULL){ // old pointer, store the reading
    ptr->val = ADC;
  }

  if (ctu >= ADC_count){
    ctu = 0;
  }

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

inline void ADC_init(void){
  if (ADC_lst != NULL){
    ADCSRA = (1<<ADEN)|(0x7<<ADPS0)|(1<<ADIE);
    ADCSRB = 0x00;
    
    ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR); // External AREF
    ADMUX = ( ADMUX & 0xE0 )|(0x1F); /* Clear MUX bits, set initial reading to internal 0V (GND) */
    ADCSRA |= (1<<ADSC);
  }
  return;
}
