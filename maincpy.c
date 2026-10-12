#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdfix.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "hibiscus/hibiscus.h"

#include "hyacinthus/hyacinthus.h"

#include "lib/macros.h"

void blink(void);

void init(){
  _delay_ms(5);
  ACM_init();

  SET(DDRB, PB0);
};

/* func_ptr, interval, offset */

struct task_mnt task_lst[] = {
  TASK_DEF(&blink, 500, 35),
};

size_t task_count = ARRAY_SIZE(task_lst);

void blink(void){
  SET(PINB, PB0);
};

void SETCLK(void) __attribute__((naked)) __attribute__((section(".init3")));
void SETCLK(void){
  #if (F_CPU==16000000UL)
    clock_prescale_set(clock_div_1); // 16/1 = 16 MHz
  #elif (F_CPU==8000000UL)
    clock_prescale_set(clock_div_2); // 16/2 = 8 MHz
  #elif (F_CPU==1000000UL)
    clock_prescale_set(clock_div_16); // 16/16 = 1 MHz
  #else 
    #error "(SETCLK) F_CPU wont divide! Try 16/8/1MHz"
  #endif
  return;
}
