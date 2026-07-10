#include <avr/io.h>
#define BAUD
#include "UART.h"
#undef BAUD

void UART_puts(char *str){
	while (*str){
		UDR0=*(str++);
		UDR0_AWAIT_CLR();
	}
	UDR0='\n';
	UDR0_AWAIT_CLR();
	return;
}

void UART_putn(char *str, int n){
	while (*str && n--){
		UDR0=*(str++);
		UDR0_AWAIT_CLR();
	}
	return;
}

void UART_putc(char str){
	UDR0=str;
	UDR0_AWAIT_CLR();
	return;
}

void UART_puti(uint32_t num){
	char buf[11] = {0};
	int8_t idx = 0;
	
	if ((int32_t)num<0){
		UART_putc('-');
		num = (uint32_t)(~num+1);
	} else {
		num = (uint32_t)(num);
	}
	
	/* read it backwards - run up the index*/
	do { 
		buf[idx] = (char)( ( num%10 ) + '0' );
	} while( !!(num/=10) && (++idx)<16); 
	/* print it forwards - run back the index*/
	do{
		UART_putc(buf[idx]);
	} while ( (idx--)>0 );
	UART_putc('\n');
	
	return;
}

void UART_putu(uint32_t num){
	char buf[11] = {0};
	int8_t idx = 0;
	
	/* read it backwards - run up the index*/
	do { 
		buf[idx] = (char)( ( num%10 ) + '0' );
	} while( !!(num/=10) && (++idx)<16); 
	/* print it forwards - run back the index*/
	do{
		UART_putc(buf[idx]);
	} while ( (idx--)>0 );
	UART_putc('\n');
	
	return;
}

char UART_getc(void){
	if (!(UCSR0A & (1<<RXC0))){
		return UDR0;
	}
	return 0;
}

