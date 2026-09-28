#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <util/atomic.h>

#include "macros.h"
#include "hibiscus.h"

/* Descriptors */

static const __flash uint8_t USB_device_descriptor[] = {
  18, // bLength
  0x01, // bDescriptorType
  LSB(0x0200), MSB(0x0200), // bcdUSB
  0xEF,  // bDeviceClass - Misc. Device
  0x02,  // bDeviceSubclass - Common Class
  0x01,  // bDeviceProtocol - IAD
  64, // bMaxPacketSize
  LSB(VENDOR_HEX), MSB(VENDOR_HEX), // idVendor
  LSB(PRODUCT_HEX), MSB(PRODUCT_HEX), // idProduct
  LSB(0x0100), MSB(0x0100), // bcdDevice
  1,                // iManufacturer
  2,              // iProduct
  0,              // iSerialNumber
  1,                // bNumConfigurations
};

static const __flash uint8_t USB_config_descriptor[] = {
  9, // bLength
  0x02, // bDescriptorType
  LSB(75), MSB(75),  // wTotalLength
  2,  // bNumInterfaces (CCI, DCI)
  1, // bConfigurationValue
  2,      // iConfiguration 
  0xC0, // bmAttributes (Self-Powered)
  100,   // bMaxPower * 2mA
  
  /* Interface Association Descriptor */
  
  8, // bLength
  0x0B,   // bDescriptorType
  0,   // bFirstInterface
  2,   // bInterfaceCount
  0x02,   // bFunctionClass - USB CDC set EFh for Device
  0x02,   // bFunctionSubclass - CDC ACM set 02h for Device
  0x00,   // bFunctionProtocol - ( No Specific Protocol ) set 01h for Device
  2,      // iFunction

  /* Interface (CCI) Descriptor */

  9, // bLength
  0x04, // bDescriptorType
  0,  // bInterfaceNumber
  0,  // bAlternateSetting
  1, // bNumEndpoints
  0x02, // bInterfaceClass 
  0x02, // bInterfaceSubClass
  0x00,   // bInterfaceProtocol ( No Specific Protocol )
  2, // iInterface

  /* CDC Header Functional Descriptor */
  
  5, // bFunctionLength
  0x24,   // bDescriptorType (CS_INTERFACE 24h)
  0x00,    // bDescriptorSubtype (Header 00h)
  LSB(0x0120), MSB(0x0120),  // bcdCDC - CDC 1.2 Specification
  
  /* Call Management Functional Descriptor */
  
  5, // bFunctionLength
  0x24,   // bDescriptorType (CS_INTERFACE 24h)
  0x01,    // bDescriptorSubtype (Call Management 01h)
  0x02,  // bmCapabilities (Call Management over DCI)
  1,     // bDataInterface (Use Interface 1 (DCI) for Call Management)
  
  /* ACM Functional Descriptor */
  
  4, // bFunctionLength
  0x24,   // bDescriptorType (CS_INTERFACE 24h)
  0x02,    // bDescriptorSubtype (ACM 02h)
  0x00,  // bmCapabilities 
  // 02h DOES support these: ( 00h DOES NOT )
  // Set_Line_Coding, Set_Control_Line_State
  // Get_Line_Coding, NOTIF Serial_State

  /* Union Functional Descriptor */

  5, // bFunctionLength
  0x24,   // bDescriptorType (CS_INTERFACE 24h)
  0x06,   // bDescriptorSubtype (Union 06h)
  0,    // bControlInterface (CCI)
  1,  // bSubordinateInterface0 (DCI)

  /* EP1 (Interrupt IN) Descriptor */

  7, // bLength
  0x05,   // bDescriptorType
  0x81,   // bEndpointAddress (0nh for OUT, 8nh for IN)
  0x03,   // bmAttributes (Data Interrupt)
  LSB(16), MSB(16), // wMaxPacketSize
  24, // bInterval * 1ms
  
  /* Interface (DCI) Descriptor */

  9, // bLength
  0x04, // bDescriptorType
  1,  // bInterfaceNumber
  0,  // bAlternateSetting
  2, // bNumEndpoints
  0x0A, // bInterfaceClass 
  0x00, // bInterfaceSubClass
  0x00,   // bInterfaceProtocol
  2, // iInterface
  
  /* EP2 (TX - Bulk IN) Descriptor */

  7, // bLength
  0x05,   // bDescriptorType
  0x82,   // bEndpointAddress
  0x02,   // bmAttributes (Data Bulk)
  LSB(64), MSB(64), // wMaxPacketSize
  0, // bInterval * 1ms
  
  /* EP3 (RX - Bulk OUT) Descriptor */

  7, // bLength
  0x05,   // bDescriptorType
  0x03,   // bEndpointAddress
  0x02,   // bmAttributes
  LSB(64), MSB(64), // wMaxPacketSize
  0, // bInterval * 1ms
  
};

