/*******************************************************************************
Dreamcatcher Kernel
Stephen Niedzielski

This file contains all USB source for the PIC18F4550.
*******************************************************************************/

#include "DK_Global.h"
#include <string.h> /* memset, memcpy, memcpypgm2ram */


/*******************************************************************************
USB packet definitions.
*******************************************************************************/
static const rom USB_SD_Device DeviceDescriptor =
  {
    sizeof(USB_SD_Device),  /* Size of the descriptor in bytes. */
    USB_SD_DEVICE,          /* Device descriptor type. */
    0x0200,                 /* USB 2.0 protocol. */
    USB_CDC_CC_CDC,         /* CDC class code. */
    0,                      /* No subclass. */
    0,                      /* No class protocol. */
    64,                     /* Size of endpoint zero in bytes. */
    USB_IDVENDOR,           /* Vendor identity. */
    USB_IDPRODUCT,          /* Product identity. */
    0,                      /* Device release number. */
    1,                      /* Manufacturer string index. */
    2,                      /* Product string index. */
    0,                      /* Device serial number string index. */
    1                       /* Number of possible configurations. */
  };


static const rom struct
{
  USB_SD_Configuration                 Configuration;
  
  USB_SD_Interface                     Interface0;
  
  USB_CDC_FD_Header                    Header;
  USB_CDC_FD_AbstractControlManagement AbstractControlManagement;
  USB_CDC_FD_Union                     Union;
  USB_CDC_FD_CallManagement            CallManagement;

  USB_SD_Endpoint                      Endpoint0;

  USB_SD_Interface                     Interface1;
  
  USB_SD_Endpoint                      Endpoint1;
} SetupPacket =
  {
    /* Configuration0. */
    sizeof(USB_SD_Configuration),    /* Size of the descriptor in bytes. */
    USB_SD_CONFIGURATION,            /* Configuration descriptor type. */
    sizeof(SetupPacket),             /* Total length of data returned for this configuration. */
    2,                               /* Number of interfaces in this configuration. */
    1,                               /* Configuration index of this configuration. */
    0,                               /* Configuration string index. */
    128,                             /* Configuration characteristics: not self powered and no remote wakeup. */
    50,                              /* 100 mA consumption (2 mA units). */

    /* Interface0. */
    sizeof(USB_SD_Interface),        /* Size of the descriptor in bytes. */
    USB_SD_INTERFACE,                /* Interface descriptor type. */
    0,                               /* Interface index of this interface. */
    0,                               /* Value used to select this interface. */
    1,                               /* Number of endpoints used by this interface. */
    2,                               /* CDC class code. */
    2,                               /* Abstract control model subclass code. */
    1,                               /* Common AT commands protocol. */
    0,                               /* Interface string index. */
    
    /* Header0. */
    sizeof(USB_CDC_FD_Header),       /* Size of the descriptor in bytes. */
    USB_CDC_CS_INTERFACE,            /* CDC interface descriptor type. */
    0,                               /* CDC header functional descriptor type. */
    0x0110,                          /* CDC 1.1. */
    
    /* AbstractControlManagement0. */
    sizeof(USB_CDC_FD_AbstractControlManagement), /* Size of the descriptor in
                                                     bytes. */
    USB_CDC_CS_INTERFACE,            /* CDC interface descriptor type. */
    2,                               /* Abstract control management functional drescriptor. */
    0x02,                            /* Device sends and receives call management information only over CDC interface. */

    /* Union0. */
    sizeof(USB_CDC_FD_Union),        /* Size of the descriptor in bytes. */
    USB_CDC_CS_INTERFACE,            /* CDC interface descriptor type. */
    6,                               /* Union functional descriptor. */
    0,                                            
    1,                                            
    
    /* CallManagement0. */
    sizeof(USB_CDC_FD_CallManagement),  /* Size of the descriptor in bytes. */
    USB_CDC_CS_INTERFACE,            /* CDC interface descriptor type. */
    1,                               /* CDC call management functional
                                        descriptor. */
    0,                               /* Device does not handle call management
                                        itself. */
    1,
    
    /* Endpoint0. */
    sizeof(USB_SD_Endpoint),         /* Size of the descriptor in bytes. */
    USB_SD_ENDPOINT,                 /* Endpoint descriptor type. */
    USB_ENDPOINT_ADDRESS_1I,         /* Endpoint zero in address. */
    3,                               /* Specify interrupt transfer. */
    8,
    2,                               /* Polling interval of 2 ^ (value - 1)
                                        frames. */

    /* Interface1. */
    sizeof(USB_SD_Interface),
    USB_SD_INTERFACE,                /* Interface descriptor type. */
    1,
    0,
    1,                               /* Two endpoints. */
    0x0A,                            /* Data interface class. */
    0,
    0,
    0,

    /* Endpoint1. */
    sizeof(USB_SD_Endpoint),
    USB_SD_ENDPOINT,                 /* Endpoint descriptor type. */
    USB_ENDPOINT_ADDRESS_1I,         /* Endpoint address. */
    2,                               /* Bulk. */
    64,
    0
  };


