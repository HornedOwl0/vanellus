#ifndef HELIANTHUS_H
#define HELIANTHUS_H

#include <stddef.h>

struct ADC_mnt{
  const uint16_t ADCn:4;
  volatile uint16_t val:12;
};

extern struct ADC_mnt ADC_lst[] __attribute__((weak)); // 2 bytes ptr -> +2 bytes per element
extern uint8_t ADC_count __attribute__((weak));

#endif // HELIANTHUS_H
