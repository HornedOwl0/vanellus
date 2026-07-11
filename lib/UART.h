#ifndef UART_H
#define UART_H

#include <avr/io.h>

#ifndef RX_BUF_SIZE 
	#define RX_BUF_SIZE ( 32U )
#endif /* RX_BUF_SIZE */

#ifndef BAUD
	#error baud rate (BAUD) not defined for UART.h
#endif /* BAUD */

#ifdef BAUD
	#define UBRRX ( F_CPU/16/BAUD-1 )
	#define UDR0_AWAIT_CLR() while ( !(UCSR0A & (1<<UDRE0)) )
	
	static inline void UART_init(void){
	 	UBRR0H = (uint8_t)(UBRRX>>8); 
		UBRR0L = (uint8_t)(UBRRX);
		UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0)|(0<<UCSZ02);
		UCSR0C = (0<<USBS0)|(1<<UCSZ01)|(1<<UCSZ00);
		return;
	}
	
	void UART_puts(const char *str);
	
	void UART_putn(const char *str, int n);
	
	void UART_putc(const char str);

	void UART_puti(uint32_t num);
	
	void UART_putu(uint32_t num);
	
	int8_t UART_bufchk(void);
	
	char UART_getc(void);
#endif /* BAUD */

#endif /* UART_H */