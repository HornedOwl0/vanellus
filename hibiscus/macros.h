#ifndef MACROS_H
#define MACROS_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ABS(n) ( n<0 ? (uint32_t)(-n) : (uint32_t)(n) )

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

#define REVERSE(b) \
( (b&0x01) << 7 | (b&0x80) >> 7 | \
	(b&0x02) << 5 | (b&0x40) >> 5 | \
	(b&0x04) << 3 | (b&0x20) >> 3 | \
	(b&0x08) << 1 | (b&0x10) >> 1 )

#define SET(REG, POS) (REG |= (1<<POS))
#define CLR(REG, POS) (REG &= ~(1<<POS))
#define TOG(REG, POS) (REG ^= (1<<POS))

#define SETBM(REG, BM) (REG |= (BM) )
#define CLRBM(REG, BM) (REG &= ~(BM) )
#define TOGBM(REG, BM) (REG ^= (BM) )

#define GET(REG, POS) ( !!(REG & _BV(POS)) )

#define MSB(b) (uint8_t)((b>>8)&0xFF)
#define LSB(b) (uint8_t)(b&0xFF)

#endif /* MACROS_H */
