#ifndef MACROS_H
#define MACROS_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

#define SET(REG, POS) (REG |= (1<<POS))
#define CLR(REG, POS) (REG &= ~(1<<POS))
#define TOG(REG, POS) (REG ^= (1<<POS))

#define SETBM(REG, BM) (REG |= (BM) )
#define CLRBM(REG, BM) (REG &= ~(BM) )
#define TOGBM(REG, BM) (REG ^= (BM) )

#define GET(REG, POS) ( !!(REG & (1<<POS)) )
#define GETBM(REG, BM) ( !!(REG & (BM)) )

#define MSB(b) (uint8_t)((b>>8)&0xFF)
#define LSB(b) (uint8_t)(b&0xFF)

#endif /* MACROS_H */