/*  This is a special string (of index zero) that defines the LANGID of the
    following strings. */
static const rom USB_UNICODE_STRING_DESCRIPTOR(1) DeviceString0 =
  {
    sizeof(DeviceString0),
    USB_SD_STRING,
    0x0409  /* UNICODE language specifier: English (United States). */
  };

static const rom USB_UNICODE_STRING_DESCRIPTOR(31) DeviceString1 = 
  {
    sizeof(DeviceString1),
    USB_SD_STRING,
    'D','i','g','i','P','e','n',' ',
    'I','n','s','t','i','t','u','t','e',' ',
    'o','f',' ',
    'T','e','c','h','n','o','l','o','g','y'
  };

static const rom USB_UNICODE_STRING_DESCRIPTOR(14) DeviceString2 =
  {
    sizeof(DeviceString2),
    USB_SD_STRING,
    'N','E','S',' ',
    'C','o','n','t','r','o','l','l','e','r'
  };


/*******************************************************************************
Global variables.
*******************************************************************************/
#pragma idata USB_BufferDescriptors = 0x400 /* Begin specific initialized data
                                               region. */
  /* Table is defined as [Buffer Number][Output/Input], where the output buffer
     is zero, and input buffer is one. */
  static volatile USB_BufferDescriptor BufferDescriptorTable[16][2] = {0};
#pragma udata  /* Return to default data region. */

/* The following data region is an unitialized one, as MPLAB does not like to
   initialize across banks, which these endpoint buffers may very well be. */
#pragma udata USB_EndpointBuffers = 0x500 /* Begin specific uninitialized data
                                             region. */
  static volatile USB_ENDPOINT(64) Buffer_EP0O; /* Setup output. */
  static volatile USB_ENDPOINT(64) Buffer_EP0I; /* Setup input. */
  static volatile USB_ENDPOINT(64) Buffer_EP1I; /* CDC input. */
#pragma udata  /* Return to default data region. */


static const rom void * const rom DeviceStringTable[] = { &DeviceString0,
                                                          &DeviceString1,
                                                          &DeviceString2 };


/*******************************************************************************
Function definitions.
*******************************************************************************/
static signed DK_USB_Reset(void);
static signed DK_USB_GetDescriptor( DK_DangerousPointer * pdPacketData,
                                    unsigned char * Length );
static signed DK_USB_StandardRequestHandler(void);
                                    
signed DK_USB_Start(void)
{
/* This function turns on the USB module.
   
   Result:
   DK_SUCCESS if successful. */

  /* Turn on USB. */
  UCONbits.USBEN = 1;
  
  return DK_SUCCESS;
}


