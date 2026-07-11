#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib/utils.h"
#include "lib/scheduler.h"

#define BAUD 57600U
#include "lib/UART.h"
#include <stdlib.h>
#include <stdio.h>

#define SERVO_MIN_US 1000U
#define SERVO_MAX_US 2000U
#include "lib/servo_timer1.h"

void blink(void);
void servo_task(void);
void UART_task(void);

static struct scheduler_task tasks[] = {
	{&blink, 0, 200},
	{&servo_task, 500, 1000},
	{&UART_task, 1000, 1000},
};

int main(void){
	cli(); /* Begin Setup - no interrupts */
	
	SET(DDRB, PB0);
	
	servo_init();
	UART_init();
	
	scheduler_init(); /* ALWAYS the last step in setup */
	sei(); /* End Setup - all interrupts */
	static uint16_t now_ms; /* scheduler code :3 */
	for(;;){
		now_ms = atomic_get_ms();
		for (int8_t i=0; i<ARRAY_SIZE(tasks); i++){
			if(now_ms - tasks[i].last_ms >= tasks[i].interval_ms){
				tasks[i].last_ms = now_ms;
				tasks[i].run();
			}
		}
	} /* end scheduler code :3 */
	
	return 0;
}

void blink(void){
	SET(PINB, PB0);
}

void servo_task(void){
	SERVO_B1 = servo_deg(-135);
	SERVO_B2 = servo_deg(+135);
}

void UART_task(void){
	while ( UART_bufchk() ){
		UART_putc(UART_getc());
	}
}
