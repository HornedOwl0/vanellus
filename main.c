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
#include "helianthus/helianthus.h"
#include "hydrangea/hydrangea.h"

#include "lib/macros.h"

#define TC4_WRITE(reg, val) ({ TC4H=MSB(val); reg=LSB(val); })
#define TC4_READ(reg, var) ({ var = reg; var |= (uint16_t)(TC4H<<8) })

#define MOTOR_SPD_DEFAULT ( 432U )

#define SENSOR_THRESHOLD (750U)
#define SENSOR_DUTY (384U)
// #define INVERTED_PWM // Undefine this for CD4011 Later

/* Helianthus */
struct ADC_mnt ADC_lst[] = {
  {.ADCn = 4},
  {.ADCn = 5},
  {.ADCn = 8},
  {.ADCn = 6},
  {.ADCn = 7},
}; uint8_t ADC_count = ARRAY_SIZE(ADC_lst);

/* Global Static Allocation */
static const float Kp = 50.0f;
static const float Ki = 0.2f;
static const float Kd = 1.0f;

static float error = 0.0f;
static float prev_error = 0.0f;
static float sum_error = 0.0f;

static float pid_out = 0.0f;

static int8_t weights[] = {2, 1, 0, -1, -2};

/* Function Prototypes */

void blink(void);
void calc_PID(void);
void motor_ctrl(void);

/* Hydrangea -> ptr, interval, offset */
static struct htask task_lst[] = {
  TASK_DEF(&blink, 100, 0),
  TASK_DEF(&calc_PID, 1, 35),
  TASK_DEF(&motor_ctrl, 10, 65),
};

/* Main Section */
int main(void){
  cli();

  // ACM_init() -> Hibiscus: Starts PLL too!
  // ADC_init() -> Helianthus: Very fast ISR, read values atomically please
  // TC4_init() -> Provides PWM (PFCPWM) for the sensor, and the motors

  SET(DDRD, PD5); // static
  SET(DDRB, PB0); // blink
  CLR(PORTD, PD5);

  SET(DDRB, PB3); // static
  SET(DDRB, PB2); // static
  SET(DDRB, PB4); // static
  SET(DDRE, PE6); // static
                 //
  CLR(PORTE, PE6); // static
  SET(PORTE, PE6); // static
  SET(PORTB, PB4); // static
  SET(PORTB, PB3); // static

  wdt_enable(WDTO_120MS); // Shorter than usual timeout

  sei();

  for(;;){
    wdt_reset();
    
    task_run(task_lst, ARRAY_SIZE(task_lst));
  }
  
  return 0;
}

/* Function Definitions */
void blink(void){
  SET(PINB, PB0);
};

void calc_PID(void){
  static char buf[56] = {0};
  char temp_buf[8] = {0};

  memset(buf, '\0', ARRAY_SIZE(buf));

  float sum = 0; // Weighted Sum
  float div = 0; // Count
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    for (int8_t i=0; i<ARRAY_SIZE(ADC_lst); i++){
      itoa(ADC_lst[i].val, temp_buf, 10);
      strcat(buf, temp_buf);
      strcat(buf, (const __memx char*)", ");

      div += !!(ADC_lst[i].val>SENSOR_THRESHOLD);
      sum += (int8_t)( (ADC_lst[i].val>SENSOR_THRESHOLD) * weights[i] );
    }
  }
  float x = sum/div;

  if ( x==x ){
    prev_error = error;
    error = x;
    sum_error = CLAMP((sum_error+error),-255, +255);
  }

  float derivative = error-prev_error;
  float integral = sum_error;
  float proportional = error;

  pid_out = (Kp*proportional) + (Ki*integral) + (Kd*derivative);

  dtostrf(pid_out, 0, 2, temp_buf);
  strcat(buf, temp_buf);
  strcat(buf, (const __memx char*)"\r\n");
  ACM_puts(buf);

  return;
}

void motor_ctrl(){
  static int16_t lspd = 0;
  static int16_t rspd = 0;

  lspd = MOTOR_SPD_DEFAULT - (int16_t)pid_out;
  rspd = MOTOR_SPD_DEFAULT + (int16_t)pid_out;

  TC4_WRITE(OCR4A, CLAMP(lspd, 0, 768));
  TC4_WRITE(OCR4B, CLAMP(rspd, 0, 768));
}

static inline void __attribute__((constructor)) TC4_init(void){
  /* TC4 Setup - set registers */
  TCCR4A = (1<<PWM4A)|(1<<PWM4B);

  #ifdef INVERTED_PWM
  TCCR4B = (1<<PWM4X)|(1<<PSR4);
  #else
  TCCR4B = (0<<PWM4X)|(1<<PSR4);
  #endif

  TCCR4C = (1<<COM4A0S)|(1<<COM4B0S)|(1<<COM4D0)|(1<<PWM4D); // Write to the shadows instead of TCCR4A
  TCCR4D = (0x01<<WGM40); // Phase and Frequency Correct Mode means halving the frequency calculation/halving the prescaler
  TCCR4E = 0x00;

  DT4 = 0x00;

  /* Push the prescaler */
  /* Remember hibiscus.h starts the PLL@96MHz with postcaler 1.5 for 64MHz */

  TCCR4B |= (0x02<<CS40); // PCK/2^(N-1)
  /* Fast PWM -- 64MHz/4*1024 (03h) = 15.625kHz */ 
  /* PFCPWM -- 64MHz/2*1024 (02h) = 31.250 kHz (Dual Slope -> 15.625kHz) */ 
  /* 1024 steps -> 15.625 <=> 768 steps -> 20.833kHz -- IDEAL */
  
  TC4_WRITE(OCR4C, 768); // TOP
  
  TC4_WRITE(OCR4A, MOTOR_SPD_DEFAULT);
  TC4_WRITE(OCR4B, MOTOR_SPD_DEFAULT);

  #ifdef INVERTED_PWM
  TC4_WRITE(OCR4D, CLAMP((768-SENSOR_DUTY), 0, 768));
  #else
  TC4_WRITE(OCR4D, CLAMP(SENSOR_DUTY, 0, 768));
  #endif

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
