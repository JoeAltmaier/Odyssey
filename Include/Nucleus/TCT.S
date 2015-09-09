#/*************************************************************************/
#// This material is a confidential trade secret and proprietary 
#// information of ConvergeNet Technologies, Inc. which may not be 
#// reproduced, used, sold or transferred to any third party without the 
#// prior written consent of ConvergeNet Technologies, Inc.  This material 
#// is also copyrighted as an unpublished work under sections 104 and 408 
#// of Title 17 of the United States Code.  Law prohibits unauthorized 
#// use, copying or reproduction.
#//
#// File: tct.s
#//
#// Description:
#// This file contains target processor dependent routines for performing
#// target dependent scheduling functions, and is part of the R5000 port
#// from ATI.  The R5000 code was developed using GNU tools.  We are using
#// Metrowerks tools.  There are one or two differences.  Besides those
#// listed, look for MetroWerkaround comments throughout this file.
#//
#// MetroWerkarounds:
#// -----------------
#// Assembler directive changes.
#//    .set noreorder       -> .option reorder off
#//    .set noat            -> .option no_at_macros on
#//    .set at              -> .option no_at_macros off
#//    .align 2             -> .align 4
#//    .extern symbol size  -> .extern symbol
#//    .set mips3           -> not supported, but not required
#//
#// Unsupported assembler syntax (MetroWerkaround).
#//    lw rt0, base(offset) -> la rt1, base
#//                            add rt1, rt1, offset
#//                            lw rt0, 0(rt1)
#//    sw rt0, base(offset) -> la rt1, base
#//                            add rt1, rt1, offset
#//                            sw rt0, 0(rt1)
#//
#// Explicitly declare external function references.
#//
#// Explicitly use jalr command when target address is held in a register.
#//
#//
#// Update Log:
#// 09/30/98 Jeff Nespor: MetroWerkarounds.
#// 10/05/98 Jeff Nespor: Cleanup for check in.
#// 10/20/98 Joe Altmaier: BuildTaskStack - round pointer and size to
#//                        8-byte boundary.
#// 10/28/98 Jeff Nespor: Changes for Metrowerks CodeWarrior Pro3 IDE.
#//                       Assembler accepts #define and #include statements.
#// 10/11/99 Jeff Nespor: Removed floating point registers from context
#//                       save and restore operations (USE_FP_REGS).
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
#/*      tct.s                                    PLUS/IDT5000/GNU 1.0    */
#/*                                                                       */
#/* COMPONENT                                                             */
#/*                                                                       */
#/*      TC - Thread Control                                              */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This file contains the target processor dependent routines for   */
#/*      performing target-dependent scheduling functions.                */
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
#/*      TCT_Control_Interrupts              Enable / disable interrupts  */
#/*      TCT_Local_Control_Interrupts        Local enable / disable ints  */
#/*      TCT_Restore_Interrupts              Restore global interrupt lck */
#/*      TCT_Build_Task_Stack                Build initial task stack     */
#/*      TCT_Build_HISR_Stack                Build initial HISR stack     */
#/*      TCT_Build_Signal_Frame              Build signal handler frame   */
#/*      TCT_Check_Stack                     Check current stack          */
#/*      TCT_Schedule                        Schedule the next thread     */
#/*      TCT_Control_To_Thread               Transfer control to a thread */
#/*      TCT_Control_To_System               Transfer control from thread */
#/*      TCT_Signal_Exit                     Exit from signal handler     */
#/*      TCT_Current_Thread                  Returns a pointer to current */
#/*                                            thread                     */
#/*      TCT_Set_Execute_Task                Sets TCD_Execute_Task under  */
#/*                                            protection from interrupts */
#/*      TCT_Protect                         Protect critical section     */
#/*      TCT_Unprotect                       Unprotect critical section   */
#/*      TCT_Unprotect_Specific              Release specific protection  */
#/*      TCT_Set_Current_Protect             Set the thread's current     */
#/*                                            protection field           */
#/*      TCT_Protect_Switch                  Switch to protected thread   */
#/*      TCT_Schedule_Protected              Schedule the protected thread*/
#/*      TCT_Interrupt_Context_Save          Save interrupted context     */
#/*      TCT_Interrupt_Context_Restore       Restore interrupted context  */
#/*      TCT_Activate_HISR                   Activate a HISR              */
#/*      TCT_HISR_Shell                      HISR execution shell         */
#/*                                                                       */
#/* DEPENDENCIES                                                          */
#/*                                                                       */
#/*      cs_extr.h                           Common Service functions     */
#/*      tc_extr.h                           Thread Control functions     */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*      NAME         DATE               REMARKS                          */
#/*                                                                       */
#/*      J. Martin    10-16-1996         Prepared initial version 1.0     */
#/*      S. Nguyen    06-30-1998         Version 1.0 for IDT5000, 64 bits */
#/*                                                                       */
#/*************************************************************************/
##define         NU_SOURCE_FILE
#
##include        "cs_extr.h"                 /* Common service functions  */
##include        "tc_extr.h"                 /* Thread control functions  */
#
#
#/* Define external inner-component global data references.  */
#
#extern TC_TCB          *TCD_Execute_Task;
#extern TC_HCB          *TCD_Execute_HISR;
#extern VOID            *TCD_Current_Thread;
#extern VOID            *TCD_System_Stack;
#extern INT              TCD_Interrupt_Count;
#extern TC_HCB          *TCD_Active_HISR_Heads[TC_HISR_PRIORITIES];
#extern TC_HCB          *TCD_Active_HISR_Tails[TC_HISR_PRIORITIES];
#extern INT              TCD_Interrupt_Level;
#extern UNSIGNED         TMD_Time_Slice;
#extern INT              TMD_Time_Slice_State;
#
#include "int_pri.h"

#
#/* These externs will be in the small data section if sdata >= 4. */
#
        .extern sdata:TCD_Execute_Task
        .extern sdata:TCD_Execute_HISR
        .extern sdata:TCD_Current_Thread
        .extern sdata:TCD_System_Stack
        .extern sdata:TCD_Interrupt_Count
        .extern sdata:TCD_Interrupt_Level
        .extern sdata:TCD_R7KInterrupt_Level
        .extern sdata:TMD_Time_Slice
        .extern sdata:TMD_Time_Slice_State
#
#
#/* These externs will be in the small data section if sdata >= 12. */
#
        .extern sdata:TCD_Active_HISR_Heads
        .extern sdata:TCD_Active_HISR_Tails


#
#/* Define external function references.  */
#
#VOID    TCC_Task_Shell(VOID);
#VOID    TCC_Signal_Shell(VOID);
#VOID    ERC_System_Error(INT error);
#
        .extern TCC_Task_Shell
        .extern TCC_Signal_Shell
        .extern ERC_System_Error


#/* Define internal function references.  */
#VOID  TCT_Schedule_Protected(VOID *thread);
 

#/* Define various constants specific to the MIPS processor family.  */

        X           = 2                     # Multiplier (32 bits:1, 64 bits:2)
        INIT_SR     = 0x34018381            # Initial SR for thread 
        INT_DISABLE = 0x34010081            # Disable interrupts SR

        INT_BITS    = 0x0000FF00            # Isolate interrupt lockout bits
        INT_MASK    = 0xFFFF00FF            # Remove interrupt mask bits
        INT_BITS_R7K= 0x000F0000            # Isolate R7K interrupt lockout bits
        INT_MASK_R7K= 0xFFF0FFFF            # Remove R7K interrupt mask bits

#ifdef INCLUDE_ODYSSEY
        TASK_STK    = 264*X                 # With float registers
#else
        TASK_STK    = 260*X                 # With float registers
#endif
        HISR_STK    = 96*X                  # With float registers

 
#
#/* Whether or not to save and restore floating point registers.  */
#
#define USE_FP_REGS 0


        .text
        .option reorder off

#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Control_Interrupts                                           */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function enables and disables interrupts as specified by    */
#/*      the caller.  Interrupts disabled by this call are left disabled  */
#/*      until the another call is made to enable them.                   */
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
#/*      new_level                           New interrupt enable level   */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      old_level                           Previous interrupt enable    */
#/*                                            level                      */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#INT  TCT_Control_Interrupts(INT new_level)
#{
        .align  4
        .globl  TCT_Control_Interrupts
TCT_Control_Interrupts:
#
#INT     old_level;                          /* Old interrupt level       */
#
#
#    /* Obtain the current interrupt lockout posture.  */
#    old_level =  TCD_Interrupt_Level;
#       
#    /* Setup new interrupt lockout posture.  */
#    TCD_Interrupt_Level =  new_level;
#        
#    /* Return old interrupt lockout level.  */
#    return(old_level);
#
        mfc0    $8, $12                     # Pickup status register
        li      $9, INT_MASK                # Build mask for interrupt bits
		nop
		nop
        and     $10, $8, $9                 # Clear the interrupt bits
        mtc0    $10, $12                    # Lockout all interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#ifdef INCLUDE_ODYSSEY
		li		$11, CONFIG_ICR_VS
		ctc0   	$11, $20                     # Lockout Interrupts for R7K
		nop
		nop
		nop
		li		$11, INT_BITS
		li		$12, INT_BITS_R7K
		and 	$9, $4, $12
		srl		$9, $9, 8
		and		$4, $4, $11
		or 		$8, $10, $4
		lw		$2, TCD_Interrupt_Level
		sw		$4, TCD_Interrupt_Level
		lw		$13, TCD_R7KInterrupt_Level
		sw		$9, TCD_R7KInterrupt_Level
		sll		$13, $13, 8
		or		$2, $2, $13
		ori		$9, $9, CONFIG_ICR_VS
		ctc0	$9, $20
		nop
		nop
		nop
#else
        or      $8, $10, $4                 # Build a new status register
        lw      $2, TCD_Interrupt_Level     # Pickup the previous value
        sw      $4, TCD_Interrupt_Level     # Save new value
#endif
        mtc0    $8, $12                     # Setup the new status register
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
#/*      TCT_Local_Control_Interrupts                                     */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function enables and disables interrupts as specified by    */
#/*      the caller.                                                      */
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
#/*      new_level                           New interrupt enable level   */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      old_level                           Previous interrupt enable    */
#/*                                            level                      */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#INT  TCT_Local_Control_Interrupts(new_level)
#{
        .align  4
        .globl  TCT_Local_Control_Interrupts
TCT_Local_Control_Interrupts:
#
#INT     old_level;                          /* Old interrupt level       */
#
#
#    /* Obtain the current interrupt lockout posture.  */
#    old_level =  SR & INT_MASK;
#
#    /* Setup new interrupt lockout posture.  */
#        
#    /* Return old interrupt lockout level.  */
#    return(old_level);
#
        mfc0    $8, $12                     # Pickup status register
        nop                                 # Delay slot
        li      $11, INT_BITS               # Build mask for interrupt bits
        and     $2, $8, $11                 # Isolate the interrupt bits
        li      $9, INT_MASK                # Build interrupt interrupt bit
                                            #   clear mask
        and     $10, $8, $9                 # Clear current interrupt bits
#ifdef INCLUDE_ODYSSEY
		li		$12, INT_BITS_R7K
		and		$12, $12, $4
		srl		$12, $12, 8
		and 	$4, $4, $11
		cfc0	$13, $20
		nop
		nop
		nop
		and		$13, $13, $11
		sll		$13, $13, 8
		or		$2, $2, $13
		ori		$12, $12, CONFIG_ICR_VS
		ctc0	$12, $20
		nop
		nop
		nop
#endif
        or      $8, $10, $4                 # Build a new status register
        mtc0    $8, $12                     # Setup the new status register
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
#/*      TCT_Restore_Interrupts                                           */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function restores interrupts to that specified in the       */
#/*      global TCD_Interrupt_Level variable.                             */
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
#/*      None                                                             */
#/*                                            level                      */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TCT_Restore_Interrupts(VOID)
#{
        .align  4
        .globl  TCT_Restore_Interrupts
TCT_Restore_Interrupts:
#
        mfc0    $8, $12                     # Pickup status register
        li      $9, INT_MASK                # Build mask for interrupt bits
        and     $10, $8, $9                 # Clear the interrupt bits
        mtc0    $10, $12                    # Lockout all interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
		ctc0	$9, $20
		nop
		nop
		nop
#endif
        lw      $9, TCD_Interrupt_Level     # Pickup the previous value
        nop                                 # Delay slot
        or      $8, $10, $9                 # Pickup current interrupt level
#ifdef INCLUDE_ODYSSEY
		lw		$13, TCD_R7KInterrupt_Level
		nop
		ori		$13, $13, CONFIG_ICR_VS
		ctc0	$13, $20
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#endif
        mtc0    $8, $12                     # Setup the new status register
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
#/*      TCT_Build_Task_Stack                                             */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function builds an initial stack frame for a task.  The     */
#/*      initial stack contains information concerning initial values of  */
#/*      registers and the task's point of entry.  Furthermore, the       */
#/*      initial stack frame is in the same form as an interrupt stack    */
#/*      frame.                                                           */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Create_Task                     Create a new task            */
#/*      TCC_Reset_Task                      Reset the specified task     */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      task                                Task control block pointer   */
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
#VOID  TCT_Build_Task_Stack(TC_TCB *task)
#{
        .align  4
        .globl  TCT_Build_Task_Stack
TCT_Build_Task_Stack:
#
#    /* Pickup the stack base.  */
#    REG_Stack_Base =  (BYTE_PTR) task -> tc_stack_start;
#    
        lw      $9, 48($4)                  # Pickup the stack size
        lw      $8, 36($4)                  # Pickup the stack starting address
#
#    /* Pickup the stack size.  */
#    REG_Stack_Size =  task -> tc_stack_size;
#    
#    /* Calculate the stack ending address.  */
#    REG_Stack_End =  REG_Stack_Base + REG_Stack_Size - 1;
#
        subu    $9, $9, 4*X                 # Subtract one word from the size
        srl     $9, $9, X+1                 # Insure long word alignment
        sll     $9, $9, X+1
        
        addu	$8, $8, 4*X-1               # Insure long word alignment
        srl     $8, $8, X+1
        sll     $8, $8, X+1

        addu    $8, $8, $9                  # Compute that actual top of the 
                                            #   stack
#
#    /* Save the stack ending address.  */
#    task -> tc_stack_end =  REG_Stack_End;
#    
        sw      $8, 40($4)                  # Store stack ending address
