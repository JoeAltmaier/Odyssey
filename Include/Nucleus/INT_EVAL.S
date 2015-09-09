#/*************************************************************************/
#// This material is a confidential trade secret and proprietary 
#// information of ConvergeNet Technologies, Inc. which may not be 
#// reproduced, used, sold or transferred to any third party without the 
#// prior written consent of ConvergeNet Technologies, Inc.  This material 
#// is also copyrighted as an unpublished work under sections 104 and 408 
#// of Title 17 of the United States Code.  Law prohibits unauthorized 
#// use, copying or reproduction.
#//
#// File: int_eval.s
#//
#// Description:
#// This file contains the target processor dependent initialization
#// routines and data, and is part of the R5000 port from ATI.  The R5000
#// code was developed using GNU tools.  We are using Metrowerks tools.
#// There are one or two differences.
#//
#// MetroWerkarounds:
#// -----------------
#// Assembler directive changes.
#//    .set noreorder       -> .option reorder off
#//    .set noat            -> .option no_at_macros on
#//    .set at              -> .option no_at_macros off
#//    .align 2             -> .align 4
#//    .align 4             -> .align 16
#//    .extern symbol size  -> .extern symbol
#//    .lcomm symbol length -> symbol:   and   .space length
#//
#// Explicitly declare linker generated symbols and access them with the
#// lw command.  GNU implicitly provided symbols which were equates to
#// the relevant locations and were accessed with the la command.
#//
#// Explicitly declare external function references.
#//
#// Added explicit bss section.
#//
#// Modified code used to clear the bss section to use the segment size.
#//
#// Added code to clear the sbss section.
#//
#//
#// Update Log:
#// 09/30/98 Jeff Nespor: MetroWerkarounds.
#//                       Changed TIMER_COMPARE for a 150MHz system clock.
#// 10/05/98 Jeff Nespor: Remove UDB specific code.  Cleanup for check in.
#// 10/28/98 Jeff Nespor: Changes for Metrowerks CodeWarrior Pro3 IDE.
#//                       Linker generated symbols are equates again.
#//                       Assembler accepts #define and #include statements.
#// 02/10/99 Jerry Lane:  Changed lines 817 and 818 per Jeff Nespor.
#// 02/18/99 Joe Altmaier:INC_Initialize needs memory pointer loaded.
#// 03/30/99 Joe Altmaier:INT_Timer_Repeat until caught up.
#// 05/06/99 Jeff Nespor: Fixed bss/sbss initialization.
#// 10/19/99 Jeff Nespor: Added support for gp relative addressing (sdata).
#// 10/20/99 RK : Added support for priority based interrupt vectors and
#//						  software workaround for RM 7000 bugs 
#/*************************************************************************/


#/*************************************************************************/
#// The following is the original Accelerated Technology file prologue.
#/*************************************************************************/
#
#/*************************************************************************/
#/*                                                                       */
#/*            Copyright (c) 1995 Accelerated Technology, Inc.            */
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
#/*      int_eval.s                            PLUS/IDT5000/GNU 1.0       */
#/*                                                                       */
#/* COMPONENT                                                             */
#/*                                                                       */
#/*      IN - Initialization                                              */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This file contains the target processor dependent initialization */
#/*      routines and data.                                               */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* DATA STRUCTURES                                                       */
#/*                                                                       */
#/*      INT_Vectors                         Interrupt vector table       */
#/*                                                                       */
#/* FUNCTIONS                                                             */
#/*                                                                       */
#/*      INT_Initialize                      Target initialization        */
#/*      INT_Vectors_Loaded                  Returns a NU_TRUE if all the */
#/*                                            default vectors are loaded */
#/*      INT_Setup_Vector                    Sets up an actual vector     */
#/*                                                                       */
#/* DEPENDENCIES                                                          */
#/*                                                                       */
#/*      nucleus.h                           System constants             */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-20-1996      Prepared version 1.0             */
#/*      S. Nguyen       06-30-1998      Version 1.0 for IDT5000, 64 bits */     
#/*                                                                       */
#/*************************************************************************/
##define         NU_SOURCE_FILE
#
##include        "nucleus.h"                 /* System constants
#
#
#
#/* Define the global system stack variable.  This is setup by the 
#   initialization routine.  */
#
#extern VOID            *TCD_System_Stack;
#
#
#/* Define the global data structures that need to be initialized by this
#   routine.  These structures are used to define the system timer management
#   HISR.  */
#   
#extern VOID     *TMD_HISR_Stack_Ptr;
#extern UNSIGNED  TMD_HISR_Stack_Size;
#extern INT       TMD_HISR_Priority;
#include "int_pri.h"
#/* These externs will be in the small data section if sdata >= 4. */
#
        .extern sdata:TCD_System_Stack
        .extern sdata:TMD_HISR_Stack_Ptr
        .extern sdata:TMD_HISR_Stack_Size
        .extern sdata:TMD_HISR_Priority
        .extern sdata:TCD_Interrupt_Level
        .extern sdata:TCD_R7KInterrupt_Level
#
#/* These externs will be in the small data section if sdata >= 8. */
#
        .extern sdata:bootblockp

#
#/* These externs will be in the small data section if sdata >= 4. */
#
        .extern sdata:TCD_Execute_Task
        .extern sdata:TCD_Execute_HISR
        .extern sdata:TCD_TASK_COUNTER
        .extern sdata:TCD_HISR_COUNTER
        .extern sdata:TCD_IDLE_COUNTER

#
#/* Define extern function references.  */
#
#VOID   INC_Initialize(VOID *first_available_memory);
#VOID   TCT_Interrupt_Context_Save(VOID);
#VOID   TCT_Interrupt_Context_Restore(VOID);
#VOID   TCC_Dispatch_LISR(INT vector_number);
#VOID   TMT_Timer_Interrupt(void);
#
        .extern INC_Initialize
        .extern MSL_Initialize
        .extern TCT_Interrupt_Context_Save
        .extern TCT_Interrupt_Context_Restore
        .extern TCC_Dispatch_LISR
        .extern TMT_Timer_Interrupt
		.extern	ttyA_out


#
#/* Explicitly declare linker generated symbols.  */
#
        .extern _gp
        .extern _fsbss
        .extern _esbss
        .extern _fbss
        .extern _ebss
        .extern _end
        .extern _stack_addr
        .extern _hisr_stack_addr
        .extern _hisr_stack_size
        .extern __static_init_end

#
#/* Define the initialization flag that indicates whether or not all of the
#   default vectors have been loaded during initialization.  This is not used
#   in MIPS ports because there is really only one interrupt vector.  */
#
#INT    INT_Loaded_Flag;
#

#
#/* Define constants for the R5000 processor family.  */
#
        X             = 2                   # Multiplier (32 bits:1, 64 bits:2)
        INT_DISABLE   = 0x34010081          # Must run in Kernel mode
        PEND_MASK     = 0x0000FF00          # Pending interrupt bit mask
        INT_BASE      = 0xA0000180          # Interrupt base address                             
        PEND_MASK_7000= 0x000FFF00          # Pending interrupt bit mask
        INT_BASE_7000 = 0xA0000200          # Interrupt base address for R7K                             
        TIMER_COMPARE = 750000              # 10ms @ 1/2 150MHz for cpu timer

#
#/* Define one of the following for INT_Monitor code. */
#
        INCLUDE_TRK   = 1                   # Flag for running with MetroTRK
        INCLUDE_IDT   = 0                   # Flag for running with IDTsim

# 
#/* Define the HISR priority.  */
#
        HISR_PRIORITY = 0                   # Timer HISR priority
