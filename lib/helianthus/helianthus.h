#ifndef HELIANTHUS_H
#define HELIANTHUS_H

#include <stddef.h>

#define ADC_CHANNEL(n) ( n&0x08<<2|n&0x07 )

struct ADC_mnt{
  const uint16_t ADCn:4;
  volatile uint16_t val:12;
};

extern struct ADC_mnt ADC_lst[] __attribute__((weak)); // 2 bytes ptr -> +2 bytes per element
extern int8_t ADC_count __attribute__((weak));

uint16_t ADC_force(uint8_t muxbits);

#endif // HELIANTHUS_H