#
#    /* Reference the task shell.  */
#    REG_Function_Ptr =  (VOID *) TCC_Task_Shell;
#
#    /* Build an initial stack frame that facilitates an interrupt return to 
#       the application.  The interrupt stack frame format is as follows:
#
#       Lower Address           Top:    Stack Type = 1  (Interrupt Frame)
#                                       $at             (at)
#                                       $2..$3          (v0-v1)
#                                       $4..$7          (a0-a3)
#                                       $8..$15         (t0-t7)
#                                       $16..$23        (s0-s7)
#                                       $24..$25        (t8-t9)
#                                       $30             (fp)
#                                       $31             (ra)
#                                       hi              (hi)
#                                       lo              (lo)
#                                       CP0 $12         (sr)
#                                       CP0 $14         (epc)
#                                       FPU Control     FPU31
#                                       $f0 - $f30
#                               Bottom: $f31
#
#    */
#
        addiu   $8, $8, -TASK_STK           # Allocate space on the stack
                                            #   256 bytes for normal stack frame
                                            #   264 bytes for FPU context
        li      $12, 1                      # Build stack type value
        la      $11, TCC_Task_Shell         # Build address of task shell
        sd      $12,  0*X($8)               # Put stack type at the top
        sd      $0,   4*X($8)               # Initial $at (at)
        sd      $0,   8*X($8)               # Initial $2  (v0)
        sd      $0,  12*X($8)               # Initial $3  (v1)
        sd      $0,  16*X($8)               # Initial $4  (a0)
        sd      $0,  20*X($8)               # Initial $5  (a1)
        sd      $0,  24*X($8)               # Initial $6  (a2)
        sd      $0,  28*X($8)               # Initial $7  (a3)
        sd      $0,  32*X($8)               # Initial $8  (t0)
        sd      $0,  36*X($8)               # Initial $9  (t1)
        sd      $0,  40*X($8)               # Initial $10 (t2)
        sd      $0,  44*X($8)               # Initial $11 (t3)
        sd      $0,  48*X($8)               # Initial $12 (t4)
        sd      $0,  52*X($8)               # Initial $13 (t5)
        sd      $0,  56*X($8)               # Initial $14 (t6)
        sd      $0,  60*X($8)               # Initial $15 (t7)
        sd      $0,  64*X($8)               # Initial $16 (s0)
        sd      $0,  68*X($8)               # Initial $17 (s1)
        sd      $0,  72*X($8)               # Initial $18 (s2)
        sd      $0,  76*X($8)               # Initial $19 (s3)
        sd      $0,  80*X($8)               # Initial $20 (s4)
        sd      $0,  84*X($8)               # Initial $21 (s5)
        sd      $0,  88*X($8)               # Initial $22 (s6)
        sd      $0,  92*X($8)               # Initial $23 (s7)
        sd      $0,  96*X($8)               # Initial $24 (t8)
        sd      $0, 100*X($8)               # Initial $25 (t9)
        sd      $0, 104*X($8)               # Initial $30 (fp)
        sd      $0, 108*X($8)               # Initial $31 (ra)
        sd      $0, 112*X($8)               # Initial HI
        sd      $0, 116*X($8)               # Initial LO
        li      $12, INIT_SR                # Build initial Status Register SR
    
        sd      $12, 120*X($8)              # Place initial SR on the stack
        sd      $11, 124*X($8)              # Place initial EPC on the stack

#    /* Floating point initial register values.  */
#

#if USE_FP_REGS
        cfc1    $10, $31                    # Pickup current FPU control reg
        sd      $0,  132*X($8)              # Initial $f0
        sd      $0,  136*X($8)              # Initial $f1
        sd      $0,  140*X($8)              # Initial $f2
        sd      $0,  144*X($8)              # Initial $f3
        sd      $10, 128*X($8)              # Initial FPU control register
        sd      $0,  148*X($8)              # Initial $f4
        sd      $0,  152*X($8)              # Initial $f5
        sd      $0,  156*X($8)              # Initial $f6
        sd      $0,  160*X($8)              # Initial $f7
        sd      $0,  164*X($8)              # Initial $f8
        sd      $0,  168*X($8)              # Initial $f9
        sd      $0,  172*X($8)              # Initial $f10
        sd      $0,  176*X($8)              # Initial $f11
        sd      $0,  180*X($8)              # Initial $f12
        sd      $0,  184*X($8)              # Initial $f13
        sd      $0,  188*X($8)              # Initial $f14
        sd      $0,  192*X($8)              # Initial $f15
        sd      $0,  196*X($8)              # Initial $f16
        sd      $0,  200*X($8)              # Initial $f17
        sd      $0,  204*X($8)              # Initial $f18
        sd      $0,  208*X($8)              # Initial $f19
        sd      $0,  212*X($8)              # Initial $f20
        sd      $0,  216*X($8)              # Initial $f21
        sd      $0,  220*X($8)              # Initial $f22
        sd      $0,  224*X($8)              # Initial $f23
        sd      $0,  228*X($8)              # Initial $f24
        sd      $0,  232*X($8)              # Initial $f25
        sd      $0,  236*X($8)              # Initial $f26
        sd      $0,  240*X($8)              # Initial $f27
        sd      $0,  244*X($8)              # Initial $f28
        sd      $0,  248*X($8)              # Initial $f29
        sd      $0,  252*X($8)              # Initial $f30
        sd      $0,  256*X($8)              # Initial $f31
#endif USE_FP_REGS

#    
#    /* Save the minimum amount of remaining stack memory.  */
#    task -> tc_stack_minimum =  REG_Stack_Size - 520;
#
        addiu   $9, $9, -TASK_STK           # Subtract the size of interrupt 
                                            #   stack frame
        sw      $9, 52($4)                  # Save initial minimum in control 
                                            #   block
#
#    /* Save the new stack pointer into the task's control block.  */
#    task -> tc_stack_pointer =  (VOID *) $sp;
#
        sw      $8, 44($4)                  # Save stack pointer in control 
                                            #   block
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
#/*      TCT_Build_HISR_Stack                                             */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function builds an HISR stack frame that allows quick       */
#/*      scheduling of the HISR.                                          */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Create_HISR                     Create HISR function         */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      hisr                                HISR control block pointer   */
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
#VOID  TCT_Build_HISR_Stack(TC_HCB *hisr)
#{
        .align  4
        .globl  TCT_Build_HISR_Stack
TCT_Build_HISR_Stack:
#
#    /* Pickup the stack base.  */
#    REG_Stack_Base =  (BYTE_PTR) hisr -> tc_stack_start;
#    
        lw      $9, 48($4)                  # Pickup the stack size
        lw      $8, 36($4)                  # Pickup the stack starting address
#
#    /* Pickup the stack size.  */
#    REG_Stack_Size =  hisr -> tc_stack_size;
#    
#    /* Calculate the stack ending address.  */
#    REG_Stack_End =  REG_Stack_Base + REG_Stack_Size;
#
        subu    $9, $9, 4*X                 # Subtract one word from the size
        srl     $9, $9, 2*X                 # Insure long word alignment
        sll     $9, $9, 2*X                 # 
        addu    $8, $8, $9                  # Compute that actual top of the 
                                            #   stack

#    /* Save the stack ending address.  */
#    hisr -> tc_stack_end =  REG_Stack_End;
#    
        sw      $8, 40($4)                  # Store stack ending address
#
#    /* Reference the HISR shell.  */
#    REG_Function_Ptr =  (VOID *) TCT_HISR_Shell;
#
#    /* Build an initial stack.  HISR initial stack frames perform a 
#       solicited return to the HISR thread.  The format of the stack 
#       frame is as follows:
#
#       Lower Address           Top:    Stack Type = 0  (Solicited Frame)
#                                       $16..$23        (s0-s7)
#                                       $30             (fp)
#                                       $31             (ra)
#                                       FPU Control     FPU31
#                                       $f20-$f30
#                               Bottom: $f31
#    */
#
        addiu   $8, $8, -HISR_STK           # Allocate space on the stack
                                            #   88 for standard frame
                                            #   104 for FPU frame
        la      $11, TCT_HISR_Shell         # Build address of HISR shell
        sd      $0,   0*X($8)               # Put stack type at the top
        sd      $0,   4*X($8)               # Initial $16 (s0)
        sd      $0,   8*X($8)               # Initial $17 (s1)
        sd      $0,  12*X($8)               # Initial $18 (s2)
        sd      $0,  16*X($8)               # Initial $19 (s3)
        sd      $0,  20*X($8)               # Initial $20 (s4)
        sd      $0,  24*X($8)               # Initial $21 (s5)
        sd      $0,  28*X($8)               # Initial $22 (s6)
        sd      $0,  32*X($8)               # Initial $23 (s7)
        sd      $0,  36*X($8)               # Initial $30 (fp)
        sd      $11, 40*X($8)               # Initial $31 (ra)
#
#    /* Floating point initial register values.  */
#

#if USE_FP_REGS
        cfc1    $10, $31                    # Pickup current FPU control reg
        sd      $0,  48*X($8)               # Initial $f20
        sd      $0,  52*X($8)               # Initial $f21
        sd      $0,  56*X($8)               # Initial $f22
        sd      $0,  60*X($8)               # Initial $f23
        sd      $10, 44*X($8)               # Initial FPU control register
        sd      $0,  64*X($8)               # Initial $f24
        sd      $0,  68*X($8)               # Initial $f25
        sd      $0,  72*X($8)               # Initial $f26
        sd      $0,  76*X($8)               # Initial $f27
        sd      $0,  80*X($8)               # Initial $f28
        sd      $0,  84*X($8)               # Initial $f29
        sd      $0,  88*X($8)               # Initial $f30
        sd      $0,  92*X($8)               # Initial $f31
#endif USE_FP_REGS

#
#    /* Save the minimum amount of remaining stack memory.  */
#    hisr -> tc_stack_minimum =  REG_Stack_Size - 192;
#
        addiu   $9, $9, -HISR_STK           # Subtract the size of solicited 
                                            #   stack frame
        sw      $9, 52($4)                  # Save initial minimum in control 
                                            #   block
#        
#    /* Save the new stack pointer into the task's control block.  */
#    hisr -> tc_stack_pointer =  (VOID *) ($sp);
#
        sw      $8, 44($4)                  # Save stack pointer in control 
                                            #   block
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
#/*      TCT_Build_Signal_Frame                                           */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function builds a frame on top of the task's stack to       */
#/*      cause the task's signal handler to execute the next time         */
#/*      the task is executed.                                            */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Send_Signals                    Send signals to a task       */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      task                                Task control block pointer   */
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
#VOID  TCT_Build_Signal_Frame(TC_TCB *task)
#{
        .align  4
        .globl  TCT_Build_Signal_Frame
TCT_Build_Signal_Frame:
#
#    /* Pickup the stack pointer.  */
#    REG_Stack_Ptr =  (BYTE_PTR) task -> tc_stack_pointer;
#
        lw      $8, 44($4)                  # Pickup current stack pointer
        nop                                 # Delay slot
#
#    /* Reference the Signal shell.  */
#    REG_Function_Ptr =  (VOID *) TCC_Signal_Shell;
#
#    /* Build a signal stack.  Signal stack frames perform a 
#       solicited return to the task.  The format of the stack 
#       frame is as follows:
#
#       Lower Address           Top:    Stack Type = 0  (Solicited Frame)
#                                       $16..$23        (s0-s7)
#                                       $30             (fp)
#                                       $31             (ra)
#                                       FPU Control     FPU31
#                                       $f20-$f30
#                               Bottom: $f31
#    */
#
        addiu   $8, $8, -HISR_STK           # Allocate space on the stack
                                            #   88 for standard frame
                                            #   104 for FPU frame
        la      $11, TCC_Signal_Shell       # Build address of signal shell
        sd      $0,   0*X($8)               # Put stack type at the top
        sd      $0,   4*X($8)               # Initial $16 (s0)
        sd      $0,   8*X($8)               # Initial $17 (s1)
        sd      $0,  12*X($8)               # Initial $18 (s2)
        sd      $0,  16*X($8)               # Initial $19 (s3)
        sd      $0,  20*X($8)               # Initial $20 (s4)
        sd      $0,  24*X($8)               # Initial $21 (s5)
        sd      $0,  28*X($8)               # Initial $22 (s6)
        sd      $0,  32*X($8)               # Initial $23 (s7)
        sd      $0,  36*X($8)               # Initial $30 (fp)
        sd      $11, 40*X($8)               # Initial $31 (ra)
#
#    /* Floating point initial register values.  */
#

#if USE_FP_REGS
        cfc1    $10, $31                    # Pickup current FPU control reg
        sd      $0,  48*X($8)               # Initial $f20
        sd      $0,  52*X($8)               # Initial $f21
        sd      $0,  56*X($8)               # Initial $f22
        sd      $0,  60*X($8)               # Initial $f23
        sd      $10, 44*X($8)               # Initial FPU control register
        sd      $0,  64*X($8)               # Initial $f24
        sd      $0,  68*X($8)               # Initial $f25
        sd      $0,  72*X($8)               # Initial $f26
        sd      $0,  76*X($8)               # Initial $f27
        sd      $0,  80*X($8)               # Initial $f28
        sd      $0,  84*X($8)               # Initial $f29
        sd      $0,  88*X($8)               # Initial $f30
        sd      $0,  92*X($8)               # Initial $f31
#endif USE_FP_REGS

#
#
#    /* Save the new stack pointer into the task's control block.  */
#    task -> tc_stack_pointer =  (VOID *) ($sp);
#
        sw      $8, 44($4)                  # Save new stack pointer
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
#/*      TCT_Check_Stack                                                  */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function checks the current stack for overflow conditions.  */
#/*      Additionally, this function keeps track of the minimum amount    */
#/*      of stack space for the calling thread and returns the current    */
#/*      available stack space.                                           */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Send_Signals                    Send signals to a task       */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      ERC_System_Error                    System error handler         */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      available bytes in stack                                         */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#UNSIGNED  TCT_Check_Stack(void)
#{
        .align  4
        .globl  TCT_Check_Stack
TCT_Check_Stack:
#
#TC_TCB         *thread;
#UNSIGNED       remaining;
#
#    /* Pickup the current task/HISR pointer.  */
#    thread =  (TC_TCB *) TCD_Current_Thread;
#
        lw      $8, TCD_Current_Thread      # Pickup the current thread pointer
        or      $2, $0, $0                  # Build default return value
