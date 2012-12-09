/******************************************************************************/
#ifndef DK_USB_H
#define DK_USB_H


/*******************************************************************************
USB, CDC, and device magic number definitions.
*******************************************************************************/
#define USB_IDVENDOR  0x04D8
#define USB_IDPRODUCT 0x000A

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Standard
   Request Codes, table 9-4, page 251. */
#define USB_SR_GET_STATUS         0x00
#define USB_SR_CLEAR_FEATURE      0x01
#define USB_SR_SET_FEATURE        0x03
#define USB_SR_SET_ADDRESS        0x05
#define USB_SR_GET_DESCRIPTOR     0x06
#define USB_SR_SET_DESCRIPTOR     0x07
#define USB_SR_GET_CONFIGURATION  0x08
#define USB_SR_SET_CONFIGURATION  0x09
#define USB_SR_GET_INTERFACE      0x0A
#define USB_SR_SET_INTERFACE      0x0B
#define USB_SR_SYNCH_FRAME        0x0C

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Descriptor
   Types, table 9-5, page 251.*/
#define USB_SD_DEVICE         0x01
#define USB_SD_CONFIGURATION  0x02
#define USB_SD_STRING         0x03
#define USB_SD_INTERFACE      0x04
#define USB_SD_ENDPOINT       0x05

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, PID types,
   table 8-1, page 196.*/
#define USB_PID_TOKEN_OUT   0x01
#define USB_PID_TOKEN_IN    0x09
#define USB_PID_TOKEN_SOF   0x05
#define USB_PID_TOKEN_SETUP 0x0D

/* Universal Serial Bus Class Definitions for Communication Devices, Version
   1.1, January 19, 1999, Type Values for the bDescriptorType Field, table 24,
   page 33.*/
#define USB_CDC_CS_INTERFACE  0x24
#define USB_CDC_CS_ENDPOINT   0x25

/* Universal Serial Bus Class Definitions for Communication Devices, Version
   1.1, January 19, 1999, Communication Device Class Code, table 14, page 28. */
#define USB_CDC_CC_CDC        0x02

/* Universal Serial Bus Class Definitions for Communication Devices, Version
   1.1, January 19, 1999, Communication Interface Class Code, table 15, page
   28. */
#define USB_CDC_CC_INTERFACE  0x02

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Standard
   Endpoint Descriptor, table 9-13., page 269. */
#define USB_ENDPOINT_ADDRESS_0O  0x00
#define USB_ENDPOINT_ADDRESS_0I  0x80
#define USB_ENDPOINT_ADDRESS_1O  0x01
#define USB_ENDPOINT_ADDRESS_1I  0x81
#define USB_ENDPOINT_ADDRESS_2O  0x02
#define USB_ENDPOINT_ADDRESS_2I  0x82
#define USB_ENDPOINT_ADDRESS_3O  0x03
#define USB_ENDPOINT_ADDRESS_3I  0x83
#define USB_ENDPOINT_ADDRESS_4O  0x04
#define USB_ENDPOINT_ADDRESS_4I  0x84
#define USB_ENDPOINT_ADDRESS_5O  0x05
#define USB_ENDPOINT_ADDRESS_5I  0x85
#define USB_ENDPOINT_ADDRESS_6O  0x06
#define USB_ENDPOINT_ADDRESS_6I  0x86
#define USB_ENDPOINT_ADDRESS_7O  0x07
#define USB_ENDPOINT_ADDRESS_7I  0x87
#define USB_ENDPOINT_ADDRESS_8O  0x08
#define USB_ENDPOINT_ADDRESS_8I  0x88
#define USB_ENDPOINT_ADDRESS_9O  0x09
#define USB_ENDPOINT_ADDRESS_9I  0x89
#define USB_ENDPOINT_ADDRESS_10O  0x0A
#define USB_ENDPOINT_ADDRESS_10I  0x8A
#define USB_ENDPOINT_ADDRESS_11O  0x0B
#define USB_ENDPOINT_ADDRESS_11I  0x8B
#define USB_ENDPOINT_ADDRESS_12O  0x0C
#define USB_ENDPOINT_ADDRESS_12I  0x8C
#define USB_ENDPOINT_ADDRESS_13O  0x0D
#define USB_ENDPOINT_ADDRESS_13I  0x8D
#define USB_ENDPOINT_ADDRESS_14O  0x0E
#define USB_ENDPOINT_ADDRESS_14I  0x8E
#define USB_ENDPOINT_ADDRESS_15O  0x0F
#define USB_ENDPOINT_ADDRESS_15I  0x8F


/*******************************************************************************
USB, CDC, and device data structure declarations.
*******************************************************************************/
/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Standard
   Device Descriptor, table 9-8, page 262. */
