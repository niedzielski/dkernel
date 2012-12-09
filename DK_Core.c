/*******************************************************************************
Dreamcatcher Kernel
Stephen Niedzielski

This file contains all kernel related functions that are not device dependent.
*******************************************************************************/

#include "DK_Global.h"


/*******************************************************************************
Global variables.
*******************************************************************************/
/* An array of TCB's for all current and future tasks. */
DK_TCB TCBSegment[DK_MAXIMUM_TASKS] = {0};

/* A pointer to the current task's TCB.  Used by the scheduler and clock
   interrupt handler. */
DK_TCB * pCurrentTaskTCB = 0;

/* Something to keep track of the total number of tasks that are not in the DEAD
   state.  Initialized to one for idle task. */
static unsigned NumberOfLivingTasks = 1,
                QuantumCount = 0;


/*******************************************************************************
Function definitions.
*******************************************************************************/
static signed DK_RegisterTask(unsigned char Identity);
static signed DK_DeregisterTask(unsigned char Identity);

signed DK_InitializeKernel(void)
{
/* Initializes kernel for use.  The user should call this function before
   calling DK_StartKernel, but before initializing any tasks.

   Result:
   DK_SUCCESS if succesful. */

  signed Result = 0;
  
  Result = DK_InitializeTCBSegment();
  DK_Assert(Result != DK_SUCCESS);

  Result = DK_InitializeScheduler();
  DK_Assert(Result != DK_SUCCESS);
  
  DK_USB_Initialize();
  DK_Assert(Result != DK_SUCCESS);

  return Result;
}


void DK_StartKernel(void)
{
/* Starts kernel execution.  The user should call this function after kernel
   initialization.  This function contains a critical section that is not exited
   until the idle task has finished initializing.  This function will
   never return. */

  /* Enter critical section. */
  DK_DisableInterrupts();

  /* Call the idle task, never to return here again. */
  DK_IdleTask();
}


signed DK_InitializeTCBSegment(void)
{
/* Initializes the task control block vector.  Called by DK_InitializeKernel.

   Result:
   DK_SUCCESS if successful. */

  signed Result = 0;

  unsigned char Count = 1; /* Initialize to 1 to skip the idle task TCB. */

  /* Initialize the idle task. */
  TCBSegment[0].StackPointer = DK_MASTER_STACK_START;
  
  TCBSegment[0].Identity = 0;
  TCBSegment[0].State = RUNNING; /* Although the idle task is technically not
                                    running at the moment, it will be very
                                    shortly and may be initialized as such. */
  TCBSegment[0].Next = &TCBSegment[0];
  TCBSegment[0].Prev = &TCBSegment[0];

  /* Initialize the rest of the TCB's. */
  while(Count < (unsigned)DK_MAXIMUM_TASKS)
  {
    /* The identity is never initialized in DK_InitializeTask(), but instead
       initialized here and nevermore. */
    TCBSegment[Count].Identity = Count;
    TCBSegment[Count].State = DEAD;

    ++Count;
  }

  return Result;
}


signed DK_InitializeScheduler(void)
{
/* Initializes the scheduler and its resources.

   Result:
   DK_SUCCESS if successful. */
   
  signed Result = 0;
  
  /* Configure the idle task to run first. */
  pCurrentTaskTCB = &(TCBSegment[0]);
  
  Result = DK_InitializeSchedulerClock();
  DK_Assert(Result != DK_SUCCESS);
  
  return Result;
}


signed DK_Scheduler(void)
{
/* Manages CPU usage.  Called by scheduler clock ISR.

   Result:
   DK_SUCCESS if successful. */
   
  /* The number of quantum the current running task.  Start at 1 so it'll
     decrement to zero on the first run. */
  static unsigned QuantumShare = 1;
  
  signed Result = 0;
  
  ++QuantumCount;
  DK_QuantumTrigger(QuantumCount);

  --QuantumShare;
  if(QuantumShare == (unsigned)0)
  {
    /* The currently running task has completed its time share.  Switch to the
       next task. */
   
    if( pCurrentTaskTCB->State == RUNNING )
    {
      /* Change the old task's state to ready. */
      pCurrentTaskTCB->State = READY;
    }
    
    /* Get the next ready task. */
    pCurrentTaskTCB = pCurrentTaskTCB->Next;
    
    /* Change the new task's state to running. */
    pCurrentTaskTCB->State = RUNNING;
    
    /* Update QuantumShare with the new task's time share. */
    QuantumShare = pCurrentTaskTCB->QuantumShare;
  }
    
  return Result;
}


