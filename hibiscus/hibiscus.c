#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "macros.h"
#include "hibiscus.h"
#include "descriptors.h"

#ifdef ACM_ALL_REQUESTS

/* Runtime Variables */

static volatile uint8_t ACM_line_coding[7] = {
  LSB(38400), MSB(38400), 0, 0, // dwDTERate
  0, // bCharformat
  0, // bParity
  8, // bDatabits
};
static volatile uint8_t USB_bRequest_pending = 0x00;

#endif

/* Useful Functions */

static void USB_ACM_EPN_disable(void){
  UERST |= (0x0E<<EPRST0); // Reset FIFO for EP1-EP2-EP3
  UERST &= ~(0x0E<<EPRST0); 

  UENUM = 1; CLR(UECONX, EPEN);
  UENUM = 2; CLR(UECONX, EPEN);
  UENUM = 3; CLR(UECONX, EPEN);
}

static void USB_ACM_EPN_init(void){
  UERST |= (0x0E<<EPRST0); // Reset FIFO for EP1-EP2-EP3
  UERST &= ~(0x0E<<EPRST0); 

  UENUM = 1; // Endpoint 1 - Interrupt IN
  SET(UECONX, EPEN);
  UECFG0X = (0x03<<EPTYPE0)|(1<<EPDIR); // IN
  UECFG1X = (0x01<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);
  UEIENX = 0x00;

  UENUM = 2; // Endpoint 2 - Bulk IN - 64B
  SET(UECONX, EPEN);
  UECFG0X = (0x02<<EPTYPE0)|(1<<EPDIR); // IN
  UECFG1X = (0x03<<EPSIZE0)|(0x01<<EPBK0)|(1<<ALLOC); // Double Banked
  UEIENX = 0x00;

  UENUM = 3; // Endpoint 3 - Bulk OUT - 64B
  SET(UECONX, EPEN);
  UECFG0X = (0x02<<EPTYPE0)|(0<<EPDIR); // OUT
  UECFG1X = (0x03<<EPSIZE0)|(0x01<<EPBK0)|(1<<ALLOC); // Double Banked
  UEIENX = 0x00;
}

static void USB_EP0_init(void);
inline void USB_EP0_init(void){
  UENUM = 0; // Endpoint 0 - Control (Bidirectional)
  SET(UERST, EPRST0);  // Reset FIFO Buffer for EP0
  CLR(UERST, EPRST0);  // Complete the Reset Operation
  
  CLR(UECONX, EPEN);
  SET(UECONX, EPEN);
  
  UECFG0X = (0x00<<EPTYPE0)|(0<<EPDIR); // OUT
  UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);
  
  UEIENX = (1<<RXSTPE)|(1<<RXOUTE); // The ONLY interrupt we need.
}

static void USB_HANDLE_GET_DESCRIPTOR(uint16_t wValue, uint16_t wLength){
  const __flash uint8_t *ptr = NULL;
  uint16_t len = 0;

  switch ( MSB(wValue) ) {
    case 0x01: // Device Descriptor
      ptr = &USB_device_descriptor[0];
      len = ARRAY_SIZE(USB_device_descriptor);
      break;
    case 0x02: // Config Descriptor
      ptr = &USB_config_descriptor[0];
      len = ARRAY_SIZE(USB_config_descriptor);
      break;
    case 0x03: // String Descriptor
      switch ( LSB(wValue) ) {
        case 0:
          ptr = &USB_supported_langid[0];
          len = ARRAY_SIZE(USB_supported_langid);
          break;
        case 1:
          ptr = &USB_str_manufacturer[0];
          len = ARRAY_SIZE(USB_str_manufacturer);
          break;
        case 2:
          ptr = &USB_str_product[0];
          len = ARRAY_SIZE(USB_str_product);
          break;
        default:
          SET(UECONX, STALLRQ);
          break;
      }
      break;
  }
  len = MIN(len, wLength);

  if ( ptr != NULL ){
    int8_t chk_ZLP = ( (0 < len) && (len < wLength) && ( len%64==0 ) );
    while ( len ) {
      while ( !GET(UEINTX, TXINI) ){} // await tx ready
      uint8_t buf_ctu = 0;
      while ( len && ( (buf_ctu++)<64) ){ // load buffer
        UEDATX = *(ptr++);
        len--;
      }
      CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON));
    }
    if( chk_ZLP ){ USB_ZLP(); }
  } else { 
    SET(UECONX, STALLRQ); 
  }
}