#/*
# * If we are using interrupt driven TRK then enable uart interrupt
# * Interrupt line for uart is
# * 	1 - Eval boards
# * 	3 - Odyssey E1 boards
# * 	4 - Odyssey E2 boards
# * If TRK is not interrupt driven then just enable timer and software
# * interrupts
# */ 
#ifdef TRK_TRANSPORT_INT_DRIVEN
#ifdef INCLUDE_EV64120
#define         NU_ENABLE_INTERRUPTS            0xC300
#else	/* INCLUDE_EV64120 */
#ifdef CONFIG_E2 
#define         NU_ENABLE_INTERRUPTS            0xC300
#else
#define         NU_ENABLE_INTERRUPTS            0xA300
#endif	/* CONFIG_E1 */
#endif	/* INCLUDE_EV64120 */
#else	/* TRK_TRANSPORT_INT_DRIVEN */
#define         NU_ENABLE_INTERRUPTS            0x8300
#endif	/* TRK_TRANSPORT_INT_DRIVEN */
#RM7000 Interrupts */
#define         NU_ENABLE_R7KINTERRUPTS          0x00

COPY_VECTOR:	.macro	dst, src, len
		la		a0, src
		li		a1, dst
		li		a2, len
		jal		_copy_vector
		nop
 .endm

        .text
        .option reorder off

#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      INT_Initialize                                                   */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function sets up the global system stack variable and       */
#/*      transfers control to the target independent initialization       */
#/*      function INC_Initialize.  Responsibilities of this function      */
#/*      include the following:                                           */
#/*                                                                       */
#/*             - Setup necessary processor/system control registers      */
#/*             - Setup the system stack                                  */
#/*             - Setup the timer interrupt                               */
#/*             - Calculate the timer HISR stack and priority             */
#/*             - Calculate the first available memory address            */
#/*             - Transfer control to INC_Initialize to initialize all of */
#/*               the system components.                                  */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      system reset                                                     */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      INC_Initialize                      Common initialization        */
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
#/*      J. Martin       10-20-1996      Prepared version 1.0             */
#/*      S. Nguyen       06-20-1998      Added floating point init        */
#/*                                                                       */
#/*************************************************************************/
#VOID    INT_Initialize(void)
#{
        .align  4
        .globl  start
start:
        .globl  INT_Initialize
INT_Initialize:
#
#    /* Initialize system registers.  */
#
        mtc0    $0, $13                     # Insure SW interrupts are clear
        nop
        nop
        nop

#
#    /* Initialize default cache mode. */
#
        li 		$2, 3
        mtc0	$2, $3
        nop
        nop
        nop

#
#    /*  Disable interupts */
#
        li      $8, INT_DISABLE             # Build disable interrupts value
        mtc0    $8, $12                     # Disable interrupts
        nop
        nop
        nop
#
#    /* Initialize the floating point register */
#
        ctc1     $0, $31
        nop
        nop
        nop

#    
#    /* Initialize the Global Pointer ($gp).  This is used to maintain a base
#       pointer into a 64K global data area.  It does not change and therefore
#       is not part of a thread's context. The linker defines the value for 
#       _gp.  */
#
        la      $gp, _gp                    # Build the value of GP before any
                                            #   static data references are 
                                            #   made

#
#    /* Store the boot block pointer here before a0 is used.
#       Note: bootblockp has to be in initialized data as
#             subsequent code clears bss.  */
#
        la $8, bootblockp
        sd a0, 0($8)

#
#    /* Initialize the system stack pointer.  */
#
        la      $sp, _stack_addr            # Pickup start of system stack area

#
#    /* Clear sbss section (dword aligned in lcf).  */
#
        la      $4, _fsbss                  # Pickup first sbss address
        la      $5, _esbss                  # Pickup last sbss address
        beq     $4, $5, INT_SBss_End        # Check for empty section
        nop                                 # Delay slot
INT_Sbss_Loop:                              #
        sd      $0, 0($4)                   # Clear dword
        addu    $4, 8                       # Increment source address
        bltu    $4, $5, INT_Sbss_Loop       # Not at end? Continue with clear
        nop                                 # Delay slot
INT_SBss_End:                               #

#
#    /* Clear bss section (dword aligned in lcf).  */
#
        la      $4, _fbss                   # Pickup first bss address
        la      $5, _ebss                   # Pickup last bss address
        beq     $4, $5, INT_Bss_End         # Check for empty section
        nop                                 # Delay slot
INT_Bss_Loop:                               #
        sd      $0, 0($4)                   # Clear dword
        addu    $4, 8                       # Increment source address
        bltu    $4, $5, INT_Bss_Loop        # Not at end? Continue with clear
        nop                                 # Delay slot
INT_Bss_End:                                #

#
#       /* Terminate static initializor table with 0 */
#       /* and reeport said bug to Metrowerks. */
#
        la $4, __static_init_end
        sw $0, 0($4)
        nop



#
#    /* IDTR5000-specific code to steal the general exception
#       vector.  */
#
        la  $8, INT_BASE                 # General exception handler address
        sd  $0, 0($8)                    # Clear vector area by storing 0's
        sd  $0, 8($8)                    #  
        sd  $0, 16($8)    
        sd  $0, 24($8)
        la  $9, INT_General_Exception    # Address of new handler
        ld  $10, 0($9)                   # Pickup the new handler word
        sd  $10, 0($8)                   # Store into handler area
        ld  $10, 8($9)                   # Pickup the new handler word
        sd  $10, 8($8)                   # Store into handler area
        ld  $10, 16($9)                  # Pickup the new handler word
        sd  $10, 16($8)                  # Store into handler area
        ld  $10, 24($9)                  # Pickup the new handler word
        sd  $10, 24($8)                  # Store into handler area
        nop
#ifdef INCLUDE_ODYSSEY
# Clear R7K Interrupts
 		ctc0	$0, $18
		nop
		nop
		nop
 		ctc0	$0, $19
		nop
		nop
		nop
		li		t0, CONFIG_ICR_VS
 		ctc0	t0, $20
		nop
		nop
		nop

# Program the R7K Interrupt Mode
		li		$8, 0x1000000
		mtc0	$8, $13
		nop
		nop

#ifdef CONFIG_INT_PRIORITY
		/*
		 * MW does not like "li" of all values
		 */
		li		t3, (INTSW0_PRIORITY << 0) | (INTSW1_PRIORITY << 4))
		li		t0, (INT0_PRIORITY << 8) | (INT1_PRIORITY << 12)
		li		t1, (INT2_PRIORITY << 16) | (INT3_PRIORITY << 20) 
		li		t2, (INT4_PRIORITY << 24) | (INT5_PRIORITY << 28)
		or		t0, t0, t1
		or		t0, t0, t2
		or		t0, t0, t3
		ctc0	t0, $18
		nop
		nop
		nop
		nop

		li		t0, (INT6_PRIORITY << 0) | (INT7_PRIORITY << 4)
		li		t1, (INT8_PRIORITY << 8) | (INT9_PRIORITY << 12) 
		li		t2, (INT10_PRIORITY << 16) | (INT11_PRIORITY << 20)
		or		t0, t0, t1
		or		t0, t0, t2
		ctc0	t0, $19
		nop
		nop
		nop
		nop

		/*
		 * Get Processor ID. Copy vectors with software workarounds if 
		 * Rev is less than 2.1.
		 * Rev field(7..0) is Major(7..4):Minor(3..0) format
		 */
		mfc0	t0, $15
		nop
		and		t0, t0, 0xFF
		li		t1, 0x21
		blt		t0, t1, copy_workaround_vecs
		nop

        COPY_VECTOR		0x80000000, INT_TLB_Exception, 0x80
        COPY_VECTOR  	0x80000180, INT_General_Exception, 0x20
		COPY_VECTOR 	INTSW0_VEC_BASE, _7k_intsw0_handler, VEC_SPACING
		COPY_VECTOR 	INTSW1_VEC_BASE, _7k_intsw1_handler, VEC_SPACING
		COPY_VECTOR 	INT0_VEC_BASE, _7k_int0_handler, VEC_SPACING
		COPY_VECTOR 	INT1_VEC_BASE, _7k_int1_handler, VEC_SPACING
		COPY_VECTOR 	INT2_VEC_BASE, _7k_int2_handler, VEC_SPACING
		COPY_VECTOR 	INT3_VEC_BASE, _7k_int3_handler, VEC_SPACING
		COPY_VECTOR 	INT4_VEC_BASE, _7k_int4_handler, VEC_SPACING
		COPY_VECTOR 	INT5_VEC_BASE, _7k_int5_handler, VEC_SPACING
		COPY_VECTOR 	INT6_VEC_BASE, _7k_int6_handler, VEC_SPACING
		COPY_VECTOR 	INT7_VEC_BASE, _7k_int7_handler, VEC_SPACING
		COPY_VECTOR 	INT8_VEC_BASE, _7k_int8_handler, VEC_SPACING
		COPY_VECTOR 	INT9_VEC_BASE, _7k_int9_handler, VEC_SPACING
		COPY_VECTOR 	INT10_VEC_BASE, _7k_int10_handler, VEC_SPACING
		COPY_VECTOR 	INT11_VEC_BASE, _7k_int11_handler, VEC_SPACING
		
		b		vec_copy_done
		nop

