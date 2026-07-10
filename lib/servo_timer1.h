#ifndef SERVO_TIMER1_H
#define SERVO_TIMER1_H

#include <stdint.h>
#include <avr/io.h>
#include <util/atomic.h>

#ifdef F_CPU
 
	#if !( defined(TCCR1A_MSK) || defined (TCCR1B_MSK) || defined(SERVO_PRESCALER) || defined(ICR1_VAL) )
		#if ( F_CPU == 16000000UL )
			#define TCCR1A_MSK (1<<COM1A1)|(1<<COM1B1) // Non-Inverted Mode
			#define TCCR1B_MSK (1<<WGM13) // Phase and Frequency Correct ICR1 - Mode 8
			#define ICR1_VAL 20000U
			#define SERVO_PRESCALER 0x2 // F_CPU/8
		#elif ( F_CPU == 8000000UL )
			#define TCCR1A_MSK (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11) // Non-Inverted Mode
			#define TCCR1B_MSK (1<<WGM13)|(1<<WGM12) // Fast PWM ICR1 - Mode 14
			#define ICR1_VAL 19999U
			#define SERVO_PRESCALER 0x2 // F_CPU/8
		#else // Presume F_CPU=1MHz
			#define TCCR1A_MSK (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11) // Non-Inverted Mode
			#define TCCR1B_MSK (1<<WGM13)|(1<<WGM12)// Fast PWM ICR1 - Mode 14
			#define ICR1_VAL 19999U
			#define SERVO_PRESCALER 0x1 // F_CPU/1
		#endif /* Freq 16 MHz */
	#endif /* Masks Defined */

#define SERVO_B1 OCR1A
#define SERVO_B2 OCR1B

static inline int16_t servo_mil(int32_t angle){
	return (int16_t)((angle<0?2000:1000) + ( ( (angle)*1000)/3140 ));
}

static inline int16_t servo_deg(int32_t angle){
	return (int16_t)((angle<0?2000:1000) + ( ( (angle)*1000 )/180 ));
}

/* 
you may set the value in raw microseconds 1000~2000us width 
OR
use one of the above functions to set position in degrees or miliradians
*/

static inline void servo_init(void){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		TCCR1A = TCCR1A_MSK;
		TCCR1B = TCCR1B_MSK; 
		
		ICR1 = ICR1_VAL;
		
		OCR1A = 1000;
		OCR1B = 1000;
		
		TCCR1B |= (SERVO_PRESCALER<<CS10);
		
		DDRB |= (1<<PB1)|(1<<PB2);
	}
	return;
}

#else
	#error "F_CPU is undefined for servo_timer1"
#endif /* F_CPU */

#endif /* SERVO_TIMER1_H */