;*******************************************************************************
;Dreamcatcher Kernel
;Stephen Niedzielski
;
;This file contains PIC18F4550 specific assembly for saving and restoring
;context data.
;******************************************************************************/

;  Include this chip's standard macros, register definitions, and other
;  symbolics.
#include <P18F4550.inc>

  ; Imports.
  extern DK_Scheduler
  extern DK_USB_ISR
  extern DK_ISR
  extern pCurrentTaskTCB
  extern __AARGB0
  extern __AARGB1
  extern __AARGB2
  extern __AARGB3
  extern __AARGB4
  extern __AARGB5
  extern __AARGB6
  extern __AARGB7
  extern __BARGB0
  extern __BARGB1
  extern __BARGB2
  extern __BARGB3
  extern __AEXP
  extern __BEXP
  extern __FPFLAGS
  extern __TEMPB0
  extern __TEMPB1
  extern __TEMPB2
  extern __TEMPB3
  extern __REMB0
  extern __REMB1
  extern __REMB2
  extern __REMB3

  code  ; Declare a region of code.

;*******************************************************************************
DK_SaveContext:
; Records registers on software stack.  These saved registers should be restored
; in reverse order.

  movff WREG,PREINC1        ; Working register (accumulator).
  movff STATUS,PREINC1      ; Status register (zero flag, negative flag, ...).
  movff BSR,PREINC1         ; Bank select register.
  movff FSR2L,PREINC1       ; Frame pointer LSB.
  movff FSR2H,PREINC1       ; ''            MSB.
  movff FSR0L,PREINC1       ; File select register zero LSB (general addressing
                            ; register).
  movff FSR0H,PREINC1       ; ''                        MSB.
  movff TABLAT,PREINC1      ;
  movff TBLPTRL,PREINC1     ;
  movff TBLPTRH,PREINC1     ;
  movff TBLPTRU,PREINC1     ;
  movff PRODL,PREINC1       ;
  movff PRODH,PREINC1       ;
  movff __AARGB0,PREINC1    ;
  movff __AARGB1,PREINC1    ;
  movff __AARGB2,PREINC1    ;
  movff __AARGB3,PREINC1    ;
  movff __FPFLAGS,PREINC1   ;
  movff __AARGB4,PREINC1    ;
  movff __AARGB5,PREINC1    ;
  movff __AARGB6,PREINC1    ;
  movff __AARGB7,PREINC1    ;
  movff __BARGB0,PREINC1    ;
  movff __BARGB1,PREINC1    ;
  movff __BARGB2,PREINC1    ;
  movff __BARGB3,PREINC1    ;
  movff __AEXP,PREINC1      ;
  movff __BEXP,PREINC1      ;
  movff __TEMPB0,PREINC1    ;
  movff __TEMPB1,PREINC1    ;
  movff __TEMPB2,PREINC1    ;
  movff __TEMPB3,PREINC1    ;
  movff __REMB0,PREINC1     ;
  movff __REMB1,PREINC1     ;
  movff __REMB2,PREINC1     ;
  movff __REMB3,PREINC1     ;

  ; Now record the the hardware stack which consists of nothing but address'.
  ; We will need the offset immiediately when we later restore the registers,
  ; so we will save it last.  For now, we save it in a temporary location.
  movff STKPTR,PRODL
  
  ; Initialize WREG to zero.  This will later allow for ORing to see if the
  ; stack offset (STKPTR) is zero.
  clrf  WREG
  
DK_SaveContext_Continue:
  movff TOSL,PREINC1          ; Save the address LSB.
  movff TOSH,PREINC1          ; ''
  movff TOSU,PREINC1          ; ''               MSB.
  pop                         ; This guy's finished.  Pop him off the top.
  iorwf STKPTR                ; ORing will throw a zero flag when we're done.
  bnz DK_SaveContext_Continue ; If the counter is not zero, continue.
  ; Otherwise, the counter is zero.  Fall through and we're done with the
  ; address' of the hardware stack.

  ; Now save the old hardware stack offset in a safe place, the top of the
  ; stack.  Do it again to force an increment and allow the stack pointer
  ; to point to an empty space.
  movff PRODL,PREINC1
  movff PRODL,PREINC1
  
  ; Finally, update the task's TCB stack pointer:
  ; pCurrentTaskTCB->StackPointer = (((unsigned)(FSR0H)) << 8) | FSR0L;
  movff pCurrentTaskTCB,FSR0L
  movff pCurrentTaskTCB+1,FSR0H
  movff FSR1L,POSTINC0
  movff FSR1H,POSTINC0
  
  ; Software stack pointer is currently pointing at a free space.  Hardware
  ; stack is empty and must be loaded prior to performing a return from
  ; interrupt command, as that will pop the stack once more.

  ; If this is not a TMR0 or USB interrupt, invoke the user's interrupt handler.
  btfsc INTCON,TMR0IF
  bra $+8

  btfss PIR2,USBIF  
  call DK_ISR

  ; If this is a USB interrupt, call the USB ISR.
  btfsc PIR2,USBIF
  call DK_USB_ISR

  ; If this is a scheduler clock interrupt, call the call the scheduler.
  btfsc INTCON,TMR0IF
  call DK_Scheduler


  ; Fall through to DK_RestoreContext.