copy_workaround_vecs:
		COPY_VECTOR		0xA0000000, _7k_fix_INT_TLB_Exception, 0x80
        COPY_VECTOR  	0xA0000180, _7k_fix_INT_General_Exception, 0x20
		COPY_VECTOR 	INTSW0_VEC_BASE, _7k_fix_intsw0_handler, VEC_SPACING
		COPY_VECTOR 	INTSW1_VEC_BASE, _7k_fix_intsw1_handler, VEC_SPACING
		COPY_VECTOR 	INT0_VEC_BASE, _7k_fix_int0_handler, VEC_SPACING
		COPY_VECTOR 	INT1_VEC_BASE, _7k_fix_int1_handler, VEC_SPACING
		COPY_VECTOR 	INT2_VEC_BASE, _7k_fix_int2_handler, VEC_SPACING
		COPY_VECTOR 	INT3_VEC_BASE, _7k_fix_int3_handler, VEC_SPACING
		COPY_VECTOR 	INT4_VEC_BASE, _7k_fix_int4_handler, VEC_SPACING
		COPY_VECTOR 	INT5_VEC_BASE, _7k_fix_int5_handler, VEC_SPACING
		COPY_VECTOR 	INT6_VEC_BASE, _7k_fix_int6_handler, VEC_SPACING
		COPY_VECTOR 	INT7_VEC_BASE, _7k_fix_int7_handler, VEC_SPACING
		COPY_VECTOR 	INT8_VEC_BASE, _7k_fix_int8_handler, VEC_SPACING
		COPY_VECTOR 	INT9_VEC_BASE, _7k_fix_int9_handler, VEC_SPACING
		COPY_VECTOR 	INT10_VEC_BASE, _7k_fix_int10_handler, VEC_SPACING
		COPY_VECTOR 	INT11_VEC_BASE, _7k_fix_int11_handler, VEC_SPACING

vec_copy_done:
#else	/* CONFIG_INT_PRIORITY */

#Initialize the Interupt Handler for RM7000 mode
        la  $8, INT_BASE_7000            # R7K Interrupt handler address
        sd  $0, 0($8)                    # Clear vector area by storing 0's
        sd  $0, 8($8)                    #  
        sd  $0, 16($8)    
        sd  $0, 24($8)
        la  $9, INT_Exception    		 # Address of new handler
        ld  $10, 0($9)                   # Pickup the new handler word
        sd  $10, 0($8)                   # Store into handler area
        ld  $10, 8($9)                   # Pickup the new handler word
        sd  $10, 8($8)                   # Store into handler area
        ld  $10, 16($9)                  # Pickup the new handler word
        sd  $10, 16($8)                  # Store into handler area
        ld  $10, 24($9)                  # Pickup the new handler word
        sd  $10, 24($8)                  # Store into handler area
        nop

#
# Grab TLB Exception vector here.
#
	la  $8, 0x80000000               # TLB exception handler address
        sd  $0, 0($8)                    # Clear vector area by storing 0's
        sd  $0, 8($8)                    #  
        sd  $0, 16($8)    
        sd  $0, 24($8)
        la  $9, INT_TLB_Exception	 # Address of new handler
        ld  $10, 0($9)                   # Pickup the new handler word
        sd  $10, 0($8)                   # Store into handler area
        ld  $10, 8($9)                   # Pickup the new handler word
        sd  $10, 8($8)                   # Store into handler area
        ld  $10, 16($9)                  # Pickup the new handler word
        sd  $10, 16($8)                  # Store into handler area
        ld  $10, 24($9)                  # Pickup the new handler word
        sd  $10, 24($8)                  # Store into handler area
        nop
#endif	/* CONFIG_INT_PRIORITY */
#endif	/* INCLUDE_ODYSSEY */

#
# Clear the entire exception vector region from primary/econdary cache.
# Note: We have written vectors to the main memory. Make sure
# the cached memory for I/D is clean in primary as well as secondary
# cache. The first exception would load the vector to I-cache from 
# main memory.
#
		li		t0, 0x80000000
		li		t1, 0x80002400
pwb_excpt_loop:
		cache	0x11, 0(t0)				# HIT_INVALIDATE_D
		nop
		cache	0x10, 0(t0)				# HIT_INVALIDATE_I
		nop
		addu	t0, t0, 32	
		blt		t0, t1, pwb_excpt_loop
		nop

#ifdef INCLUDE_ODYSSEY

# Flush the secodary cache 
		li		t0, 0x80000000
		li		t1, 0x80002400
swb_excpt_loop:
		cache	0x13, 0(t0)				# HIT_INVALIDATE_S
		addu	t0, t0, 32	
		blt		t0, t1, swb_excpt_loop
		nop
#endif


#
#    /* End 5000 Exception Handler Steal.  */
#

#
#    /* Define the global data structures that need to be initialized by this
#       routine.  These structures are used to define the system timer 
#       management HISR.  */
#    TMD_HISR_Stack_Ptr =        _HISR_Stack;
#    TMD_HISR_Stack_Size =       HISR_SIZE;
#    TMD_HISR_Priority =         HISR_PRIORITY;
#
        la      $8, _hisr_stack_addr        # Build address of HISR stack
        sw      $8, TMD_HISR_Stack_Ptr      # Save the HISR stack address
        la      $8, _hisr_stack_size        # Pickup the HISR stack size
        sw      $8, TMD_HISR_Stack_Size     # Save the HISR stack size
        li      $8, HISR_PRIORITY           # Pickup the HISR priority
        sw      $8, TMD_HISR_Priority       # Save the HISR priority
#
#    /* Initialize internal R5000 timer to interrupt at a periodic rate.  */
#

        li      $8, TIMER_COMPARE           # Build compare value
        mtc0    $8, $11                     # Setup timer compare register
        nop
        nop
        nop
        mtc0    $0, $9                      # Clear counter register
        nop
        nop
        nop

        sw      $sp, TCD_System_Stack       # Save system stack pointer
#
#    /* Call MSL_Initialize with a pointer to the first available memory 
#       address after the compiler's global data.  This memory may be used
#       by the application.  */
#    MSL_Initialize((VOID *) _end);
#
        la      $4, _end                    # Pickup address of available
					    # memory
        jal     MSL_Initialize              # Call MSL initialization function
        nop
        
        jal     INC_Initialize              # Call Nucleus initialization function
        addu    $4, v0, $0                  # MSL_Initialize returned the new available memory
#
#}
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      INT_Vectors_Loaded                                               */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function returns a value of NU_TRUE, since there is really  */
#/*      only one MIPS interrupt vector.                                  */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      William E. Lamie, Accelerated Technology, Inc.                   */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Register_LISR                   Register LISR for vector     */
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
#/*      NU_TRUE                                                          */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      W. Lamie        04-01-1995      Created initial version 1.0      */
#/*      D. Lamie        04-01-1995      Verified version 1.0             */
#/*                                                                       */
#/*************************************************************************/
#INT    INT_Vectors_Loaded(void)
#{
		.align  4
        .globl  INT_Vectors_Loaded
