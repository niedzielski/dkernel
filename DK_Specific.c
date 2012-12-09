/*******************************************************************************
Dreamcatcher Kernel
Stephen Niedzielski

This file contains all USB source for the PIC18F4550.
*******************************************************************************/

#include "DK_Global.h"


signed DK_InitializeSchedulerClock(void)
{
/* Initializes the scheduler clock.  Called by dk_InitializeScheduler.

  Result:
  DK_SUCCESS if successful. */

  signed Result = 0;

  unsigned char Prescaler = 0;
  
  unsigned short Modulo = 0;

  #ifdef __18F4550
  /*  Configure scheduler clock.
      7: TMR0ON     0
      6: T08BIT     0
      5: T0CS       0
      4: T0SE       0
      3: PSA        0
    2-0: T0PS2-0  000
  */
  T0CON = 0;
  
  /* Configure interrupt. */
  /* Disable interrupt priorities. */
  RCONbits.IPEN = 0;
  
  /* Enable TMR0 overflow interrupt. */
  INTCONbits.TMR0IE = 1;
  
  /* Clear out the TMR0 interrupt flag. */
  INTCONbits.TMR0IF = 0;
  #endif
  
  #ifdef M52233DEMO
  /* Configure scheduler clock.
     15-12: Reserved  0000
     11- 8: PRE       0000
         7: Reserved     0
         6: DOZE         0
         5: DBG          0
         4: OVW          1
         3: PIE          1
         2: PIF          1 (Setting this bit actually clears it.)
         1: RLD          1
         0: EN           0 */
  MCF_PIT0_PCSR = (unsigned short)(0x001E);

  /* Configure interrupt.  Interrupt controller zero, level six, maximum
     priority.  The scheduler clock interrupt should not be configured to
     unmaskable, level seven, as doing this removes the ability to atomicly
     disable all interrupts. */
  MCF_INTC0_ICR55 =  0x37;
  
  /* Unmask interrupt. */
  MCF_INTC0_IMRH &= ~0x00800000;
  #endif

  /* Calculate the prescale and modulo for the quantum specified. */
  Result = DK_CalculatePrescaleAndModulo
           (
             (DK_QUANTUM),
             &Prescaler,
             &Modulo
           );
  DK_Assert(Result != DK_SUCCESS);
  
  /* Initialize the clock to the prescale and modulo found. */
  Result = DK_ConfigureSchedulerClock(Prescaler, Modulo);
  DK_Assert(Result != DK_SUCCESS);

  return Result;
}


