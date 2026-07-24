#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib/macros.h"

#define BAUD 57600
#include "lib/UART.h"

typedef int16_t fix7p8_t;

fix7p8_t fix_ADD(const fix7p8_t a, const fix7p8_t b){
	return a+b;
}

fix7p8_t fix_SUB(const fix7p8_t a, const fix7p8_t b){
	return a-b;
}

/* 
the AVR instruction set does include multiply and
fixed multiply instructions, however, theyre tailored to 
8 bit fixed point, as such, I've opted for pseudo-multiplication
and pseudo-division 
*/

fix7p8_t fix_MUL(fix7p8_t a, fix7p8_t b){
	uint32_t hold = 0;
	uint32_t wide_a = (a<0) ? (uint32_t)(~a+1) : (uint32_t)(a);
	uint32_t wide_b = (b<0) ? (uint32_t)(~b+1) : (uint32_t)(b);
	

	while (wide_b>0){
		if (wide_b&0x01){
			hold+=wide_a;
		}
		wide_a <<= 1;
		wide_b >>= 1;
	}

	int32_t result = hold>>8;
	if ( (a^b)<0 ){
		result = ~(result+1);
	}

	return (fix7p8_t)(result);
}


fix7p8_t fix_DIV(fix7p8_t a, fix7p8_t b){
	return a+b;
}

int main(void){
	UART_init();
	for(;;){
		UART_puti((int16_t)fix_MUL(384, 512)); // 1.5 * 2
		UART_putc('\n');
	}
	return 0;
}

