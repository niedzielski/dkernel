/*******************************************************************************
NES Controller
Stephen Niedzielski
*******************************************************************************/

#include "DK_Global.h"
#include "main.h"


/*******************************************************************************
User defined functions called by the kernel.
*******************************************************************************/
void DK_IdleTaskHook(void)
{
/* User defined hook. */

  /* Statistic function calls. */
}


void DK_ISR()
{
/* User enabled interrupts (interrupts not caused by a TMR0 rollover) should be
   handled in this ISR.  All context data is managed by the kernel, as such,
   operations performed here will not unexpectantly affect other tasks (unless
   the task stack is blown).  This function should return normally. */

  /* Turn on an LED. */
  LED7 = 1;

  /* Filter interrupt to appropriate handler. */
  /* Process interrupt. */
  /* Clear interrupt flag. */
}


void DK_QuantumTrigger(unsigned QuantumCount)
{
/* This function is called at each clock interrupt.  Users may invoke functions,
   or really do anything here without affecting the other task contexts (so long
   as the task stack is not blown).  Users will wish to define quantum to be as
   long as possible without affecting the timing constraints of functions within
   this funciton.  This function should return normally. */
  
  /* This function is called every quantum.  A quantum was defined by the needs
     of this function, and other functions triggered from here are based on
     multiples of that quantum. */
  NESControllerManager();

  /* Toggle an LED every quantum. */
  LED4 = !LED4;

  if(QuantumCount == (unsigned)30 )
  {
    /* Toggle an LED. */
    LED5 = !LED5;
  }
  else if(QuantumCount == (unsigned)60 )
  {
    /* Toggle an LED. */
    LED6 = !LED6;
    
    /* Reset the counter so that a rollover will not produce an unexpected
       QuantumCount condition. */
    DK_ResetQuantumCount();
  }
}


/*******************************************************************************
Some simple tasks and functions for debugging.
*******************************************************************************/
signed InitializeLEDs(void)
{
/* Initializes several LEDS for use.

   Result:
   1 if successful. */

  /* Turn off LEDs. */
  LED0 = 0;
  LED1 = 0;
  LED2 = 0;
  LED3 = 0;
  LED4 = 0;
  LED5 = 0;
  LED6 = 0;
  LED7 = 0;

  #ifdef __18F4550
  /* Configure data direction to output. */
  TRISD &= ~0xFF;
  #endif

  #ifdef M52233DEMO
  /* Configure LED pins for IO operation. */
  MCF_GPIO_PTCPAR &= ~0xFF;
    
  /* Configure data direction to output. */
  MCF_GPIO_DDRTC |= 0xFF;
  #endif
  
  return 1;
}


void Task_Test0(void)
{
/* A simple test task. */

  unsigned Count = 0;
  
  /* printf("Entered Task_Test0.\n\r"); */
  
  while(1)
  {    
    /* Toggle an LED. */
    LED0 = !LED0;
    
    #if 0
    /* If it's been a while, write to hyperterminal. */
    if(++Count == 0)
    {
      printf("In Task_Test0.\n\r");
    }
    #endif
  }
}


void Task_Test1(void)
{
/* A second simple test task. */

  unsigned Count = 0;

  /* printf("Entered Task_Test1.\n\r"); */

  while(1)
  {    
    /* Toggle an LED. */
    LED1 = !LED1;

    #if 0
    /* If it's been a while, write to hyperterminal. */
    if(++Count == 0)
    {
      printf("In Task_Test1.\n\r");
    }
    #endif
  }
}


void Task_Test2(void)
{
/* A third simple test task. */

  unsigned Count = 0;

  /* printf("Entered Task_Test2.\n\r" ); */
  
  while(1)
  {
    /* Toggle an LED. */
    LED2 = !LED2;

    #if 0
    /* If it's been a while, write to hyperterminal. */
    if(++Count == 0)
    {
      printf("In Task_Test2.\n\r");
    }
    #endif
    
    /* If there is room, spawn another task. */
    if(DK_GetNumberOfLivingTasks() < (unsigned)DK_MAXIMUM_TASKS)
    {
      DK_InitializeTask((DK_TaskAddress)Task_Test3, READY, 1);
    }
  }
}


void Task_Test3(void)
{
/* A test task that toggles an LED, then releases its resources. */

  static unsigned TaskNumber = 0;
  unsigned Result = 0;

  #if 0
  DK_DisableInterrupts();
  /* Task number is, effectively, a variable with mutex permissions. */
  printf("Entered Task_Test3 - %u.\n\r", TaskNumber++);
  DK_EnableInterrupts();
  #endif

  /* Toggle an LED. */
  LED3 = !LED3;

  /*  Kill this task and force a scheduler assertion -- this is the proper means
      to deallocate a task. */
  Result = DK_ConfigureTaskState(DK_GetRunningTaskIdentity(), DEAD);
  DK_Assert(Result != DK_SUCCESS);
  DK_InvokeScheduler(); /* <-- No guarantee that the task will make make it this
                               far, but that's O.K., this scheduler invocation
                               only prevents tasks from returning to no task's
                               land. */
}


