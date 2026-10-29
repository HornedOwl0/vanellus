#ifndef AUTO_REGISTER_H
#define AUTO_REGISTER_H

#include <stdint.h>

struct auto_reg {
  const uint8_t D;
  const uint8_t CLK;
};

void auto_reg_init(const struct auto_reg *restrict pR);

void auto_reg_write(const struct auto_reg *restrict pR, const uint8_t data);

#endif /* AUTO_REGISTER_H */