#
#    /* Determine if there is a current thread.  */
#    if (thread)
#    {
#
        beq     $8, $0, _TCT_No_Stack_Check # If no thread, skip stack check
        nop                                 # Delay slot
#
#        /* Determine if the stack pointers are out of range.  */
#        if ((thread -> tc_stack_pointer < thread -> tc_stack_start) ||
#            (thread -> tc_stack_pointer > thread -> tc_stack_end))
#
        lw      $9,  36($8)                 # Pickup the start of the stack
        lw      $10, 40($8)                 # Pickup the end of the stack
        bltu    $sp, $9, _TCT_Range_Error   # If current sp is less than the
        nop                                 #   start, sp is out of range
        lw      $11, 52($8)                 # Pickup stack minimum available in
                                            #   delay slot
        bleu    $sp, $10, _TCT_Range_Okay   # If current sp is less than the
                                            #   end, sp is withing range
        nop                                 # Delay slot
#
#            /* Stack overflow condition exits.  */
#            ERC_System_Error(NU_STACK_OVERFLOW);
#
_TCT_Range_Error:
#
        addiu   $sp, $sp, -16               # Allocate room to save ra on stack
        sw      $31, 0($sp)                 # Save return address

.if (X-1)
        or      $4, $0, $sp
.endif
        jal     ERC_System_Error            # Call system error handler
        li      $4, 3                       # Build NU_STACK_OVERFLOW value
                                            # Note: system error handler does
                                            #   not return!
_TCT_Range_Okay:
#
#        /* Calculate the amount of available space on the stack.  */
#        remaining =  (BYTE_PTR) thread -> tc_stack_pointer -
#                          (BYTE_PTR) thread -> tc_stack_start;
#
        subu    $2, $sp, $9                 # Calculate the amount of space 
                                            #   available on the thread's stack
#
#        /* Determine if there is enough memory on the stack to save all of the
#           registers.  */
#        if (remaining < 520)
#
        li      $12, TASK_STK               # Build amount of stack needed for
                                            #   a context save operation
        bgtu    $2, $12, _TCT_Enough_Stack  # If current available is more than
        nop                                 #   minimum, stack is still okay
#
#            /* Stack overflow condition is about to happen.  */
#            ERC_System_Error(NU_STACK_OVERFLOW);
#
        addiu   $sp, $sp, -16               # Allocate room to save ra on stack
        or      $4, $0, $sp
        sw      $31, 0($sp)                 # Save return address
        jal     ERC_System_Error            # Call system error handler
        li      $4, 3                       # Build NU_STACK_OVERFLOW value
                                            # Note: system error handler does
                                            #   not return!
_TCT_Enough_Stack:
#
#        /* Determine if this is a new minimum amount of stack space.  */
#        if (remaining < thread -> tc_stack_minimum)
#
        bgeu    $2, $11, _TCT_Not_Minimum   # See if a new minimum is present
        nop                                 # Delay slot
#
#            /* Save the new stack minimum.  */
#            thread -> tc_stack_minimum =  remaining;
#
        sw      $2, 52($8)                  # Store new minimum
#
_TCT_Not_Minimum:
#    }
#    else
#
#        /* Set the remaining bytes to 0.  */
#        remaining =  0;
#
#    /* Return the remaining number of bytes on the stack.  */
#    return(remaining);
#
_TCT_No_Stack_Check:
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
#/*      TCT_Schedule                                                     */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function waits for a thread to become ready.  Once a thread */
#/*      is ready, this function initiates a transfer of control to that  */
#/*      thread.                                                          */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      INC_Initialize                      Main initialization routine  */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      TCT_Control_To_Thread               Transfer control to a thread */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      TCD_Execute_Task                    Pointer to task to execute   */
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
#VOID  TCT_Schedule(void)
#{
        .align  4
        .globl  TCT_Schedule
TCT_Schedule:
#
#    /* Restore interrupts according to the value contained in 
#       TCD_Interrupt_Level.  */
#

        lw      $8, TCD_Interrupt_Level     # Pickup current interrupt level
        mfc0    $10, $12                    # Pickup current SR
        nop
        li      $9, INT_MASK                # Build interrupt bit mask
        and     $9, $9, $10                 # Clear the interrupt enable bits
        or      $9, $9, $8                  # Build a new SR with appropriate
                                            #   interrupts enabled
#ifdef INCLUDE_ODYSSEY
		lw		$10, TCD_R7KInterrupt_Level
		ori		$10, $10, CONFIG_ICR_VS
		ctc0	$10, $20
		nop
		nop
		nop
#endif
        mtc0    $9, $12                     # Setup new SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#
#    /* Wait until a thread (task or HISR) is available to execute.  */
#    do
#    {
#
        li      $10, INT_DISABLE            # Build interrupt disable value
TCT_Schedule_Loop:
        lw      $4, TCD_Execute_HISR        # Pickup the highest priority HISR
        lw      $8, TCD_Execute_Task        # Pickup the highest priority task
        bne     $4, $0, TCT_Found_Something # If not NULL, found a HISR to run
        nop                                 # Delay slot
        beq     $8, $0, TCT_Schedule_Loop   # If NULL, continue in looking at 
        ori     $4, $8, 0                   # Move task pointer into $a0 for 
                                            #   control to thread
#
#    } while ((!TCD_Execute_HISR) && (!TCD_Execute_Task));
#
TCT_Found_Something:
#
#    /* Yes, either a task or an HISR is ready to execute.  Lockout 
#       interrupts while the thread is transferred to.  */
#
        mtc0    $10, $12                    # Disable interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#ifdef INCLUDE_ODYSSEY
		li		$8, CONFIG_ICR_VS
		ctc0	$8, $20
		nop
		nop
		nop
#endif
#    
#    /* Transfer control to the thread by falling through to the following
#       routine.  */
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Control_To_Thread                                            */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function transfers control to the specified thread.  Each   */
#/*      time control is transferred to a thread, its scheduled counter   */
#/*      is incremented.  Additionally, time-slicing for task threads is  */
#/*      enabled in this routine.  The TCD_Current_Thread pointer is      */
#/*      setup by this function.                                          */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCT_Schedule                        Indirectly called            */
#/*      TCT_Protect                         Protection task switch       */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      thread                              Thread control block pointer */
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
#VOID  TCT_Control_To_Thread(TC_TCB *thread)
#{
        .align  4
        .globl  TCT_Control_To_Thread
TCT_Control_To_Thread:
#
#    /* Setup the current thread pointer.  At this pointer a0 contains the
#       pointer of the thread to transfer control to.  Interrupts are also
#       assumed to be locked out at this point.  */
#    TCD_Current_Thread =  (VOID *) thread;
#
        lw      $8, 28($4)                  # Pickup scheduled count
        sw      $4, TCD_Current_Thread      # Show the thread as active
#
#    /* Increment the thread scheduled counter.  */
#    thread -> tc_scheduled++;
#
        lw      $9, 32($4)                  # Pickup the time slice value
        addu    $8, $8, 1                   # Increment scheduled count
        lw      $sp, 44($4)                 # Switch to thread's stack pointer
                                            #   in the delay slot
#
#    /* Check for time slice option.  */
#    if (thread -> tc_cur_time_slice)
#    {
#
        sw      $8, 28($4)                  # Place back into the thread 
                                            #   control block
        beq     $9, $0, TCT_No_TS_Start     # If 0, no time slice is active
        nop                                 # Delay slot
#
#        /* Start a time slice.  */
#        TMD_Time_Slice =        thread -> tc_cur_time_slice;
#        TMD_Time_Slice_State =  0;
#
        sw      $9, TMD_Time_Slice          # Setup time slice value
        sw      $0, TMD_Time_Slice_State    # Setup time slice state to active
#    }
TCT_No_TS_Start:
#
#    /* Pickup the stack pointer and resume the thread.  */
#    REG_Stack_Ptr =  thread -> tc_stack_pointer;
#         
#     /* Pop off the saved information associated with the thread. After we
#        determine which type of stack is present.  A 1 on the top of the 
#        stack indicates an interrupt stack, while a 0 on the top of the
#        stack indicates a solicited type of stack.  */
#        
#     /* Remember that the interrupt level that is restored must represent
#        the interrupt level in TCD_Interrupt_Level.  */
#

        ld      $8,  0*X($sp)               # Pickup the type of stack
        ld      $16, 4*X($sp)               # Recover saved s0 in delay slot
        ld      $17, 8*X($sp)               # Recover saved s1 in delay slot
        beq     $8, $0, TCT_Solicited_Stack # If stack type is 0, a solicited
                                            #   stack is present.
        nop                                 # Delay slot
#
#     /* At this point, an interrupt stack type is present.  Most of the 
#        registers need to be recovered.  Note:  Initial task stacks are 
#        designed to "fake" an interrupt return to the entry of the task.  */
#
#     /* Recover floating point registers.  */
#

#if USE_FP_REGS
        ld      $8,   128*X($sp)            # Pickup FPU control register
        ldc1    $f0,  132*X($sp)            # Recover $f0-$f1
        ldc1    $f1,  136*X($sp)            #
        ldc1    $f2,  140*X($sp)            # Recover $f2-$f3
        ldc1    $f3,  144*X($sp)            #
        ldc1    $f4,  148*X($sp)            # Recover $f4-$f5
        ldc1    $f5,  152*X($sp)            #
        ldc1    $f6,  156*X($sp)            # Recover $f6-$f7
        ldc1    $f7,  160*X($sp)            #
        ctc1    $8,   $31                   # Recover FPU control register
        ldc1    $f8,  164*X($sp)            # Recover $f8-$f9
        ldc1    $f9,  168*X($sp)            #
        ldc1    $f10, 172*X($sp)            # Recover $f10-$f11
        ldc1    $f11, 176*X($sp)            #
        ldc1    $f12, 180*X($sp)            # Recover $f12-$f13
        ldc1    $f13, 184*X($sp)            # 
        ldc1    $f14, 188*X($sp)            # Recover $f14-$f15
        ldc1    $f15, 192*X($sp)            #
        ldc1    $f16, 196*X($sp)            # Recover $f16-$f17
        ldc1    $f17, 200*X($sp)            # 
        ldc1    $f18, 204*X($sp)            # Recover $f18-$f19
        ldc1    $f19, 208*X($sp)            #
        ldc1    $f20, 212*X($sp)            # Recover $f20-$f21
        ldc1    $f21, 216*X($sp)            #
        ldc1    $f22, 220*X($sp)            # Recover $f22-$f23
        ldc1    $f23, 224*X($sp)            #
        ldc1    $f24, 228*X($sp)            # Recover $f24-$f25
        ldc1    $f25, 232*X($sp)            # 
        ldc1    $f26, 236*X($sp)            # Recover $f26-$f27
        ldc1    $f27, 240*X($sp)            #
        ldc1    $f28, 244*X($sp)            # Recover $f28-$f29
        ldc1    $f29, 248*X($sp)            #
        ldc1    $f30, 252*X($sp)            # Recover $f30-$f31
        ldc1    $f31, 256*X($sp)            #
#endif USE_FP_REGS
        # MetroWerkaround:
        # There used to be an 'option no_at_macros on' directive here, in
        # addition to the one seven lines down, but CodeWarrior wants $at for
        # the next instruction.  It was not really required until the register
        # recovery anyway, so it has been deleted.

#
#     /* Insure that the thread returns with the global interrupt posture in 
#        force.  */
#
        lw      $8, TCD_Interrupt_Level     # Pickup current interrupt level
        ld      $9, 120*X($sp)              # Pickup saved SR
        li      $10, INT_MASK               # Build interrupt mask
        and     $9, $9, $10                 # Clear interrupt bits
        or      $9, $9, $8                  # Build SR new interrupt bits
        sd      $9, 120*X($sp)              # Place the new SR back into stack
#
        .option no_at_macros on
        ld      $at,   4*X($sp)             # Recover $at
        ld      $2,    8*X($sp)             # Recover $2 (v0-v1)
        ld      $3,   12*X($sp)             # Recover $3 
        ld      $4,   16*X($sp)             # Recover $4 (a0-a3)
        ld      $5,   20*X($sp)             # Recover $5
        ld      $6,   24*X($sp)             # Recover $6
        ld      $7,   28*X($sp)             # Recover $7
        ld      $8,   32*X($sp)             # Recover $8 (t0-t7)
        ld      $9,   36*X($sp)             # Recover $9
        ld      $10,  40*X($sp)             # Recover $10
        ld      $11,  44*X($sp)             # Recover $11
        ld      $12,  48*X($sp)             # Recover $12
        ld      $13,  52*X($sp)             # Recover $13
        ld      $14,  56*X($sp)             # Recover $14
        ld      $15,  60*X($sp)             # Recover $15
        ld      $16,  64*X($sp)             # Recover $16 (s0-s7)
        ld      $17,  68*X($sp)             # Recover $17
        ld      $18,  72*X($sp)             # Recover $18
        ld      $19,  76*X($sp)             # Recover $19
        ld      $20,  80*X($sp)             # Recover $20
        ld      $21,  84*X($sp)             # Recover $21
        ld      $22,  88*X($sp)             # Recover $22
        ld      $23,  92*X($sp)             # Recover $23
        ld      $24,  96*X($sp)             # Recover $24 (t8-t9)
        ld      $25, 100*X($sp)             # Recover $25
        ld      $30, 104*X($sp)             # Recover $30 ($fp)
        ld      $31, 108*X($sp)             # Recover $31 (ra)
        ld      $27, 112*X($sp)             # Recover HI
        nop                                 # Delay slot
        mthi    $27                         # Restore HI register
        ld      $27, 116*X($sp)             # Recover LO
        nop                                 # Delay slot
        mtlo    $27                         # Restore LO register
        ld      $27, 120*X($sp)             # Recover SR
        nop                                 # Delay slot
        mtc0    $27, $12                    # Restore SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#ifdef INCLUDE_ODYSSEY
# I have to avoid using $at here - Sudhir Kasargod
		la		$27, TCD_R7KInterrupt_Level
		lw		$27, 0($27)
		ori		$27, $27, CONFIG_ICR_VS
		ctc0	$27, $20
		nop
		nop
		nop
		nop
#endif
        ld      $27, 124*X($sp)             # Recover EPC
        addiu   $sp, $sp, TASK_STK          # Recover stack space
        mtc0    $27, $14                    # Restore EPC
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot

        nop                                 # Delay slot
        eret                                # Return to point of exception
        nop                                 # Delay slot
        nop                                 # Delay slot
        .option no_at_macros off