INT_Vectors_Loaded:
#
#    /* Just return a value of NU_TRUE.  */
#    return(NU_TRUE);
#
        j       $31                         # Return to caller
        li      $2, 1                       # Return a value of NU_TRUE

#}
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      INT_Setup_Vector                                                 */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function does nothing in the MIPS port, since the one real  */
#/*      interrupt vector has already been initialized.                   */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      William E. Lamie, Accelerated Technology, Inc.                   */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      Application                                                      */
#/*      TCC_Register_LISR                   Register LISR for vector     */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      vector                              Vector number to setup       */
#/*      new                                 Pointer to new assembly      */
#/*                                            language ISR               */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      NU_NULL                                                          */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      W. Lamie        04-01-1995      Created initial version 1.0      */
#/*      D. Lamie        04-01-1995      Verified version 1.0             */
#/*                                                                       */
#/*************************************************************************/
#VOID  *INT_Setup_Vector(INT vector, VOID *new)
#{
		.align  4
        .globl  INT_Setup_Vector
INT_Setup_Vector:

# 
#    /* Return NU_NULL.  */
#    return(NU_NULL);
#
        j       $31                         # Return to caller
        li      $2, 0                       # Return a NULL

#}
#
#
#/*************************************************************************/
#/*                                                                       */
#/* FUNCTION                                                              */
#/*                                                                       */
#/*      INT_Retrieve_Shell                                               */
#/*                                                                       */
#/* DESCRIPTION                                                           */
#/*                                                                       */
#/*      This function does nothing in the MIPS port, since there is      */
#/*      effectively one interrupt vector and one shell.                  */
#/*                                                                       */
#/* AUTHOR                                                                */
#/*                                                                       */
#/*      Joe Martin, Accelerated Technology, Inc.                         */
#/*                                                                       */
#/* CALLED BY                                                             */
#/*                                                                       */
#/*      TCC_Register_LISR                   Register LISR for vector     */
#/*                                                                       */
#/* CALLS                                                                 */
#/*                                                                       */
#/*      None                                                             */
#/*                                                                       */
#/* INPUTS                                                                */
#/*                                                                       */
#/*      vector                              Vector number to setup       */
#/*                                                                       */
#/* OUTPUTS                                                               */
#/*                                                                       */
#/*      shell pointer                                                    */
#/*                                                                       */
#/* HISTORY                                                               */
#/*                                                                       */
#/*         NAME            DATE                    REMARKS               */
#/*                                                                       */
#/*      J. Martin       10-20-1996      Prepared version 1.0             */
#/*                                                                       */
#/*************************************************************************/
#VOID  *INT_Retrieve_Shell(INT vector)
#{
		.align  4
        .globl  INT_Retrieve_Shell
INT_Retrieve_Shell:
#
#    /* Return NU_NULL.  */
#    return(NU_NULL);
#
        j       $31                         # Return to caller
        li      $2, 0                       # Return a NULL

#}

#/*************************************************************************/
#/*                 E X C E P T I O N   H A N D L E R                     */
#/*                                                                       */
#/*************************************************************************/
#
#/* This is the vector that directs interrupts to Nucleus PLUS.  During
#   initialization it is copied to address 0x80000180 
#                                       or 0xA0000180 */
        .globl  INT_General_Exception
        .align  16   # Align the Exception Handler to a doubleword address.
                     # This is important on the R5000.
                     # It was actually quad aligned, so I left it that way.
INT_General_Exception:
        .option no_at_macros on
        la      $26, INT_General_Exception_Hdlr
        j       $26
        nop
        .option no_at_macros off

#ifndef CONFIG_INT_PRIORITY 
#
#/* The following code provides the General Exception Handler for processing
#   interrupts on the MIPS processor.  */
#
        .globl  INT_General_Exception_Hdlr
INT_General_Exception_Hdlr:
#
#/* Build a mini-stack frame with $at, $31 (ra), $16-$17 (s0-s1) so we can
#   figure out what type of interrupt is present.  */
#
        addiu   $sp, $sp, -16*X             # Allocate room on current stack
        .option no_at_macros on
        sd      $at,  0*X($sp)              # Save $at
        sd      $31,  4*X($sp)              # Save $31 (ra)
        sd      $16,  8*X($sp)              # Save $16 (s0)
        sd      $17, 12*X($sp)              # Save $17 (s1)
        .option no_at_macros off

        mfc0    $16, $13                    # Pickup the Cause Register
#############################################
# Serial interrupt for interrupt driven TRK?
#############################################
#if defined(INCLUDE_EV64120) && defined(TRK_TRANSPORT_INT_DRIVEN)

	li		k1, 0x4000						# Uart Int
	and		k0, k1, $16
	bne		k0, k1, no_uart_int
	nop

	li	k0, 0xBD000000
	li	k1, 0x03
	sb	k1, 8(k0)	
	li	k1, 10
dly:
	bnez		k1, dly
	addiu		k1, k1, -1
	lb		k1, 8(k0)
	li		k0, 0x04
	and		k1, k1, k0
	bnez		k1, INT_Error	
	nop
no_uart_int:
#endif
#############################################
#
#/* Now these registers are available.  Figure out what type of interrupt is
#   present.  */
#

        srl     $17, $16, 2                 # Shift the Exception Code down 2 
        andi    $17, $17, 0x1F              #   and isolate the 5-bit field
        bne     $17, $0, INT_Error          # If non-0, an error or breakpoint
        nop                                 #   interrupt is present.

#
#/* Otherwise, a normal interrupt is present.  Each interrupt possibility must
#   be checked since more than one can be present at a time.  Be sure to 
#   isolate interrupts that are really enabled at this point.  */
#
#        mfc0    $16, $13                    # Pickup the Cause Register again
#        nop                                 # Delay slot
#        nop                                 # Delay slot
        li      $17, PEND_MASK              # Build pending interrupts mask 
        and     $16, $16, $17               # Clear all non-interrupt bits
        mfc0    $17, $12                    # Pickup the current SR
        nop                                 # Delay slot
        nop

#
#/* Call TCT_Interrupt_Context_Save to save the system context.  Note that
#   context saving leaves $16 (s0) intact.  */
#
        jal     TCT_Interrupt_Context_Save  # Call context save
        and     $16, $16, $17               # Only accept the interrupts that
                                            #   are enabled in SR

#/* Test for each type of interrupt.  Note: $16 (s0) contains the enabled-
#   interrupt pending bits of the cause register at the time of the interrupt.
#   Also not that Interrupt Service Routines called from here must observe the 
#   compiler register usage conventions.  */
#
        srl     $17, $16, 8                 # Shift down the interrupt codes
        andi    $18, $17, 1                 # Is Software Interrupt 0 present?
        nop


INT_Check_SW0:
        beq     $18, $0, INT_Check_SW1      # No, check next interrupt
        andi    $18, $17, 2                 # Is Software Interrupt 1 present?
#
#/* Processing to dispatch to LISR entry 6 for processing Software 
#   Interrupt 0.  The cause register is automatically cleared to clear 
#   the sofware interrupt.  */
#
        mtc0    $0, $13                     # Clear Software Interrupt 0
        nop
        nop
        nop
        li      $4, 10                      # Index for Software Interrupt 0
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        nop                                 # Delay slot


INT_Check_SW1:
        beq     $18, $0, INT_Check_INT0     # No, Check for next interrupt
        andi    $18, $17, 4                 # Is Interrupt 0 present?
#
#/* Processing to dispatch to LISR entry 7 for processing Software 
#   Interrupt 1.  The cause register is automatically cleared to clear 
#   the sofware interrupt.  */
#
        mtc0    $0, $13                     # Clear Software Interrupt 0
        nop
        nop
        nop
        li      $4, 11                      # Index for Software Interrupt 1
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        nop                                 # Delay slot


INT_Check_INT0:
        beq     $18, $0, INT_Check_INT1     # No, Check for next interrupt
        andi    $18, $17, 8                 # Is Interrupt 1 present?
#
#/* Processing for Interrupt 0 LISR dispatch (index 0).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 0                       # Index for Interrupt 0
        nop


