#ifndef F_CPU
	#define F_CPU 1000000UL
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/utils.h"
#include "lib/scheduler.h"

#define BAUD 57600U
#include "lib/UART.h"
#include <stdlib.h>
#include <stdio.h>

void adcloop(void){
	static int16_t voltage;
	ADCSRA |= (1<<ADSC);
	while( READ(ADCSRA, ADSC) ){}
	voltage = ((int32_t)ADC*5000)/1024;		
	
	UART_putu(voltage);
	
	_delay_ms(100);
}

void blink(void){
	TOG(PORTB, PB0);
}

static struct scheduler_task tasks[] = {
	{&adcloop, 0, 500},
	{&blink, 0, 350},
};

int main(){
	cli(); /* Begin Setup - no interrupts */
	
	ADCSRA = (1<<ADEN)|(0x7<<ADPS0);
	ADCSRB = 0x00;
	
	ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR);
	ADMUX = ( ADMUX & 0xF0 )|(0x00); /* Clear MUX bits, ADC on C0 */
	
	CLR(DDRC, PC0);
	
	SET(DDRB, PB0);
	
	UART_init();
	scheduler_init();
	sei(); /* End Setup - all interrupts */
	static uint16_t now_ms;
	for(;;){
		now_ms = atomic_get_ms();
		for (int8_t i=0; i<ARRAY_SIZE(tasks); i++){
			if(now_ms - tasks[i].last_ms >= tasks[i].interval_ms){
				tasks[i].last_ms = now_ms;
				tasks[i].run();
			}
		}
	}
	
	return 0;
}