#
#    /* Solicited stack is present.  This type of stack only contains the 
#       registers that need to be preserved accross function boundaries by 
#       the compiler.  Note:  s0 and s1 have already been restored.  */
#
TCT_Solicited_Stack:
#
#    /* Recover floating point preserved registers.  */
#

#if USE_FP_REGS
        ld      $8,   44*X($sp)             # Pickup FPU control register
        ldc1    $f20, 48*X($sp)             # Recover $f20-$f21
        ldc1    $f21, 52*X($sp)             #
        ldc1    $f22, 56*X($sp)             # Recover $f22-$f23
        ldc1    $f23, 60*X($sp)             #
        ldc1    $f24, 64*X($sp)             # Recover $f24-$f25
        ldc1    $f25, 68*X($sp)             #
        ctc1    $8,   $31                   # Recover FPU control register
        ldc1    $f26, 72*X($sp)             # Recover $f26-$f27
        ldc1    $f27, 76*X($sp)             #
        ldc1    $f28, 80*X($sp)             # Recover $f28-$f29
        ldc1    $f29, 84*X($sp)             #
        ldc1    $f30, 88*X($sp)             # Recover $f30-$f31
        ldc1    $f31, 92*X($sp)             #
#endif USE_FP_REGS

        ld      $18, 12*X($sp)              # Recover $18 (s2-s7)
        ld      $19, 16*X($sp)              # Recover $19
        ld      $20, 20*X($sp)              # Recover $20
        ld      $21, 24*X($sp)              # Recover $21
        ld      $22, 28*X($sp)              # Recover $22
        ld      $23, 32*X($sp)              # Recover $23
        ld      $30, 36*X($sp)              # Recover $30 (fp)
        ld      $31, 40*X($sp)              # Recover $31 (ra)
        addiu   $sp, $sp, HISR_STK          # Recover stack space
#
#    /* Insure that the thread returns with the global interrupt posture in 
#       force.  */
#
        mfc0    $9, $12                     # Pickup current SR
        lw      $8, TCD_Interrupt_Level     # Pickup the interrupt level
        li      $10, INT_MASK               # Build interrupt mask
        and     $9, $9, $10                 # Clear interrupt bits
        or      $9, $9, $8                  # Build new SR
#ifdef INCLUDE_ODYSSEY
		lw      $8, TCD_R7KInterrupt_Level
		ori		$8, $8, CONFIG_ICR_VS
		ctc0	$8, $20
		nop
		nop
		nop
#endif
        mtc0    $9, $12                     # Setup new SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        j       $31                         # Return to caller
        nop                                 # Delay slot
#
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Control_To_System                                            */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function returns control from a thread to the system.  Note */
#/*      that this service is called in a solicited manner, i.e. it is    */
#/*      not called from an interrupt thread.  Registers required by the  */
#/*      compiler to be preserved across function boundaries are saved by */
#/*      this routine.  Note that this is usually a sub-set of the total  */
#/*      number of available registers.                                   */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Other Components                                                 */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      TCT_Schedule                        Schedule the next thread     */
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
#VOID  TCT_Control_To_System(void)
#{
        .align  4
        .globl  TCT_Control_To_System
TCT_Control_To_System:
#
#    /* Lockout interrupts.  */
#    
#ifdef INCLUDE_ODYSSEY
		li		$8, CONFIG_ICR_VS
		ctc0	$8, $20
		nop
		nop
		nop
#endif
        li      $8, INT_DISABLE             # Build disable interrupt value
        mtc0    $8, $12                     # Disable interrupts in SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#    /* Save a minimal context of the thread.  */
#
        addiu   $sp, $sp, -HISR_STK         # Allocate stack space for a 
                                            #   solicited stack
        sd      $0,   0*X($sp)              # Place stack type on top of stack
        sd      $16,  4*X($sp)              # Save $16 (s0-s7)
        sd      $17,  8*X($sp)              # Save $17
        sd      $18, 12*X($sp)              # Save $18
        sd      $19, 16*X($sp)              # Save $19
        sd      $20, 20*X($sp)              # Save $20
        sd      $21, 24*X($sp)              # Save $21
        sd      $22, 28*X($sp)              # Save $22
        sd      $23, 32*X($sp)              # Save $23
        sd      $30, 36*X($sp)              # Save $30 (fp)
        sd      $31, 40*X($sp)              # Save $31 (ra)
#
#    /* Save preserved floating point registers.  */
#

#if USE_FP_REGS
        cfc1    $8, $31                     # Pickup FPU control register
        sdc1    $f20, 48*X($sp)             # Save $f20-$f21
        sdc1    $f21, 52*X($sp)             #
        sdc1    $f22, 56*X($sp)             # Save $f22-$f23
        sdc1    $f23, 60*X($sp)             #
        sdc1    $f24, 64*X($sp)             # Save $f24-$f25
        sdc1    $f25, 68*X($sp)             #
        sd      $8,   44*X($sp)             # Save FPU control register
        sdc1    $f26, 72*X($sp)             # Save $f26-$f27
        sdc1    $f27, 76*X($sp)             #
        sdc1    $f28, 80*X($sp)             # Save $f28-$f29
        sdc1    $f29, 84*X($sp)             #
        sdc1    $f30, 88*X($sp)             # Save $f30-$f31
        sdc1    $f31, 92*X($sp)             #
#endif USE_FP_REGS
#
#
#    /* Setup a pointer to the thread control block.  */
#    REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#
        lw      $10, TCD_Current_Thread     # Pickup the current thread pointer
#
#    /* Clear the current thread control block pointer.  */
#    TCD_Current_Thread =  NU_NULL;
#
        lw      $8, TMD_Time_Slice_State    # Pickup the time slice state
        sw      $0, TCD_Current_Thread      # Set current thread pointer to 
                                            #   NULL
#        
#    /* Check to see if a time slice is active.  If so, copy the original time
#       slice into the current time slice field of the task's control block. */
#    if (TMD_Time_Slice_State == 0)
#    {
        lw      $9, 64($10)                 # Pickup the full value again 
        bne     $8, $0, TCT_No_TS_1         # If time slice not active skip the
                                            #   disable time slice code
        nop                                 # Delay slot
#
#        /* Insure that the next time the task runs it gets a fresh time 
#           slice.  */
#       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;
#
#        /* Clear any active time slice by setting the state to NOT_ACTIVE.  */
#        TMD_Time_Slice_State =  1;
#
        li      $8, 1                       # Build NOT_ACTIVE value
        sw      $9, 32($10)                 # Initialize next time slice
        sw      $8, TMD_Time_Slice_State    # Disable time slice
#
#    }
TCT_No_TS_1:
#
#    /* Save off the current stack pointer in the control block.  */
#    REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;
#
        sw      $sp, 44($10)                # Save the stack pointer
#
#    /* Clear the task's current protection.  */
#    (REG_Thread_Ptr -> tc_current_protect) -> tc_tcb_pointer =  NU_NULL;
#    REG_Thread_Ptr -> tc_current_protect =  NU_NULL;
#
        lw      $8, 56($10)                 # Pickup the current protection ptr
        sw      $0, 56($10)                 # Set the protect pointer to NULL
        sw      $0, 0($8)                   # Release protection
#
#    /* Switch to the system stack.  */
#    REG_Stack_Ptr =  TCD_System_Stack;
#
#    /* Finished, return to the scheduling loop.  */
#
        lw      $sp, TCD_System_Stack       # Switch to the system stack
        j       TCT_Schedule                # Transfer control to scheduler
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Signal_Exit                                                  */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function exits from a signal handler.  The primary purpose  */
#/*      of this function is to clear the scheduler protection and switch */
#/*      the stack pointer back to the normal task's stack pointer.       */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Signal_Shell                    Signal handling shell func   */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      TCT_Schedule                        Scheduler                    */
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
#VOID  TCT_Signal_Exit(void)
#{
        .align  4
        .globl  TCT_Signal_Exit
TCT_Signal_Exit:
#
#    /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$10, CONFIG_ICR_VS
		ctc0	$10, $20
		nop
		nop
		nop
#endif
        li      $10, INT_DISABLE            # Build disable interrupts value
        mtc0    $10, $12                    # Disable interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#    /* Setup a pointer to the thread control block.  */
#    REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#
        lw      $10, TCD_Current_Thread     # Pickup current thread pointer
        lw      $8, TMD_Time_Slice_State    # Pickup time-slice state
#
#    /* Clear the current thread control block.  */
#    TCD_Current_Thread =  NU_NULL;
#
        sw      $0, TCD_Current_Thread      # Set current thread to NULL
#
#    /* Check to see if a time slice is active.  If so, copy the original time
#       slice into the current time slice field of the task's control block. */
#    if (TMD_Time_Slice_State == 0)
#    {
        lw      $9, 64($10)                 # Pickup the full value again 
        bne     $8, $0, TCT_No_TS_1a        # If time slice not active skip the
                                            #   disable time slice code
        nop                                 # Delay slot
#
#        /* Insure that the next time the task runs it gets a fresh time 
#           slice.  */
#       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;
#
#        /* Clear any active time slice by setting the state to NOT_ACTIVE.  */
#        TMD_Time_Slice_State =  1;
#
        li      $8, 1                       # Build NOT_ACTIVE value
        sw      $9, 32($10)                 # Initialize next time slice
        sw      $8, TMD_Time_Slice_State    # Disable time slice
#
#    }
TCT_No_TS_1a:
#
#    /* Switch back to the saved stack.  The saved stack pointer was saved
#       before the signal frame was built.  */
#    REG_Thread_Ptr -> tc_stack_pointer =  
#                                REG_Thread_Ptr -> tc_saved_stack_ptr;
#
        lw      $9, 60($10)                 # Pickup saved stack pointer
#
#    /* Switch to the system stack.  */
#    REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack;
#
#    /* Clear the task's current protection.  */
#    (REG_Thread_Ptr -> tc_current_protect) -> tc_tcb_pointer =  NU_NULL;
#    REG_Thread_Ptr -> tc_current_protect =  NU_NULL;
#
        lw      $8, 56($10)                 # Pickup the current protection ptr
        sw      $0, 56($10)                 # Set the protect pointer to NULL
        sw      $0, 0($8)                   # Release protection
#
#    /* Finished, return to the scheduling loop.  */
#
        lw      $sp, TCD_System_Stack       # Switch to system stack pointer
        sw      $9, 44($10)                 # Restore pre-signal stack pointer
        j       TCT_Schedule                # Return to scheduling loop
        nop                                 # Delay slot
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Current_Thread                                               */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function returns the current thread pointer.                */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*      System Components                                                */
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
#/*      Pointer to current thread                                        */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  *TCT_Current_Thread(void)
#{
        .align  4
        .globl  TCT_Current_Thread
TCT_Current_Thread:
#
#    /* Return the current thread pointer.  */
#    return(TCD_Current_Thread);
#
        lw      $2, TCD_Current_Thread      # Return the current thread pointer
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
#/*      TCT_Set_Execute_Task                                             */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function sets the current task to execute variable under    */
#/*      protection against interrupts.                                   */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC Scheduling Routines                                          */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      task                                Pointer to task control block*/
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      TCD_Execute_Task                    Modified variable            */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#VOID  TCT_Set_Execute_Task(TC_TCB *task)
#{
        .align  4
        .globl  TCT_Set_Execute_Task
TCT_Set_Execute_Task:
#
#    /* Now setup the TCD_Execute_Task pointer.  */
#    TCD_Execute_Task =  task;
#
        sw      $4, TCD_Execute_Task        # Setup the next task to execute
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
#/*      TCT_Protect                                                      */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function protects against multiple thread access.           */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*      System Components                                                */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      protect                            Pointer to protection block   */
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
#VOID  TCT_Protect(TC_PROTECT *protect)
#{
        .align  4
        .globl  TCT_Protect
TCT_Protect:
#
#    /* Determine if the caller is in a task or HISR thread.  */
#    if (TCD_Current_Thread)
#    {
#
        lw      $8, TCD_Current_Thread      # Pickup the current thread pointer
        beq     $8, $0, _TCT_Skip_Protect   # In initialization, skip protect
        nop                                 # Delay slot
#    
#        /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
		ctc0	$9, $20
		nop
		nop
		nop
#endif
        li      $9, INT_DISABLE            # Build interrupt disable SR
        mtc0    $9, $12                     # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#    
#        /* Wait until the protect structure is available.  */
#        while (protect -> tc_tcb_pointer != NU_NULL)
#        {
#
_TCT_Protect_Loop:
        lw      $9, 0($4)                   # Pickup owner of protection
        li      $10, 1                      # Build waiting value
        beq     $9, $0, _TCT_Protect_Avail  # If NULL, protection is available
        nop                                 # Delay slot
#        
#            /* Protection structure is not available.  */
#            
#            /* Indicate that another thread is waiting.  */
#            protect -> tc_thread_waiting =  1;
#
        sw      $10, 4($4)                  # Indicate another thread is 
                                            #   waiting
#            
#            /* Directly schedule the thread waiting.  */
#            TCT_Schedule_Protected(protect -> tc_tcb_pointer);
#
        addiu   $sp, $sp, -8*X              # Allocate room on stack
        sd      $31, 0($sp)                 # Save ra on stack
        sd      $4,  4*X($sp)               # Save protect pointer on stack
        jal     TCT_Schedule_Protected      # Schedule the thread owning the
                                            #   protection.  Note: $8 (t0) 
                                            #   points to current thread and
                                            #   $9 (t1) points to thread that
                                            #   has the protection.
        nop                                 # Delay slot
        ld      $31, 0($sp)                 # Recover ra from stack
        ld      $4,  4*X($sp)               # Recover protect ptr from stack
        addiu   $sp, $sp, 8*X               # Recover stack space

#            
#            /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
		ctc0	$9, $20
		nop
		nop
		nop
#endif
        li      $9, INT_DISABLE             # Build interrupt disable value
        mtc0    $9, $12                     # Disable interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
        j       _TCT_Protect_Loop           # Continue waiting for protection
        nop                                 # Delay slot
#        }
#
_TCT_Protect_Avail:
#        
#        /* Protection structure is available.  */
#        
#        /* Indicate that this thread owns the protection.  */
#        protect -> tc_tcb_pointer =  TCD_Current_Thread;
#
        sw      $8, 0($4)                   # Indicate that this thread owns 
                                            #   the protection
