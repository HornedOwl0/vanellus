#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdfix.h>
#include <string.h>

#include <math.h>
#ifndef M_PI
  #define M_PI 3.14159f
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>
#include <util/twi.h>

#include <hibiscus.h> // this is special and should always be included
#include "hydrangea.h"
#include "hyacinthus.h"
#include "hedera.h"
#include "helianthus.h"

#include "framebuf.h"
#include "imu.h"
#include "spi.h"
#include "oled.h"

#include "macros.h"
#include "config.h"

/* Helianthus */
struct ADC_mnt ADC_lst[] = {
  {.ADCn = 4}, // Edge Sensor
  {.ADCn = 5},
}; int8_t ADC_count = ARRAY_SIZE(ADC_lst);

/* Global Static Allocation */

static float error = 0.0f;
static float prev_error = 0.0f;
static float sum_error = 0.0f;

static float pid_out = 0.0f;

static int8_t weights[] = {-12, +12};

static float accPitch = 0;
static float accRoll = 0;

static int16_t lspd;
static int16_t rspd;

/* Non-Task Function Prototypes */

void clr_leds(void);
void sanity_check(void);
void try_find_line(void);

/* Task Function Prototypes */

void display_task(void);
void IMU_task(void);

void calc_PID(void);
void motor_ctrl(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  TASK_DEF(&display_task, 85, 17),
  TASK_DEF(&IMU_task, 135, 43),

  TASK_DEF(&calc_PID, 20, 173),
  TASK_DEF(&motor_ctrl, 50, 179),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  wdt_enable(WDTO_2S);

  clr_leds(); // After Bootloader/Flashing

  wdt_enable(WDTO_120MS);

	sei(); /* End Setup - all interrupts */

  /* Pre Loop Setup */
    
  try_find_line(); // You spin me right round baby

  /* Task Loop */

	for(;;){
    wdt_reset();
    task_run(task_main, ARRAY_SIZE(task_main));
  }

	return 0;
}

/* Non-Task Function Definitions */

void clr_leds(void){
  gpio_t LED_STATIC = {&PIND, &DDRD, &PORTD, PD5};
  gpio_output(&LED_STATIC);
  gpio_set(&LED_STATIC);

  gpio_t LED_BLINK = {&PINB, &DDRB, &PORTB, PB0};
  gpio_output(&LED_BLINK);
  gpio_set(&LED_BLINK); // SET SLAVE SELECT AS HIGH

  return;
}

void try_find_line(void){
  // Steer Hard Right;
  
  SPI_write(RIGHT_REV);

  for(;;){
    int16_t ctu = 0; // Count
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
      for (int8_t i=0; i<ADC_count; i++){
        ctu +=  !!(ADC_lst[i].val<SENSOR_THRESHOLD);
      }
    }

    _delay_ms(1);
    if (ctu)
      break;
  }

}

/* Task Function Definitions */

void display_task(void){
  char buf[16] = {0}; // Temp Buf

  framebuf_fill(0);
  
  framebuf_puts(0, 0, FRAMEBUF_SMALL, (const __memx char*)"ATT");
  framebuf_puts(0, 16, FRAMEBUF_SMALL, (const __memx char*)"P:");
  framebuf_puts(0, 24, FRAMEBUF_SMALL, (const __memx char*)"R:");

  framebuf_puts(64, 0, FRAMEBUF_SMALL, (const __memx char*)"SPD");
  framebuf_puts(64, 16, FRAMEBUF_SMALL, (const __memx char*)"L:");
  framebuf_puts(64, 24, FRAMEBUF_SMALL, (const __memx char*)"R:");

  dtostrf(accPitch, 0, 0, buf);
  framebuf_puts(0+16, 16, FRAMEBUF_SMALL, buf); // Put
  
  dtostrf(accRoll, 0, 0, buf);
  framebuf_puts(0+16, 24, FRAMEBUF_SMALL, buf); // Put
  
  itoa(lspd, buf, 10);
  framebuf_puts(64+16, 16, FRAMEBUF_SMALL, buf); // Put
  
  itoa(rspd, buf, 10);
  framebuf_puts(64+16, 24, FRAMEBUF_SMALL, buf); // Put

  OLED_refresh();
  
  return;
}

void IMU_task(void){
  IMU_read();

  accRoll = atan2f(-IMU.ax, sqrtf(powf(IMU.ay,2) + powf(IMU.az, 2))) * 180/M_PI;
  accPitch = atan2f(IMU.ay, sqrtf(powf(IMU.ax,2) + powf(IMU.az, 2))) * 180/M_PI;

  return;
}

void calc_PID(void){
  static char buf[56] = {0};
  char temp_buf[8] = {0};

  memset(buf, '\0', ARRAY_SIZE(buf));

  int16_t sum = 0; // Weighted Sum
  int16_t div = 0; // Count
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    for (int8_t i=0; i<ADC_count; i++){
      itoa(ADC_lst[i].val, temp_buf, 10);
      strcat(buf, temp_buf);
      strcat(buf, (const __memx char*)", ");

      int8_t sensor_state =  !!(ADC_lst[i].val<SENSOR_THRESHOLD);

      div += sensor_state;
      sum += (int8_t)( sensor_state * weights[i] );
    }
  }

  float x = (float)sum/(float)div;
  
  if ( x==x ){
    prev_error = error;
    error = x;
    sum_error = CLAMP((sum_error+error),-96, +96);
  }

  float derivative = error-prev_error;
  float integral = sum_error;
  float proportional = error;

  pid_out = (PID_KP*proportional) + (PID_KI*integral) + (PID_KD*derivative);

  dtostrf(pid_out, 0, 2, temp_buf);
  strcat(buf, temp_buf);
  strcat(buf, (const __memx char*)"\r\n");
  ACM_puts(buf);

  return;
}

#define TC4_WRITE(reg, val) ({ TC4H=MSB(val); reg=LSB(val); })
#define TC4_READ(reg, var) ({ var = reg; var |= (uint16_t)(TC4H<<8); })

void motor_ctrl(){
  int16_t pid_val = (int16_t)pid_out;

  lspd = CLAMP(((MOTOR_SPD_OFFSET_L + MOTOR_SPD_FORWARD) - pid_val), -768, 768);
  rspd = CLAMP(((MOTOR_SPD_OFFSET_R + MOTOR_SPD_FORWARD) + pid_val), -768, 768);
  
  TC4_WRITE(OCR4A, ABS(lspd));
  TC4_WRITE(OCR4B, ABS(rspd));
  
  if ( lspd < MOTOR_SPD_DEAD )
    SPI_write(LEFT_REV);
  else if ( rspd < MOTOR_SPD_DEAD )
    SPI_write(RIGHT_REV);
  else
    SPI_write(FWD);

  return;
}

static inline void __attribute__((constructor)) TC4_init(void){
  /* TC4 Setup - set registers */
  TCCR4A = (1<<PWM4A)|(1<<PWM4B);

  #if (INVERTED_PWM == 1)
  TCCR4B = (1<<PWM4X)|(1<<PSR4);
  #else
  TCCR4B = (0<<PWM4X)|(1<<PSR4);
  #endif

  TCCR4C = (1<<COM4A0S)|(1<<COM4B0S); // Write to the shadows instead of TCCR4A
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

  DDRC |= (1<<PC6)|(1<<PC7);
  DDRB |= (1<<PB5)|(1<<PB6);
  DDRD |= (1<<PD6)|(1<<PD7);
  
  /* end TC4 setup*/
  return;
}