INT_Check_INT1:
        beq     $18, $0, INT_Check_INT2     # No, Check for next interrupt
        andi    $18, $17, 0x10              # Is Interrupt 2 present?
#
#/* Processing for Interrupt 1 LISR dispatch (index 1).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 1                       # Index for Interrupt 1
        nop


INT_Check_INT2:
        beq     $18, $0, INT_Check_INT3     # No, Check for next interrupt
        andi    $18, $17, 0x20              # Is Interrupt 3 present?
#
#/* Processing for Interrupt 2 LISR dispatch (index 2).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 2                       # Index for Interrupt 2
        nop


INT_Check_INT3:
        beq     $18, $0, INT_Check_INT4     # No, Check for next interrupt
        andi    $18, $17, 0x40              # Is Interrupt 4 present?
#
#/* Processing for Interrupt 3 LISR dispatch (index 3).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 3                       # Index for Interrupt 3
        nop


INT_Check_INT4:

        beq     $18, $0, INT_Check_INT5     # No, Check for next interrupt
        andi    $18, $17, 0x80              # Is Interrupt 5 present?

#
#/* Processing for Interrupt 4 LISR dispatch (index 4).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 4                       # Index for Interrupt 4
        nop


INT_Check_INT5:
        beq     $18, $0, INT_Int_Finished   # Finished, if not set
        nop                                 # Delay slot
#
#/* Processing for Interrupt 5 LISR dispatch (index 5).  */
#
#       jal     TCC_Dispatch_LISR           # Call LISR dispatch function
#       li      $4, 5                       # Index for Interrupt 5
#       nop
#
#/* Timer processing is placed here for using the R5000 on-chip timer
#   as the Nucleus system timer.  It is optionally connected to Int5.
#   This code starts processing of all timer and time-slice expirations. */
#/* Clear the R5000 timer and prepare for next interrupt.  */
#
INT_Timer_Repeat:
        mfc0    $19, $11                     # Pickup current compare contents
        nop                          
        nop
        li      $9, TIMER_COMPARE           # Build compare value
        nop                                 # Delay 
        nop                                 # Delay
        addu    $19, $19, $9                  # Build new compare value
        mtc0    $19, $11                     # Set up new compare value.  Note:
        nop                                 #   this also clears the current
        nop                                 #   timer interrupt.
        nop                                 # Delay

# Calculate cpu statistics
        lw      $4, TCD_Execute_HISR        # Pickup the highest priority HISR
        beq		$4, $0, INT_Timer_Task_Interrupted
        # Interrupted an HISR
        lw		$4, TCD_HISR_COUNTER
        addi	$4, 1
		b		INT_Timer_Test
        sw		$4, TCD_HISR_COUNTER

INT_Timer_Task_Interrupted:
        lw      $4, TCD_Execute_Task        # Pickup the highest priority task
        beq		$4, $0, INT_Timer_Idle
        # Interrupted a Task
        lw		$4, TCD_TASK_COUNTER
        addi	$4, 1
		b		INT_Timer_Test
        sw		$4, TCD_TASK_COUNTER

INT_Timer_Idle:
		# Neither an HISR or Task was interrupted.  Must be in TCT_Schedule, idle.
        lw		$4, TCD_IDLE_COUNTER
        addi	$4, 1
        sw		$4, TCD_IDLE_COUNTER

INT_Timer_Test:
        jal     TMT_Timer_Interrupt         # Call timer interrupt handler
        nop                                 # Delay slot
		mfc0	$8, $9
		nop
		nop
		subu	$8, $8, $19
        bgt     $8, $0, INT_Timer_Repeat   # Count already exceeded next match?  Do it again.
		nop

INT_Int_Finished:
#
#/* Interrupt processing is complete.  Close interrupt processing with a call 
#   to context restore.  */
#
        j       TCT_Interrupt_Context_Restore
        nop


INT_Error:
#
#/* IDT79S465-specific code.  All non-interrupt exceptions are 
#   directed to the IDT/sim monitor.  This code is setup for version 7.0, 64
#   bit, beta of the IDT/sim monitor.  Other versions might require the
#   following code to be modified.  */
#
        .option no_at_macros on
        ld      $at,   0*X($sp)             # Recover $at
        ld      $31,   4*X($sp)             # Recover $31 (ra)
        ld      $16,   8*X($sp)             # Recover $16 (s0)
        ld      $17,  12*X($sp)             # Recover $17 (s1)
        addiu   $sp, $sp, 16*X              # Recover stack space
        .option no_at_macros off


INT_Monitor:
#
#/* The following code must match that of address 80000180 prior to 
#   loading and running Nucleus.  */
#

.if (INCLUDE_TRK)
#
#/* TRK SPECIFIC CODE - Code at 0x80000180 for MetroTRK on the EV-64120 */
#
        lui     $26, 0x8000                 # copied from address 0x80000180
        addiu   $26, $26, 0x0080            # copied from address 0x80000184
        jr      $26                         # copied from address 0x80000188
        nop                                 # copied from address 0x8000018C
.endif

.if (INCLUDE_IDT)
#
#/* IDT SPECIFIC CODE - Code at 0x80000180 for IDTsim on the EV-64120 */
#
        .option no_at_macros on
        lui     $26, 0xa001                 # copied from address 0x80000180
        daddiu  $26, $26, 0xffff8640        # copied from address 0x80000184
        sd      $1,  0x8($26)               # copied from address 0x80000188
        sd      $28, 0xe0($26)              # copied from address 0x8000018C
        sd      $2,  0x10($26)              # copied from address 0x80000190
        li      $2,  0x1                    # copied from address 0x80000194
        lui     $1,  0xbfc0                 # copied from address 0x80000198
        daddiu  $1,  $1, 0xfac              # copied from address 0x8000019C
        jr      $1                          # copied from address 0x800001A0
        nop                                 # copied from address 0x800001A4
        .option no_at_macros off
.endif

#
#/* Error processing is user-specified.  By default, just stick here in a 
#   loop.  */
#
INT_Error_Loop:
        j       INT_Error_Loop
        nop
#endif	/* CONFIG_INT_PRIORITY */

#ifdef INCLUDE_ODYSSEY
# ----------------------------------------------------------------
# 
# 			TLB Exception Vector
# 	
# ----------------------------------------------------------------
	.globl  INT_TLB_Exception
	.extern  tlb_exception_handler
	.align 16 
INT_TLB_Exception:
	.option no_at_macros on
	la      $26, tlb_exception_handler
	j       $26
	nop
	.option no_at_macros off
#endif	/* INCLUDE_ODYSSEY */

#if defined(INCLUDE_ODYSSEY) && !defined (CONFIG_INT_PRIORITY)
#/*************************************************************************/
#/*                 R7K  I N T E R R U P T  H A N D L E R                 */
#/*                                                                       */
#/*************************************************************************/
#
#/* This is the vector that directs interrupts to Nucleus PLUS.  During
#   initialization it is copied to address 0x80000200 
#                                       or 0xA0000200 */
        .globl  INT_Exception
        .align  16   # Align the Exception Handler to a doubleword address.
                     # This is important on the R7000.
                     # It was actually quad aligned, so I left it that way.
INT_Exception:
        .option no_at_macros on
        la      $26, INT_Exception_Hdlr
        j       $26
        nop
        .option no_at_macros off

#
#/* The following code provides the Interrupt Exception Handler for processing
#   interrupts on the R7K MIPS processor.  */
#
        .globl  INT_Exception_Hdlr
INT_Exception_Hdlr:
#
#/* Build a mini-stack frame with $at, $31 (ra), $16-$17 (s0-s1) so we can
#   figure out what type of interrupt is present.  */
#
        addiu   $sp, $sp, -16*X             # Allocate room on current stack
        .option no_at_macros on
        sd      $at,  0*X($sp)              # Save $at
        sd      $31,  4*X($sp)              # Save $31 (ra)
        sd      $16,  8*X($sp)              # Save $16 (s0)
        sd      $17, 12*X($sp)              # Save $17 (s1)
        .option no_at_macros off