signed DK_CalculatePrescaleAndModulo( double Duration,
                                      unsigned char * pPrescaler,
                                      unsigned short * pModulo )
{
/* Determines the scheduler clock prescale and modulo for the length of time
   specified in seconds by Duration.  The final result is truncated down to
   integer form to fit in a short.
  
   Parameters:
   Duration   Length of time to calculate prescale and modulo for in quantum.
              Value must be greater than zero.
   pPrescale  The storage location for the prescale result.
   pModulo    The storage location for the modulo result.

   Result:
   DK_SUCCESS if successful. */

  signed Result = 0;
  unsigned char Prescaler = 0;
  double Modulo = 0.0;

  #ifdef __18F4550
  /* Solving for modulo:
     Multiply duration by the constants to remove them from future
     calculations. */
  Modulo = -Duration * DK_SYSTEM_CLOCK_HZ / (4 * 2);  
  
  /* Account for the first pass, which should really be division by 2^0. */
  Modulo *= 2;
  
  /* Find the correct modulo for the smallest prescaler.  Each pass increments
     Prescaler (2^Prescaler), and thus halves modulo, according to the
     formula. */
  while( Prescaler < (unsigned)8 /* The highest prescaler is 7. */ )
  {
    /* Halve the modulo. */
    Modulo /= 2;
  
    /* Make sure the Modulo can fit within the constraints of the 16 bit
       register. */
    if( Modulo >= -(65535 + 2 * 4) &&
        Modulo <= -(2 * 4) )
    {
      Modulo += 65535 + 2 * 4;

      /* We have a winner. */
      Result = DK_SUCCESS;
      
      /* Store the results. */
      *pPrescaler = Prescaler;
      *pModulo = (unsigned short)Modulo;
      
      break;
    }

    ++Prescaler;
  } /* End while. */
  #endif
  
  #ifdef M52233DEMO
  /* Task Duration = (2^Prescaler * 4 * Modulo) / Clock Frequency;
     Solving for modulo:
     Multiply duration by the constants to remove them from future
     calculations. */
  Modulo = Duration * SYSTEM_CLOCK * 1000000 / 4;
  
  /* Account for the first pass, which should really be division by 2^0. */
  Modulo *= 2;
  
  /* Find the correct modulo for the smallest prescaler.  Each pass increments
     Prescaler (2^Prescaler), and thus halves modulo, according to the
     formula. */
  while( Prescaler < 16 /* The highest prescaler is 15. */ )
  {
    /* Halve the modulo. */
    Modulo /= 2;
  
    /* Make sure the ModuloGuess can fit within the constraints of a short. */
    if( (unsigned)Modulo <= ((unsigned short)(-1)) /* Capacity of short type. */ )
    {
      /* We have a winner. */
      Result = DK_SUCCESS; /* Result = success. */
      
      /* Store the results. */
      *pPrescale = Prescaler;
      *pModulo = (unsigned short)Modulo;
      
      break;
    }

    ++Prescaler;
  } /* End while. */
  #endif
  
  return Result;
}


signed DK_ConfigureSchedulerClock( unsigned char Prescaler,
                                   unsigned short Modulo )
{
/* Modifies the scheduler clock to interrupt at a specified time from the next
   clock start.  It is recommended this function only be called within a
   critical, uninterruptable section.  Callers should call
   dk_StopSchedulerClock prior to calling this function.  Callers should obtain
   the appropriate prescale and modulo values by calling
   dk_CalculateSchedulerClockPrescaleAndModulo or by deriving it themselves
   using the manual method described in the kernel manual.  

   Parameters:
   Prescale    Clock prescale value.
   Modulo      Clock rollover modulo.

   Result:
   DK_SUCCESS if successful.*/

  #ifdef __18F4550
  /* Load the prescaler. */
  T0CONbits.T0PS2 = (Prescaler >> 2) & 1;
  T0CONbits.T0PS1 = (Prescaler >> 1) & 1;
  T0CONbits.T0PS0 = (Prescaler) & 1;

  /* Reset the current timer count to the specified modulo. */
  TMR0H = (Modulo >> 8) & 255;
  TMR0L = Modulo & 255;
  #endif

  #ifdef M52233DEMO
  /* As a side-effect, this function clears out the scheduler clock interrupt
     flag. */
  MCF_PIT0_PCSR = (unsigned short)((MCF_PIT0_PCSR & 0xFF)
                                    | (((unsigned short)(Prescaler)) << 8));

  /* Configure timer countdown rollover modulo.  This will overwrite current
     timer count since OVW is enabled. */
  MCF_PIT0_PMR = Modulo;
  #endif

  return DK_SUCCESS;
}


signed DK_InvokeScheduler(void)
{
/* Invokes the scheduler immediately and resets the scheduler clock.  The
   calling task forfeits any remaining time share.

   Result:
   DK_SUCCESS if successful. */

  #ifdef __18F4550
  /* Set the interrupt flag. */
  INTCONbits.TMR0IF = 1;
  #endif

  #ifdef M52233DEMO
   /* Force interrupt. */
   MCF_INTC0_INTFRCH |= MCF_INTC_INTFRCH_INTFRC55;
  #endif
  
  return DK_SUCCESS;
}