static signed DK_USB_Reset(void)
{
/* This function performs all of the tasks necessary to reset the USB module
   at any point after initialization.  The function clears out all associated
   USB flags, the USB address register, the USTAT FIFO, and reinitializes the
   endpoint control registers after clearing them out.
   
   Result:
   DK_SUCCESS if successful. */
   
  /* Clear out the four unsigned char USTAT FIFO.  Each time TRNIF is cleared,
     the FIFO is popped. */
  UIRbits.TRNIF = 0;
  UIRbits.TRNIF = 0;
  UIRbits.TRNIF = 0;
  UIRbits.TRNIF = 0;

  /* Clear out all the endpoint control registers, UEP0-15. */
  memset( ((unsigned char *)(&UEP0)),
          0,
          16 );
  
  /* Enable endpoint zero handshakes. */
  UEP0bits.EPHSHK = 1;

  /* Enable endpoint zero output. */
  UEP0bits.EPOUTEN = 1;

  /* Enable endpoint zero input. */
  UEP0bits.EPINEN = 1;
  
  /* Enable endpoint one handshakes. */
  UEP1bits.EPHSHK = 1;
  
  /* Enable endpoint one output. */
  UEP1bits.EPOUTEN = 1;
  
  /* Enable endpoint one input. */
  UEP1bits.EPINEN = 1;

  /* Clear assigned USB address. */
  UADDR = 0;

  /* Clear out all USB flags. */
  UIR = 0;
  UEIR = 0;
  
  return DK_SUCCESS;
}


signed DK_USB_Initialize(void)
{
/* Initializes the USB module.

   Result:
   DK_SUCCESS if successful. */ 
   
  DK_USB_Reset();

  /* Eye pattern test disabled.  Only enable this when debugging and not
     connected to a computer. */
  UCFGbits.UTEYE  = 0;
  
  /* Clear out UOE activity flag. */
  UCFGbits.UOEMON = 0;

  /* Enable internal pull-up resistors. */
  UCFGbits.UPUEN  = 1;
  
  /* Enable internal transceiver. */
  UCFGbits.UTRDIS = 0;
  
  /* Enable full-speed mode. */
  UCFGbits.FSEN   = 1;

  /* No ping pong buffering. */
  UCFGbits.PPB1    = 0;
  UCFGbits.PPB0    = 0;
  
  /* Clear out single ended zero flag. */
  UCONbits.SE0 = 0;
  
  /* Enable SIE token and packet processing. */
  UCONbits.PKTDIS = 0;

  /* Resume signaling disabled. */
  UCONbits.RESUME = 0;

  /* Device is awake, exit suspend mode. */
  UCONbits.SUSPND = 0;
  
  /* Assign the buffer locations. */
  BufferDescriptorTable[0][0].ADR = (unsigned char *)(&Buffer_EP0O);
  BufferDescriptorTable[0][1].ADR = (unsigned char *)(&Buffer_EP0I);
  BufferDescriptorTable[1][1].ADR = (unsigned char *)(&Buffer_EP1I);
  
  /* SIE owns the output buffer; data toggle synchronization enabled. */
  BufferDescriptorTable[0][0].STAT = 0x88;

  /* MCU owns the buffer; data toggle synchronization enabled. */
  BufferDescriptorTable[0][1].STAT = 0x08;  
  BufferDescriptorTable[1][1].STAT = 0x08;

  BufferDescriptorTable[0][0].CNT = 64;  
  BufferDescriptorTable[0][1].CNT = 0;
  BufferDescriptorTable[1][1].CNT = 0;

  /*  Disable all USB interrupts. */
  UIE = 0;
  UEIE = 0;
  
  /* Enable transaction complete interrupts. */
  UIEbits.TRNIE = 1;

  /* Enable reset interrupts. */
  UIEbits.URSTIE = 1;
  
  /* Clear out global USB interrupt flag. */
  PIR2bits.USBIF = 0;
  
  /* Enable global USB interrupts. */
  PIE2bits.USBIE = 1;
  
  /* Enable peripheral interrupts. */
  INTCONbits.PEIE = 1;
  
  return 1;
}


signed DK_USB_SendCharacter( unsigned char Character )
{
/* This function transmits a single character.  This function should not be used
   in a loop as a means to transmit an array, as this function makes an entire
   packet of one character.
   
   Result:
   DK_SUCCESS if successful, DK_FAILURE if USB buffer is unavailable. */
  
  unsigned Result = 0;
  
  /* Make a dangerous pointer. */
  DK_DangerousPointer pdCharacter;
  pdCharacter.IsROMPointer = FALSE;
  pdCharacter.Anywhere.pRAM = (near ram unsigned char *)&Character;

  Result = DK_USB_SendPacket( (USB_BufferDescriptor *)
                                &BufferDescriptorTable[1][1],
                              pdCharacter,
                              1 );

  return Result;
}