#
#/* Now these registers are available.  Figure out what type of interrupt is
#   present.  */
#

        mfc0    $16, $13                    # Pickup the Cause Register
        nop                                 # Delay
        nop                                 # Delay
		nop

        srl     $17, $16, 2                 # Shift the Exception Code down 2 
        andi    $17, $17, 0x1F              #   and isolate the 5-bit field
        bne     $17, $0, INT_Invalid_Error  # If non-0, an error or breakpoint
        nop                                 #   interrupt is present.

#############################################
# Serial interrupt for interrupt driven TRK?
#############################################
#if defined(INCLUDE_ODYSSEY) && defined(TRK_TRANSPORT_INT_DRIVEN)
#ifdef CONFIG_E1
		li		k1, 0x2000						# Uart Int
#else
		li		k1, 0x4000						# Uart Int
#endif
		and		k0, k1, $16
		bne		k0, k1, no_uart_int
		nop
		li		k0, 0xBC010005
		lb		k1, 0(k0)
		andi	k1, k1, 0x01
		bnez	k1, INT_Invalid_Error
		nop
no_uart_int:
#endif
#
#/* Otherwise, a normal interrupt is present.  Each interrupt possibility must
#   be checked since more than one can be present at a time.  Be sure to 
#   isolate interrupts that are really enabled at this point.  */
#
#        mfc0    $16, $13                    # Pickup the Cause Register again
        nop                                 # Delay slot
        nop                                 # Delay slot
        li      $17, PEND_MASK_7000         # Build pending interrupts mask 
        and     $16, $16, $17               # Clear all non-interrupt bits

        mfc0    $17, $12                    # Pickup the current SR
        nop                                 # Delay slot
        nop
		li		$31, 0xFF00
		and		$17, $17, $31

		cfc0	$31, $20					# Get the Interrupt Control Reg
		nop
		nop
		nop
		sll		$31, $31, 8
		
		or		$17, $17, $31				# Carete the entire Mask
		
#
#/* Call TCT_Interrupt_Context_Save to save the system context.  Note that
#   context saving leaves $16 (s0) intact.  */
#
        jal     TCT_Interrupt_Context_Save  # Call context save
        and     $16, $16, $17               # Only accept the interrupts that
                                            #   are enabled in SR and IC

#/* Test for each type of interrupt.  Note: $16 (s0) contains the enabled-
#   interrupt pending bits of the cause register at the time of the interrupt.
#   Also not that Interrupt Service Routines called from here must observe the 
#   compiler register usage conventions.  */
#
        srl     $17, $16, 8                 # Shift down the interrupt codes
        andi    $18, $17, 1                 # Is Software Interrupt 0 present?
        nop


INT_R7KCheck_SW0:
        beq     $18, $0, INT_R7KCheck_SW1      # No, check next interrupt
        andi    $18, $17, 2                 # Is Software Interrupt 1 present?
#
#/* Processing to dispatch to LISR entry 6 for processing Software 
#   Interrupt 0.  The cause register is automatically cleared to clear 
#   the sofware interrupt.  */
#
		li		$19, 0x1000000
		mtc0	$19, $13
        nop
        nop
        nop
        li      $4, 10                       # Index for Software Interrupt 0
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        nop                                 # Delay slot


INT_R7KCheck_SW1:
        beq     $18, $0, INT_R7KCheck_INT0     # No, Check for next interrupt
        andi    $18, $17, 4                 # Is Interrupt 0 present?
#
#/* Processing to dispatch to LISR entry 7 for processing Software 
#   Interrupt 1.  The cause register is automatically cleared to clear 
#   the sofware interrupt.  */
#
		li		$19, 0x1000000
		mtc0	$19, $13
        nop
        nop
        nop
        li      $4, 11                      # Index for Software Interrupt 1
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        nop                                 # Delay slot


INT_R7KCheck_INT0:
        beq     $18, $0, INT_R7KCheck_INT1     # No, Check for next interrupt
        andi    $18, $17, 8                 # Is Interrupt 1 present?
#
#/* Processing for Interrupt 0 LISR dispatch (index 0).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 0                       # Index for Interrupt 0
        nop


INT_R7KCheck_INT1:
        beq     $18, $0, INT_R7KCheck_INT2     # No, Check for next interrupt
        andi    $18, $17, 0x10              # Is Interrupt 2 present?
#
#/* Processing for Interrupt 1 LISR dispatch (index 1).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 1                       # Index for Interrupt 1
        nop


INT_R7KCheck_INT2:
        beq     $18, $0, INT_R7KCheck_INT3     # No, Check for next interrupt
        andi    $18, $17, 0x20              # Is Interrupt 3 present?
#
#/* Processing for Interrupt 2 LISR dispatch (index 2).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 2                       # Index for Interrupt 2
        nop


INT_R7KCheck_INT3:
        beq     $18, $0, INT_R7KCheck_INT4     # No, Check for next interrupt
        andi    $18, $17, 0x40              # Is Interrupt 4 present?
#
#/* Processing for Interrupt 3 LISR dispatch (index 3).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 3                       # Index for Interrupt 3
        nop


INT_R7KCheck_INT4:
        beq     $18, $0, INT_R7KCheck_INT5     # No, Check for next interrupt
        andi    $18, $17, 0x80              # Is Interrupt 5 present?


/*
 * When we interrupt driven UART in TRK
 */
#
#/* Processing for Interrupt 4 LISR dispatch (index 4).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 4                       # Index for Interrupt 4
        nop


INT_R7KCheck_INT5:
        beq     $18, $0, INT_R7KCheck_INT6     # No, Check for next interrupt
        andi	$18, $17, 0x100             # Is Interrupt 6 present?
#
#/* Processing for Interrupt 5 LISR dispatch (index 5).  */
#
#       jal     TCC_Dispatch_LISR           # Call LISR dispatch function
#       li      $4, 5                       # Index for Interrupt 5
#       nop
#
#/* Timer processing is placed here for using the R5000 on-chip timer
#   as the Nucleus system timer.  It is optionally connected to Int5.
#   This code starts processing of all timer and time-slice expirations. */
#/* Clear the R5000 timer and prepare for next interrupt.  */
#
INT_R7KTimer_Repeat:

        mfc0    $19, $11                     # Pickup current compare contents
        nop                          
        nop
        li      $9, TIMER_COMPARE           # Build compare value
        nop                                 # Delay 
        nop                                 # Delay
        addu    $19, $19, $9                  # Build new compare value
        mtc0    $19, $11                     # Set up new compare value.  Note:
        nop                                 #   this also clears the current
        nop                                 #   timer interrupt.
        nop                                 # Delay

# Calculate cpu statistics
        lw      $4, TCD_Execute_HISR        # Pickup the highest priority HISR
        beq		$4, $0, INT_R7KTimer_Task_Interrupted
        # Interrupted an HISR
        lw		$4, TCD_HISR_COUNTER
        addi	$4, 1
		b		INT_R7KTimer_Test
        sw		$4, TCD_HISR_COUNTER

INT_R7KTimer_Task_Interrupted:
        lw      $4, TCD_Execute_Task        # Pickup the highest priority task
        beq		$4, $0, INT_R7KTimer_Idle
        # Interrupted a Task
        lw		$4, TCD_TASK_COUNTER
        addi	$4, 1
		b		INT_R7KTimer_Test
        sw		$4, TCD_TASK_COUNTER

INT_R7KTimer_Idle:
		# Neither an HISR or Task was interrupted.  Must be in TCT_Schedule, idle.
        lw		$4, TCD_IDLE_COUNTER
        addi	$4, 1
        sw		$4, TCD_IDLE_COUNTER

INT_R7KTimer_Test:
        jal     TMT_Timer_Interrupt         # Call timer interrupt handler
        nop                                 # Delay slot
		mfc0	$8, $9
		nop
		nop
		subu	$8, $8, $19
        bgt     $8, $0, INT_R7KTimer_Repeat   # Count already exceeded next match?  Do it again.
		nop

