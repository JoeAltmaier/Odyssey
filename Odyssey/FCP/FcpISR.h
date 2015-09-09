/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpISR.h
// 
// Description:
// This file defines the Interrupt Service Routine interfaces. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/FCP/FcpISR.h $
// 
// 8     7/23/99 1:43p Mpanas
// fix for Immediate Notify race
// 
// 7     7/15/99 11:38p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// 
// 6     6/06/99 4:20p Mpanas
// 2200 cleanup
//
// 5/7/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
// 11/30/98 Michael G. Panas: new async event handler
/*************************************************************************/

#if !defined(FcpISR_H)
#define FcpISR_H

#include "FcpISP.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


#if defined(_ODYSSEY)
// Odyssey
#if defined(_NAC)
// NAC
// Names assigned to HISR
#define FCP_HISR_NAME "FCHISR"
// later
#define FCP_HISR_NAME0 "FCHISR0"
#define FCP_HISR_NAME1 "FCHISR1"
#define FCP_HISR_NAME2 "FCHISR2"

#define	ISP_INTERRUPT_I0	9				// Instance 0 (chip 0) interrupt vector
#define	ISP_INTERRUPT_I1	8				// Instance 1 (chip 1) interrupt vector
#define	ISP_INTERRUPT_I2	7				// Instance 2 (chip 2) interrupt vector
#else
// RAC
// Names assigned to HISR
#define FCP_HISR_NAME "FCHISR"

#define	ISP_INTERRUPT_I0	2				// Instance 0 (chip 0) interrupt vector
#define	ISP_INTERRUPT_I1	2				// Instance 1 (chip 1) interrupt vector
#define	ISP_INTERRUPT_I2	2				// Instance 2 (chip 2) interrupt vector
#endif


#else
// Eval PCI Interrupt values
// These all must be 0, since the QLA2100/2200 can only assert INTA*
#define	ISP_INTERRUPT_I0	0				// Instance 0 (chip 0) interrupt vector
#define	ISP_INTERRUPT_I1	0				// Instance 1 (chip 1) interrupt vector
#define	ISP_INTERRUPT_I2	0				// Instance 2 (chip 2) interrupt vector

// Name assigned to HISR
#define FCP_HISR_NAME "FCHISR"
#endif

// all chips have the save priority
// 2 is the lowest priority, 0 is the highest
#define FCP_HISR_PRIORITY	2

STATUS	FCP_ISR_Create(PINSTANCE_DATA Id);
void	FCP_ISR_Destroy();

// here so everyone can throw an event
void	FCP_Handle_Async_Event(PINSTANCE_DATA Id,
					FCP_EVENT_ACTION action,
					U16 event_code);

STATUS		FCP_Handle_Immediate_Notify(FCP_EVENT_CONTEXT *p_context);


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpISR_H  */
