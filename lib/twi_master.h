#ifndef TWI_MASTER_H
#define TWI_MASTER_H

#include <avr/io.h>
#include <util/twi.h>

#ifndef TWI_PRESCALER
	#define TWI_PRESCALER 0
#endif

#ifndef TWI_BITRATE
	#if F_CPU==16000000UL
		#define TWI_BITRATE 72 // 100kHz SCL @ 16MHz CLK
	#elif F_CPU==8000000UL
		#define TWI_BITRATE 32 // 100kHz SCL @ 8MHz CLK
	#else
		#define TWI_BITRATE 2		// 50kHz SCL @ 1MHz CLK, should be changed
	#endif
#endif

#define TWI_INIT() ({ TWBR=TWI_BITRATE; TWSR&=(0xF8|TWI_PRESCALER) })
// F_SCL = F_CPU/16+2(TWBR)*4^PRESCALER

#define TWI_START() ({ TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN); while(!(TWCR & (1<<TWINT))){} })
#define TWI_STOP() ({ TWCR=(1<<TWINT)|(1<<TWSTO)|(1<<TWEN); while (TWCR & (1<<TWSTO)){} })

#define TWI_WRITE(data) ({ TWDR=data;TWCR=(1<<TWINT)|(1<<TWEN); while(!(TWCR & (1<<TWINT))){} })

#define SLA_R(addr) (addr << 1 | 1)
#define SLA_W(addr) (addr << 1 | 0)

#endif /* TWI_MASTER_H */