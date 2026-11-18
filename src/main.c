#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdfix.h>
#include <string.h>

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
#include "oled.h"

#include "macros.h"
#include "config.h"

/* Helianthus */
struct ADC_mnt ADC_lst[] = {
  {.ADCn = 4}, // Edge Sensor
  {.ADCn = 5},
}; int8_t ADC_count = sizeof(ADC_lst);

/* Global Static Allocation */

static const float Kp = 14.4f;
static const float Ki = 1.0f;
static const float Kd = 3.6f;

static float error = 0.0f;
static float prev_error = 0.0f;
static float sum_error = 0.0f;

static float pid_out = 0.0f;

static int8_t weights[] = {-12, -8, 0, +8, +12};

/* Non-Task Function Prototypes */

void SPI_write(uint8_t data);

/* Task Function Prototypes */

void update_counter(void);
void calc_PID(void);
void motor_ctrl(void);

/* Hydrangea -> ptr, interval, offset */
task_t task_main[] = {
  TASK_DEF(&OLED_refresh, 85, 65),
  TASK_DEF(&IMU_read, 135, 165),

  TASK_DEF(&calc_PID, 5, 173),
  TASK_DEF(&motor_ctrl, 10, 179),

  TASK_DEF(&update_counter, 100, 115),
};

int main(void){
	cli(); /* Begin Setup - no interrupts */

  {
    gpio_t LED_STATIC = {&PIND, &DDRD, &PORTD, PD5};
    gpio_output(&LED_STATIC);
    gpio_set(&LED_STATIC);
    gpio_t LED_BLINK = {&PINB, &DDRB, &PORTB, PB0};
    gpio_output(&LED_BLINK);
    gpio_set(&LED_BLINK); // SET SLAVE SELECT AS HIGH
  }

  wdt_enable(WDTO_120MS);

	sei(); /* End Setup - all interrupts */

	for(;;){
    wdt_reset();
    task_run(task_main, ARRAY_SIZE(task_main));
  }

	return 0;
}

void update_counter(void){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    char buf[16] = {0}; // Temp Buf

    framebuf_fill(0);
    
    framebuf_puts(8, 0, FRAMEBUF_SMALL, (const __memx char*)"ACCEL");

    framebuf_puts(0, 8, FRAMEBUF_SMALL, (const __memx char*)"X"); // Put
    framebuf_puts(0, 16, FRAMEBUF_SMALL, (const __memx char*)"Y"); // Put
    framebuf_puts(0, 24, FRAMEBUF_SMALL, (const __memx char*)"Z"); // Put

    ltoa(IMU.ax, buf, 10); // Get
    framebuf_puts(16, 8, FRAMEBUF_SMALL, buf); // Put
    
    ltoa(IMU.ay, buf, 10); // Get
    framebuf_puts(16, 16, FRAMEBUF_SMALL, buf); // Put
    
    ltoa(IMU.az, buf, 10); // Get
    framebuf_puts(16, 24, FRAMEBUF_SMALL, buf); // Put
    
    framebuf_puts(80, 0, FRAMEBUF_SMALL, (const __memx char*)"GYRO");
    
    ltoa(IMU.gx, buf, 10); // Get
    framebuf_puts(80, 8, FRAMEBUF_SMALL, buf); // Put
    
    ltoa(IMU.gy, buf, 10); // Get
    framebuf_puts(80, 16, FRAMEBUF_SMALL, buf); // Put
    
    ltoa(IMU.gz, buf, 10); // Get
    framebuf_puts(80, 24, FRAMEBUF_SMALL, buf); // Put
  }
  
  return;
}

void calc_PID(void){
  static char buf[56] = {0};
  char temp_buf[8] = {0};

  memset(buf, '\0', ARRAY_SIZE(buf));

  float sum = 0; // Weighted Sum
  float div = 0; // Count
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

  float x = sum/div;
  
  if ( x==x ){
    prev_error = error;
    error = x;
    sum_error = CLAMP((sum_error+error),-96, +96);
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

#define TC4_WRITE(reg, val) ({ TC4H=MSB(val); reg=LSB(val); })
#define TC4_READ(reg, var) ({ var = reg; var |= (uint16_t)(TC4H<<8) })

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
  
  SPI_write(dir);

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
