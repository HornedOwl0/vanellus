#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

#include "lib/macros.h"
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <util/atomic.h>

#define USBSTDREQ_GET_STATUS 0x00
#define USBSTDREQ_CLEAR_FEATURE 0x01
  // RESERVED 0x02
#define USBSTDREQ_SET_FEATURE 0x03,
  // RESERVED 0x04
#define USBSTDREQ_SET_ADDRESS 0x05
#define USBSTDREQ_GET_DESCRIPTOR 0x06
#define USBSTDREQ_SET_DESCRIPTOR 0x07
#define USBSTDREQ_GET_CONFIGURATION 0x08
#define USBSTDREQ_SET_CONFIGURATION 0x09
#define USBSTDREQ_GET_INTERFACE 0x0A
#define USBSTDREQ_SET_INTERFACE 0x0B
#define USBSTDREQ_SYNCH_FRAME 0x0C

#define MSB(b) (uint8_t)((b>>8)&0xFF)
#define LSB(b) (uint8_t)(b&0xFF)

static uint8_t USB_device_descriptor[] = {
  18, // bLength
  0x01, // bDescriptorType
  MSB(0x0110), // bcdUSB
  LSB(0x0110),
  0x02,  // bDeviceClass - USB CDC Device
  0x02,  // bDeviceSubclass - Abstract Control Model
  0x00,  // bDeviceProtocol - None
  MSB(0x03EB),       // idVendor
  LSB(0x03EB),       // Atmel
  MSB(0x6124),       // idProduct
  LSB(0x6124),       // Atmel CDC Examples
  MSB(0x0100),      // bcdDevice
  LSB(0x0100),      // 1.00
  0,                // iManufacturer
  0,              // iProduct
  0,              // iSerialNumber
  1,                // bNumConfigurations
};

void EP0_init(void){
  UENUM = 0; // Endpoint 0 - Control (Bidirectional)
               //
  UECONX |= (1<<EPEN);
  UECFG0X = (0x00<<EPTYPE0)|(0<<EPDIR); // OUT
  UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);

  UEIENX |= (1<<RXSTPE);
}

ISR(USB_GEN_vect){
  if ( UDINT & (1<<EORSTI) ){
    UDINT = (1<<EORSTI);

    EP0_init();
  }
}

ISR(USB_COM_vect){
  UENUM = 0;

  if ( UEINTX & (1<<RXSTPI) ){
    uint8_t bmRequestType = UEDATX;
    uint8_t bRequest = UEDATX;
    uint16_t wValue = UEDATX | (uint16_t)(UEDATX<<8);
    uint16_t wIndex = UEDATX | (uint16_t)(UEDATX<<8);
    uint16_t wLength = UEDATX | (uint16_t)(UEDATX<<8);

    UEINTX &= ~(1<<RXSTPI);

    if (bRequest == USBSTDREQ_SET_ADDRESS){
      (void)bmRequestType;
      (void)wIndex;
      (void)wLength;
      // wValue is our Address -> Record it in UADD, keep ADDEN clr
      UDADDR = (uint8_t)(wValue&0x7F);
      // send a ZLP
      while ( !(UEINTX & (1<<TXINI) ) ){}
      UEINTX &= ~(1<<TXINI);
      while ( !(UEINTX & (1<<TXINI) ) ){}
      // then, ADDEN can be set
      UDADDR |= (1<<ADDEN);
    }

    if (bRequest == USBSTDREQ_GET_DESCRIPTOR){
      (void)bmRequestType;
      (void)wIndex;
      (void)wLength;
      if ( MSB(wValue) == 0x01 ){ // Device Descriptor
        for (int8_t i=0; i<sizeof(USB_device_descriptor); i++){
          UEDATX = USB_device_descriptor[i];
        } 
      while ( !(UEINTX & (1<<TXINI) ) ){}
      UEINTX &= ~(1<<TXINI);
      while ( !(UEINTX & (1<<TXINI) ) ){}
      }
    }
  }
}

void ACM_EPN_init(void){
  UERST = (0x7F<<EPRST0); // Reset FIFO for all Endpoints
  UERST = (0x00<<EPRST0); 

  UENUM = 1; // Endpoint 1 - Interrupt IN
  UECONX |= (1<<EPEN);
  UECFG0X = (0x03<<EPTYPE0)|(1<<EPDIR); // IN
  UECFG1X = (0x01<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);

  UENUM = 2; // Endpoint 2 - Bulk IN - 64B
  UECONX |= (1<<EPEN);
  UECFG0X = (0x02<<EPTYPE0)|(1<<EPDIR); // IN
  UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);

  UENUM = 3; // Endpoint 3 - Bulk OUT - 64B
  UECONX |= (1<<EPEN);
  UECFG0X = (0x02<<EPTYPE0)|(0<<EPDIR); // OUT
  UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);

}

int main(void){
	cli(); /* Begin Setup - no interrupts */

  SET(DDRD, PD5);
	SET(DDRB, PB0);
  SET(PORTB, PB0);
  SET(PORTD, PD5);

  _delay_ms(1000);

  PLLFRQ = (1<<PLLUSB)|(0x0A<<PDIV0);
  PLLCSR = (1<<PINDIV)|(1<<PLLE);
  while( !(PLLCSR & (1<<PLOCK)) ){}
  
  UHWCON = (1<<UVREGE);
  UDCON &= ~(1<<DETACH);
  USBCON = (1<<USBE);
  
  UDIEN |= (1<<EORSTE);
  
  EP0_init();
  ACM_EPN_init();

  _delay_ms(100);

	sei(); /* End Setup - all interrupts */

	for(;;){
    SET(PINB, PB0);
    _delay_ms(500);
  }

	return 0;
}
