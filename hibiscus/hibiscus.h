#ifndef HIBISCUS_H
#define HIBISCUS_H

#include <stdint.h>

/* USB Standard Request Codes */

#define USBSTDREQ_GET_STATUS (0x00)
#define USBSTDREQ_SET_ADDRESS (0x05)
#define USBSTDREQ_GET_DESCRIPTOR (0x06)
#define USBSTDREQ_SET_CONFIGURATION (0x09)    

#define ACMSTDREQ_SET_LINE_CODING (0x20)
#define ACMSTDREQ_GET_LINE_CODING (0x21)
#define ACMSTDREQ_SET_CONTROL_LINE_STATE (0x22)

/* End USBSTDREQ-ACMSTDREQ */

#define ACM_init() ({ PLL_init(); USB_init(); })

#define VENDOR_HEX (0x1209)
#define PRODUCT_HEX (0x0007)

/* Starts PLL @96MHz (Postcaler Div 2 for USB, Div 1.5 for TC4 - Ideal according to specification) */
void PLL_init(void);

/* Starts USB peripheral and interrupts */
void USB_init(void);

/* Wait for TXINI and send empty UEDATX */
void USB_ZLP(void);

/* Writes a string (up to 64B), raw into the FIFO and sends it immediately */
void ACM_puts(char *str);

/* Writes a char, raw into the FIFO and sends it immediately */
void ACM_putc(const char c);

/* Returns the number of chars available for reading in the RX FIFO */
int8_t ACM_available(void);

/* Returns a single char from the RX FIFO */
char ACM_getc(void);

/* Reads n (up to 64B) chars from the FIFO into into the provided buffer */
void ACM_gets(char *ptr, uint8_t n);

#endif /* HIBISCUS_H */