#        
#        /* Clear the thread waiting flag.  */
#        protect -> tc_thread_waiting =  0;
#
        sw      $0, 4($4)                   # Clear the thread waiting flag
#        
#        /* Save the protection pointer in the thread's control block.  Note 
#           that both task and HISR threads share the same control block 
#           format.  */
#        REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#        REG_Thread_Ptr -> tc_current_protect =  protect;
#
        sw      $4, 56($8)                  # Save protection pointer in the
                                            #   thread control block

#
#        /* Restore interrupts.  */
#
        mfc0    $10, $12                    # Pickup SR
        lw      $8, TCD_Interrupt_Level     # Pickup current interrupt level
        li      $9, INT_MASK                # Build interrupt mask
        and     $10, $10, $9                # Isolate the interrupt bits
        or      $10, $10, $8                # Build new SR
        mtc0    $10, $12                    # Setup actual SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#ifdef INCLUDE_ODYSSEY
		lw		$10, TCD_R7KInterrupt_Level
		nop
		ori		$10, $10, CONFIG_ICR_VS
		ctc0	$10, $20
		nop
		nop
		nop
#endif
#    }
_TCT_Skip_Protect:
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
#/*      TCT_Unprotect                                                    */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function releases protection of the currently active        */
#/*      thread.  If the caller is not an active thread, then this        */
#/*      request is ignored.                                              */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*      System Components                                                */
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
#VOID  TCT_Unprotect(void)
#{
        .align  4
        .globl  TCT_Unprotect
TCT_Unprotect:
#
#
#    /* Determine if the caller is in a task or HISR thread.  */
#    if (TCD_Current_Thread)
#    {
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
        nop                                 # Delay slot
        beq     $8, $0, _TCT_Skip_Unprotect # If NULL, skip unprotect action
        nop                                 # Delay slot
#
#        /* Setup a task control block pointer.  */
#        REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#        
#        /* Determine if there is a currently active protection.  */
#        if (REG_Thread_Ptr -> tc_current_protect)
#        {
        lw      $9, 56($8)                  # Pickup the current protection
        li      $10, INT_DISABLE            # Build interrupt disable value
        beq     $9, $0, _TCT_Skip_Unprotect # Protection is no longer in force
        nop                                 # Delay slot
#
#        
#            /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$11, CONFIG_ICR_VS
		ctc0	$11, $20
		nop
		nop
		nop
#endif
        mtc0    $10, $12                    # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay
#
#            /* Yes, this thread still has this protection structure.  */
#            REG_Protect_Ptr =  REG_Thread_Ptr -> tc_current_protect;
#
#            /* Is there a higher priority thread waiting for the protection
#               structure?  */
#            if (REG_Protect_Ptr -> tc_thread_waiting)
#
        lw      $11, 4($9)                  # Pickup the waiting flag
        li      $13, INT_MASK               # Build interrupt mask
        lw      $12, TCD_Interrupt_Level    # Pickup the current interrupt bits
        beq     $11, $0, _TCT_Not_Waiting   # If 0, no other thread is waiting
        nop                                 # Delay slot
#            
#                /* Transfer control to the system.  Note that this 
#                   automatically clears the current protection.  */
#                TCT_Control_To_System();
#
        j       TCT_Control_To_System       # Branch to transfer control back
        nop                                 #   to thread waiting.  Note: 
                                            #   control returns to the caller 
                                            #   of this routine!!!
#            else
#            {
#
_TCT_Not_Waiting:
#
#                /* Clear the protection.  */
#                REG_Thread_Ptr -> tc_current_protect =  NU_NULL;
#                REG_Protect_Ptr -> tc_tcb_pointer =  NU_NULL;
#
        sw      $0, 0($9)                   # Clear protection
        sw      $0, 56($8)                  # Clear current protect in control
                                            #   block
#                
#                /* Restore interrupts again.  */
#
        mfc0    $10, $12                    # Pickup SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        and     $10, $10, $13               # Isolate the interrupt bits
        or      $10, $10, $12               # Build new SR
#ifdef INCLUDE_ODYSSEY
        lw      $12, TCD_R7KInterrupt_Level # Pickup the current interrupt bits
		ori		$12, $12, CONFIG_ICR_VS
		ctc0	$12, $20
		nop
		nop
		nop
#endif
        mtc0    $10, $12                    # Setup actual SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#            }
#        }
#    }
_TCT_Skip_Unprotect:
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
#/*      TCT_Unprotect_Specific                                           */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function releases a specific protection structure.          */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*      System Components                                                */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      protect                            Pointer to protection block   */
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
#VOID  TCT_Unprotect_Specific(TC_PROTECT *protect)
#{
        .align  4
        .globl  TCT_Unprotect_Specific
TCT_Unprotect_Specific:
#
#    /* Determine if the caller is in a task or HISR thread.  */
#    if (TCD_Current_Thread)
#    {
        lw      $8, TCD_Current_Thread      # Pickup the current thread pointer
        li      $9, INT_DISABLE             # Build interrupt disable SR
        beq     $8, $0, _TCT_Skip_Unp_Spec  # If NULL, skip unprotect specific
        nop                                 # Delay slot
#     
#        /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$10, CONFIG_ICR_VS
		ctc0	$10, $20
		nop
		nop
		nop
#endif
        mtc0    $9, $12                     # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#        /* Clear the protection pointer.  */
#        protect -> tc_tcb_pointer =  NU_NULL;
#
        lw      $10, 4($4)                  # Pickup the waiting flag
        sw      $0, 0($4)                   # Clear protection 
#        
#        /* Determine if a thread is waiting.  */
#        if (protect -> tc_thread_waiting)
#        {
#
        lw      $12, TCD_Interrupt_Level    # Delay slot
        beq     $10, $0, _TCT_Not_Spec_Wait # If 0, no other thread is waiting
        nop                                 # Delay slot
#
#    /* Save a minimal context of the thread.  */
#
        addiu   $sp, $sp, -HISR_STK         # Allocate stack space for a 
                                            #   solicited stack
        sd      $0,   0*X($sp)              # Place stack type on top of stack
        sd      $16,  4*X($sp)              # Save $16 (s0-s7)
        sd      $17,  8*X($sp)              # Save $17
        sd      $18, 12*X($sp)              # Save $18
        sd      $19, 16*X($sp)              # Save $19
        sd      $20, 20*X($sp)              # Save $20
        sd      $21, 24*X($sp)              # Save $21
        sd      $22, 28*X($sp)              # Save $22
        sd      $23, 32*X($sp)              # Save $23
        sd      $30, 36*X($sp)              # Save $30 (fp)
        sd      $31, 40*X($sp)              # Save $31 (ra)
#
#    /* Save preserved floating point registers.  */
#

#if USE_FP_REGS
        cfc1    $9, $31                     # Pickup FPU control register
        sdc1    $f20, 48*X($sp)             # Save $f20-$f21
        sdc1    $f21, 52*X($sp)             #
        sdc1    $f22, 56*X($sp)             # Save $f22-$f23
        sdc1    $f23, 60*X($sp)             #
        sdc1    $f24, 64*X($sp)             # Save $f24-$f25
        sdc1    $f25, 68*X($sp)             #
        sd      $9,   44*X($sp)             # Save FPU control register
        sdc1    $f26, 72*X($sp)             # Save $f26-$f27
        sdc1    $f27, 76*X($sp)             #
        sdc1    $f28, 80*X($sp)             # Save $f28-$f29
        sdc1    $f29, 84*X($sp)             #
        sdc1    $f30, 88*X($sp)             # Save $f30-$f31
        sdc1    $f31, 92*X($sp)             #
#endif USE_FP_REGS


#    /* Clear the current thread control block pointer.  */
#    TCD_Current_Thread =  NU_NULL;
#
        lw      $10, TMD_Time_Slice_State   # Pickup the time slice state
        sw      $0, TCD_Current_Thread      # Set current thread pointer to 
                                            #   NULL
#        
#    /* Check to see if a time slice is active.  If so, copy the original time
#       slice into the current time slice field of the task's control block. */
#    if (TMD_Time_Slice_State == 0)
#    {
        lw      $9, 64($8)                  # Pickup the full value again 
        bne     $10, $0, TCT_No_TS_2        # If time slice not active skip the
                                            #   disable time slice code
        nop                                 # Delay slot
#
#        /* Insure that the next time the task runs it gets a fresh time 
#           slice.  */
#       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;
#
#        /* Clear any active time slice by setting the state to NOT_ACTIVE.  */
#        TMD_Time_Slice_State =  1;
#
        li      $10, 1                      # Build NOT_ACTIVE value
        sw      $9, 32($8)                  # Initialize next time slice
        sw      $10, TMD_Time_Slice_State   # Disable time slice
#
#    }
TCT_No_TS_2:
#
#    /* Save off the current stack pointer in the control block.  */
#    REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;
#
        sw      $sp, 44($8)                 # Save the stack pointer
#
#    /* Switch to the system stack.  */
#    REG_Stack_Ptr =  TCD_System_Stack;
#
#    /* Finished, return to the scheduling loop.  Note: control returns to the
#       caller of this routine.  */
#
        lw      $sp, TCD_System_Stack       # Switch to the system stack
        j       TCT_Schedule                # Transfer control to scheduler
        nop                                 # Delay slot
#        
#        }
#        else
#        {
_TCT_Not_Spec_Wait:
#        
#            /* No higher-priority thread is waiting.  */
#            
#            /* Restore interrupts.  */
#
        mfc0    $10, $12                    # Pickup SR
        li      $13, INT_MASK               # Build interrupt mask
        and     $10, $10, $13               # Isolate the interrupt bits
        or      $10, $10, $12               # Build new SR
#ifdef INCLUDE_ODYSSEY
		lw		$12, TCD_R7KInterrupt_Level 	
		ori		$12, $12, CONFIG_ICR_VS
		ctc0	$12, $20
		nop
		nop
		nop
#endif
        mtc0    $10, $12                    # Setup actual SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#        }
#    }
_TCT_Skip_Unp_Spec:
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
#/*      TCT_Set_Current_Protect                                          */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function sets the current protection field of the current   */
#/*      thread's control block to the specified protection pointer.      */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Resume_Task                    Resume task function          */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      protect                            Pointer to protection block   */
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
#VOID  TCT_Set_Current_Protect(TC_PROTECT *protect)
#{
        .align  4
        .globl  TCT_Set_Current_Protect
TCT_Set_Current_Protect:
#
#    /* Determine if the caller is in a task or HISR thread.  */
#    if (TCD_Current_Thread)
#    {
#
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
        nop                                 # Delay slot
        beq     $8, $0, _TCT_Skip_Set_Cur   # If NULL, skip setting current
        nop                                 # Delay slot
#
#        /* Point at the current thread control block.  */
#        REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#    
#        /* Modify the current protection.  */
#        REG_Thread_Ptr -> tc_current_protect =  protect;
#
        sw      $4, 56($8)                  # Save pointer in control block
#    }
_TCT_Skip_Set_Cur:
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
#/*      TCT_Protect_Switch                                               */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function waits until a specific task no longer has any      */
#/*      protection associated with it.  This is necessary since task's   */
#/*      cannot be suspended or terminated unless they have released all  */
#/*      of their protection.                                             */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      System Components                                                */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      thread                             Pointer to thread control blk */
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
#VOID  TCT_Protect_Switch(VOID *thread)
#{
        .align  4
        .globl  TCT_Protect_Switch
TCT_Protect_Switch:
#
#    /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$10, CONFIG_ICR_VS
		ctc0    $10, $20
		nop
		nop
		nop
#endif
        li      $10, INT_DISABLE            # Build interrupt disable value
        mtc0    $10, $12                    # Disable interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#    
#    REG_Thread_Ptr =  (TC_TCB *) thread;
#    
#    /* Wait until the specified task has no protection associated with it.  */
#    while (REG_Thread_Ptr -> tc_current_protect)
#    {
#
_TCT_Switch_Loop:
        li      $11, 1                      # Build waiting value
        lw      $10, 56($4)                 # Pickup the protect field of the
                                            #   specified thread
        lw      $12, TCD_Interrupt_Level    # Pickup the interrupt level
        lw      $8, TCD_Current_Thread      # Pickup the current thread pointer
        beq     $10, $0, _TCT_Switch_Done   # If NULL, the specified thread 
                                            #   longer has protection
        nop                                 # Delay slot
#    
#        /* Let the task run again in an attempt to clear its protection.  */
#        
#        /* Indicate that a higher priority thread is waiting.  */
#        (REG_Thread_Ptr -> tc_current_protect) -> tc_thread_waiting =  1;
#        
        sw      $11, 4($10)                 # Set the waiting bit
#
#        /* Directly schedule the thread waiting.  */
#        TCT_Schedule_Protected((REG_Thread_Ptr -> tc_current_protect)  
#                                                        -> tc_tcb_pointer);
        addiu   $sp, $sp, -8*X              # Allocate room on the stack
        sd      $31, 0($sp)                 # Save ra on the stack
        sd      $4,  4*X($sp)               # Save protect pointer on stack
        jal     TCT_Schedule_Protected      # Schedule the protected thread
        or      $9, $4, $0                  # Move the thread to schedule into
                                            #  $9
        ld      $31, 0*X($sp)               # Recover saved ra
        ld      $4,  4*X($sp)               # Recover protect pointer
        addiu   $sp, $sp, 8*X               # Recover stack space
#            
#        /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		li		$10, CONFIG_ICR_VS
		ctc0    $10, $20
		nop
		nop
		nop
#endif
        li      $10, INT_DISABLE            # Build interrupt disable value
        mtc0    $10, $12                    # Lockout interrupts in SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        j       _TCT_Switch_Loop            # Continue at top of loop
        nop                                 # Delay slot
#}
_TCT_Switch_Done:
#
#    /* Restore interrupts.  */
#
        mfc0    $10, $12                    # Pickup SR
        li      $13, INT_MASK               # Build interrupt mask
        and     $10, $10, $13               # Isolate the interrupt bits
        or      $10, $10, $12               # Build new SR
#ifdef INCLUDE_ODYSSEY
        lw      $12, TCD_R7KInterrupt_Level
		ori		$12, $12, CONFIG_ICR_VS
        ctc0    $12, $20
        nop
        nop
        nop
