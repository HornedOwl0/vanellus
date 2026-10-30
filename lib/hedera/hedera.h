#ifndef HEDERA_H
#define HEDERA_H

/* Primitives */
void TWI_init(uint32_t freq);

int8_t TWI_start(uint8_t addr, uint8_t read_write);

int8_t TWI_stop(void);

int8_t TWI_write(uint8_t data);

int8_t TWI_read_ack(uint8_t *var);

int8_t TWI_read_nack(uint8_t *var);

/* High Level */ 

int8_t TWI_writebuf(const __memx uint8_t* buf, uint16_t len);

int8_t TWI_readinto(uint8_t *buf, uint8_t len);

#endif /* HEDERA_H */
