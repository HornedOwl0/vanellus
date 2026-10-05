#ifndef HEDERA_H
#define HEDERA_H

#include <stdint.h>

/* Servo Macros */
#if !defined(SERVO_MIN_US) || !defined(SERVO_MAX_US)
  #define SERVO_MIN_US 480U
  #define SERVO_MAX_US 2400U
#endif
#define SERVO_RANGE (SERVO_MAX_US - SERVO_MIN_US)
#define SERVO_MICROS(x) (F_CPU==16000000UL ? x<<1 : x)

struct mntlst{
	volatile uint8_t *port;
	const uint16_t mask:4;
	volatile uint16_t micros:12;
};

/* Starts TC3 and interrupts */
void servo_init(void);

#endif /* HEDERA_H */
