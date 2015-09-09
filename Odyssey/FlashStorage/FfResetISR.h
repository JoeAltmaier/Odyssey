/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfResetISR.h
// 
// Description:
// This file defines the Interrupt Service Routine interfaces
// used by the flash device. 
// 
// 5/11/99 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FfResetISR_H)
#define FfResetISR_H

#include "FlashDevice"

// Name assigned to HISR
#define FF_RESET_HISR_NAME "FRHISR"
#define FF_RESET_HISR_STACK_SIZE 2000

#ifdef SIM
// For simulated system, use software interrupts
#define FF_RESET_INTERRUPT_VECTOR_NUMBER 7 
#else
#define FF_RESET_INTERRUPT_VECTOR_NUMBER 2
#endif

#define FF_RESET_HISR_PRIORITY 0 // 2 is the lowest priority, 0 is the highest

// The FF_INTERRUPT_CALLBACK is called by the interrupt handler when an
// interrupt occurs for a device.
typedef void FF_INTERRUPT_CALLBACK(U32 device);

Status	FF_Reset_ISR_Open(FF_Mem *p_mem, FF_INTERRUPT_CALLBACK *p_interrupt_callback);
Status	FF_Reset_ISR_Close();


#endif /* FfResetISR_H  */
