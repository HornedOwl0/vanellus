#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

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
  0,      // iConfiguration 
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
  0,      // iFunction

  /* Interface (CCI) Descriptor */

  9, // bLength
  0x04, // bDescriptorType
  0,  // bInterfaceNumber
  0,  // bAlternateSetting
  1, // bNumEndpoints
  0x02, // bInterfaceClass 
  0x02, // bInterfaceSubClass
  0x00,   // bInterfaceProtocol ( No Specific Protocol )
  0, // iInterface

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
  0, // iInterface
  
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
#endif /* DESCRIPTORS_H */