INT_R7KCheck_INT6:
        beq     $18, $0, INT_R7KCheck_INT7     # No, Check for next interrupt
        andi    $18, $17, 0x200             # Is Interrupt 7 present?
#
#/* Processing for Interrupt 6 LISR dispatch (index 6).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 6                       # Index for Interrupt 6
        nop

INT_R7KCheck_INT7:
        beq     $18, $0, INT_R7KCheck_INT8     # No, Check for next interrupt
        andi    $18, $17, 0x400             # Is Interrupt 8 present?
#
#/* Processing for Interrupt 7 LISR dispatch (index 7).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 7                       # Index for Interrupt 7
        nop

INT_R7KCheck_INT8:
        beq     $18, $0, INT_R7KCheck_INT9     # No, Check for next interrupt
        andi    $18, $17, 0x800             # Is Interrupt 9 present?
#
#/* Processing for Interrupt 8 LISR dispatch (index 8).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 8                       # Index for Interrupt 8
        nop

INT_R7KCheck_INT9:
        beq     $18, $0, INT_R7KInt_Finished   # Finished, if not set
        nop	                                # Delay slot
#
#/* Processing for Interrupt 9 LISR dispatch (index 9).  */
#
        jal     TCC_Dispatch_LISR           # Call LISR dispatch function
        li      $4, 9                       # Index for Interrupt 9
        nop

INT_R7KInt_Finished:
#
#/* Interrupt processing is complete.  Close interrupt processing with a call 
#   to context restore.  */
#
        j       TCT_Interrupt_Context_Restore
        nop


INT_Invalid_Error:
#
#/* IDT79S465-specific code.  All non-interrupt exceptions are 
#   directed to the IDT/sim monitor.  This code is setup for version 7.0, 64
#   bit, beta of the IDT/sim monitor.  Other versions might require the
#   following code to be modified.  */
#
        .option no_at_macros on
        ld      $at,   0*X($sp)             # Recover $at
        ld      $31,   4*X($sp)             # Recover $31 (ra)
        ld      $16,   8*X($sp)             # Recover $16 (s0)
        ld      $17,  12*X($sp)             # Recover $17 (s1)
        addiu   $sp, $sp, 16*X              # Recover stack space
        .option no_at_macros off


#
#/* The following code must match that of address 80000180 prior to 
#   loading and running Nucleus.  */
#

.if (INCLUDE_TRK)
#
#/* TRK SPECIFIC CODE - Code at 0x80000180 for MetroTRK on the EV-64120 */
#
        lui     $26, 0x8000                 # copied from address 0x80000180
        addiu   $26, $26, 0x0080            # copied from address 0x80000184
        jr      $26                         # copied from address 0x80000188
        nop                                 # copied from address 0x8000018C
.endif

.if (INCLUDE_IDT)
#
#/* IDT SPECIFIC CODE - Code at 0x80000180 for IDTsim on the EV-64120 */
#
        .option no_at_macros on
        lui     $26, 0xa001                 # copied from address 0x80000180
        daddiu  $26, $26, 0xffff8640        # copied from address 0x80000184
        sd      $1,  0x8($26)               # copied from address 0x80000188
        sd      $28, 0xe0($26)              # copied from address 0x8000018C
        sd      $2,  0x10($26)              # copied from address 0x80000190
        li      $2,  0x1                    # copied from address 0x80000194
        lui     $1,  0xbfc0                 # copied from address 0x80000198
        daddiu  $1,  $1, 0xfac              # copied from address 0x8000019C
        jr      $1                          # copied from address 0x800001A0
        nop                                 # copied from address 0x800001A4
        .option no_at_macros off
.endif

#
#/* Error processing is user-specified.  By default, just stick here in a 
#   loop.  */
#
INT_Invalid_Error_Loop:
        j       INT_Invalid_Error_Loop
        nop

#endif	/* defined(INCLUDE_ODYSSEY) && !defined(CONFIG_INT_PRIORITY) */

#if defined(INCLUDE_ODYSSEY) && defined(CONFIG_INT_PRIORITY)

PROCESS_TIMER_INT: .macro
__INT_R7KTimer_Repeat_\@:
        mfc0    $19, $11                     # Pickup current compare contents
        nop                          
        nop
        li      $9, TIMER_COMPARE           # Build compare value
        nop                                 # Delay 
        nop                                 # Delay
        addu    $19, $19, $9                  # Build new compare value
        mtc0    $19, $11                     # Set up new compare value.  Note:
        nop                                 #   this also clears the current
        nop                                 #   timer interrupt.
        nop                                 # Delay

# Calculate cpu statistics
        lw      $4, TCD_Execute_HISR        # Pickup the highest priority HISR
        beq		$4, $0, __INT_R7KTimer_Task_Interrupted_\@
#        nop - removed to optimize busy case
        # Interrupted an HISR
        lw		$4, TCD_HISR_COUNTER
        addi	$4, 1
		b		__INT_R7KTimer_Test_\@
        sw		$4, TCD_HISR_COUNTER

__INT_R7KTimer_Task_Interrupted_\@:
        lw      $4, TCD_Execute_Task        # Pickup the highest priority task
        beq		$4, $0, __INT_R7KTimer_Idle_\@
#        nop - removed to optimize busy case
        # Interrupted a Task
        lw		$4, TCD_TASK_COUNTER
        addi	$4, 1
		b		__INT_R7KTimer_Test_\@
        sw		$4, TCD_TASK_COUNTER

__INT_R7KTimer_Idle_\@:
		# Neither an HISR or Task was interrupted.  Must be in TCT_Schedule, idle.
        lw		$4, TCD_IDLE_COUNTER
        addi	$4, 1
        sw		$4, TCD_IDLE_COUNTER

__INT_R7KTimer_Test_\@:
        jal     TMT_Timer_Interrupt         # Call timer interrupt handler
        nop                                 # Delay slot
		mfc0	$8, $9
		nop
		nop
		subu	$8, $8, $19
        bgt     $8, $0, __INT_R7KTimer_Repeat_\@
		nop
	.endm

/*
 * Check for input on PORTB Uart.
 */
CHECK_DBG_INT: .macro
#ifdef TRK_TRANSPORT_INT_DRIVEN
		li		k0, 0xBC010005
		lb		k1, 0(k0)
		andi	k1, k1, 0x01
		beqz	k1, no_dbg_int_\@
		nop
		la		k1, _7k_fix_INT_General_Exception
		jr		k1
		nop
no_dbg_int_\@:
#endif
	.endm

/*
 * Fix for Errata #13. We save hi, lo, perform a dummy multiply and restore
 * them.
 * Note. This also covers Errata #7. Having 2 instructions which may safely
 * be executed twice 
 */
dummy_mul: .macro	reg1, reg2
		mfhi	reg1
		mflo	reg2
		nop
		nop
		multu	reg1, reg2
		nop
		nop
		mthi	reg1
		mtlo	reg2
		nop
		nop
	.endm

CONTEXT_SAVE: .macro int_no
		/*
		 * TCT_Interrupt_Context_Save expects ra,s0,at to be saved
		 */
        addiu   $sp, $sp, -16*X             # Allocate room on current stack
		sd      $at,  0*X($sp)              # Save $at
		sd      $31,  4*X($sp)              # Save $31 (ra)
		sd      $16,  8*X($sp)              # Save $16 (s0)

		jal		TCT_Interrupt_Context_Save
		sd      $17, 12*X($sp)              # Save $17 (s1)
 .endm

CONTEXT_RESTORE: .macro
		jal		TCT_Interrupt_Context_Restore
		nop
 .endm

PROCESS_INT: .macro int_no
		CONTEXT_SAVE	int_no

		jal		TCC_Dispatch_LISR
		li		$4, int_no

		CONTEXT_RESTORE
 .endm

