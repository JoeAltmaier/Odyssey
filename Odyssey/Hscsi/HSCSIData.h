/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiData.h
// 
// Description:
// This file contains static and global data structure definitions used
// within the QL1040 DDM.
// 
// Update Log 
//	$Log: /Gemini/Odyssey/Hscsi/HSCSIData.h $
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/
#if !defined(HscsiData_H)
#define HscsiData_H

#include "HscsiIOCB.h"
#include "HscsiISP.h"
#include "Nucleus.h"
#include "HscsiConfig.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

/*************************************************************************/
//    Global Instance Data
/*************************************************************************/

typedef	struct _HSCSI_INSTANCE_DATA {
	UNSIGNED			  HSCSI_instance;				// instance number
	HSCSI_CONFIG	  	  HSCSI_config;				// configuration data
	NU_SEMAPHORE		  HSCSI_Loop_Sema;			// flag loop up
	
	UNSIGNED			  HSCSI_if_print_ISR;			// TraceLevel to print ISP regs
	UNSIGNED			  HSCSI_state;
	UNSIGNED			  HSCSI_flags;				// misc flags

	/*************************************************************************/
	// FIFO globals
	/*************************************************************************/
	IOCB_STATUS_ENTRY	 *HSCSI_p_IOCB_request_FIFO;	// memory allocated for request FIFO
	UNSIGNED			  HSCSI_request_FIFO_index;	// index into request FIFO
	
	IOCB_STATUS_ENTRY	 *HSCSI_p_IOCB_response_FIFO;	// memory allocated for response FIFO
	UNSIGNED			  HSCSI_response_FIFO_index;	// index into response FIFO
	
	/*************************************************************************/
	// HscsiEvent globals
	/*************************************************************************/
	NU_QUEUE			  HSCSI_event_queue; 			// queue of HSCSI_EVENT_CONTEXT waiting to be handled
	VOID				 *HSCSI_p_event_queue;		// queue memory
	VOID				 *HSCSI_p_event_stack;		// event stack memory
	NU_PARTITION_POOL	  HSCSI_event_context_pool;
	VOID				 *HSCSI_p_event_context_pool;	// event context pool memory
	NU_TASK		 		  HSCSI_event_task;

	/*************************************************************************/
	// Buffer globals
	/*************************************************************************/
	NU_QUEUE			  HSCSI_buffer_queue;			// list of pointers to buffers
	VOID				 *HSCSI_p_buffer_queue;		// points to buffer queue allocated
	VOID				 *HSCSI_p_buffer_memory;		// points to buffer memory allocated
	
	/*************************************************************************/
	// ISR globals
	/*************************************************************************/
	UNSIGNED			  HSCSI_LISR_Registered;		// Flag LISR as installed
	NU_HISR				  HSCSI_HISR;					// HISR object
	VOID				 *HSCSI_p_HISR_stack;			// points to HISR stack area
	
	/*************************************************************************/
	// Mailbox globals
	/*************************************************************************/
	HSCSI_MAILBOX_MESSAGE	  HSCSI_mailbox_message;
	NU_QUEUE			  HSCSI_mailbox_queue; 		// queue of mailbox completions
													// waiting to be handled
	VOID				 *HSCSI_p_mb_queue;			// mailbox queue memory
	
	/*************************************************************************/
	// ISP Chip Data values
	/*************************************************************************/
	VOID				 *ISP_Regs;					// Physical address of registers
	VOID				 *Regs;						// Logical address of registers
	UNSIGNED			  HSCSI_interrupt;			// ISP 1040 Interrupt vector

	/*************************************************************************/
	// DEBUG Data values
	/*************************************************************************/
#if defined(HSCSI_DEBUG)	
	UNSIGNED			  CmdsExecuting;			// number of commands left
	VOID				 *Last;						// last event started
	VOID				 *LastDone;					// last event deleted
	U32					  Num_low_isr;
	U32					  Num_high_isr;
	U32					  Num_high_isr_entry;
#endif
	
} HSCSI_INSTANCE_DATA, *PHSCSI_INSTANCE_DATA;

typedef enum {
	HSCSI_STATE_RESET,
	HSCSI_STATE_ACTIVE,
	HSCSI_STATE_QUIET,
	HSCSI_STATE_INIT
} HSCSI_STATE;

// These values must be defined globally since they are referenced in the interrupt code
extern	HSCSI_INSTANCE_DATA		H_Instance_Data;	// Instance data

#define	TARGET_INSTANCE		0
#define	INITIATOR_INSTANCE	1
#define	BOTH_INSTANCE		3			// Target and Initiator
#define	INIT_ONLY_INSTANCE	4			// Initiator only
#define	TRGT_ONLY_INSTANCE	5			// Target only

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif // HscsiData_H