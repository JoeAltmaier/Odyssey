#/*************************************************************************/
#// This material is a confidential trade secret and proprietary 
#// information of ConvergeNet Technologies, Inc. which may not be 
#// reproduced, used, sold or transferred to any third party without the 
#// prior written consent of ConvergeNet Technologies, Inc.  This material 
#// is also copyrighted as an unpublished work under sections 104 and 408 
#// of Title 17 of the United States Code.  Law prohibits unauthorized 
#// use, copying or reproduction.
#//
#// File: tmt.s
#//
#// Description:
#// This file contains the target dependent routines of the timer management
#// component, and is part of the R5000 port from ATI.  The R5000 code was
#// developed using GNU tools.  We are using Metrowerks tools.  There are
#// one or two differences.
#//
#// MetroWerkarounds:
#// -----------------
#// Assembler directive changes.
#//    .set noreorder       -> .option reorder off
#//    .align 2             -> .align 4
#//    .extern symbol size  -> .extern symbol
#//
#// Explicitly declare external function references.
#//
#//
#// Update Log:
#// 09/30/98 Jeff Nespor: MetroWerkarounds.
#// 10/05/98 Jeff Nespor: Cleanup for check in.
#// 10/28/98 Jeff Nespor: Changes for Metrowerks CodeWarrior Pro3 IDE.
#//                       Assembler accepts #define and #include statements.
#// 10/19/99 Jeff Nespor: Added support for gp relative addressing (sdata).
#/*************************************************************************/


#/*************************************************************************/
#// The following is the original Accelerated Technology file prologue.
#/*************************************************************************/
#
#/*************************************************************************/
#/*                                                                       */
#/*            Copyright (c) 1996 Accelerated Technology, Inc.            */
#/*                                                                       */
#/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
#/* subject matter of this material.  All manufacturing, reproduction,    */
#/* use, and sales rights pertaining to this subject matter are governed  */
#/* by the license agreement.  The recipient of this software implicitly  */
#/* accepts the terms of the license.                                     */
#/*                                                                       */
#/*************************************************************************/
#
#/*************************************************************************/
#/*                                                                       */
#/* FILE NAME                                            VERSION          */
#/*                                                                       */
#/*      tmt.s                                   PLUS/IDT5000/GNU 1.0     */
#/*                                                                       */
#/* COMPONENT                                                             */
#/*                                                                       */
#/*      TM - Timer Management                                            */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This file contains the target dependent routines of the timer    */
#/*      management component.                                            */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* DATA STRUCTURES                                                       */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* FUNCTIONS                                                             */
#/*                                                                       */
#/*      TMT_Set_Clock                       Set system clock             */
#/*      TMT_Retrieve_Clock                  Retrieve system clock        */
#/*      TMT_Read_Timer                      Read count-down timer        */
#/*      TMT_Enable_Timer                    Enable count-down timer      */
#/*      TMT_Adjust_Timer                    Adjust count-down timer      */
#/*      TMT_Disable_Timer                   Disable count-down timer     */
#/*      TMT_Retrieve_TS_Task                Retrieve time-sliced task ptr*/
#/*      TMT_Timer_Interrupt                 Process timer interrupt      */
#/*                                                                       */
#/* DEPENDENCIES                                                          */
#/*                                                                       */
#/*      tc_extr.h                           Thread Control functions     */
#/*      tm_extr.h                           Timer functions              */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996      Prepared initial version 1.0     */
#/*      S. Nguyen       06-30-1998      Version 1.0 for IDT5000, 64 bits */
#/*                                                                       */
#/*************************************************************************/
##define         NU_SOURCE_FILE
#
#
##include        "tc_extr.h"                 /* Thread control functions  */
##include        "tm_extr.h"                 /* Timer functions           */
#include "int_pri.h"
#
#
#/* Define external inner-component global data references.  */
#
#extern VOID           *TCD_Current_Thread;
#extern UNSIGNED        TMD_System_Clock;
#extern UNSIGNED        TMD_Timer;
#extern INT             TMD_Timer_State;
#extern UNSIGNED        TMD_Time_Slice;
#extern TC_TCB         *TMD_Time_Slice_Task;
#extern INT             TMD_Time_Slice_State;
#extern TC_HCB          TMD_HISR;
#
#/* These externs will be in the small data section if sdata >= 4. */
#
        .extern sdata:TCD_Current_Thread
        .extern sdata:TCD_Interrupt_Level
        .extern sdata:TCD_R7KInterrupt_Level
        .extern sdata:TMD_System_Clock
        .extern sdata:TMD_Timer
        .extern sdata:TMD_Timer_State
        .extern sdata:TMD_Time_Slice
        .extern sdata:TMD_Time_Slice_Task
        .extern sdata:TMD_Time_Slice_State
#
#/* These externs will be in the small data section if sdata >= 88. */
#
        .extern sdata:TMD_HISR


#
#/* Define external function references.  */
#
#STATUS  TCT_Activate_HISR(NU_HISR *pointer);
#
        .extern TCT_Activate_HISR

