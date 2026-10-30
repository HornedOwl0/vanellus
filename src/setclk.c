#include <avr/power.h>

void __attribute__((naked)) __attribute__((section(".init3"))) SETCLK(void){
  #if (F_CPU==16000000UL)
    clock_prescale_set(clock_div_1); // 16/1 = 16 MHz
  #elif (F_CPU==8000000UL)
    clock_prescale_set(clock_div_2); // 16/2 = 8 MHz
  #else 
    #error "(SETCLK) F_CPU wont divide! Try 16/8/MHz"
  #endif
  return;
}
