#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <stdint.h>
#include <Arduino.h>

struct shift_reg {
  const uint8_t STB;
  const uint8_t D;
  const uint8_t CLK;
};

void shift_reg_init(struct shift_reg *pR){
  pinMode(pR->STB, OUTPUT);
  pinMode(pR->D, OUTPUT);
  pinMode(pR->CLK, OUTPUT);
}

void shift_reg_write(struct shift_reg *pR, const uint8_t data, const uint8_t order){
  for (int i=0; i < 8; i++){
    if (order == LSBFIRST){
      digitalWrite(pR->D, !!(data & _BV(i)));
    } else {
      digitalWrite(pR->D, !!(data & _BV(7-i)));
    }
    digitalWrite(pR->CLK, HIGH);
    digitalWrite(pR->CLK, LOW);
  }
  
  digitalWrite(pR->STB, HIGH);
  digitalWrite(pR->STB, LOW);
}

#endif /* SHIFT_REGISTER_H */