/*******************************************************************************
NES controller specific functions and tasks.
*******************************************************************************/
signed InitializeNESController(void)
{
/* Initializes the NES controller pins for use.

   Result:
   1 if successful. */

  /* Zero the data on each pin. */
  NES_IsParallelLoad = 0;
  NES_Clock = 0;
  NES_DataStream = 0;

  /* Set the data direction of each pin. */  
  TRISBbits.TRISB0 = 0; /* IsParallelLoad is output. */
  TRISBbits.TRISB1 = 0; /* Clock is output. */
  TRISBbits.TRISB2 = 1; /* DataStream is input. */
  
  return 1;
}


unsigned char ReadNESController(void)
{
/* This function manages the NES controller.  When called, the function triggers
   the NES controller clock line and reads the serial output for each button.

   Result:
   The current state of the controller in an active high unsigned character
   format as follows:

   Bit  Button
   7:   A
   6:   B
   5:   Select
   4:   Start
   3:   Up
   2:   Down
   1:   Left
   0:   Right */

  unsigned char Result = 0,
                Count = 0;

  /* Clock to zero. */
  NES_Clock = 0;

  /* Change from parallel to serial mode. */
  NES_IsParallelLoad = FALSE;

  /* Get the first bit without pulsing (it's always right at the beginning).
     Note that NES_DataStream is inherently active low, negating it restores it
     to active high. */
  Result = ((!NES_DataStream) << 7);

  /* Pulse the clock, get the output, pulse the clock, get the output, ... */
  while(Count < (unsigned)7)
  {
    /* Pulse the clock. */
    NES_Clock = 1; /* Clock high. */
    NES_Clock = 0; /* Clock low. */
    
    /* Grab the current bit and push it into the result. */
    Result |= ((!NES_DataStream) << (6 - Count));

    ++Count;
  }

  /* Revert to parallel mode. */
  NES_IsParallelLoad = TRUE;
  
  return Result;
}

signed NESControllerManager(void)
{
/* This function manages the NES controller;  it is responsible for reading from
   the controller and transmiting the result.
   
   Result:
   1 if successful; */
   
  signed Result = 0;
  unsigned char Input = 0;
  
  Input = ReadNESController();

  Result = DK_USB_SendCharacter(Input);
  DK_Assert(Result != DK_SUCCESS);
  
  return 1;
}


/******************************************************************************/
void main(void)
{
  unsigned Result = 0;
  unsigned Count = 0;

  /* Disable the watchdog timer in software (must also be disabled in
     hardware). */
  WDTCONbits.SWDTEN = 0;
  
  /* Pause for a moment so that it is more obvious if a reset occurs. */
  while(++Count != (unsigned)0)
  {
  }
  while(++Count != (unsigned)0)
  {
  }
  while(++Count != (unsigned)0)
  {
  }

  #ifdef __18F4550
  OpenUSART( USART_TX_INT_OFF
             & USART_RX_INT_OFF
             & USART_ASYNCH_MODE
             & USART_EIGHT_BIT
             & USART_CONT_RX
             & USART_BRGH_HIGH,
             42 /* 115.2 kbaud at 20 MHz oscillator. */
             );

  /* This processor supports "enhanced" USART, which means BAUDCON must be
     configured. */
  baudUSART( BAUD_IDLE_CLK_LOW
             & BAUD_16_BIT_RATE
             & BAUD_WAKEUP_OFF
             & BAUD_AUTO_OFF );
  #endif /* __18F4550. */
  
  /* USART initialization for the MCF52233 is performed by the default project. */

  /* Put a little greeting up on hyperterminal. */
  /* printf( "\n\rDreamcatcher Kernel\n\rLast compiled on " \
          __DATE__ " at " __TIME__ "\n\r\n\r" ); */

  Result = InitializeLEDs();
  DK_Assert(Result != 1);

  Result = InitializeNESController();
  DK_Assert(Result != 1);

  Result = DK_InitializeKernel();
  DK_Assert(Result != DK_SUCCESS);
  
  /* Create some tasks now that the kernel is initialized.  Note: the number of
     tasks initialized must be less then DK_MAXIMUM_TASKS - 1.  The reason for
     this is because the idle task is included in this number. */
  //DK_InitializeTask((DK_TaskAddress)Task_Test0, READY, 10 );    
  //DK_InitializeTask((DK_TaskAddress)Task_Test1, READY, 6  );
  //DK_InitializeTask((DK_TaskAddress)Task_Test2, READY, 30  );
  //DK_InitializeTask((DK_TaskAddress)Task_Test3, READY, 1 );
  
  /* Start the kernel and never return. */
  DK_StartKernel();
}
