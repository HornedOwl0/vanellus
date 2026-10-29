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
#include "hyacinthus/hyacinthus.h"

#include "lib/macros.h"

#define TC4_WRITE(reg, val) ({ TC4H=MSB(val); reg=LSB(val); })
#define TC4_READ(reg, var) ({ var = reg; var |= (uint16_t)(TC4H<<8) })

#define MOTOR_SPD_FORWARD ( 384 )
#define MOTOR_SPD_OFFSET_L ( +0 )
#define MOTOR_SPD_OFFSET_R ( +0 )
#define MOTOR_SPD_DEAD ( 0 ) // Reverse Threshold

#define SENSOR_DUTY (48U)
#define VBANDGAP ( 1110 ) // millivolts

#define STATE_DEFAULT ( 0 )

#define FWD (0x55)
#define LEFT_REV (0x99)
#define RIGHT_REV (0x66)
#define STP (0xFF)
// reverse both is an impossibility!

#define MIN_RUNTIME_MS ( 3000 )
#define DEBOUNCE_MS ( 100 )
#define MAX_BAD_CALCULATIONS ( 240U )

#define INVERTED_PWM

/* Global Static Allocation */
static const float Kp = 19.2f;
static const float Ki = 0.2f;
static const float Kd = 3.6f;

static float error = 0.0f;
static float prev_error = 0.0f;
static float sum_error = 0.0f;

static float pid_out = 0.0f;

static int8_t weights[] = {-24, 0, +24};
static int8_t state = STATE_DEFAULT;
static int32_t vcc_mv = 0;

static uint16_t thresholds[] = {930, 915, 700};

static uint16_t stateswap_tick = 0;

static struct {
  gpio_t data;
  gpio_t clk;
} reg_mnt = {
  {&PINB, &DDRB, &PORTB, PB2},
  {&PINB, &DDRB, &PORTB, PB3},
};

void reg_write(uint8_t val){
  gpio_output(&reg_mnt.data);
  gpio_output(&reg_mnt.clk);
  for (int8_t i=7; i>=0; i--) {
    gpio_clr(&(reg_mnt.clk));
    if ( val&(1<<i) ){
      gpio_set(&(reg_mnt.data));
    } else {
      gpio_clr(&(reg_mnt.data));
    }
    gpio_set(&(reg_mnt.clk));
  }
}


/* Helianthus */
struct ADC_mnt ADC_lst[] = {
  {.ADCn = 7}, // Edge Sensor
  // {.ADCn = 5},
  {.ADCn = 8},
  // {.ADCn = 6},
  {.ADCn = 4},
}; int8_t ADC_count = ARRAY_SIZE(ADC_lst);

/* Task Function Prototypes */
void blink_stp(void);
void sensor_debug(void);

void blink_run(void);
void calc_PID(void);
void motor_ctrl(void);

void get_vcc(void);

/* Non-Task Function Prototypes */
void reset_switch(void);

/* Hydrangea -> ptr, interval, offset */
static struct htask task_stp[] = {
  TASK_DEF(&blink_stp, 500, 0),
  TASK_DEF(&sensor_debug, 100, 35),
};

static struct htask task_main[] = {
  TASK_DEF(&blink_run, 100, 0),
  TASK_DEF(&calc_PID, 5, 35),
  TASK_DEF(&motor_ctrl, 10, 65),
};

static struct htask task_essential[] = {
  TASK_DEF(&get_vcc, 1000, 115),
};

ISR(INT2_vect){
  EIMSK &= ~(1<<INT2);
  EIFR |= (1<<INTF2);

  stateswap_tick = hydrangea_tick();

  switch (state){
    case 0:
      state++;
      break;
    case 1:
      state--;
      break; 
  }
  
  error = 0.0f;
  prev_error = 0.0f;
  sum_error = 0.0f;

  pid_out = 0.0f;
}

/* Main Section */
int main(void){
  cli();

  // ACM_init() -> Hibiscus: Starts PLL too!
  // ADC_init() -> Helianthus: Very fast ISR, read values atomically please
  // TC4_init() -> Provides PWM (PFCPWM) for the sensor, and the motors
  //
  // Toggle High Speed ADC & High Frequency 
  ADCSRA = (ADCSRA & 0xF1)|(0x05<<ADPS0);
  SET(ADCSRB, ADHSM);

  // Set PD2 with Pull Up
  CLR(DDRD, PD2);
  SET(PORTD, PD2);
  // Set INT2 (PD2) on FALLING EDGE
  EICRA = (0x02<<ISC20); 
  // Clear Flags and Enable INT2
  EIFR |= (1<<INTF2);
  EIMSK = (1<<INT2);

  SET(DDRD, PD5); // static
  SET(DDRB, PB0); // blink

  reg_write(0x00);

  wdt_enable(WDTO_120MS); // Shorter than usual timeout

  sei();

  for(;;){
    wdt_reset();
    
    switch (state){
      case 0:
        task_run(task_stp, ARRAY_SIZE(task_stp));
        break;
      case 1:
        task_run(task_main, ARRAY_SIZE(task_main));
        break;
    }

    task_run(task_essential, ARRAY_SIZE(task_essential));
    
    reset_switch();
  }
  
  return 0;
}

