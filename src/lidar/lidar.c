#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>

#include <util/delay.h>
#include <util/twi.h>

#include "hedera.h"

#define LIDAR_ADDR ( 0x29 )

int16_t LIDAR_get(void){
  if (!TWI_available()) { return -1; }

  uint8_t buf[2] = {0};
  
  if (!TWI_start(LIDAR_ADDR, TW_WRITE)){
    TWI_write(0x00); TWI_write(0x62);
    TWI_start(LIDAR_ADDR, TW_READ);
    TWI_read_ack(&buf[0]); TWI_read_nack(&buf[1]);
    
    TWI_stop();
  }

  return (uint16_t)( ((buf[0]<<8)|(buf[1]))>>7 ); // Format 9.7, erase the fractional part for result in mm
}

void __attribute__((constructor)) LIDAR_init(void){
  static const __flash uint8_t cmd[][3] = { // VL6180X
    {0x00, 0x1B, 16, }, // SYSRANGE_INTERMEASUREMENT_PERIOD (0-254) -> [10(n+1)] ms
    {0x00, 0x1B, 63, }, // SYSRANGE_MAX_CONVERGENCE_TIME (0-63) -> ms
    {0x00, 0x18, 0x03, }, // SYSRANGE_START -> Set Continuous + Set Start
  };

  _delay_ms(35);
  
  if (!TWI_start(LIDAR_ADDR, TW_WRITE)){
    uint8_t hold = 0x00;

    TWI_write(0x00); TWI_write(0x00); // IDENTIFICATION_MODEL_ID
    TWI_start(LIDAR_ADDR, TW_READ);
    TWI_read_nack(&hold);

    if (hold != 0xB4)
      goto exit;

    for (int8_t i=0; i<sizeof(cmd)/sizeof(cmd[0]); i++) {
      TWI_start(LIDAR_ADDR, TW_WRITE);
      for (int8_t j=0; i<sizeof(cmd[0]); i++) {
        TWI_write(cmd[i][j]);
      }
    }
    exit:
    TWI_stop();
  }

  return;
}
