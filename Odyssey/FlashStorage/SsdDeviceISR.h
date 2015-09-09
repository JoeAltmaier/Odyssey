/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDeviceISR.h
// 
// Description:
// This file defines the Interrupt Service Routine interfaces
// used by the flash device. 
// 
// 8/20/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(SsdDeviceISR_H)
#define SsdDeviceISR_H

#include "FlashDevice.h"

// Name assigned to HISR
#define FF_HISR_NAME "FBHISR"
#define FF_HISR_STACK_SIZE 2000

#ifdef SIM
// For simulated system, use software interrupts
#define FF_INTERRUPT_VECTOR_NUMBER 7 
#else
#define FF_INTERRUPT_VECTOR_NUMBER 9
#endif

#define FF_HISR_PRIORITY 2 // 2 is the lowest priority, 0 is the highest

// The FF_INTERRUPT_CALLBACK is called by the interrupt handler when an
// interrupt occurs for a device.
typedef void FF_INTERRUPT_CALLBACK(U32 device, UI64 device_status);

Status	FF_ISR_Open(FF_Mem *p_mem, FF_INTERRUPT_CALLBACK *p_interrupt_callback);
Status	FF_ISR_Close();

U32 FF_ISR_Memory_Required();

// Get interrupt status from FPGA status
void FF_Get_Interrupt_Status(FF_INTERRUPT_STATUS *p_interrupt_status);


#endif /* SsdDeviceISR_H  */
