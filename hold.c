#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "hibiscus/hibiscus.h"
#include "hyacinthus/hyacinthus.h"

#include "lib/macros.h"

#define TC4_WRITE(reg, val) ({ TC4H=MSB(val); reg=LSB(val); })
#define TC4_READ(reg, var) ({ var = reg; var |= (uint16_t)(TC4H<<8) })

#define SENSOR_THRESHOLD (720U)
#define SENSOR_DUTY (384U)

/*
static const float Kp = 5.0f;
static const float Ki = 1.0f;
static const float Kd = 0.0f;
*/

static float error = 0.0f;
static float prev_error = 0.0f;
static float sum_error = 0.0f;

static int8_t weights[] = {-2, -1, 0, 1, 2};

/* Helianthus */
#include "helianthus/helianthus.h"
struct ADC_mnt ADC_lst[] = {
  {.ADCn = 4},
  {.ADCn = 5},
  {.ADCn = 8},
  {.ADCn = 6},
  {.ADCn = 7},
};
size_t ADC_count = ARRAY_SIZE(ADC_lst);
/* Helianthus */

void TC4_init(void);
void blink(void);
void raw_ADC(void);
void calc_PID(void);

/* Main Section */
void init(){
  ACM_init();
  TC4_init(); // Provides PWM for the sensor, and the motors
  ADC_init();

  SET(DDRD, PD5);
  SET(DDRB, PB0);
  CLR(PORTD, PD5);

  wdt_enable(WDTO_2S);
  
  return;
}

/* ptr, interval, offset */
struct task_mnt task_lst[] = {
  TASK_DEF(&blink, 100, 0),
  TASK_DEF(&raw_ADC, 500, 20),
  TASK_DEF(&calc_PID, 500, 40),
};
size_t task_count = ARRAY_SIZE(task_lst);
/* Function Definitions */

void blink(void){
  wdt_reset();
  SET(PINB, PB0);
};

void raw_ADC(void){
  static char buf[56];

  memset(buf, '\0', ARRAY_SIZE(buf));
  char temp_buf[8];
  for (int8_t i=0; i<ARRAY_SIZE(ADC_lst); i++){
    itoa(ADC_lst[i].val, temp_buf, 10);
    strcat(buf, temp_buf);
    strcat(buf, (const __memx char*)", ");
  }
  strcat(buf, (const __memx char*)"\r\n");
  ACM_puts(buf);
}

void calc_PID(void){
  static char buf[8];

  memset(buf, '\0', ARRAY_SIZE(buf));

  prev_error = error;

  float sum = 0; // Weighted Sum
  float div = 0; // Count
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    for (int8_t i=0; i<ARRAY_SIZE(ADC_lst); i++){
      div += !!(ADC_lst[i].val>SENSOR_THRESHOLD);
      sum += (int8_t)( (ADC_lst[i].val>SENSOR_THRESHOLD) * weights[i] );
    }
  }

  error = sum/div;
  sum_error += error;

  dtostrf(error, 0, 2, buf);
  strcat(buf, (const __memx char*)"\r\n");
  ACM_puts(buf);

  return;
}

void TC4_init(void){
  /* TC4 Setup - set registers */

  TCCR4A = (1<<PWM4A)|(1<<PWM4B);
  TCCR4B = (1<<PWM4X)|(1<<PSR4);
  TCCR4C = (1<<COM4A0S)|(1<<COM4B0S)|(1<<COM4D0)|(1<<PWM4D); // Write to the shadows instead of TCCR4A
  TCCR4D = (0x01<<WGM40); // Phase and Frequency Correct Mode means halving the frequency calculation/halving the prescaler
  TCCR4E = 0x00;

  /* Push the prescaler */
  /* Remember hibiscus.h starts the PLL@96MHz with postcaler 1.5 for 64MHz */

  TCCR4B |= (0x0A<<CS40); // PCK/2^(N-1)
  /* Fast PWM -- 64MHz/4096 (0Bh) = 15.625kHz */ 
  /* PFCPWM -- 64MHz/2048 (0Ah) = 31.250 kHz (Dual Slope) */ 
  /* 1024 steps -> 15.625 <=> 768 steps -> 20.833kHz -- IDEAL */
  
  TC4_WRITE(OCR4C, 768); // TOP
  
  TC4_WRITE(OCR4A, 384);
  TC4_WRITE(OCR4B, 384);
  TC4_WRITE(OCR4D, CLAMP((768-SENSOR_DUTY), 0, 768));

  DDRC |= (1<<PC6)|(1<<PC7);
  DDRB |= (1<<PB5)|(1<<PB6);
  DDRD |= (1<<PD6)|(1<<PD7);
  
  /* end TC4 setup*/
  return;
}

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