signed DK_StartScheduler(void)
{
/* Enables the scheduler, allowing it to assert itself once the current time
   share expires.

   Result:
   DK_SUCCESS if successful. */

  #ifdef __18F4550
  /* Set scheduler clock enable bit. */
  T0CONbits.TMR0ON = 1;
  #endif
  
  #ifdef M52233DEMO
  /* Set scheduler clock enable bit. */
  MCF_PIT0_PCSR |= 1;
  #endif 

  return DK_SUCCESS;
}


signed DK_StopScheduler(void)
{
/* Disables the scheduler from asserting itself and preserves the current time
   share.

   Result:
   DK_SUCCESS if successful. */

  #ifdef __18F4550
  /* Clear scheduler clock enable bit. */
  T0CONbits.TMR0ON = 0;
  #endif
  
  #ifdef M52233DEMO
  /* Clear scheduler clock enable bit. */
  MCF_PIT0_PCSR &= ~0x00000001;
  #endif
  
  return DK_SUCCESS;
}


signed DK_InitializeTask( DK_TaskAddress Task,
                          DK_TaskState State,
                          unsigned QuantumShare )
{
/* Initializes a task.  This function contains a critical section.

   Parameters:
   Task     The task address.
   State    State to initialize task to.
   Duration Time share to initialize task to.

   Result:
   Task identity if successful (positive non-zero), 0 if there are no task
   control blocks available. */

  signed TaskIdentity = 0;
  unsigned char Count = 1;

  /* The size of an individual task's stack space. */
  const unsigned TaskStackSize = (DK_MASTER_STACK_SIZE / DK_MAXIMUM_TASKS);
  
  /* Enter critical section. */
  DK_DisableInterrupts();
  
  /* Find the next available task control block in memory.*/
  while(Count < (unsigned)DK_MAXIMUM_TASKS)
  {
    if(TCBSegment[Count].State == DEAD)
    {
      /* If dead, then TCB is free to use.  Initialize to safe values for new
         task. */

      /* Update TaskIdentity to the current TCB number. */
      TaskIdentity = Count;
      
      TCBSegment[TaskIdentity].StackPointer
      = DK_MASTER_STACK_START /* The beginning. */
        + 41 /*  Number of bytes to offset to leave room for
                                    context data.*/
        + Count * TaskStackSize; /* Account for the other task's stacks. */

      *((unsigned *)(  TCBSegment[TaskIdentity].StackPointer
                      - 37)) = TCBSegment[TaskIdentity].StackPointer - 41;

      
      /*  Load the task address into the program counter in the context space. */
      *((unsigned char *)(  TCBSegment[TaskIdentity].StackPointer - 1)) = 1;
      *((DK_TaskAddress *)(  TCBSegment[TaskIdentity].StackPointer
                      - 4)) = Task;


      #ifdef M52233DEMO
      TCBSegment[TaskIdentity].StackPointer
      = DK_MASTER_STACK_START /* The beginning. */
        - DK_CONTEXT_DATA_OFFSET /*  Number of bytes to offset to leave room for
                                    context data.*/
        - Count * TaskStackSize; /* Account for the other task's stacks. */
      
      /*  Load the task address into the program counter in the context 
          space. */
      *((unsigned *)(  TCBSegment[TaskIdentity].StackPointer
                      + DK_PROGRAM_COUNTER_OFFSET)) = (unsigned)Task;
      
      
      /* Load the top four bytes of the exception frame context space with a
      bonine value. */
      *((unsigned *)(TCBSegment[TaskIdentity].StackPointer + 15 * 4 )) = 0x40002000; /* 0x41DC2004*/
      #endif

      TCBSegment[TaskIdentity].Next = 0;
      TCBSegment[TaskIdentity].Prev = 0;

      DK_ConfigureTaskState( Count,
                             State);
      
      TCBSegment[TaskIdentity].QuantumShare = QuantumShare;

      break;
    }
    ++Count;
  }

  /* Exit critical section. */
  DK_EnableInterrupts();
  
  return TaskIdentity;
}