#endif
        mtc0    $10, $12                    # Setup actual SR
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
#/*      TCT_Schedule_Protected                                           */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function saves the minimal context of the thread and then   */
#/*      directly schedules the thread that has protection over the       */
#/*      the thread that called this routine.                             */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCT_Protect                                                      */
#/*      TCT_Protect_Switch                                               */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      TCT_Control_To_Thread               Transfer control to protected*/
#/*                                              thread                   */
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
#VOID  TCT_Schedule_Protected(VOID *thread)
#{
		.align  4
        .globl  TCT_Schedule_Protected
TCT_Schedule_Protected:
#
#    
#    /* Save minimal context required by the system.  Note:  $8 (t0) contains
#       the current thread and $9 (t1) contains the thread to schedule 
#       pointer.  Interrupts are already locked out.  */
#
#    /* Setup a pointer to the thread control block.  */
#    REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#
#    /* Save a minimal context of the thread.  */
#
        addiu   $sp, $sp, -HISR_STK         # Allocate stack space for a 
                                            #   solicited stack
        sd      $0,   0*X($sp)              # Place stack type on top of stack
        sd      $16,  4*X($sp)              # Save $16 (s0-s7)
        sd      $17,  8*X($sp)              # Save $17
        sd      $18, 12*X($sp)              # Save $18
        sd      $19, 16*X($sp)              # Save $19
        sd      $20, 20*X($sp)              # Save $20
        sd      $21, 24*X($sp)              # Save $21
        sd      $22, 28*X($sp)              # Save $22
        sd      $23, 32*X($sp)              # Save $23
        sd      $30, 36*X($sp)              # Save $30 (fp)
        sd      $31, 40*X($sp)              # Save $31 (ra)
#
#    /* Save preserved floating point registers.  */
#

#if USE_FP_REGS
        cfc1    $11, $31                    # Pickup FPU control register
        sdc1    $f20, 48*X($sp)             # Save $f20-$f21
        sdc1    $f21, 52*X($sp)             #
        sdc1    $f22, 56*X($sp)             # Save $f22-$f23
        sdc1    $f23, 60*X($sp)             # 
        sdc1    $f24, 64*X($sp)             # Save $f24-$f25
        sdc1    $f25, 68*X($sp)             # 
        sd      $11,  44*X($sp)             # Save FPU control register
        sdc1    $f26, 72*X($sp)             # Save $f26-$f27
        sdc1    $f27, 76*X($sp)             #
        sdc1    $f28, 80*X($sp)             # Save $f28-$f29
        sdc1    $f29, 84*X($sp)             #
        sdc1    $f30, 88*X($sp)             # Save $f30-$f31
        sdc1    $f31, 92*X($sp)             #
#endif USE_FP_REGS


#    /* Clear the current thread control block pointer.  */
#    TCD_Current_Thread =  NU_NULL;
#
        lw      $10, TMD_Time_Slice_State   # Pickup the time slice state
        sw      $0, TCD_Current_Thread      # Set current thread pointer to 
                                            #   NULL
#        
#    /* Check to see if a time slice is active.  If so, copy the original time
#       slice into the current time slice field of the task's control block. */
#    if (TMD_Time_Slice_State == 0)
#    {
        lw      $11, 64($8)                 # Pickup the full value again 
        bne     $10, $0, TCT_No_TS_3        # If time slice not active skip the
                                            #   disable time slice code
        nop                                 # Delay slot
#
#        /* Insure that the next time the task runs it gets a fresh time 
#           slice.  */
#       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;
#
#        /* Clear any active time slice by setting the state to NOT_ACTIVE.  */
#        TMD_Time_Slice_State =  1;
#
        li      $10, 1                      # Build NOT_ACTIVE value
        sw      $11, 32($8)                 # Initialize next time slice
        sw      $10, TMD_Time_Slice_State   # Disable time slice
#
#    }
TCT_No_TS_3:
#
#    /* Save off the current stack pointer in the control block.  */
#    REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;
#
        sw      $sp, 44($8)                 # Save the stack pointer

#
#    /* Switch to the system stack.  */
#    REG_Stack_Ptr =  TCD_System_Stack;
#
        lw      $sp, TCD_System_Stack       # Switch to the system stack
#
#    /* Jump to schedule the specified thread.  The scheduling loop is 
#       bypassed since this thread is really lower priority than the caller
#       of this routine.  */
#    TCT_Control_To_Thread(thread);
#
        j       TCT_Control_To_Thread   # Transfer control to other thread
        or      $4, $9, $0              # Move thread to execute pointer 
                                        #   into $4 (a0)
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Interrupt_Context_Save                                       */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function saves the interrupted thread's context.  Nested    */
#/*      interrupts are also supported.  If a task or HISR thread was     */
#/*      interrupted, the stack pointer is switched to the system stack   */
#/*      after the context is saved.                                      */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application ISRs                    Assembly language ISRs       */
#/*      INT_Interrupt_Shell                 Interrupt handler shell      */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      vector                              Interrupt's vector number    */
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
#VOID  TCT_Interrupt_Context_Save(INT vector)
#{
        .align  4
        .globl  TCT_Interrupt_Context_Save
TCT_Interrupt_Context_Save:
#
#    /* Upon entry to this routine, the current stack has the following 
#       frame on it:  (Current values for $16 and $31 must be left alone!) 
#
#               sp + 0  -> $at
#               sp + 4  -> $31
#               sp + 8  -> $16
#               sp + 12 -> $17
#               sp + 16 -> Original stack value   */
#   
#    /* Determine if this is a nested interrupt.  */
#    if (TCD_Interrupt_Count++)
#    {
#
        lw      $17, TCD_Interrupt_Count    # Pickup current interrupt count
        nop                                 # Delay slot
        beq     $17, $0, _TCT_Not_Nest_Sav  # If 0, not a nested interrupt save
        addiu   $17, $17, 1                 # Increment the nested counter
        sw      $17, TCD_Interrupt_Count    # Store the new interrupt count
#    
#        /* Save complete context on the current stack.  Build a nested stack
#           frame.  */
#
        addiu   $sp, $sp, -(TASK_STK-32)    # Allocate room on current stack
                                            #   (don't bother putting stack
                                            #    type on a nested stack frame)
                                            #   132 bytes for FPU registers
        ld      $17, (TASK_STK-32)($sp)     # Recover original saved $at
        nop                                 # Delay slot
        sd      $17,   4*X($sp)             # Save at register
        sd      $2,    8*X($sp)             # Save $2 (v0-v1)
        sd      $3,   12*X($sp)             # Save $3
        sd      $4,   16*X($sp)             # Save $4 (a0-a3)
        sd      $5,   20*X($sp)             # Save $5
        sd      $6,   24*X($sp)             # Save $6
        sd      $7,   28*X($sp)             # Save $7
        sd      $8,   32*X($sp)             # Save $8 (t0-t7)
        sd      $9,   36*X($sp)             # Save $9
        sd      $10,  40*X($sp)             # Save $10
        sd      $11,  44*X($sp)             # Save $11
        sd      $12,  48*X($sp)             # Save $12
        sd      $13,  52*X($sp)             # Save $13
        sd      $14,  56*X($sp)             # Save $14
        sd      $15,  60*X($sp)             # Save $15
        ld      $8,  (TASK_STK-16)($sp)     # Recover original saved $16 (s0)
        ld      $9,  (TASK_STK-8)($sp)      # Recover original saved $17 (s1)
        sd      $8,   64*X($sp)             # Save $16 (s0-s7)
        sd      $9,   68*X($sp)             # Save $17
        sd      $18,  72*X($sp)             # Save $18
        sd      $19,  76*X($sp)             # Save $19
        sd      $20,  80*X($sp)             # Save $20
        sd      $21,  84*X($sp)             # Save $21
        sd      $22,  88*X($sp)             # Save $22
        sd      $23,  92*X($sp)             # Save $23
        sd      $24,  96*X($sp)             # Save $24 (t8-t9)
        sd      $25, 100*X($sp)             # Save $25
        ld      $8,  (TASK_STK-24)($sp)     # Recover original saved $31 (ra)
        sd      $30, 104*X($sp)             # Save $30 (fp)
        sd      $8,  108*X($sp)             # Save $31 (ra)
        mfhi    $8                          # Pickup HI register
        mflo    $9                          # Pickup LO register
        mfc0    $10, $12                    # Pickup interrupted SR
        mfc0    $11, $14                    # Pickup point of interrupt (EPC)
        sd      $8,  112*X($sp)             # Save HI register
        sd      $9,  116*X($sp)             # Save LO register
        sd      $10, 120*X($sp)             # Save SR
        sd      $11, 124*X($sp)             # Save point of interrupt (EPC)
#
#        /* Save floating point registers.  */
#

#if USE_FP_REGS
        cfc1    $8, $31                     # Pickup FPU control register
        sdc1    $f0,  132*X($sp)            # Save $f0-$f1
        sdc1    $f1,  136*X($sp)            #
        sdc1    $f2,  140*X($sp)            # Save $f2-$f3
        sdc1    $f3,  144*X($sp)            #
        sdc1    $f4,  148*X($sp)            # Save $f4-$f5
        sdc1    $f5,  152*X($sp)            #
        sd      $8,   128*X($sp)            # Save FPU control register
        sdc1    $f6,  156*X($sp)            # Save $f6-$f7
        sdc1    $f7,  160*X($sp)            #
        sdc1    $f8,  164*X($sp)            # Save $f8-$f9
        sdc1    $f9,  168*X($sp)            #
        sdc1    $f10, 172*X($sp)            # Save $f10-$f11
        sdc1    $f11, 176*X($sp)            #
        sdc1    $f12, 180*X($sp)            # Save $f12-$f13
        sdc1    $f13, 184*X($sp)            #
        sdc1    $f14, 188*X($sp)            # Save $f14-$f15
        sdc1    $f15, 192*X($sp)            #
        sdc1    $f16, 196*X($sp)            # Save $f16-$f17
        sdc1    $f17, 200*X($sp)            #
        sdc1    $f18, 204*X($sp)            # Save $f18-$f19
        sdc1    $f19, 208*X($sp)            #
        sdc1    $f20, 212*X($sp)            # Save $f20-$f21
        sdc1    $f21, 216*X($sp)            #
        sdc1    $f22, 220*X($sp)            # Save $f22-$f23
        sdc1    $f23, 224*X($sp)            #
        sdc1    $f24, 228*X($sp)            # Save $f24-$f25
        sdc1    $f25, 232*X($sp)            #
        sdc1    $f26, 236*X($sp)            # Save $f26-$f27
        sdc1    $f27, 240*X($sp)            #
        sdc1    $f28, 244*X($sp)            # Save $f28-$f29
        sdc1    $f29, 248*X($sp)            #
        sdc1    $f30, 252*X($sp)            # Save $f30-$f31
        sdc1    $f31, 256*X($sp)            #
#endif USE_FP_REGS
#ifdef INCLUDE_ODYSSEY
		cfc0	$9, $20
		nop
		nop
		nop
        sd      $9,  260*X($sp)             # Save IC
	
		li		$9, CONFIG_ICR_VS	
        ctc0    $9, $20
        nop
        nop
        nop
#endif
 
        li      $9, INT_DISABLE             # Build interrupt disable value
        mtc0    $9, $12                     # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot


        j       $31                         # Return to ISR shell
        nop                                 # Delay slot
#        
#    }
#    else
#    {
_TCT_Not_Nest_Sav:
        sw      $17, TCD_Interrupt_Count    # Store the new interrupt count
#    
#        /* Determine if a thread was interrupted.  */
#        if (TCD_Current_Thread)
#        {
#
        lw      $17, TCD_Current_Thread     # Pickup the current thread pointer
        addiu   $sp, $sp, -(TASK_STK-32)    #   context save
        beq     $17, $0, _TCT_Idle_Save     # If NULL, no thread was 
                                            #   interrupted
        nop                                 # Delay slot
#        
#            /* Yes, a thread was interrupted.  Save complete context on the
#               thread's stack.  */
#
        li      $17, 1                      # Build interrupt stack type
        sd      $17, 0($sp)                 # Place stack type on top of stack
        ld      $17, (TASK_STK-32)($sp)     # Recover original saved $at
        nop                                 # Delay slot
        sd      $17, 4*X($sp)               # Save at register
        sd      $2,  8*X($sp)               # Save $2 (v0-v1)
        sd      $3,  12*X($sp)              # Save $3
        sd      $4,  16*X($sp)              # Save $4 (a0-a3)
        sd      $5,  20*X($sp)              # Save $5
        sd      $6,  24*X($sp)              # Save $6
        sd      $7,  28*X($sp)              # Save $7
        sd      $8,  32*X($sp)              # Save $8 (t0-t7)
        sd      $9,  36*X($sp)              # Save $9
        sd      $10, 40*X($sp)              # Save $10
        sd      $11, 44*X($sp)              # Save $11
        sd      $12, 48*X($sp)              # Save $12
        sd      $13, 52*X($sp)              # Save $13
        sd      $14, 56*X($sp)              # Save $14
        sd      $15, 60*X($sp)              # Save $15
        ld      $8,  (TASK_STK-16)($sp)     # Recover original saved $16 (s0)
        ld      $9,  (TASK_STK-8)($sp)      # Recover original saved $17 (s1)
        sd      $8,   64*X($sp)             # Save $16 (s0-s7)
        sd      $9,   68*X($sp)             # Save $17
        sd      $18,  72*X($sp)             # Save $18
        sd      $19,  76*X($sp)             # Save $19
        sd      $20,  80*X($sp)             # Save $20
        sd      $21,  84*X($sp)             # Save $21
        sd      $22,  88*X($sp)             # Save $22
        sd      $23,  92*X($sp)             # Save $23
        sd      $24,  96*X($sp)             # Save $24 (t8-t9)
        sd      $25, 100*X($sp)             # Save $25
        ld      $8,  (TASK_STK-24)($sp)     # Recover original saved $31 (ra)
        sd      $30, 104*X($sp)             # Save $30 (fp)
        sd      $8,  108*X($sp)             # Save $31 (ra)
        mfhi    $8                          # Pickup HI register
        mflo    $9                          # Pickup LO register
        mfc0    $10, $12                    # Pickup interrupted SR
        mfc0    $11, $14                    # Pickup point of interrupt (EPC)
        sd      $8,  112*X($sp)             # Save HI register
        sd      $9,  116*X($sp)             # Save LO register
        sd      $10, 120*X($sp)             # Save SR
        sd      $11, 124*X($sp)             # Save point of interrupt (EPC)
#
#        /* Save floating point registers.  */
#

