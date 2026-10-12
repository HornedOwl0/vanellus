#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "helianthus.h"

ISR(ADC_vect){
  static volatile int8_t volatile_ADC_ctu;
  static volatile struct ADC_mnt *pADC = NULL;

  if (pADC != NULL){ // old pointer, store the reading
    pADC->val = ADC;
  }

  if (++volatile_ADC_ctu >= ADC_count){
    volatile_ADC_ctu = 0;
  }

  pADC = &ADC_lst[volatile_ADC_ctu]; // new pointer, start the reading

  if (pADC != NULL){
    ADMUX = ( ADMUX & 0xE0 )|( (pADC->ADCn)&0x07 ); // 3 low bits into MUX2:0 
    if ( (pADC->ADCn)&0x08 ){ // 4th High bit into MUX5
      ADCSRB |= (1<<MUX5);
    } else {
      ADCSRB &= ~(1<<MUX5);
    }
    ADCSRA |= (1<<ADSC); // start conversion
  }
}

inline void ADC_init(){
  ADCSRA = (1<<ADEN)|(0x7<<ADPS0)|(1<<ADIE);
	ADCSRB = 0x00;
	
	ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR); // External AREF
	ADMUX = ( ADMUX & 0xE0 )|(0x1F); /* Clear MUX bits, set initial reading to internal 0V (GND) */
  ADCSRA |= (1<<ADSC);
}