static signed DK_RegisterTask( unsigned char Identity )
{
/* Registers a task with the scheduler.

   Result:
   DK_SUCCESS if succesful. */

  /* Update the ready list. */
  
  /*  Possible cases:
      1. Idle task is running and this is the first ready task.
      2. Idle task is running and this is not the first ready task.
      3. Idle task is not running and this is the first ready task.
      4. Idle task is not running and this is not the first ready task. */
  
  if( pCurrentTaskTCB == &TCBSegment[0] )
  {
    /* Case 1 & 2 & 3. */
    
    if(pCurrentTaskTCB->Next == &TCBSegment[0])
    {
      /* Case 1 & 3. */
      
      /*  This is the only ready task, so it should point to itself and take the
          idle task out of the loop. */
      TCBSegment[Identity].Next = &TCBSegment[Identity];
      TCBSegment[Identity].Prev = &(TCBSegment[Identity]);
      
      /* The next task to run should be this task. */
      pCurrentTaskTCB->Next = &TCBSegment[Identity];
    }
    else /* There are other ready tasks. */
    {
      /* Case 2. */
      
      /*  There are other ready task(s), but the current task is the idle task.
          Put at the end. */
      pCurrentTaskTCB->Prev->Next = &TCBSegment[Identity];
      
      /*  Skip the idle task and tell the next ready task's previous pointer to
          point to the new task. */
      pCurrentTaskTCB->Next->Prev = &TCBSegment[Identity];
      
      /* Point to the next task to keep the link (skip the idle task). */
      TCBSegment[Identity].Next = pCurrentTaskTCB->Next;
      
      /* Point to the previous task to keep the link. */
      TCBSegment[Identity].Prev = pCurrentTaskTCB->Prev;
    }
  }
  else /* The idle task is not running. */
  {
    /* Case 4. */
    
    /* Set the previous pointer to the previous previous task. */
    TCBSegment[Identity].Prev = pCurrentTaskTCB->Prev;
    
    /* Set the last task's next pointer. */
    pCurrentTaskTCB->Prev->Next = &TCBSegment[Identity];
    
    /* Set the next pointer to the current task. */
    TCBSegment[Identity].Next = pCurrentTaskTCB;
  }
  
  /* Put at end. */
  pCurrentTaskTCB->Prev = &TCBSegment[Identity];
  
  return DK_SUCCESS;
}


static signed DK_DeregisterTask(unsigned char Identity)
{
/* Deregisters a task with the scheduler.

   Result:
   DK_SUCCESS if succesful. */

  /* Update the ready list. */
  
  /*  Possible cases:
      1. Idle task is running and this is the only ready task.
      2. Idle task is running and this is not the only ready task.
      3. Idle task is not running and this is the only ready task.
      4. Idle task is not running and this is not the only ready task. */
  
  if( pCurrentTaskTCB->Next == pCurrentTaskTCB
      || pCurrentTaskTCB == &TCBSegment[0])
  {
    /* Case 1 & 3. */

    /* This is the only ready task, so it should reinstitute the idle task. */
    pCurrentTaskTCB->Next = &TCBSegment[0];
    
    /* Configure the idle task to point at itself. */
    TCBSegment[0].Prev = &TCBSegment[0];
    TCBSegment[0].Next = &TCBSegment[0];
  }
  else /* There are other ready tasks. */
  {
    /* Case 2 & 4. */
    
    /*  There are other ready task(s), but the current task is the idle task.
        Update the previous task's next pointer to this task's next pointer. */
    pCurrentTaskTCB->Prev->Next = TCBSegment[Identity].Next;
    
    /*  Update the next task's previous pointer to this task's previous
        pointer. */
    pCurrentTaskTCB->Next->Prev = TCBSegment[Identity].Prev;
  }
  
  return DK_SUCCESS;
}


signed DK_ConfigureTaskState( unsigned char Identity,
                              DK_TaskState NewState )
{
/* Changes the specified tasks state.

   Result:
   DK_SUCCESS if succesful. */

  /* Enter critical section. */
  DK_DisableInterrupts();
  
  /* If the old task state was DEAD and the new task state is not DEAD,
     increment the number of living tasks. */
  if( TCBSegment[Identity].State == DEAD &&
      NewState != DEAD)
  {
    ++NumberOfLivingTasks;
  }
  else if( TCBSegment[Identity].State != DEAD &&
           NewState == DEAD)
  {
    /* If the old task state was not DEAD and the new task state is DEAD,
       decrement the number of living tasks. */
    --NumberOfLivingTasks;
  }
  
  if(NewState == READY)
  {
    /* Consider updating the ready list. */
    
    /* Possible cases:
       1. Task was ready or running and is still ready or running.
       2. Task was not ready or running but now is.
    */
    if( TCBSegment[Identity].State != READY &&
        TCBSegment[Identity].State != RUNNING )
    {
      DK_RegisterTask(Identity);
    }
    /* Else the task should be in the ready list already since it was
       previously ready or running ready. */
  }
  else /* Task should not be considered for the ready list. */
  {
    /* Possible cases:
       1. Task was ready and should be removed from ready list.
       2. Task was not ready.
    */
    if( TCBSegment[Identity].State == READY ||
        TCBSegment[Identity].State == RUNNING )
    {
      DK_DeregisterTask(Identity);
    }
    /* Else the task was not in the ready list and does not be removed. */
  }
  
  /* Finally, update the state of the task. */
  TCBSegment[Identity].State = NewState;
  
  /* Exit critical section. */
  DK_EnableInterrupts();
  
  return DK_SUCCESS;
}


unsigned char DK_GetRunningTaskIdentity(void)
{
/* Result:
   The identity of the currently running task. */

  return pCurrentTaskTCB->Identity;
}


unsigned DK_GetNumberOfLivingTasks(void)
{
/* Result:
   The current number of tasks not in the DEAD state. */

  return NumberOfLivingTasks;
}


unsigned DK_GetQuantumCount(void)
{
  return QuantumCount;
}


signed DK_ResetQuantumCount(void)
{
  QuantumCount = 0;

  return DK_SUCCESS;
}


void DK_IdleTask(void)
{
/* A special task that is always READY or RUNNING for use when no other other
   task is available. */
   
  /* It's the first time.  Discard entire stack contents. */
  DK_DiscardStack();

  DK_StartScheduler();

  DK_USB_Start();
  DK_Assert(Result != 1);
  
  /* Exit critical section. */
  DK_EnableInterrupts();  

  while(1)
  {
    /* Engage in the tiresome responsabilities of the idle task, nothing but the
       user defined function. */
    DK_IdleTaskHook();
  }
}