#
#/* Define constants */
# 

        X           = 2                     # Multiplier (32 bits:1, 64 bits:2)
        INT_MASK    = 0xFFFF00FF            # Remove interrupt mask bits

        .text
        .option reorder off

#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Set_Clock                                                    */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function sets the system clock to the specified value.      */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      new_value                           New value for the clock      */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TMT_Set_Clock(UNSIGNED new_value)
#{
        .align  4
        .globl  TMT_Set_Clock
TMT_Set_Clock:
#
#    /* Set the system clock to the specified value.  */
#    TMD_System_Clock =  new_value;
#
        sw      $4, TMD_System_Clock        # Setup the system clock
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Retrieve_Clock                                               */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function returns the current value of the system clock.     */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      TMD_System_Clock                    Value of system clock        */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#UNSIGNED  TMT_Retrieve_Clock(void)
#{
        .align  4
        .globl  TMT_Retrieve_Clock
TMT_Retrieve_Clock:
#
#    /* Return the current value of the system clock.  */
#    return(TMD_System_Clock);
#
        lw      $2, TMD_System_Clock        # Pickup current system clock
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Read_Timer                                                   */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function returns the current value of the count-down timer. */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TMC_Start_Timer                     Start timer function         */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      TMD_Timer                           Value of count-down timer    */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#UNSIGNED  TMT_Read_Timer(void)
#{
        .align  4
        .globl  TMT_Read_Timer
TMT_Read_Timer:
#
#    /* Return the current value of the count-down timer.  */
#    return(TMD_Timer);
#
        lw      $2, TMD_Timer               # Pickup timer contents
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Enable_Timer                                                 */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function enables the count-down timer with the specified    */
#/*      value.                                                           */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TMC_Start_Timer                     Start timer function         */
#/*      TMC_Timer_Task                      Timer expiration task        */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      time                                New count-down time          */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TMT_Enable_Timer(UNSIGNED time)
#{
        .align  4
        .globl  TMT_Enable_Timer
TMT_Enable_Timer:
#
#    /* Place the new time value into the count-down timer.  */
#    TMD_Timer =  time;
#    
#    /* Indicate that the timer is active.  */
#    TMD_Timer_State =  TM_ACTIVE;
#
        sw      $4, TMD_Timer               # Setup the system timer
        sw      $0, TMD_Timer_State         # Activate the timer
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Adjust_Timer                                                 */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function adjusts the count-down timer with the specified    */
#/*      value- if the new value is less than the current.                */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TMC_Start_Timer                     Start timer function         */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      time                                New count-down time          */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TMT_Adjust_Timer(UNSIGNED time)
#{
        .align  4
        .globl  TMT_Adjust_Timer
TMT_Adjust_Timer:
#
#    /* Lockout interrupts.  */
#
        mfc0    $8, $12                     # Pickup status register
        li      $9, INT_MASK                # Build mask for interrupt bits
        and     $10, $8, $9                 # Clear the interrupt bits
        mtc0    $10, $12                    # Lockout all interrupts
        nop                                 # Delay 
        nop                                 # Delay 
        nop                                 # Delay 
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
		ctc0    $9, $20
		nop
		nop
		nop
#endif

#
#    /* See if new time value is less than current time value.  */
#    if (time < TMD_Timer)
#   
        lw      $8, TMD_Timer               # Pickup current timer
        lw      $9, TCD_Interrupt_Level     # Pickup interrupt level
        sltu    $11, $4, $8                 # Is new timer less than current?
        beq     $11, $0, TMT_No_Adjust      # If not less, skip adjustment
        or      $10, $10, $9                # Build restore interrupt value    
   
#
#        /* Adjust timer.  */
#        TMD_Timer =  time;
#
        sw      $4, TMD_Timer               # Setup new timer value
#
TMT_No_Adjust:
#
#    /* Restore interrupt level.  */
#
#    /* Return to caller.  */
#
#ifdef INCLUDE_ODYSSEY
		lw		$9, TCD_R7KInterrupt_Level
		ori		$9, $9, CONFIG_ICR_VS
		ctc0	$9, $20
		nop
		nop
		nop
#endif
        mtc0    $10, $12                    # Restore interrupt level
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Disable_Timer                                                */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function disables the count-down timer.                     */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TMC_Start_Timer                     Start timer function         */
#/*      TMC_Timer_Task                      Timer expiration task        */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TMT_Disable_Timer(void)
#{
        .align  4
        .globl  TMT_Disable_Timer
TMT_Disable_Timer:
#
#    /* Disable the count-down timer.  */
#    TMD_Timer_State =  TM_NOT_ACTIVE;
#
        li      $8, 1                       # Build TM_NOT_ACTIVE value
        sw      $8, TMD_Timer_State         # De-activate the timer
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Retrieve_TS_Task                                             */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function returns the time-sliced task pointer.              */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TMC_Timer_HISR                      Timer HISR                   */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      TMD_Time_Slice_Task                 Time sliced task pointer     */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#NU_TASK  *TMT_Retrieve_TS_Task(VOID)
#{
        .align  4
        .globl  TMT_Retrieve_TS_Task