_FIX_PROCESS_INT: .macro int_no
		.option reorder off


		/*
		 * Fix for Errata #7. First 2 instructions may be executed twice.
		 * Place 2 instructions here which are safe for execution twice 
		 */
		nop
		nop

		CONTEXT_SAVE	int_no

		/*
		 * Fix for Errata #13. Dummy Multiply. At this point lo, hi are
		 * saved. Do the dummy multiply. CONTEXT_RESTORE would
		 * restore lo, hi 
		 */
		multu	$16, $17

		jal		TCC_Dispatch_LISR
		li		$4, int_no

		CONTEXT_RESTORE
 .endm

		.option reorder off
        .option no_at_macros on

/*
 * SW INT 0
 */
		.align	16
		.globl	_7k_intsw0_handler
_7k_intsw0_handler:
		li		k0, 0x1000000
		mtc0	k0, $13
		nop
		nop
		nop
		PROCESS_INT	10

/*
 * SW INT 1
 */
		.align	16
		.globl	_7k_intsw1_handler
_7k_intsw1_handler:
		li		k0, 0x1000000
		mtc0	k0, $13
		nop
		nop
		nop
		PROCESS_INT	11 

		.align	16
		.globl	_7k_int0_handler
_7k_int0_handler:
		PROCESS_INT	0x00

		.align	16
		.globl	_7k_int1_handler
_7k_int1_handler:
		PROCESS_INT	1

		.align	16
		.globl	_7k_int2_handler
_7k_int2_handler:
		PROCESS_INT	2

		.align	16
		.globl	_7k_int3_handler
_7k_int3_handler:
		PROCESS_INT	3

		.align	16
		.globl	_7k_int4_handler
_7k_int4_handler:
		CHECK_DBG_INT
		PROCESS_INT	4

		.align	16
		.globl	_7k_int5_handler
_7k_int5_handler:
		CONTEXT_SAVE	5

		.option	no_at_macros off
		PROCESS_TIMER_INT
		.option	no_at_macros on
		CONTEXT_RESTORE

		.align	16
		.globl	_7k_int6_handler
_7k_int6_handler:
		PROCESS_INT	6

		.align	16
		.globl	_7k_int7_handler
_7k_int7_handler:
		PROCESS_INT	7

		.align	16
		.globl	_7k_int8_handler
_7k_int8_handler:
		PROCESS_INT	8

		.align	16
		.globl	_7k_int9_handler
_7k_int9_handler:
		PROCESS_INT	9

	/*
	 * We don't support the following 2 interrupts. Should not
	 * get any interrupts. If we do get an interrupt, jump to debugger
	 * thru' general exception handler
	 */
		.align	16
		.globl	_7k_int10_handler
_7k_int10_handler:							/* Alternate Timer interrupt */
		la	k0, INT_General_Exception_Hdlr
		jr	k0
		nop

		.align	16
		.globl	_7k_int11_handler
_7k_int11_handler:							/* Performance Counter interrupt */
		la	k0, INT_General_Exception_Hdlr
		jr	k0
		nop

		.align 16
		.globl INT_General_Exception_Hdlr
INT_General_Exception_Hdlr:
        lui     $26, 0x8000                 # copied from address 0x80000180
        addiu   $26, $26, 0x0080            # copied from address 0x80000184
        jr      $26                         # copied from address 0x80000188
        nop  
		
/*
 * ******************************************************************
 * RM 7000 Interrupt vectors with software workarounds
 * ******************************************************************
 */
		.align	16
		.globl	_7k_fix_intsw0_handler
_7k_fix_intsw0_handler:
		li		k0, 0x1000000
		mtc0	k0, $13
		nop
		nop
		nop
		_FIX_PROCESS_INT	10

		.align	16
		.globl	_7k_fix_intsw1_handler
_7k_fix_intsw1_handler:
		li		k0, 0x1000000
		mtc0	k0, $13
		nop
		nop
		nop
		_FIX_PROCESS_INT	11 

		.align	16
		.globl	_7k_fix_int0_handler
_7k_fix_int0_handler:
		_FIX_PROCESS_INT	0x00

		.align	16
		.globl	_7k_fix_int1_handler
_7k_fix_int1_handler:
		_FIX_PROCESS_INT	1

		.align	16
		.globl	_7k_fix_int2_handler
_7k_fix_int2_handler:
		_FIX_PROCESS_INT	2

		.align	16
		.globl	_7k_fix_int3_handler
_7k_fix_int3_handler:
		_FIX_PROCESS_INT	3

		.align	16
		.globl	_7k_fix_int4_handler
_7k_fix_int4_handler:
		CHECK_DBG_INT
		_FIX_PROCESS_INT	4

		.align	16
		.globl	_7k_fix_int5_handler
_7k_fix_int5_handler:
		nop								# Fix for Errate #7. 
		nop
		CONTEXT_SAVE	5
		multu	$16, $17				# Fix for Errata #13.  Dummy mul

		.option	no_at_macros off
		PROCESS_TIMER_INT
		.option	no_at_macros on
		CONTEXT_RESTORE

		.align	16
		.globl	_7k_fix_int6_handler
_7k_fix_int6_handler:
		_FIX_PROCESS_INT	6

		.align	16
		.globl	_7k_fix_int7_handler
_7k_fix_int7_handler:
		_FIX_PROCESS_INT	7

		.align	16
		.globl	_7k_fix_int8_handler
_7k_fix_int8_handler:
		_FIX_PROCESS_INT	8

		.align	16
		.globl	_7k_fix_int9_handler
_7k_fix_int9_handler:
		_FIX_PROCESS_INT	9

	/*
	 * We don't support the following 2 interrupts. Should not
	 * get any interrupts. If we do get an interrupt, jump to debugger
	 * thru' general exception handler
	 */
		.align	16
		.globl	_7k_fix_int10_handler
_7k_fix_int10_handler:						/* Alternate Timer interrupt */
		la		k0, _7k_fix_INT_General_Exception
		jr		k0
		nop

		.align	16
		.globl	_7k_fix_int11_handler
_7k_fix_int11_handler:						/* Performance Counter interrupt */
		la		k0, _7k_fix_INT_General_Exception
		jr		k0
		nop

		.align 16
		.globl _7k_fix_INT_General_Exception_Hdlr
_7k_fix_INT_General_Exception_Hdlr:
		dummy_mul	k0, k1
        lui     $26, 0x8000                 # copied from address 0x80000180
        addiu   $26, $26, 0x0080            # copied from address 0x80000184
        jr      $26                         # copied from address 0x80000188
		nop

		.align 16
		.globl _7k_fix_INT_General_Exception
_7k_fix_INT_General_Exception:
        la		$26, _7k_fix_INT_General_Exception_Hdlr
        jr		$26
        nop

		.align 16
		.globl  _7k_fix_INT_TLB_Exception
_7k_fix_INT_TLB_Exception:
		.option no_at_macros on
		dummy_mul	k0, k1
		la		$26, tlb_exception_handler
		jr		$26
		nop

#endif	/* defined(INCLUDE_ODYSSEY) && defined(CONFIG_INT_PRIORITY) */
		
#
# _copy_vector(dst, src, len)
#
_copy_vector:
		.option no_at_macros off
_cvl1:
		li		t0, 0
		lw		a3, 0(a0)
		sw		a3, 0(a1)
		addi	a0, a0, 4
		addi	a1, a1, 4
		sub		a2, a2, 4
		bgt		a2, zero, _cvl1
		nop

		jr		ra
		nop


	.global	_7k_get_icr
_7k_get_icr:
		cfc0	v0, $20
		nop
		nop
		jr		ra
		nop

	.global	_7k_get_iplo
_7k_get_iplo:
		cfc0	v0, $18
		nop
		nop
		jr		ra
		nop

		.global	_7k_get_iphi
_7k_get_iphi:
		cfc0	v0, $19
		nop
		nop
		jr		ra
		nop

        .globl  set_interrupt_level
set_interrupt_level:
        la	t0, TCD_Interrupt_Level
		li	t1, NU_ENABLE_INTERRUPTS
        sw	t1, 0(t0)

        la	t0, TCD_R7KInterrupt_Level
		li	t1, NU_ENABLE_R7KINTERRUPTS
        sw	t1, 0(t0)
		
		jr	ra
        nop
