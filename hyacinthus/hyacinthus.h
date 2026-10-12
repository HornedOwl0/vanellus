#ifndef HYACINTHUS_H
#define HYACINTHUS_H

#include <stdint.h>
#include <stddef.h>
#include <avr/io.h>

#define TASK_DEF(func_ptr, interval, offset) {func_ptr, -(interval-offset), interval}

#ifdef F_CPU 
	#ifndef PRELOAD
		#if ( F_CPU == 16000000UL )
			#define PRELOAD ( 249U )
		#else
			#define PRELOAD ( 124U )
		#endif /* Freq 16 MHz */
	#endif /* PRELOAD */
	
	#ifndef PRESCALER
		#if ( ( F_CPU == 8000000UL ) || ( F_CPU == 16000000UL ) )
			#define PRESCALER (0x3)
		#else // Presume 1MHz
			#define PRESCALER (0x2)
		#endif /* Freq 8MHz or 16MHz */ 
	#endif /* PRESCALER */
#else
	#error "F_CPU is undefined for scheduler"
#endif


struct task_mnt {
	const void (*run)(void);
	volatile uint16_t last_ms;
	const uint16_t interval_ms;
};

extern struct task_mnt task_lst[];
extern size_t task_count;

extern void init(void);

uint16_t atomic_get_ms(void);

#endif /* HYACINTHUS_H */
