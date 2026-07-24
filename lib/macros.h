#ifndef MACROS_H
#define MACROS_H

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

#endif /* MACROS_H */