DK_RestoreContext:
  ; Restore the new task's context data, this is backwards the way it was
  ; saved since this is a stack data structure - first in, last out.

  ; Load the new task's software stack pointer.
  movff pCurrentTaskTCB,FSR0L
  movff pCurrentTaskTCB+1,FSR0H
  movff POSTINC0,FSR1L
  movff POSTINC0,FSR1H

  ; Stack pointer is currently pointing at a free space, so predecrement it
  ; and load the hardware stack's offset into a counter variable.  The offset
  ; will grow automatically as we load the address data.
  movff POSTDEC1,FSR0L
  movff POSTDEC1,FSR0L

  ; Calculate the number of hardware stack pushes needed and store it in a
  ; convenient place.
  movff STKPTR,WREG
  subwf FSR0L,0
  movff WREG,PRODL

  ; Now load the hardware stack address'.
DK_RestoreContext_Continue:
  incf  STKPTR,1,0                ; Increment the stack pointer.
  movff POSTDEC1,WREG             ; Load the address MSB into the working. */
  movwf TOSU,0                    ;
  movff POSTDEC1,WREG             ;
  movwf TOSH,0                    ;
  movff POSTDEC1,WREG             ;
  movwf TOSL,0                    ;
  decf PRODL                      ;
  bnz DK_RestoreContext_Continue  ;

  ; Time for the massive register restoration.
  movff POSTDEC1,__REMB3
  movff POSTDEC1,__REMB2
  movff POSTDEC1,__REMB1
  movff POSTDEC1,__REMB0
  movff POSTDEC1,__TEMPB3
  movff POSTDEC1,__TEMPB2
  movff POSTDEC1,__TEMPB1
  movff POSTDEC1,__TEMPB0
  movff POSTDEC1,__BEXP
  movff POSTDEC1,__AEXP
  movff POSTDEC1,__BARGB3
  movff POSTDEC1,__BARGB2
  movff POSTDEC1,__BARGB1
  movff POSTDEC1,__BARGB0
  movff POSTDEC1,__AARGB7
  movff POSTDEC1,__AARGB6
  movff POSTDEC1,__AARGB5
  movff POSTDEC1,__AARGB4
  movff POSTDEC1,__FPFLAGS
  movff POSTDEC1,__AARGB3
  movff POSTDEC1,__AARGB2
  movff POSTDEC1,__AARGB1
  movff POSTDEC1,__AARGB0
  movff POSTDEC1,PRODH
  movff POSTDEC1,PRODL
  movff POSTDEC1,TBLPTRU
  movff POSTDEC1,TBLPTRH
  movff POSTDEC1,TBLPTRL
  movff POSTDEC1,TABLAT
  movff POSTDEC1,FSR0H
  movff POSTDEC1,FSR0L
  movff POSTDEC1,FSR2H
  movff POSTDEC1,FSR2L
  movff POSTDEC1,BSR
  movff POSTDEC1,STATUS
  movff POSTDEC1,WREG

  ; Clear out scheduler clock interrupt flag.
  bcf INTCON,TMR0IF

  ; It is now safe to turn on the scheduler clock.
  bsf T0CON,7,0

  ; Jump back to the restored process and re-enable interrupts.
  retfie 0


;*******************************************************************************
  org  0x08  ; Place in the high priority interrupt vector.
DK_ISR_SchedulerClock:

  ; Disable all interrupts.  (This instruction does not affect the status or
  ; WREG registers.)
  bcf  INTCON,GIE

	; Else, invoke the context save routine and never return.  (BTW, this
	; instruction does not affect the status or WREG registers.)
	goto DK_SaveContext
  
  end ; executable code region.