#if USE_FP_REGS
        cfc1    $8, $31                     # Pickup FPU control register
        sdc1    $f0,  132*X($sp)            # Save $f0-$f1
        sdc1    $f1,  136*X($sp)            #
        sdc1    $f2,  140*X($sp)            # Save $f2-$f3
        sdc1    $f3,  144*X($sp)            #
        sdc1    $f4,  148*X($sp)            # Save $f4-$f5
        sdc1    $f5,  152*X($sp)            #
        sd      $8,   128*X($sp)            # Save FPU control register
        sdc1    $f6,  156*X($sp)            # Save $f6-$f7
        sdc1    $f7,  160*X($sp)            #
        sdc1    $f8,  164*X($sp)            # Save $f8-$f9
        sdc1    $f9,  168*X($sp)            #
        sdc1    $f10, 172*X($sp)            # Save $f10-$f11
        sdc1    $f11, 176*X($sp)            #
        sdc1    $f12, 180*X($sp)            # Save $f12-$f13
        sdc1    $f13, 184*X($sp)            #
        sdc1    $f14, 188*X($sp)            # Save $f14-$f15
        sdc1    $f15, 192*X($sp)            #
        sdc1    $f16, 196*X($sp)            # Save $f16-$f17
        sdc1    $f17, 200*X($sp)            #
        sdc1    $f18, 204*X($sp)            # Save $f18-$f19
        sdc1    $f19, 208*X($sp)            #
        sdc1    $f20, 212*X($sp)            # Save $f20-$f21
        sdc1    $f21, 216*X($sp)            #
        sdc1    $f22, 220*X($sp)            # Save $f22-$f23
        sdc1    $f23, 224*X($sp)            #
        sdc1    $f24, 228*X($sp)            # Save $f24-$f25
        sdc1    $f25, 232*X($sp)            #
        sdc1    $f26, 236*X($sp)            # Save $f26-$f27
        sdc1    $f27, 240*X($sp)            #
        sdc1    $f28, 244*X($sp)            # Save $f28-$f29
        sdc1    $f29, 248*X($sp)            #
        sdc1    $f30, 252*X($sp)            # Save $f30-$f31
        sdc1    $f31, 256*X($sp)            #
#endif USE_FP_REGS
#ifdef INCLUDE_ODYSSEY
		cfc0	$9, $20
		nop
		nop
		nop
        sd      $9,  260*X($sp)             # Save IC
		
#endif

     
#            /* Save the thread's stack pointer in the control block.  */
#            REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
#            REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;
#   
        lw      $8, TCD_Current_Thread      # Pickup the current thread pointer
       
        sw      $sp, 44($8)               # Save stack pointer in thread's 
                                            #   control block
#
#            /* Switch to the system stack.  */
#            REG_Stack_Ptr =  TCD_System_Stack;
#
        lw      $sp, TCD_System_Stack       # Switch to system stack pointer
#
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
        ctc0    $9, $20
        nop
        nop
        nop
#endif
     	li      $9, INT_DISABLE             # Build interrupt disable value
        mtc0    $9, $12                     # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot

        j       $31                         # Return to ISR shell
        nop                                 # Delay slot
#
#        }
_TCT_Idle_Save:
#    }
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
        ctc0    $9, $20
        nop
        nop
        nop
#endif
        li      $9, INT_DISABLE             # Build interrupt disable value
        mtc0    $9, $12                     # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot

        addiu   $sp, $sp, TASK_STK          # Recover stack space
        j       $31                         # Return to ISR shell
        nop                                 # Delay slot
#    
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Interrupt_Context_Restore                                    */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function restores the interrupt context if a nested         */
#/*      interrupt condition is present.  Otherwise, this routine         */
#/*      transfers control to the scheduling function.                    */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application ISRs                    Assembly language ISRs       */
#/*      INT_Interrupt_Shell                 Interrupt handler shell      */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      TCT_Schedule                        Thread scheduling function   */
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
#VOID  TCT_Interrupt_Context_Restore(void)
#{
        .align  4
        .globl  TCT_Interrupt_Context_Restore
TCT_Interrupt_Context_Restore:
#
#    /* Lockout interrupts.  */
#    
#ifdef INCLUDE_ODYSSEY
		li		$9, CONFIG_ICR_VS
        ctc0    $9, $20
        nop
        nop
        nop
#endif
        li      $9, INT_DISABLE             # Build interrupt disable value
        mtc0    $9, $12                     # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot

#
#    /* Decrement and check for nested interrupt conditions.  */
#    if (--TCD_Interrupt_Count)
#    {
#
        lw      $8, TCD_Interrupt_Count     # Pickup interrupt count 
        nop                                 # Delay slot
        addiu   $8, $8, -1                  # Decrement interrupt count
        sw      $8, TCD_Interrupt_Count     # Store new interrupt count
        beq     $8, $0, _TCT_Not_Nest_Rest  # If 0, not nested restore
        nop                                 # Delay slot
#    
#        /* Restore previous context.  */
#
#       /* Recover floating point registers.  */
#

#if USE_FP_REGS
        ld      $8,   128*X($sp)            # Pickup FPU control register
        ldc1    $f0,  132*X($sp)            # Recover $f0-$f1
        ldc1    $f1,  136*X($sp)            #
        ldc1    $f2,  140*X($sp)            # Recover $f2-$f3
        ldc1    $f3,  144*X($sp)            #
        ldc1    $f4,  148*X($sp)            # Recover $f4-$f5
        ldc1    $f5,  152*X($sp)            #
        ldc1    $f6,  156*X($sp)            # Recover $f6-$f7
        ldc1    $f7,  160*X($sp)            #
        ctc1    $8,   $31                   # Recover FPU control register
        ldc1    $f8,  164*X($sp)            # Recover $f8-$f9
        ldc1    $f9,  168*X($sp)            #
        ldc1    $f10, 172*X($sp)            # Recover $f10-$f11
        ldc1    $f11, 176*X($sp)            #
        ldc1    $f12, 180*X($sp)            # Recover $f12-$f13
        ldc1    $f13, 184*X($sp)            #
        ldc1    $f14, 188*X($sp)            # Recover $f14-$f15
        ldc1    $f15, 192*X($sp)            #
        ldc1    $f16, 196*X($sp)            # Recover $f16-$f17
        ldc1    $f17, 200*X($sp)            #
        ldc1    $f18, 204*X($sp)            # Recover $f18-$f19
        ldc1    $f19, 208*X($sp)            #
        ldc1    $f20, 212*X($sp)            # Recover $f20-$f21
        ldc1    $f21, 216*X($sp)            #
        ldc1    $f22, 220*X($sp)            # Recover $f22-$f23
        ldc1    $f23, 224*X($sp)            #
        ldc1    $f24, 228*X($sp)            # Recover $f24-$f25
        ldc1    $f25, 232*X($sp)            #
        ldc1    $f26, 236*X($sp)            # Recover $f26-$f27
        ldc1    $f27, 240*X($sp)            # 
        ldc1    $f28, 244*X($sp)            # Recover $f28-$f29
        ldc1    $f29, 248*X($sp)            #
        ldc1    $f30, 252*X($sp)            # Recover $f30-$f31
        ldc1    $f31, 256*X($sp)            #
#endif USE_FP_REGS
 
        .option no_at_macros on
        ld      $at,   4*X($sp)             # Recover $at
        ld      $2,    8*X($sp)             # Recover $2 (v0-v1)
        ld      $3,   12*X($sp)             # Recover $3
        ld      $4,   16*X($sp)             # Recover $4 (a0-a3)
        ld      $5,   20*X($sp)             # Recover $5
        ld      $6,   24*X($sp)             # Recover $6
        ld      $7,   28*X($sp)             # Recover $7
        ld      $8,   32*X($sp)             # Recover $8 (t0-t7)
        ld      $9,   36*X($sp)             # Recover $9
        ld      $10,  40*X($sp)             # Recover $10
        ld      $11,  44*X($sp)             # Recover $11
        ld      $12,  48*X($sp)             # Recover $12
        ld      $13,  52*X($sp)             # Recover $13
        ld      $14,  56*X($sp)             # Recover $14
        ld      $15,  60*X($sp)             # Recover $15
        ld      $16,  64*X($sp)             # Recover $16 (s0-s7)
        ld      $17,  68*X($sp)             # Recover $17
        ld      $18,  72*X($sp)             # Recover $18
        ld      $19,  76*X($sp)             # Recover $19
        ld      $20,  80*X($sp)             # Recover $20
        ld      $21,  84*X($sp)             # Recover $21
        ld      $22,  88*X($sp)             # Recover $22
        ld      $23,  92*X($sp)             # Recover $23
        ld      $24,  96*X($sp)             # Recover $24 (t8-t9)
        ld      $25, 100*X($sp)             # Recover $25
        ld      $30, 104*X($sp)             # Recover $30 (fp)
        ld      $31, 108*X($sp)             # Recover $31 (ra)
        ld      $27, 112*X($sp)             # Recover HI register
        nop                                 # Delay
        mthi    $27                         # Restore HI register
        ld      $27, 116*X($sp)             # Recover LO register
        nop                                 # Delay
        mtlo    $27                         # Restore LO register

#ifdef INCLUDE_ODYSSEY
		ld		$27, 260*X($sp)
		ctc0	$27, $20
		nop
		nop
		nop
#endif
        ld      $27, 120*X($sp)             # Recover SR 
        nop                                 # Delay
        mtc0    $27, $12                    # Restore interrupted SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        ld      $27, 124*X($sp)             # Recover EPC
        addiu   $sp, $sp, TASK_STK          # Recover stack space
        mtc0    $27, $14                    # Restore EPC
        nop                                 # Delay slot
        nop                                 # Delay slot

        nop                                 # Delay slot
        eret                                # Return to point of exception
        nop                                 # Delay slot
        nop                                 # Delay slot
        .option no_at_macros off
#
#    }
#    else
#    {
_TCT_Not_Nest_Rest:
#    
#
#            /* Determine if a time slice is active.  If so, the remaining
#               time left on the time slice must be saved in the task's
#               control block.  Normally, saving the time-slice value is done
#               in context save.  This won't work in the MIPS port since 
#               context save is called prior to the timer routine!  */
#            if (TMD_Time_Slice_State == 0)
#            {
        lw      $10, TMD_Time_Slice_State   # Pickup the time slice state
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
        bne     $10, $0, _TCT_No_TS_4       # If not active, skip saving TS
        li      $9, 1                       # Build NOT_ACTIVE value
#            
#                /* Pickup the remaining portion of the time slice and save it
#                   in the task's control block.  */
#                REG_Thread_Ptr -> tc_cur_time_slice =  TMD_Time_Slice;
#                TMD_Time_Slice_State =  1;
#
        lw      $11, TMD_Time_Slice         # Pickup remaining time slice value
        sw      $9, TMD_Time_Slice_State    # Indicate TS is not active 
        sw      $11, 32($8)                 # Save remaining TS in thread's 
                                            #   control block
#            }
_TCT_No_TS_4:
#
#        /* Clear the current thread pointer.  */
#        TCD_Current_Thread =  NU_NULL;
#
        sw      $0, TCD_Current_Thread      # Set current thread to NULL
#        
#        /* Reset the system stack.  */
#        REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack;
#
#        /* Return to scheduler.  */
#
        lw      $sp, TCD_System_Stack       # Reset system stack just in case
        j       TCT_Schedule                # Return to scheduler
        nop                                 # Delay slot
#    }
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_Activate_HISR                                                */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function activates the specified HISR.  If the HISR is      */
#/*      already activated, the HISR's activation count is simply         */
#/*      incremented.  Otherwise, the HISR is placed on the appropriate   */
#/*      HISR priority list in preparation for execution.                 */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application LISRs                                                */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      hisr                                Pointer to HISR to activate  */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      NU_SUCCESS                          Successful completion        */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-16-1996          Prepared initial version 1.0 */
#/*                                                                       */
#/*************************************************************************/
#STATUS  TCT_Activate_HISR(TC_HCB *hisr)
#{
        .align  4
        .globl  TCT_Activate_HISR
TCT_Activate_HISR:
#
#INT     priority;
#
#
#    /* Lockout interrupts.  */
#
#ifdef INCLUDE_ODYSSEY
		cfc0	$13, $20
		nop
		nop
		nop
		li		$10, CONFIG_ICR_VS
		ctc0	$10, $20
		nop
		nop
		nop
#endif
        mfc0    $11, $12                    # Pickup current SR
        li      $10, INT_DISABLE            # Pickup disable interrupt value
        mtc0    $10, $12                    # Disable interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#    /* Determine if the HISR is already active.  */
#    if (hisr -> tc_activation_count)
#    {
#
        lw      $8, 64($4)                  # Pickup HISR activation count
        nop                                 # Delay slot
        beq     $8, $0, _TCT_First_Activate # If 0, initial HISR activation
#
# 	We allow each HISR to run only once prior to relinquishing
#	control to allow other processes to run.
#
.if (X-1)
        addiu   $8, $8, 1                   # Increment HISR activation count
.else
        addiu   $8, $8, 1                   # Increment HISR activation count
.endif
#
#        /* Increment the activation count.  Make sure that it does not go
#           to zero.  */
#        hisr -> tc_activation_count++;
#
#        if (hisr -> tc_activation_count == 0)
#
        sw      $8, 64($4)                  # Store new activation count
        li      $9, -1                      # Build all ones value
        bne     $8, $0, _TCT_Activate_Done  # Determine if the counter rolled
        nop                                 # Delay slot
#        
#            hisr -> tc_activation_count =  0xFFFFFFFFUL;
#
        sw      $9, 64($4)                  # Save all ones activation count
        j       _TCT_Activate_Done          # Finished with activation
        nop                                 # Delay slot
#    }
#    else
#    {
#
_TCT_First_Activate:
#
#
#        /* Set the activation count to 1.  */
#        hisr -> tc_activation_count =  1;
#
.if (X-1)
#        li      $8, 1 
        sw      $8, 64($4)                  # Store activation count
.endif
        sw      $8, 64($4)                  # Store activation count
#
#        /* Pickup the HISR's priority.  */
#        priority =  hisr -> tc_priority;
#
        lbu     $8, 26($4)                  # Pickup priority of HISR
        nop                                 # Delay slot
        sll     $8, $8, 2                   # Multiply by 4 for this offset
