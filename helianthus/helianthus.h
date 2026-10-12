#ifndef HELIANTHUS_H
#define HELIANTHUS_H

#include <stddef.h>

struct ADC_mnt{
  const uint16_t ADCn:4;
  volatile uint16_t val:12;
};

extern struct ADC_mnt ADC_lst[];
extern size_t ADC_count;

void ADC_init(void);

#endif // HELIANTHUS_H
