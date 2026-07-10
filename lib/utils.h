#ifndef UTILS_H
#define UTILS_H

#include <avr/io.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ABS(n) ( n<0 ? (uint32_t)(-n) : (uint32_t)(n) )

#define REVERSE(b) \
( (b&0x01) << 7 | (b&0x80) >> 7 | \
	(b&0x02) << 5 | (b&0x40) >> 5 | \
	(b&0x04) << 3 | (b&0x20) >> 3 | \
	(b&0x08) << 1 | (b&0x10) >> 1 )

#define SET(REG, POS) (REG |= _BV(POS))
#define CLR(REG, POS) (REG &= ~_BV(POS))
#define TOG(REG, POS) (REG ^= _BV(POS))

#define READ(REG, POS) ( !!(REG & _BV(POS)) )

#define CFG_OUTPUT(REG, PIN) (REG |= _BV(PIN))
#define CFG_INPUT(REG, PIN) (REG &= ~_BV(PIN))

#define pin_to_port(p) ( (p) <= (7) ? &PORTD : ( (p) <= (13) ? &PORTB : &PORTC ) )
#define pin_to_ddrx(p) ( (p) <= (7) ? &DDRD : ( (p) <= (13) ? &DDRB : &DDRC ) )

#define pin_to_shift(p) ( (p) <= (7) ? p : ( (p) <= (13) ? p-8 : p-14 ) )
#define pin_to_mask(p) ( (p) <= (7) ? _BV(p) : ( (p) <= (13) ? _BV(p-8) : _BV(p-14) ) )

#define io_mode(p, mode) \
( (mode) ? (*pin_to_ddrx(p) |= pin_to_mask(p)) : (*pin_to_ddrx(p) &= ~pin_to_mask(p)) )
#define io_write(p, state) \
( (state) ? (*pin_to_port(p) |= pin_to_mask(p)) : (*pin_to_port(p) &= ~pin_to_mask(p)) )
#define io_toggle(p) ( *pin_to_port(p) ^= pin_to_mask(p) )

#define D0 (0)
#define D1 (1)
#define D2 (2)
#define D3 (3)
#define D4 (4)
#define D5 (5)
#define D6 (6)
#define D7 (7)

#define B0 (8)
#define B1 (9)
#define B2 (10)
#define B3 (11)
#define B4 (12)
#define B5 (13)

#define C0 (14)
#define C1 (15)
#define C2 (16)
#define C3 (17)
#define C4 (18)
#define C5 (19)

#ifndef HIGH
  #define HIGH (1)
#endif

#ifndef LOW
  #define LOW (0)
#endif

#ifndef OUTPUT
  #define OUTPUT (1)
#endif

#ifndef INPUT
  #define INPUT (0)
#endif

#endif // UTILS_H