#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib/macros.h"
#include "lib/scheduler.h"

#define BAUD 57600U
#include "lib/UART.h"
#include <stdlib.h>
#include <stdio.h>

#define SERVO_MIN_US 1000U
#define SERVO_MAX_US 2000U
#include "lib/servo_timer1.h"

#include "lib/twi_master.h"
#include <util/delay.h>

struct xtask {
	void (*run)(void);
	uint16_t last_ms;
	const uint16_t interval_ms;
};

void blink(void);
void twi_task(void);
void UART_task(void);

static struct xtask tasks[] = {
	{&blink, 0, 200},
	{&twi_task, 500, 1000},
	{&UART_task, 1000, 1000},
};

void ssd1306_cmd(uint8_t cmd){
	TWI_START();
	TWI_WRITE(SLA_W(60));
	TWI_WRITE(0x00);
	TWI_WRITE(cmd);
	TWI_STOP();
}

int main(void){
	cli(); /* Begin Setup - no interrupts */
	
	_delay_ms(100);
	
	if ( TWI_CHK() ){
	
	TWI_INIT();
	
	ssd1306_cmd(0xAE); // Display OFF (sleep mode)

  ssd1306_cmd(0xD5); // Set Display Clock Divide Ratio / Oscillator Frequency
  ssd1306_cmd(0x80); // Default ratio
  
  ssd1306_cmd(0xA8); // Set Multiplex Ratio
  ssd1306_cmd(0x3F); // 64 duty (128x64 pixels)
  
  ssd1306_cmd(0xD3); // Set Display Offset
  ssd1306_cmd(0x00); // No offset
  
  ssd1306_cmd(0x40); // Set Start Line (Line 0)
  
  ssd1306_cmd(0x8D); // Charge Pump Setting
  ssd1306_cmd(0x14); // Enable charge pump during display on
  
  ssd1306_cmd(0x20); // Memory Addressing Mode
  ssd1306_cmd(0x00); // 0x00 for Horizontal Addressing Mode
  
  ssd1306_cmd(0xA1); // Set Segment Re-map (Column 127 mapped to SEG0)
  ssd1306_cmd(0xC8); // Set COM Output Scan Direction (Reversed direction)
  
  ssd1306_cmd(0xDA); // Set COM Pins Hardware Configuration
  ssd1306_cmd(0x12); 
  
  ssd1306_cmd(0x81); // Set Contrast Control
  ssd1306_cmd(0xCF); // Higher contrast
  
  ssd1306_cmd(0xD9); // Set Pre-charge Period
  ssd1306_cmd(0xF1); 
  
  ssd1306_cmd(0xDB); // Set VCOMH Deselect Level
  ssd1306_cmd(0x40); 
  
  ssd1306_cmd(0xA4); // Entire Display ON (Resume to RAM content display: 0xA4)
  ssd1306_cmd(0xA6); // Set Normal Display (0xA6 = non-inverted)

  ssd1306_cmd(0xAF); // Display ON

	ssd1306_cmd(0xA5);
	
	}
	
	SET(DDRB, PB0);
	
	timer0_init(); /* ALWAYS the last step in setup */
	sei(); /* End Setup - all interrupts */
	static uint16_t now_ms; /* scheduler code :3 */
	for(;;){
		atomic_get_ms(&now_ms);
		for (int8_t i=0; i<ARRAY_SIZE(tasks); i++){
			if(now_ms - tasks[i].last_ms >= tasks[i].interval_ms){
				tasks[i].last_ms = now_ms;
				tasks[i].run();
			}
		}
	} /* end scheduler code :3 */
	// peeeenis
	return 0;
}

void blink(void){
	SET(PINB, PB0);
	return;
}

void twi_task(void){
	return;
}

void UART_task(void){
	while ( UART_bufchk() ){
		UART_putc( UART_getc() );
	}
	return;
}