ISR(USB_GEN_vect){
  if ( GET(UDINT, EORSTI) ){
    CLR(UDINT, EORSTI);
    
    USB_EP0_init();
  } 
}

ISR(USB_COM_vect){
  UENUM = 0;

  if ( GET(UEINTX, RXSTPI) ){
    uint8_t bmRequestType = UEDATX;
    (void)bmRequestType;

    uint8_t bRequest = UEDATX;
    (void)bRequest;

    #ifdef ACM_ALL_REQUESTS
    USB_bRequest_pending = bRequest;
    #endif 

    uint16_t wValue = UEDATX; 
    wValue |= ((uint16_t)UEDATX<<8);
    (void)wValue;

    uint16_t wIndex = UEDATX;
    wIndex |= ((uint16_t)UEDATX<<8);
    (void)wIndex;

    uint16_t wLength = UEDATX; 
    wLength |= ((uint16_t)UEDATX<<8);
    (void)wLength;

    CLR(UEINTX, RXSTPI);
    
    switch (bRequest) {
      case USBSTDREQ_GET_STATUS:
        while ( !GET(UEINTX, TXINI) ){}
        while(wLength--){
          UEDATX = 0x00;
        }
        CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON));

        break;

      case USBSTDREQ_SET_ADDRESS:
        // wValue is our Address -> Record it in UADD, keep ADDEN clr
        UDADDR = (uint8_t)(wValue&0x7F);
        // send a ZLP
        USB_ZLP();
        // wait for TX done
        while ( !GET(UEINTX, TXINI) ){}
        // then, ADDEN can be set
        SET(UDADDR, ADDEN);
        break;

      case USBSTDREQ_GET_DESCRIPTOR:
        USB_HANDLE_GET_DESCRIPTOR(wValue, wLength);
        break;

      case USBSTDREQ_SET_CONFIGURATION:
        switch (wValue){
          case 0: // Unconfigured State
            USB_ZLP();
            USB_ACM_EPN_disable();
            break;
          case 1: // CDC ACM enable
            USB_ZLP();
            USB_ACM_EPN_init();
            break;
          default: // Unsupported Config
            SET(UECONX, STALLRQ);
            break;
        }
        break;

      case ACMSTDREQ_SET_LINE_CODING: // Pass to next stage, do not STALL as long as wLength is correct
        if (wLength!=7){ SET(UECONX, STALLRQ); }
        break;

      #ifdef ACM_ALL_REQUESTS
      case ACMSTDREQ_GET_LINE_CODING: // Return Current CFG
        { // Scope GET_LINE_CODING
        while ( !GET(UEINTX, TXINI) ){}
        for (int8_t i=0; i<MIN(ARRAY_SIZE(ACM_line_coding), wLength); i++){
          UEDATX = ACM_line_coding[i];
        }
        CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON));
        } // Scope GET_LINE_CODING
        break; 
      #endif

      case ACMSTDREQ_SET_CONTROL_LINE_STATE: // just acknowledge the packet.
        USB_ZLP();
        break;

      case ACMSTDREQ_SEND_BREAK: // just acknowledge the packet.
        USB_ZLP();
        break;

      default:
        /* Stall Bad (Unsupported) Requests */
        SET(UECONX, STALLRQ);
        break;
    }
  }
  #ifdef ACM_ALL_REQUESTS
  if ( GET(UEINTX, RXOUTI) ){ 
    switch (USB_bRequest_pending){
      case ACMSTDREQ_SET_LINE_CODING:
        for (int8_t i=0; i<ARRAY_SIZE(ACM_line_coding); i++){
          ACM_line_coding[i] = UEDATX;
        }
        CLRBM(UEINTX, (1 << RXOUTI) | (1 << FIFOCON)); // Handshake and ACK
        USB_ZLP();

        uint32_t baud = (uint32_t)ACM_line_coding[0] | (uint32_t)ACM_line_coding[1]<<8;
        baud |= (uint32_t)ACM_line_coding[2]<<16 | (uint32_t)ACM_line_coding[3]<<24;

        if ( baud == 1200 ){
          uint16_t *addr = (uint16_t*)0x0800;

          *(addr) = 0x7777; // Key to bootloader

          wdt_enable(WDTO_15MS);
          for(;;){}
        }

        break;
        default:
        CLRBM(UEINTX, (1 << RXOUTI) | (1 << FIFOCON)); // Handshake and STALL
        SET(UECONX, STALLRQ);
        break;
    }
  }
  #else 
  if ( GET(UEINTX, RXOUTI) ){ // Since we only accept ONE request with a data field:
    uint32_t baud = ((uint32_t)UEDATX);
    baud |= ((uint32_t)UEDATX<<8);
    baud |= ((uint32_t)UEDATX<<16);
    baud |= ((uint32_t)UEDATX<<24);
    CLRBM(UEINTX, (1 << RXOUTI) | (1 << FIFOCON)); // Handshake and ACK
    USB_ZLP();

    if ( baud == 1200 ){
      uint16_t *addr = (uint16_t*)0x0800;

      *(addr) = 0x7777; // Key to bootloader

      wdt_enable(WDTO_15MS);
      for(;;){}
    }
  }
  #endif

}