/* Function Definitions */
void blink_run(void){
  CLR(PORTD, PD5);
  SET(PINB, PB0);
  return;
};

void blink_stp(void){
  CLR(PORTB, PB0);
  SET(PIND, PD5);
  return;
};

void sensor_debug(void){
  static char screenshot_buf[48] = {0};
  static char buf[48] = {0};
  char temp_buf[12] = {0};

  #ifdef INVERTED_PWM
    TC4_WRITE(OCR4A, 0);
    TC4_WRITE(OCR4B, 0);
  #else
    TC4_WRITE(OCR4A, 768);
    TC4_WRITE(OCR4B, 768);
  #endif

  reg_write(STP);

  memset(buf, '\0', ARRAY_SIZE(buf));

  if (vcc_mv<4000){
    memset(screenshot_buf, '\0', ARRAY_SIZE(screenshot_buf));
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    for (int8_t i=0; i<ADC_count; i++){
      itoa(ADC_lst[i].val, temp_buf, 10);
      strcat(buf, temp_buf);
      strcat(buf, (const __memx char*)", ");
    }
  }
  ltoa(vcc_mv, temp_buf, 10);
  strcat(buf, temp_buf);
  strcat(buf, (const __memx char*)"mV");

  if (vcc_mv<4000){
    memcpy(screenshot_buf, buf, ARRAY_SIZE(screenshot_buf));
    strcat(screenshot_buf, (const __memx char*)" | ");
  }
  strcat(buf, (const __memx char*)"\r\n");

  ACM_puts(screenshot_buf);
  ACM_puts(buf);

  return;
}

void calc_PID(void){
  static int16_t timeout = MAX_BAD_CALCULATIONS;

  static char buf[56] = {0};
  char temp_buf[8] = {0};

  memset(buf, '\0', ARRAY_SIZE(buf));

  strcat(buf, (const __memx char*)"T:");
  itoa(timeout, temp_buf, 10);
  strcat(buf, temp_buf);
  strcat(buf, (const __memx char*)" | ");

  float sum = 0; // Weighted Sum
  float div = 0; // Count
  int8_t stopcheck = 0; // Stop check moved from motor_ctrl()  
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    for (int8_t i=0; i<ADC_count; i++){
      itoa(ADC_lst[i].val, temp_buf, 10);
      strcat(buf, temp_buf);
      strcat(buf, (const __memx char*)", ");

      int8_t sensor_state =  !!(ADC_lst[i].val<thresholds[i]);

      stopcheck |= ( (sensor_state) << i );

      div += sensor_state;
      sum += (int8_t)( sensor_state * weights[i] );
    }
  }

  if ( (hydrangea_tick()-stateswap_tick) > MIN_RUNTIME_MS ){
    if ( (stopcheck==0x1F) || (stopcheck==0x1C) || (stopcheck==0x07) || ( timeout<0 ) ){
      // these correspond to: all off, all right&centre, all left&centre, all active
      state = 0;
      return;
    }
  }

  float x = sum/div;
  
  if ( x==x ){
    timeout = MAX_BAD_CALCULATIONS;

    prev_error = error;
    error = x;
    sum_error = CLAMP((sum_error+error),-96, +96);
  } else {
    timeout--;
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
  int16_t lspd = 0;
  int16_t rspd = 0;

  int16_t pid_val = (int16_t)pid_out;

  int8_t dir = FWD;

  lspd = (MOTOR_SPD_OFFSET_L + MOTOR_SPD_FORWARD) - pid_val;
  rspd = (MOTOR_SPD_OFFSET_R + MOTOR_SPD_FORWARD) + pid_val;

  if ( lspd < MOTOR_SPD_DEAD ){
    dir = LEFT_REV;
  } 
  if ( rspd < MOTOR_SPD_DEAD ){
    dir = RIGHT_REV;
  }

  TC4_WRITE(OCR4A, CLAMP(ABS(lspd), 0, 768));
  TC4_WRITE(OCR4B, CLAMP(ABS(rspd), 0, 768));
  
  reg_write(dir);

  return;
}

void get_vcc(void){
  uint16_t ADCr = ADC_force(0x1E); // Force 1.1V Bandgap read
  vcc_mv = ( ((uint32_t)VBANDGAP*1024)/ADCr ); // 1100mV * 1024/ADCr -> Where a reading of 1024 is just VCC=1.1V
  return;
}

void reset_switch(void){
  if ( !GET(EIMSK, INT2) && GET(PIND, PD2) && ( (hydrangea_tick()-stateswap_tick)>DEBOUNCE_MS ) ){
    SET(EIFR, INTF2); // CLR
    SET(EIMSK, INT2); // ENABLE
  }
  return;
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
  
  TC4_WRITE(OCR4A, 0);
  TC4_WRITE(OCR4B, 0);

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
