#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <util/twi.h>

#include "hedera.h"
#include "config.h"

// F_SCL = F_CPU/16+2(TWBR)*4^PRESCALER
// prescaler is zero -> 4^PRESCALER => 1
// F_SCL = F_CPU/16+2(TWBR)
// 16+2(TWBR) = F_CPU/F_SCL
// 2*(8+(TWBR)) = F_CPU/F_SCL
// 8+TWBR = F_CPU/2*F_SCL
// TWBR = (F_CPU/(F_SCL<<1)) - 8

#define BITRATE ((F_CPU/(F_SCL<<1)-8))

/* Primitives */
void __attribute__((naked)) __attribute__((section(".init3"))) TWI_init(void){
  /* Set Bitrate and Prescaler */
  TWBR = BITRATE; 
  TWSR &= (0xF8); // Prescaler is ZERO
  /* Enable Pull-Ups on the I2C Pins */
  DDRD &= ~((1<<PD1)|(1<<PD0));
  PORTD |= ((1<<PD1)|(1<<PD0));
  /* Enable TWI module */
  TWCR = (1<<TWEN);
  return;
}

int8_t TWI_start(uint8_t addr, uint8_t read_write){
  if ( !(TWCR&(1<<TWEN)) ) { return -128; }
  /* Generate new Start condition */
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 
  while( !(TWCR & (1<<TWINT)) ){}
  if ( TW_STATUS != TW_START && TW_STATUS != TW_REP_START ){ return 1; } 

  TWDR = (addr<<1)|(read_write&0x01);
  TWCR = (1<<TWINT)|(1<<TWEN); 
  while( !(TWCR & (1<<TWINT)) ){} 
  if ( (TW_STATUS != TW_MT_SLA_ACK) && (TW_STATUS != TW_MR_SLA_ACK) ){ return -1; } // Error on sending addr+SLA_W/R

  return 0;
}

int8_t TWI_stop(void){
  if ( !(TWCR&(1<<TWEN)) ) { return -128; }
  /* Generate new Stop Condition */
  TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN); 
  uint16_t timeout = 0xFFFF;
  while ( (TWCR & (1<<TWSTO)) ){
    if (--timeout == 0){
      TWCR = (0<<TWSTO)|(1<<TWEN); 
      return -1; // Invalid Stop Condition
    }
  }
  return 0;
}

int8_t TWI_write(uint8_t data){
  if ( !(TWCR&(1<<TWEN)) ) { return -128; }

  TWDR=data;
  TWCR=(1<<TWINT)|(1<<TWEN); 
  while( !(TWCR & (1<<TWINT)) ){} 
  if ( TW_STATUS != TW_MT_DATA_ACK ){ return 1; } // Error During Transmission
  return 0;
}

int8_t TWI_read_ack(uint8_t *buf){
  if ( !(TWCR&(1<<TWEN)) ) { return -128; }
  if (buf == NULL) { return 127; }

  TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN); 
  while( !(TWCR & (1<<TWINT)) ){} 
  if ( TW_STATUS != TW_MR_DATA_ACK ){ return -1; } // Error During Reception
  *(buf) = TWDR;
  return 0;
}

int8_t TWI_read_nack(uint8_t *buf){
  if ( !(TWCR&(1<<TWEN)) ) { return -128; }
  if (buf == NULL) { return 127; }

  TWCR = (1<<TWINT)|(1<<TWEN); 
  while( !(TWCR & (1<<TWINT)) ){} 
  if ( TW_STATUS != TW_MR_DATA_NACK ){ return -1; } // Error During Reception
  *(buf) = TWDR;
  return 0;
}

/* High Level */ 
int8_t TWI_writebuf(const __memx uint8_t* buf, uint16_t len){
  if (buf == NULL) { return 127; }

  int8_t err = 0;
  while (len--){
    err = TWI_write(*(buf++));
    if (err){ return err; }
  }
  return err;
}

int8_t TWI_readinto(uint8_t *buf, uint8_t len){
  if (buf == NULL) { return 127; }

  int8_t err = 0;
  while ((len--)>1){
    err = TWI_read_ack(buf++);
    if (err){ return err; }
  }
  err = TWI_read_nack(buf);
  return err;
}