inline void PLL_init(void){
  PLLFRQ = (1<<PLLUSB)|(0x0A<<PDIV0)|(0x02<<PLLTM0);
  #if (F_CPU==16000000UL)
    PLLCSR = (1<<PINDIV)|(1<<PLLE);
  #elif (F_CPU==8000000UL)
    PLLCSR = (0<<PINDIV)|(1<<PLLE);
  #endif /* if 0 */
  while( !(PLLCSR & (1<<PLOCK)) ){}

  return;
}

inline void USB_init(void){
  SET(UHWCON, UVREGE);
  SET(USBCON, USBE);

  SET(USBCON, OTGPADE);
  CLR(USBCON, FRZCLK);

  CLR(UDCON, DETACH);
  
  UDIEN = (1<<EORSTE); // Only Interrupt we need
  return;
}

void USB_ZLP(void){
  while ( !GET(UEINTX, TXINI) ){}
  CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON) );
  return;
}

void ACM_puts(char *str){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    UENUM = 2; // Bulk IN
    if( GET(UEINTX, TXINI) && str != NULL ){
      while ( (*str) && GET(UEINTX, RWAL) ){ // deliberately limited to 64B endpoint limit
        UEDATX = *(str++);
      }
      if( !GET(UEINTX, RWAL) && !(*str) ){ // Buffer = wMaxPacketSize -- TX a ZLP to confirm
        CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON) ); // Done TX -- This should TX and flush the buffer so the next packet is a ZLP
        while ( !GET(UEINTX, TXINI) ){}
      }
      CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON) ); // Done TX
    }
  }
  return;
}

void ACM_putc(const char c){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    UENUM = 2; // Bulk IN

    if( GET(UEINTX, TXINI) ){
      UEDATX = c;
      CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON) ); // Done TX
    }
  }
  return;
}

int8_t ACM_available(void){
  int8_t count;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    UENUM = 3;
    count = UEBCLX;
  }
  return count;
}

char ACM_getc(void){
  char c = '\0';
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    UENUM = 3; // Bulk OUT
    if ( GET(UEINTX, RXOUTI) ){;
      c = UEDATX;
      if ( !UEBCLX ){
        CLRBM(UEINTX, (1<<RXOUTI)|(1<<FIFOCON)); // Done RX
      }
    }
  }
  return c;
}

void ACM_gets(char *ptr, uint8_t n){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    UENUM = 3; // Bulk OUT

    if ( GET(UEINTX, RXOUTI) ){;
      while ( (n--)>1 && GET(UEINTX, RWAL)){
        *(ptr++) = UEDATX;
      }
      *(ptr) = '\0';
      if ( !UEBCLX ){
        CLRBM(UEINTX, (1<<RXOUTI)|(1<<FIFOCON)); // Done RX
      }
    }
  }
  return;
}
