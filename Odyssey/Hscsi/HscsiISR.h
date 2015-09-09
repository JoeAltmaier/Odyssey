/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiISR.h
// 
// Description:
// This file defines the Interrupt Service Routine interfaces. 
// 
// Update Log 
//	$Log: /Gemini/Odyssey/Hscsi/HscsiISR.h $
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiISR_H)
#define HscsiISR_H

#include "HscsiISP.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Name assigned to HISR
#define HSCSI_HISR_NAME "HSCSIHISR"

// #define ISP_INTERRUPT_VECTOR_NUMBER 0 // TODO FIND OUT WHAT THIS IS
#define HSCSI_HISR_PRIORITY 2 // 2 is the lowest priority, 0 is the highest

STATUS	HSCSI_ISR_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_ISR_Destroy();

// here so everyone can throw an event
void	HSCSI_Handle_Async_Event(PHSCSI_INSTANCE_DATA Id,
					HSCSI_EVENT_ACTION action,
					U16 event_code);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiISR_H  */