typedef struct
{
  unsigned char  bLength,
                 bDescriptorType;

  unsigned short bcdUSB;

  unsigned char  bDeviceClass,
                 bDeviceSubclass,
                 bDeviceProtocol,
                 bMaxPacketSize0;

  unsigned short idVendor,
                 idProduct,
                 bcdDevice;

  unsigned char  iManufacturer,
                 iProduct,
                 iSerialNumber,
                 bNumConfigurations;
} USB_SD_Device;

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Standard
   Configuration Descriptor, table 9-10, page 265. */
typedef struct
{
  unsigned char  bLength,
                 bDescriptorType;

  unsigned short wTotalLength;

  unsigned char  bNumInterfaces,
                 bConfigurationValue,
                 iConfiguration,
                 bmAttributes,
                 bMaxPower;
} USB_SD_Configuration;

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Standard
   Interface Descriptor, table 9-12, page 268.*/
typedef struct
{
  unsigned char bLength,
                bDescriptorType,
                bInterfaceNumber,
                bAlternateSetting,
                bNumEndpoints,
                bInterfaceClass,
                bInterfaceSubClass,
                bInterfaceProtocol,
                iInterface;
} USB_SD_Interface;

/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, Standard
   Endpoint Descriptor, table 9-13, page 269. */
typedef struct
{
  unsigned char  bLength,
                 bDescriptorType,
                 bEndpointAddress,
                 bmAttributes;

  unsigned short wMaxPacketSize;

  unsigned char  bInterval;
} USB_SD_Endpoint;


/* Universal Serial Bus Specification, Revision 2.0, April 27, 2000, UNICODE
   String Descriptor, table 9-16, 274.  bString is an array of wide
   characters. */
#define USB_UNICODE_STRING_DESCRIPTOR( Length ) \
  struct \
  {\
    unsigned char  bLength,\
                   bDescriptorType;\
    \
    unsigned short bString[Length];\
  }

/* Universal Serial Bus Class Definitions for Communication Devices, Version
   1.1, January 19, 1999, Class-Specific Descriptor Header Format, table 26,
   page 34. */
typedef struct
{
  unsigned char bFunctionLength,
                bDescriptorType,
                bDescriptorSubtype;

  unsigned short bcdCDC;                
} USB_CDC_FD_Header;

/*Universal Serial Bus Class Definitions for Communication Devices, Version
  1.1, January 19, 1999, Call Management Functional Descriptor, table 27, page
  34. */
typedef struct
{
  unsigned char bFunctionLength,
                bDescriptorType,
                bDescriptorSubtype,
                bmCapabilities,
                bDataInterface;
} USB_CDC_FD_CallManagement;

/* Universal Serial Bus Class Definitions for Communication Devices, Version
   1.1, January 19, 1999, Abstract Control Management Functional Descriptor,
   table 28, page 35. */
typedef struct
{
  unsigned char bFunctionLength,
                bDescriptorType,
                bDescriptorSubtype,
                bmCapabilities;
} USB_CDC_FD_AbstractControlManagement;

/* Universal Serial Bus Class Definitions for Communication Devices, Version
   1.1, January 19, 1999, Union Interface Functional Descriptor, table 33, page
   40. */
typedef struct
{
  unsigned char bFunctionLength,
                bDescriptorType,
                bDescriptorSubtype,
                bMasterInterface,
                bSlaveInterface0;
} USB_CDC_FD_Union;


/* PIC18F2455/2550/4455/4550 Data Sheet, January, 2007, 17.4 Buffer Descriptors
   and the Buffer Descriptor Table. */
typedef struct
{
  unsigned char STAT,
                CNT,
                * ADR;
} USB_BufferDescriptor;

#define USB_ENDPOINT( Length ) \
  union\
  {\
    struct\
    {\
      unsigned char   bmRequestType,\
                      bRequest;\
      \
      unsigned short  wValue,\
                      wIndex,\
                      wLength;\
    };\
    \
    struct\
    {\
      unsigned      Recipient:             5,\
                    Type:                  2,\
                    DataTransferDirection: 1,\
                    :                      8;\
      \
      unsigned char DescriptorIndex,\
                    DescriptorType;\
      \
      unsigned      :                      8,\
                    :                      8,\
                    :                      8,\
                    :                      8,\
                    :                      8;\
    };\
    \
    struct\
    {\
      unsigned      :         8,\
                    :         8;\
      \
      unsigned char Address;\
      \
      unsigned      :         8,\
                    :         8,\
                    :         8,\
                    :         8,\
                    :         8,\
                    :         8;\
    };\
    \
    unsigned char Byte[Length];\
  }


signed DK_USB_Start(void);
signed DK_USB_Initialize(void);
signed DK_USB_SendCharacter( unsigned char Character );
signed DK_USB_SendPacket( USB_BufferDescriptor * BufferDescriptor,
                          DK_DangerousPointer pdPacketData,
                          unsigned char bLength );
void DK_USB_ISR(void);


#endif /* DK_USB_H */
