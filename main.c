#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "hibiscus/hibiscus.h"
#include "lib/macros.h"

#define SENSOR_THRESHOLD (230U)
#define SENSOR_DUTY (76U)

void SETCLK(void) __attribute__((naked)) __attribute__((section(".init3")));

static struct ADC_mnt{
  const uint16_t ADCn:4;
  volatile uint16_t val:12;
} ADC_lst[] = {
  {.ADCn = 4},
  {.ADCn = 5},
  {.ADCn = 13},
  {.ADCn = 6},
  {.ADCn = 7},
};

#define ADC_COUNT (ARRAY_SIZE(ADC_lst))

static volatile int8_t volatile_ADC_ctu;
static volatile struct ADC_mnt *pADC = NULL;
ISR(ADC_vect){
  if (pADC != NULL){ // old pointer, store the reading
    pADC->val = ADC;
  }

  if (++volatile_ADC_ctu >= ADC_COUNT){
    volatile_ADC_ctu = 0;
  }

  pADC = &ADC_lst[volatile_ADC_ctu]; // new pointer, start the reading

  if (pADC != NULL){
    ADMUX = ( ADMUX & 0xE0 )|( (pADC->ADCn)&0x07 ); // 3 low bits into MUX2:0 
    if ( (pADC->ADCn)&0x08 ){ // 4th High but into MUX5
      SET(ADCSRB, MUX5);
    } else {
      CLR(ADCSRB, MUX5);
    }
    ADCSRA |= (1<<ADSC); // start conversion
  }
}

void ADC_init(){
  ADCSRA = (1<<ADEN)|(0x5<<ADPS0)|(1<<ADIE);
	ADCSRB = (1<<ADHSM);
	
	ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR); // External AREF
	ADMUX = ( ADMUX & 0xE0 )|(0x1F); /* Clear MUX bits, set initial reading to internal 0V (GND) */
  ADCSRA |= (1<<ADSC);
}

void TC1_init(void);

int main(void){
  (void)ADC_lst;

	cli(); _delay_ms(5); /* Begin Setup - no interrupts */
  
  ACM_init();

  CLRBM(DDRF, (1<<PF7)|(1<<PF6)|(1<<PF5)|(1<<PF4) ); // OUT5/4 - OUT2/1
  CLR(DDRB, PB1); // OUT3

  SET(DDRD, PD5);
  SET(DDRB, PB0);
  CLR(PORTD, PD5);

  TC1_init();

  ADC_init();

  wdt_enable(WDTO_8S);

  static char buf[32];

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    _delay_ms(70);
    SET(PINB, PB0);

    memset(buf, '\0', ARRAY_SIZE(buf));

    char temp_buf[8];
    for (int8_t i=0; i<ARRAY_SIZE(ADC_lst); i++){
      itoa(ADC_lst[i].val, temp_buf, 10);
      strcat(buf, temp_buf);
      strcat(buf, (const __memx char*)", ");
    }
    ACM_puts(buf);
    ACM_puts( (const __memx char*)"\r\n" );
  }
	return 0;
}

void SETCLK(void){
  #if (F_CPU==16000000UL)
    clock_prescale_set(clock_div_1); // 16/1 = 16 MHz
  #elif (F_CPU==8000000UL)
    clock_prescale_set(clock_div_2); // 16/2 = 8 MHz
  #elif (F_CPU==1000000UL)
    clock_prescale_set(clock_div_16); // 16/16 = 1 MHz
  #else 
    #error "(SETCLK) F_CPU wont divide! Try 16/8/1MHz"
  #endif
  return;
}

void TC1_init(void){
  /* TC1 Setup - set registers */
  TCCR1A = (1<<COM1A1)|(0<<COM1B1); // Non-Inverted Mode;
  TCCR1B = (1<<WGM13); // Phase and Frequency Correct ICR1 - Mode 8;
  /* 16MHz frequency */
  ICR1 = 256U; // set period

  OCR1A = SENSOR_DUTY;

  DDRB |= (1<<PB5);

  TCCR1B |= (0x01<<CS10); // Push the clock prescaler for timer startup
  /* end TC1 setup*/
  return;
}
