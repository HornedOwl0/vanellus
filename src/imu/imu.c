#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <avr/io.h>

#include <util/delay.h>
#include <util/twi.h>

#include "hedera.h"

#include "imu.h"

#define IMU_ADDR (0x68) // AD0->GND

struct IMU_data IMU;

void IMU_read(void){
  if ( !TWI_available() ){ return; }

  uint8_t buf[14] = {0};

  if ( !TWI_writebuf(IMU_ADDR, (uint8_t[]){0x3B}, 1, TWI_START, TWI_NOSTOP) ){
    while ( !TWI_available() ){}
    TWI_readinto(IMU_ADDR, buf, sizeof(buf), TWI_START, TWI_STOP);
    while ( !TWI_available() ){}

    int16_t ax_raw = (int16_t)((buf[0]<<8)|(buf[1]));
    int16_t ay_raw = (int16_t)((buf[2]<<8)|(buf[3]));
    int16_t az_raw = (int16_t)((buf[4]<<8)|(buf[5]));

    /*
    int16_t temp_raw = (int16_t)((buf[6]<<8)|(buf[7]));
    (void)temp_raw;
    */

    TWI_readinto(IMU_ADDR, buf, sizeof(buf), TWI_START, TWI_STOP);
    while ( !TWI_available() ){}

    int16_t gx_raw = (int16_t)((buf[8]<<8)|(buf[9]));
    int16_t gy_raw = (int16_t)((buf[10]<<8)|(buf[11]));
    int16_t gz_raw = (int16_t)((buf[12]<<8)|(buf[13]));
    
    IMU.ax = (((int32_t)ax_raw*1000)>>14);
    IMU.ay = (((int32_t)ay_raw*1000)>>14);
    IMU.az = (((int32_t)az_raw*1000)>>14);

    IMU.gx = (((int32_t)gx_raw*100)/131);
    IMU.gy = (((int32_t)gy_raw*100)/131);
    IMU.gz = (((int32_t)gz_raw*100)/131);

    /*
    IMU.ax = ax_raw;
    IMU.ay = ay_raw;
    IMU.az = az_raw;

    IMU.gx = gx_raw;
    IMU.gy = gy_raw;
    IMU.gz = gz_raw;
    */
  }

  return;
}

void __attribute__((constructor)) IMU_init(void){
  static const __flash uint8_t cmd[][2] = { 
    {0x6B, 0x01,}, // PWR_MGMT_1
    {0x6C, 0x00,}, // PWR_MGMT_2
    {0x19, 0x07,}, // SMPLRT_DIV
    {0x1A, 0x03,}, // CONFIG
    {0x1B, 0x00,}, // GYRO_CONFIG
    {0x1C, 0x00,}, // ACCEL_CONFIG
  };
  
  _delay_ms(135);

  setup:
  if (!TWI_start(IMU_ADDR, TW_WRITE)){
    TWI_write(0x6B); // PWR_MGMT_1
    TWI_write(0x80); // RESET
    TWI_stop();
    _delay_ms(135);

    for (int8_t i=0; i<sizeof(cmd)/sizeof(cmd[0]); i++) {
      TWI_start(IMU_ADDR, TW_WRITE);
      TWI_write(cmd[i][0]);
      TWI_write(cmd[i][1]);
    }

    /* Sanity Check */
    uint8_t status = 0x00;
    for (int8_t i=0; i<sizeof(cmd)/sizeof(cmd[0]); i++) {
      TWI_start(IMU_ADDR, TW_WRITE);
      TWI_write(cmd[i][0]);
      TWI_start(IMU_ADDR, TW_READ);
      TWI_read_nack(&status);

      if ( status != cmd[i][1] ) {
        TWI_stop();
        goto setup;
      }
    }

    TWI_stop();
    _delay_ms(135);
  }

  return;
}
