/*******************************************************************************
Dreamcatcher Kernel
Stephen Niedzielski

This file contains all device independent declarations.
*******************************************************************************/

#ifndef DK_CORE_H
#define DK_CORE_H


/*******************************************************************************
USER
Kernel function declarations, symbols, macros, and types that users may use.
*******************************************************************************/
/* Standard function success return value. */
#define DK_SUCCESS (1)

/* Standard function failure value. */
#define DK_FAILURE (-1)

#ifndef TRUE
  #define TRUE  1
#elif TRUE != 1
  #error Conflicting definintions for TRUE macro.  Dreamcatcher Kernel \
         requires TRUE be of value 1.
#endif

#ifndef FALSE
  #define FALSE  0
#elif FALSE != 0
  #error Conflicting definintions for FALSE macro.  Dreamcatcher Kernel \
         requires FALSE be of value 0.
#endif

/* Possible task states.  Only ready and running tasks are processed by the
   scheduler; dead tasks are inactive and may be initialized to a new task;
   other task states are the responsibility of the user. */
typedef enum
{
   DEAD = 0, /* This task is inactive and its resources are deallocated. */

   READY,    /* This task is not being executed, but the scheduler is allowed to
                select it for execution. */

   RUNNING,  /* This task is currently being executed. */

   BLOCKED,  /* This task cannot be executed because it is awaiting access to
                some resource. */

   WAITING,  /* This task is being forced to wait. */

   DORMANT   /* This task should be ignored and is not processed by the task
                scheduler. */
} DK_TaskState;


/* A pointer to a task typedef.  Tasks should have a signature of
   void Task(void). */
typedef long short unsigned  DK_TaskAddress;


signed DK_InitializeKernel(void);
void DK_StartKernel(void);
signed DK_ConfigureTaskState( unsigned char Identity,
                              DK_TaskState NewState );
unsigned char DK_GetRunningTaskIdentity(void);
unsigned DK_GetNumberOfLivingTasks(void);
unsigned DK_GetQuantumCount(void);
signed DK_ResetQuantumCount(void);


/*******************************************************************************
KERNEL
Kernel function declarations, symbols, macros, and types that users should not
use.
*******************************************************************************/
typedef struct DK_TCB
{
   unsigned StackPointer; /* This variable should be declared at the beginning
                             of the structure, which C gurantees to be at the
                             address of the struct itself. */

   unsigned char Identity;

   unsigned QuantumShare;

   DK_TaskState   State;

   /* These pointers allow for TCB link lists. */
   struct DK_TCB * Next,
                 * Prev;
} DK_TCB;


extern DK_TCB TCBSegment[];
extern DK_TCB * pCurrentTaskTCB;


#if !__STDC__
  #error  This compiler does not accept Standard C or it has a really shoddy \
          implementation that does not define __STDC__.  If you #define your \
          way out of this error (#define __STDC__), you're probably in for a \
          lot more.
#endif

#ifdef __cplusplus__
  #error This compiler is attempting to compile in C++, which is not \
         supported.  If you know what you are doing, #undef __cplusplus__ \
         around this preprocessor check.  Make sure you redefine it \
         immiediately after the check.  You're probably in for some trouble.
#endif


signed DK_Scheduler(void);
signed DK_InitializeTCBSegment(void);
void DK_IdleTask(void);
signed DK_InitializeScheduler(void);


#endif /* DK_CORE_H */
