/*******************************************************************************
Dreamcatcher Kernel
Stephen Niedzielski

This file contains all kernel related declarations and defines that are device
dependent.
*******************************************************************************/

#ifndef DK_SPECIFIC_H
#define DK_SPECIFIC_H


signed DK_InitializeSchedulerClock(void);
signed DK_CalculatePrescaleAndModulo( double Duration,
                                      unsigned char * pPrescaler,
                                      unsigned short * pModulo );
signed DK_ConfigureSchedulerClock( unsigned char Prescaler,
                                     unsigned short Modulo );
signed DK_InvokeScheduler(void);
signed DK_StartScheduler(void);
signed DK_StopScheduler(void);
signed DK_InitializeTask( DK_TaskAddress Task,
                          DK_TaskState State,
                          unsigned QuantumShare );
void DK_QuantumTrigger(unsigned QuantumCount);
void DK_IdleTaskHook(void);


#ifndef __18F4550 | M52233DEMO
  #error Error: Dreamcatcher Kernel does not support this device.
#endif


#ifdef __18F4550
  #include <stdio.h>
  #include <p18f4550.h>
  #include <usart.h>

/* Dreamcatcher Kernel expects the configuration bits to be as follows:
  
   Full-Speed USB Clock Source Selection   Clock src from 96MHz PLL/2
   CPU System Clock Postscaler             [OSC1/OSC2 Src: /1][96MHz PLL Src: /2]
   96MHz PLL Prescaler                     Divide by 5 [20MHz input]
   Oscillator                              EC: EC+CLKO{RA6}, USB-EC
   USB Voltage Regulator                   Enabled
   Watchdog Timer                          Disabled-Controlled by SWDTEN bit
   Extended CPU Enable                     Disabled
   PortB A/D Enable                        PORTB<4:0> configured as digital I/O on RESET */


#ifndef __TRADITIONAL18__
  #error Dreamcatcher Kernel does not support the PIC18F4550 extended\
         instruction set by default.
#endif


/* Allows for a pointer that may point anywhere, hence even more dangerous than
   the typical pointers you normally find in these parts.  I wonder what will I
   call one of these anywhere pointers that points to type void... */
typedef struct
{
  union
  {
    far rom unsigned char * pROM;
    near ram unsigned char * pRAM;
  } Anywhere;

  unsigned char IsROMPointer;
} DK_DangerousPointer;


/* User definable.  Specifies the maximum number of tasks that will be in
   existence at any point in time.  Used to determine TCB allocation quantity
   and individual task stack size.  Must be greater than or equal to 1.  This
   number should be made to include the idle task, so an application with one
   task should set DK_MAXIMUM_TASKS to two. */
#define DK_MAXIMUM_TASKS  (5)


/* Master stack start.  Used to calculate stack position for each task. */
#define DK_MASTER_STACK_START 0x100

/* Size of the master stack. */
#define DK_MASTER_STACK_SIZE  0x300

/* User definable.  A quantum is the minimum amount of time between scheduler
   assertions. */
#define DK_QUANTUM (0.001)

/* System clock speed in hertz. */
#define DK_SYSTEM_CLOCK_HZ 20000000


/* This macro is used to maximize resource use by eliminating any unneeded
   allocations between main and DK_IdleTask. */
#define DK_DiscardStack();  _asm\
                              lfsr  1,DK_MASTER_STACK_START \
                              lfsr  2,DK_MASTER_STACK_START \
                              clrf  STKPTR,0    /* Need to pop the hardware stack. */\
                           _endasm


/* If this macro is zero, debugging features are disabled, such as DK_Assert. */
#define DK_DEBUG_MODE 0

#if DK_DEBUG_MODE
  #define DK_Assert( a ); \
    if( (a) != 0 )\
      {\
        DK_DisableInterrupts();\
        printf( "\7 Assertion occured: (" #a ")\n\rFile: " __FILE__ " ["\
                __TIME__ "]\n\rLine: %lu \n\r", (long unsigned)__LINE__);\
        DK_EnableInterrupts();\
      }
#else
  #define DK_Assert( a ); /* */
#endif


/* Atomic macros for disabling or enabling interrupts in critical sections.
   These macros do not affect the scheduler. */
#define DK_EnableInterrupts();  _asm\
                                  bsf INTCON, 7, 0\
                                _endasm

#define DK_DisableInterrupts(); _asm\
                                  bcf INTCON, 7, 0\
                                _endasm

#endif DK_SPECIFIC_H