TMT_Retrieve_TS_Task:
#
#    /* Return time-sliced task pointer.  */
#    return(TMD_Time_Slice_Task);
#
        lw      $2, TMD_Time_Slice_Task     # Pickup time-slice task pointer
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TMT_Timer_Interrupt                                              */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function processes the actual hardware interrupt.           */
#/*      Processing includes updating the system clock and the count-     */
#/*      down timer and the time-slice timer.  If one or both of the      */
#/*      timers expire, the timer HISR is activated.                      */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Interrupt Vector                                                 */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      TCT_Activate_HISR                   Activate timer HISR          */
#/*      TCT_Interrupt_Context_Save          Save interrupted context     */
#/*      TCT_Interrupt_Context_Restore       Restore interrupted context  */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TMT_Timer_Interrupt(void)
#{
        .align  4
        .globl  TMT_Timer_Interrupt
TMT_Timer_Interrupt:
#
#    /* Increment the system clock.  */
#    TMD_System_Clock++;
#
        lw      $8,  TMD_System_Clock       # Pickup system clock
        lw      $10, TMD_Timer_State        # Pickup the timer state
        addiu   $8, $8, 1                   # Increment the system clock
        lw      $11, TMD_Time_Slice_State   # Pickup the time-slice state
#    
#    /* Determine if the count-down timer is active.  */
#    if (TMD_Timer_State == TM_ACTIVE)
#    {
#
        lw      $9, TMD_Timer               # Pickup the timer
        sw      $8, TMD_System_Clock        # Store the new clock value
        bne     $10, $0, _TMT_No_Timer_Exp  # See if the timer is active
        nop                                 # Delay slot
#    
#        /* Decrement the count-down timer.  */
#        TMD_Timer--;
#        
        addiu   $9, $9, -1                  # Decrement the timer
#
#        /* Determine if the timer has expired. If so, modify the state 
#           to indicate that it has expired.  */
#        if (TMD_Timer == 0)
#
        sw      $9, TMD_Timer               # Store the new timer
        bne     $9, $0, _TMT_No_Timer_Exp   # If not 0, timer has not expired
        nop                                 # Delay slot
#
#            TMD_Timer_State =  TM_EXPIRED;
#
        li      $10, 2                      # Build TM_EXPIRED value
        sw      $10, TMD_Timer_State        # Indicate state is expired
#    }
_TMT_No_Timer_Exp:
#    
#    /* Determine if the time-slice timer is active.  Note that the parameters
#       for the time-slice are controlled by the Thread Control (TC) 
#       component.  */
#    if (TMD_Time_Slice_State == TM_ACTIVE)
#    {
#
        lw      $9, TMD_Time_Slice          # Pickup the time slice
        bne     $11, $0, _TMT_No_TS_Exp     # See if time-slice is active
        nop                                 # Delay slot        
#    
#        /* Decrement the time slice counter.  */
#        TMD_Time_Slice--;
#
        addiu   $9, $9, -1                  # Decrement the time-slice value
#        
#        /* Determine if the time-slice timer has expired. If so, modify the
#           time-slice state to indicate that it has.  */
#        if (TMD_Time_Slice == 0)
#        {
#
        sw      $9, TMD_Time_Slice          # Store new time-slice value
        bne     $9, $0, _TMT_No_TS_Exp      # See if time-slice has expired
        nop                                 # Delay slot
#                
#            TMD_Time_Slice_State =  TM_EXPIRED;
#
        li      $11, 2                      # Build TM_EXPIRED value
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
        sw      $11, TMD_Time_Slice_State   # Store into time-slice state flag
#
#            /* Copy the current thread into the time-slice task pointer.  */
#            TMD_Time_Slice_Task =  TCD_Current_Thread;
#
#            ((TC_TCB *) TCD_Current_Thread) -> tc_cur_time_slice =  1;
#
        li      $9, 1                       # Build a one-tick value
        sw      $9, 32($8)                  # Store in task control block
        sw      $8, TMD_Time_Slice_Task     # Indicate the task that needs to 
                                            #   time-sliced
#        }
#    }
_TMT_No_TS_Exp:
#    
#    /* Determine if either of the basic timers have expired.  If so, 
#       activate the timer HISR.  */
#    if ((TMD_Timer_State == TM_EXPIRED) ||
#        (TMD_Time_Slice_State == TM_EXPIRED))
#    {
#
        la      $4, TMD_HISR                # Build Timer HISR handle
        beq     $10, 2, _TMT_Expiration     # Check for timer expiration
        nop                                 # Delay slot
        bne     $11, 2, _TMT_No_Expiration  # Check for time-slice expiration
        nop                                 # Delay slot
_TMT_Expiration:
#
#        /* Activate the HISR timer function.  */
#        TCT_Activate_HISR(&TMD_HISR);
#

        addiu   $sp, $sp, -4*X              # Allocate space to save ra
        sd      $31, 0($sp)                 # Save return address on the stack
        jal     TCT_Activate_HISR           # Activate timer HISR
        nop                                 # Delay slot
        ld      $31, 0($sp)                 # Recover saved return address
        addiu   $sp, $sp, 4*X               # Recover stack space

#    }
_TMT_No_Expiration:
#
        j       $31                         # Return to caller
        nop                                 # Delay slot
#}