#
#        /* Determine if there is something in the given priority list.  */
#        if (TCD_Active_HISR_Tails[priority])
#        {
#
        # MetroWerkaround:
        # Unsupported assembler syntax: lw  rt, base(offset) -> la, add, lw.
        # lw    $9,  TCD_Active_HISR_Tails($8)  # Pickup priority tail pointer
        la      $12, TCD_Active_HISR_Tails      #  get base address of array
        add     $12, $12, $8                    #  add offset of desired element
        lw      $9,  0($12)                     #  load array element value

        # MetroWerkaround:
        # Unsupported assembler syntax: lw  rt, base(offset) -> la, add, lw.
        # lw    $10, TCD_Active_HISR_Heads($8)  # Pickup priority head pointer
        la      $12, TCD_Active_HISR_Heads      #  get base address of array
        add     $12, $12, $8                    #  add offset of desired element
        lw      $10, 0($12)                     #  load array element value

        beq     $9,  $0, _TCT_Activate_Priority # If NULL, no HISR at this 
        nop                                     #   priority is ready
#
#            /* Something is already on this list.  Add after the tail.  */
#            (TCD_Active_HISR_Tails[priority]) -> tc_active_next =  hisr;
#            TCD_Active_HISR_Tails[priority] =  hisr;
#
        # MetroWerkaround:
        # Unsupported assembler syntax: sw  rt, base(offset) -> la, add, sw.
        # sw    $4,  TCD_Active_HISR_Tails($8)  # Setup new tail pointer
        la      $12, TCD_Active_HISR_Tails      #  get base address of array
        add     $12, $12, $8                    #  add offset of desired element
        sw      $4,  0($12)                     #  store array element value

        sw      $4,  60($9)                     # Setup next field of last HISR
        b       _TCT_Activate_Done              # Activate done
        nop                                     # Delay slot
#        }
#        else
#        {
_TCT_Activate_Priority:
#
#           /* Nothing is on this list.  */
#           TCD_Active_HISR_Heads[priority] =  hisr;
#           TCD_Active_HISR_Tails[priority] =  hisr;
#
        # MetroWerkaround:
        # Unsupported assembler syntax: sw  rt, base(offset) -> la, add, sw.
        # sw    $4,  TCD_Active_HISR_Heads($8)  # Setup priority level head pointer
        la      $12, TCD_Active_HISR_Heads      #  get base address of array
        add     $12, $12, $8                    #  add offset of desired element
        sw      $4,  0($12)                     #  store array element value

        # MetroWerkaround:
        # Unsupported assembler syntax: sw  rt, base(offset) -> la, add, sw.
        # sw    $4,  TCD_Active_HISR_Tails($8)  # Setup priority level tail pointer
        la      $12, TCD_Active_HISR_Tails      #  get base address of array
        add     $12, $12, $8                    #  add offset of desired element
        sw      $4,  0($12)                     #  store array element value

#
#           /* Determine the highest priority HISR.  */
#           if (TCD_Active_HISR_Heads[0])
#
        lw      $8, TCD_Active_HISR_Heads   # Pickup priority 0 head pointer
        lw      $9, TCD_Active_HISR_Heads+4 # Pickup priority 1 head pointer
        lw      $10,TCD_Active_HISR_Heads+8 # Pickup priority 2 head pointer
        beq     $8, $0, _TCT_Activate_Chk1  # If NULL, check priority 1
        nop                                 # Delay slot
#
#               TCD_Execute_HISR =  TCD_Active_HISR_Heads[0];
#
        sw      $8, TCD_Execute_HISR        # Set highest priority HISR
        b       _TCT_Activate_Done          # Finished with activate
        nop                                 # Delay slot
#
#           else if (TCD_Active_HISR_Heads[1])
#
_TCT_Activate_Chk1:
        beq     $9, $0, _TCT_Activate_Pri2  # If NULL, priority 2 is active
        nop                                 # Delay slot
#
#               TCD_Execute_HISR =  TCD_Active_HISR_Heads[1];
#
        sw      $9, TCD_Execute_HISR        # Set highest priority HISR
        b       _TCT_Activate_Done          # Finished with activate
        nop                                 # Delay slot
#           else
_TCT_Activate_Pri2:
#               TCD_Execute_HISR =  TCD_Active_HISR_Heads[2];
#
        sw      $10, TCD_Execute_HISR       # Set highest priority HISR
#       }
#    }
#
_TCT_Activate_Done:
#
#    return(NU_SUCCESS);
#
#ifdef INCLUDE_ODYSSEY
		ori		$13, $13, CONFIG_ICR_VS
		ctc0	$13, $20
		nop
		nop
		nop
#endif
        mtc0    $11, $12                    # Restore interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        j       $31                         # Return to caller
        or      $2, $0, $0                  # Return NU_SUCCESS to caller
#}
#
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      TCT_HISR_Shell                                                   */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function is the execution shell of each and every HISR.  If */
#/*      the HISR has completed its processing, this shell routine exits  */
#/*      back to the system.  Otherwise, it sequentially calls the HISR   */
#/*      routine until the activation count goes to zero.                 */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      HISR Scheduling                                                  */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      hisr -> tc_entry                   Actual entry function of HISR */
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
#VOID    TCT_HISR_Shell(void)
#{
        .align  4
        .globl  TCT_HISR_Shell
TCT_HISR_Shell:
#
#    /* Point at the HISR.  */
#    REG_HISR_Ptr =  (TC_HCB *) TCD_Current_Thread;
#
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
        nop                                 # Delay slot
#
#    do
#    {
_TCT_HISR_Loop:
#
#        /* Call the HISR's entry routine.  */
#        (*(REG_HISR_Ptr -> tc_entry)) ();
#
        lw      $9, 68($8)                  # Pickup HISR function pointer
        nop                                 # Delay slot
        jalr    $9                          # Call HISR entry
        nop                                 # Delay slot
#
#        /* Lockout interrupts.  */
#
        li      $10, INT_DISABLE            # Build disable interrupts value
        lw      $8, TCD_Current_Thread      # Pickup current thread pointer
#ifdef INCLUDE_ODYSSEY
		cfc0	$13, $20
		nop
		nop
		nop
		li		$9, CONFIG_ICR_VS
		ctc0	$9, $20
		nop
		nop
		nop
#endif
        mtc0    $10, $12                    # Lockout interrupts
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
#
#        /* On return, decrement the activation count and check to see if 
#           it is 0.  Once it reaches 0, the HISR should be made inactive. */
#        REG_HISR_Ptr -> tc_activation_count--;
#
        lw      $9, 64($8)                  # Pickup activation count
        li      $11, INT_MASK               # Build interrupt mask
        addiu   $9, $9, -1                  # Decrement by 1
        sw      $9, 64($8)                  # Store new activation count
#
#    }
#    while (REG_HISR_Ptr -> tc_activation_count);
#
        lw      $10, TCD_Interrupt_Level    # Pickup the interrupt level
        beq     $9, $0,_TCT_HISR_Done       # If 0, HISR activations are done
        nop                                 # Delay slot
        mfc0    $12, $12                    # Pickup current SR
        nop                                 # Delay slot
        nop                                 # Delay slot
        and     $12, $12, $11               # Clear current interrupt bits
        or      $12, $12, $10               # Place new interrupt bits in SR
#ifdef INCLUDE_ODYSSEY
		ori		$13, $13, CONFIG_ICR_VS
		ctc0	$13, $20
		nop
		nop
		nop
#endif
        mtc0    $12, $12                    # Restore normal interrupt posture
        nop                                 # Delay slot
        nop                                 # Delay slot
        nop                                 # Delay slot
        b       _TCT_HISR_Loop              # Another HISR activation
        nop                                 # Delay slot
#
#    /* At this point, the HISR needs to be made inactive.  */
_TCT_HISR_Done:
#
#    /* Determine if this is the only HISR on the given priority list.  */
#    if (REG_HISR_Ptr == TCD_Active_HISR_Tails[REG_HISR_Ptr -> tc_priority])
#    {
#
        lbu     $9, 26($8)                  # Pickup the HISR priority
        nop                                 # Delay slot
        sll     $9, $9, 2                   # Multiply by 4 to get offset

        # MetroWerkaround:
        # Unsupported assembler syntax: lw  rt, base(offset) -> la, add, lw.
        # lw    $10, TCD_Active_HISR_Tails($9)  # Is there another HISR?
        la      $12, TCD_Active_HISR_Tails      #  get base address of array
        add     $12, $12, $9                    #  add offset of desired element
        lw      $10, 0($12)                     #  load array element value

        nop                                 # Delay slot
        bne     $8, $10, _TCT_Other_HISRs   # If different, other HISRs at 
        nop                                 #   the same priority are ready
#
#        /* The only HISR on the list.  Clean up the list and check for the
#           highest priority HISR.  */
#       TCD_Active_HISR_Heads[REG_HISR_Ptr -> tc_priority] =  NU_NULL;
#       TCD_Active_HISR_Tails[REG_HISR_Ptr -> tc_priority] =  NU_NULL;
#      
        # MetroWerkaround:
        # Unsupported assembler syntax: sw  rt, base(offset) -> la, add, sw.
        # sw    $0,  TCD_Active_HISR_Heads($9)  # Set HISR head pointer to NULL
        la      $12, TCD_Active_HISR_Heads      #  get base address of array
        add     $12, $12, $9                    #  add offset of desired element
        sw      $0,  0($12)                     #  store array element value

        # MetroWerkaround:
        # Unsupported assembler syntax: sw  rt, base(offset) -> la, add, sw.
        # sw    $0,  TCD_Active_HISR_Tails($9)  # Set HISR tail pointer to NULL
        la      $12, TCD_Active_HISR_Tails      #  get base address of array
        add     $12, $12, $9                    #  add offset of desired element
        sw      $0,  0($12)                     #  store array element value

#
#       /* Determine 'the highest priority HISR.  */
#       if (TCD_Active_HISR_Heads[0])
#
        lw      $10, TCD_Active_HISR_Heads    # Pickup the Priority 0 head ptr
        lw      $11, TCD_Active_HISR_Heads+4  # Pickup the Priority 1 head ptr
        lw      $12, TCD_Active_HISR_Heads+8  # Pickup the Priority 2 head ptr
        beq     $10, $0, _TCT_Shell_Chk1      # If NULL, check priority 1
        nop                                   # Delay slot
#
#           TCD_Execute_HISR =  TCD_Active_HISR_Heads[0];
#
        sw      $10, TCD_Execute_HISR       # Setup HISR pointer
        b       _TCT_HISR_Exit              # Finished in HISR shell
        nop                                 # Delay slot
# 
#       else if (TCD_Active_HISR_Heads[1])
#
_TCT_Shell_Chk1:
        beq     $11, $0, _TCT_Shell_Pri2    # If NULL, must be use priority 2
        nop                                 # Delay slot
#
#           TCD_Execute_HISR =  TCD_Active_HISR_Heads[1];
#
        sw      $11, TCD_Execute_HISR       # Setup HISR pointer
        b       _TCT_HISR_Exit              # Finished in HISR shell
        nop                                 # Delay slot
#
#       else
#
_TCT_Shell_Pri2:
#
#           TCD_Execute_HISR =  TCD_Active_HISR_Heads[2];
#
        sw      $12, TCD_Execute_HISR       # Use Priority 2 HISR head pointer
        b       _TCT_HISR_Exit              # Finished in HISR shell
        nop                                 # Delay slot
#
#    }
#    else
#    {
_TCT_Other_HISRs:
#
#        /* Move the head pointer to the next HISR in the list.  */
#        TCD_Active_HISR_Heads[REG_HISR_Ptr -> tc_priority] =  
#                                        REG_HISR_Ptr -> tc_active_next;
#
        lw      $10, 60($8)                 # Pickup HISR's next pointer
        nop                                 # Delay slot

        # MetroWerkaround:
        # Unsupported assembler syntax: sw  rt, base(offset) -> la, add, sw.
        # sw    $10, TCD_Active_HISR_Heads($9)  # Move the head pointer
        la      $12, TCD_Active_HISR_Heads      #  get base address of array
        add     $12, $12, $9                    #  add offset of desired element
        sw      $10, 0($12)                     #  store array element value

#
#        /* Also set the TCD_Execute_HISR pointer.  */
#        TCD_Execute_HISR =  REG_HISR_Ptr -> tc_active_next;
#
        sw      $10, TCD_Execute_HISR       # Setup new HISR pointer
#    }
#
_TCT_HISR_Exit:
#
#    /* Build fake return to the top of this loop.  The next time the HISR
#       is activated, it will return to the top of this function.  */
#
#    /* Save the minimal amount of information on the stack.  */
#
        addiu   $sp, $sp, -HISR_STK         # Allocate a stack frame
        la      $10, TCT_HISR_Shell         # Build address to shell
        sd      $0,   0*X($sp)              # Setup solicited stack type
        sd      $0,  36*X($sp)              # Setup frame pointer
        sd      $10, 40*X($sp)              # Setup a return to the top of this
                                            #   shell function
#
#    /* Save preserved floating point registers.  */
#

#if USE_FP_REGS
        cfc1    $9, $31                     # Pickup FPU control register
        sdc1    $f20, 48*X($sp)             # Save $f20-$f21
        sdc1    $f21, 52*X($sp)             #
        sdc1    $f22, 56*X($sp)             # Save $f22-$f23
        sdc1    $f23, 60*X($sp)             #
        sdc1    $f24, 64*X($sp)             # Save $f24-$f25
        sdc1    $f25, 68*X($sp)             #
        sd      $9,   44*X($sp)             # Save FPU control register
        sdc1    $f26, 72*X($sp)             # Save $f26-$f27
        sdc1    $f27, 76*X($sp)             #
        sdc1    $f28, 80*X($sp)             # Save $f28-$f29
        sdc1    $f29, 84*X($sp)             #
        sdc1    $f30, 88*X($sp)             # Save $f30-$f31
        sdc1    $f31, 92*X($sp)             #
#endif USE_FP_REGS

#
#    /* Clear the current thread control block.  */
#    TCD_Current_Thread =  NU_NULL;
#
        sw      $0, TCD_Current_Thread      # Set current thread to NULL
#
#    /* Save off the current stack pointer in the control block.  */
#    REG_HISR_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;
#
        sw      $sp, 44($8)                 # Save the system stack pointer 
#
#    /* Switch to the system stack.  */
#    REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack;
#
#    /* Transfer control to the main scheduling loop.  */
#
        lw      $sp, TCD_System_Stack       # Switch to system stack pointer
        j       TCT_Schedule                # Return to scheduling loop
        nop                                 # Delay slot
#}
#
        

