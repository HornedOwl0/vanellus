#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "hedera.h"

/* Useful Macros */

#define SET(REG, POS) (REG |= (1<<POS))
#define CLR(REG, POS) (REG &= ~(1<<POS))
#define TOG(REG, POS) (REG ^= (1<<POS))

#define SERVO_MICROS(x) (F_CPU==16000000UL ? x<<1 : x)

/* End Macros */

#define TC3_MODE 14 // Fast PWM ICR3
#define TC3_WGMH ((TC3_MODE&0x0C)>>2)
#define TC3_WGML (TC3_MODE&0x03)

/* Frequency Check */

#if (F_CPU==16000000UL)
  #define ICR3_MATCH (9999U)
  #define TC3_PRESCALER (0x02)
#elif (F_CPU==8000000UL)
  #define ICR3_MATCH (4999U)
  #define TC3_PRESCALER (0x02)
#elif (F_CPU==1000000UL)
  #define ICR3_MATCH (4999U)
  #define TC3_PRESCALER (0x01)
#else 
  #error "(Hedera) F_CPU undefined/unsupported! Try 16/8/1MHz"
#endif

/* List per channel */
extern struct mntlst servo_OCRA_mnt[4] __attribute__((weak));
extern struct mntlst servo_OCRB_mnt[4] __attribute__((weak));
extern struct mntlst servo_OCRC_mnt[4] __attribute__((weak));

/* Init function */
inline void servo_init(void){
	/* TC3 Setup - set registers */
	TCCR3A = (TC3_WGML<<WGM30); // Non-Inverted Mode;
	TCCR3B = (TC3_WGMH<<WGM32)|(TC3_PRESCALER<<CS30);
	/* Interrupt Masks */
	
	TIMSK3 = (1<<TOIE3);
  
  if ( &servo_OCRA_mnt != NULL ){
    TIMSK3 |= (1<<OCIE3A);
  }
  if ( &servo_OCRB_mnt != NULL ){
    TIMSK3 |= (1<<OCIE3B);
  }
  if ( &servo_OCRC_mnt != NULL ){
    TIMSK3 |= (1<<OCIE3C);
  }

	/* 1MHz frequency */
	ICR3 = ICR3_MATCH;

	OCR3A = SERVO_MIN_US;
	OCR3B = SERVO_MIN_US;
	OCR3C = SERVO_MIN_US;

	/* end TC3 setup*/
	return;
}

/* ISR Section */
static volatile int8_t volatile_servo_ctu = 0;
static volatile struct mntlst *lst = NULL;

ISR(TIMER3_OVF_vect){
	volatile_servo_ctu++;

	lst = &servo_OCRA_mnt[volatile_servo_ctu&0x03];
	if ( lst->port != NULL ){
		SET( *(lst->port), lst->mask );
		OCR3A = SERVO_MICROS(lst->micros);
	}

	lst = &servo_OCRB_mnt[volatile_servo_ctu&0x03];
	if ( lst->port != NULL ){
		SET( *(lst->port), lst->mask );
		OCR3B = SERVO_MICROS(lst->micros);
	}
	
	lst = &servo_OCRC_mnt[volatile_servo_ctu&0x03];
	if ( lst->port != NULL ){
		SET( *(lst->port), lst->mask );
		OCR3C = SERVO_MICROS(lst->micros);
	}
}

ISR(TIMER3_COMPA_vect){
	lst = &servo_OCRA_mnt[volatile_servo_ctu&0x03];

	if ( lst->port != NULL ){
		CLR( *(lst->port), lst->mask );
	}
}

ISR(TIMER3_COMPB_vect){
	lst = &servo_OCRB_mnt[volatile_servo_ctu&0x03];

  if ( lst->port != NULL ){
		CLR( *(lst->port), lst->mask );
	}
}

ISR(TIMER3_COMPC_vect){
	lst = &servo_OCRC_mnt[volatile_servo_ctu&0x03];

	if ( lst->port != NULL ){
		CLR( *(lst->port), lst->mask );
	}
}