static const __flash uint8_t USB_supported_langid[] = {
  4, // bLength
  0x03, // bDescriptorType
  LSB(0x0409), MSB(0x0409), // English
};

static const __flash uint8_t USB_str_manufacturer[] = {
  22, // bLength
  0x03, // bDescriptorType
  'H',0,'o',0,'r',0,'n',0,'e',0,'d',0,'O',0,'w',0,'l',0,'_',0,
};

static const __flash uint8_t USB_str_product[] = {
  18, // bLength
  0x03, // bDescriptorType
  'H',0,'i',0,'b',0,'i',0,'s',0,'c',0,'u',0,'s',0,
};

/* End Descriptors */

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

  UENUM = 2; // Endpoint 2 - Bulk IN - 64B
  SET(UECONX, EPEN);
  UECFG0X = (0x02<<EPTYPE0)|(1<<EPDIR); // IN
  UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);

  UENUM = 3; // Endpoint 3 - Bulk OUT - 64B
  SET(UECONX, EPEN);
  UECFG0X = (0x02<<EPTYPE0)|(0<<EPDIR); // OUT
  UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);
}

ISR(USB_GEN_vect){
  if ( GET(UDINT, EORSTI) ){
    CLR(UDINT, EORSTI);
    
    /* EP0 Init */
    UENUM = 0; // Endpoint 0 - Control (Bidirectional)
    SET(UERST, EPRST0);  // Reset FIFO Buffer for EP0
    CLR(UERST, EPRST0);  // Complete the Reset Operation
    
    CLR(UECONX, EPEN);
    SET(UECONX, EPEN);
    
    UECFG0X = (0x00<<EPTYPE0)|(0<<EPDIR); // OUT
    UECFG1X = (0x03<<EPSIZE0)|(0x00<<EPBK0)|(1<<ALLOC);
    
    SET(UEIENX, RXSTPE);
  } 
}

ISR(USB_COM_vect){
  UENUM = 0;

  if ( GET(UEINTX, RXSTPI) ){
    uint8_t bmRequestType = UEDATX;
    uint8_t bRequest = UEDATX;

    uint16_t wValue = UEDATX; 
    wValue |= ((uint16_t)UEDATX)<<8;

    uint16_t wIndex = UEDATX;
    wIndex |= ((uint16_t)UEDATX)<<8;

    uint16_t wLength = UEDATX; 
    wLength |= ((uint16_t)UEDATX)<<8;

    CLRBM(UEINTX, (1<<FIFOCON)|(1<<RXSTPI) );

    switch (bRequest) {
        case USBSTDREQ_SET_ADDRESS:
        (void)bmRequestType;
        (void)wIndex;
        (void)wLength;
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
        (void)bmRequestType;
        (void)wIndex;

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
            CLRBM(UEINTX, (1<<TXINI)|(1<<FIFOCON) );
          }
          if( chk_ZLP ){ USB_ZLP(); }
        } else { SET(UECONX, STALLRQ); }
        break;

      case USBSTDREQ_SET_CONFIGURATION:
        (void)bmRequestType;
        (void)wIndex;
        (void)wLength;
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

      default:
        /* Stall Bad (Unsupported) Requests */
        SET(UECONX, STALLRQ);
        break;
    }
  }
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

inline void USB_ZLP(void){
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
