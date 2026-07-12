#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define BAUD
#include "UART.h"
#undef BAUD

static volatile struct {
	char raw[RX_BUF_SIZE];
	uint8_t r_pos;
	uint8_t w_pos;
} buf={ .raw={0}, .r_pos=0, .w_pos=0 };

ISR(USART_RX_vect){
	if ( (buf.w_pos+1)!=buf.r_pos ){
		buf.raw[ buf.w_pos++ ] = UDR0;
		buf.w_pos%=RX_BUF_SIZE;
	} else {
		(void)UDR0;
	}
} 

void UART_puts(const char *str){
	while (*str){
		UDR0=*(str++);
		UDR0_AWAIT_CLR();
	}
	UDR0='\n';
	UDR0_AWAIT_CLR();
	return;
}

void UART_putn(const char *str, uint8_t n){
	while (*str && n--){
		UDR0=*(str++);
		UDR0_AWAIT_CLR();
	}
	return;
}

void UART_putc(const char str){
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

int8_t UART_bufchk(void){
	return !!( buf.r_pos != buf.w_pos ); 
}

char UART_getc(void){
	char hold = buf.raw[ (buf.r_pos++) ];
	buf.r_pos %= RX_BUF_SIZE;
	return hold;
}



