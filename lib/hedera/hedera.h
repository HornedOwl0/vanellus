#ifndef HEDERA_H
#define HEDERA_H

/* Constants */
enum: int8_t {
  TWI_NOSTART = 0,
  TWI_START = 1,
  TWI_NOSTOP = 0,
  TWI_STOP = 1,
};

/* Primitives */
void TWI_init(void);

int8_t TWI_start(uint8_t addr, uint8_t read_write);

int8_t TWI_stop(void);

int8_t TWI_write(uint8_t data);

int8_t TWI_read_ack(uint8_t *var);

int8_t TWI_read_nack(uint8_t *var);

/* High Level */ 

int8_t TWI_writebuf(uint8_t addr, const __memx uint8_t* buf, uint16_t n, int8_t start, int8_t stop);

int8_t TWI_readinto(uint8_t addr, uint8_t* buf, uint16_t n, int8_t start, int8_t stop);

int8_t TWI_available(void);

#endif /* HEDERA_H */