signed DK_USB_SendPacket( USB_BufferDescriptor * BufferDescriptor,
                          DK_DangerousPointer pdPacketData,
                          unsigned char bLength )
{
/* This function transmits the contents of the array pointed to by pdPacketData
   in packets up to 64 bytes in size.  Assuming the buffer is free, transmission
   is guaranteed, allowing zero length packets to be sent.
   
   Parameters:
   BufferDescriptor     A pointer to the buffer descriptor for the endpoint
                        buffer to be used.
   pdPacketData         A ROM / RAM contstruct containing a pointer to the data
                        to be sent, as well as a boolean for the pointer type.
   bLength              The size in bytes of the data from zero to 64.

   Result:
   DK_SUCCESS if successful, DK_FAILURE if USB buffer is unavailable. */

  signed Result = DK_FAILURE;

  if((BufferDescriptor->STAT & 128) == 0)
  {
    /* MCU owns the buffer, so it is safe to write to. */
    Result = DK_SUCCESS;
    
    /* Copy from ROM / RAM to the USB buffer. */
    if(pdPacketData.IsROMPointer == (unsigned)TRUE)
    {
      memcpypgm2ram( BufferDescriptor->ADR,
                     (const far rom void *)pdPacketData.Anywhere.pROM,
                     bLength);
    }
    else
    {
      memcpy( (void *)BufferDescriptor->ADR,
              (const void *)pdPacketData.Anywhere.pRAM,
              bLength);
    }

    /* Update the packet size. */
    BufferDescriptor->CNT = bLength;
    
    /* Give control to the USB SIE, but preserve the DTS bit. */
    BufferDescriptor->STAT &= 0x40;

    /* Toggle the DTS bit. */
    BufferDescriptor->STAT ^= 0x40;

    /* SIE owns the buffer; data toggle synchronization enabled. */
    BufferDescriptor->STAT |= 0x88;
  }

  return Result;
}


static signed DK_USB_GetDescriptor( DK_DangerousPointer * pdPacketData,
                                    unsigned char * Length )
{
/* This function examines the descriptor request and assigns a data pointer and
   data length to the passed in parameters.
   
   Parameters:
   pdPacketData   A pointer to a DK_DangerousPointer to store a pointer to the
                  appropriate packet data.
   Length         A pointer to store the length of the packet data in.
   
   Result:
   TRUE if the request is supported, FALSE if the request is not supported. */

  signed Result = DK_FAILURE;
  
  if(Buffer_EP0O.bmRequestType == 0x80)
  {
    /* The request is from the host. */
    
    switch(Buffer_EP0O.DescriptorType)
    {
      case USB_SD_DEVICE:
      {
        pdPacketData->IsROMPointer = TRUE;
        pdPacketData->Anywhere.pROM
          = (far rom unsigned char *)&DeviceDescriptor;

        *Length = sizeof(DeviceDescriptor);
        
        /* The request is supported. */
        Result = TRUE;
      }
      break;

      case USB_SD_CONFIGURATION:
      {
        pdPacketData->IsROMPointer = TRUE;
        pdPacketData->Anywhere.pROM = (far rom unsigned char *)&SetupPacket;

        *Length = sizeof(SetupPacket);

        /* The request is supported. */
        Result = TRUE;
      }
      break;

      case USB_SD_STRING:
      {
        pdPacketData->IsROMPointer = TRUE;
        pdPacketData->Anywhere.pROM =
          (far rom unsigned char *)
            (DeviceStringTable[Buffer_EP0O.DescriptorIndex]);

        /* Access the bLength member of the DeviceString (each device string is
        an anonymous struct and cannot be accessed with ->). */
        *Length = *(pdPacketData->Anywhere.pROM);

        /* The request is supported. */
        Result = TRUE;
      }
      break;
      
      default:
      {
        /* The request is not supported. */
        Result = FALSE;
      }
      break;
    }
  }
  
  return Result;
}


