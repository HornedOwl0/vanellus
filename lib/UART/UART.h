#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdint.h>

#ifndef RX_BUF_SIZE 
	#define RX_BUF_SIZE ( 32U )
#endif /* RX_BUF_SIZE */

#ifndef BAUD
	#error baud rate (BAUD) not defined for UART.h
#endif /* BAUD */

#ifdef BAUD
	#define UBRRX ( F_CPU/16/BAUD-1 )
	#define UDR1_AWAIT_CLR() while ( !(UCSR1A & (1<<UDRE1)) )
	
	static inline void UART_init(void){
	 	UBRR1H = (uint8_t)(UBRRX>>8); 
		UBRR1L = (uint8_t)(UBRRX);
		UCSR1B = (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1)|(0<<UCSZ12);
		UCSR1C = (0<<USBS1)|(1<<UCSZ11)|(1<<UCSZ10);
		return;
	}
	
	void UART_puts(const char *str);
	
	void UART_putn(const char *str, uint8_t n);
	
	void UART_putc(const char str);

	void UART_puti(uint32_t num);
	
	void UART_putu(uint32_t num);
	
	int8_t UART_bufchk(void);
	
	char UART_getc(void);
#endif /* BAUD */

#endif /* UART_H */