void DK_USB_ISR(void)
{
/* This function checks each of the flags in the USB interrupt register and
   calls the appropriate functions for each flag. */

  if(UIRbits.SOFIF == (unsigned)1)
  {
    /* Clear out the SOF flag. */
    UIRbits.SOFIF = 0;
  }
  
  if(UIRbits.STALLIF == (unsigned)1)
  {
    /*  Stall handshake (unsupported control request, control request failed,
        or endpoint failed). */

    /* Clear out stall flag. */
    UIRbits.STALLIF = 0;
  }
  
  if(UIRbits.IDLEIF == (unsigned)1)
  {
    /* Idle condition detected. */
        
    /* Clear out the idle flag. */
    UIRbits.IDLEIF = 0;
  }
  
  if(UIRbits.TRNIF == (unsigned)1)
  {
    /* Transaction complete. */

    if(USTAT == (unsigned)0x04)
    {
      /* Last transaction was an IN token to endpoint zero. */
  
      if(UADDR == (unsigned)0) 
      {
        /* No address has been assigned yet, set the address. */
        UADDR = Buffer_EP0O.Address;
      }

      /* Reset the output buffer. */
      BufferDescriptorTable[0][0].CNT = 64;
      
      /* SIE owns the buffer. */
      BufferDescriptorTable[0][0].STAT |= 0x80;
    }
    else if(USTAT == (unsigned)0x00)
    {
      /* Last transaction was an OUT or SETUP token to endpoint zero. */
  
      if( ((BufferDescriptorTable[0][0].STAT << 2) >> 4)
          == (unsigned)USB_PID_TOKEN_SETUP )
      {
        DK_USB_StandardRequestHandler();
      }
    }

    /* Clear transaction complete flag. */
    UIRbits.TRNIF = 0;  
  }

  if(UIRbits.ACTVIF == (unsigned)1)
  {
    /* Bus activity detected. */
    
    /* Kick out of suspend mode. */
    UCONbits.SUSPND = 0;

    /* Clear out active flag. */
    UIRbits.ACTVIF = 0;
  }
  
  if(UIRbits.UERRIF == (unsigned)1)
  {
    /* Error condition detected. */

    /* Clear out the specific error flags and pretend that nothing happened. */
    UEIR = 0;
    
    /* Clear out error flag. */
    UIRbits.UERRIF = 0;
  }
  
  if(UIRbits.URSTIF == (unsigned)1)
  {
    /* Reset. */
    
    DK_USB_Reset();
  }
  
  /* Clear out global USB interrupt flag. */
  PIR2bits.USBIF = 0;
}


static signed DK_USB_StandardRequestHandler(void)
{
/* This function manages endpoint zero, the control endpoint.  Specifically, it
   routes requests to the appropiate functions. 
   
   Result:
   DK_SUCCESS if successful. */

  DK_DangerousPointer pdPacketData = {0, 0};
  unsigned char Length = 0;
  signed RequestIsSupported = FALSE;

  switch(Buffer_EP0O.bRequest)
  {
    case USB_SR_GET_DESCRIPTOR:
    {
      RequestIsSupported = DK_USB_GetDescriptor( &pdPacketData,
                                                 &Length );
    }
    break;

    default:
    {
      RequestIsSupported = FALSE;
    }
    break;
  }

  if( Buffer_EP0O.DataTransferDirection == (unsigned)1 )
  {
    /* Device is sending to host. */

    unsigned char bLength = 0;

    if(RequestIsSupported == TRUE)
    {
      if(Buffer_EP0O.wLength < Length)
      {
        Length = Buffer_EP0O.wLength;
      }
  
      if(Length > (unsigned)64)
      {
        bLength = 64;
      }
      else
      {
        bLength = Length;
      }
      Length -= bLength;
        
      DK_USB_SendPacket( (USB_BufferDescriptor *)&BufferDescriptorTable[0][1],
                         pdPacketData,
                         bLength );
    }
  
    /* Reset endpoint zero. */
    BufferDescriptorTable[0][0].STAT = 0x88;
  }
  else
  {
    /* Host is sending to device. */

    BufferDescriptorTable[0][1].CNT = 0;
    
    /* SIE owns, no sync checking, and data one packet. */
    BufferDescriptorTable[0][0].STAT = 0xC8;
  }
  
  BufferDescriptorTable[0][0].CNT = 64;
  BufferDescriptorTable[0][1].STAT = 0xC8;
  
  UCONbits.PKTDIS = 0;
  
  return DK_SUCCESS;
}
