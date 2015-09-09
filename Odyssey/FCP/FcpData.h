/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpData.h
// 
// Description:
// This file contains static and global data structure definitions used
// within the QL2100 DDM.
// 
// Update Log 
// 
// 4/14/98 Jim Frandeen: Create file
// 8/25/98 Michael G. Panas: Change global data to a structure to allow 
//							 multiple instances of the FCP driver and to
//							 allow multiple QL2100 chips to be used 
// 9/2/98 Michael G. Panas: add C++ stuff
// 10/1/98 Michael G. Panas: changes to support init and target in same instance
// 10/6/98 Michael G. Panas: changes to support interrupt driven mailbox commands
// 11/20/98 Michael G. Panas: add DEBUG info, clean unused fields
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/
#if !defined(FcpData_H)
#define FcpData_H

#include "FcpIOCB.h"
#include "FcpISP.h"
#include "Nucleus.h"
#include "FcpConfig.h"

#include "FCPTargetPerformance.h"
#include "FCPTargetStatus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#pragma	pack(4)
/*************************************************************************/
//    Global Instance Data
/*************************************************************************/

typedef	struct _FCP_INSTANCE_DATA {
	UNSIGNED			  FCP_instance;				// instance number
	FCP_CONFIG		  	  FCP_config;				// configuration data
	NU_SEMAPHORE		  FCP_Loop_Sema;			// flag loop up
	
	UNSIGNED			  FCP_if_print_ISR;			// TraceLevel to print ISP regs
	UNSIGNED			  FCP_state;
	UNSIGNED			  FCP_flags;				// misc flags
	void				 *pFCP_DDM;					// our ddm instance

	/*************************************************************************/
	// FIFO globals
	/*************************************************************************/
	IOCB_STATUS_TYPE0	 *FCP_p_IOCB_request_FIFO;	// memory allocated for request FIFO
	UNSIGNED			  FCP_request_FIFO_index;	// index into request FIFO
	
	IOCB_STATUS_TYPE0	 *FCP_p_IOCB_response_FIFO;	// memory allocated for response FIFO
	UNSIGNED			  FCP_response_FIFO_index;	// index into response FIFO
	
	/*************************************************************************/
	// FcpEvent globals
	/*************************************************************************/
	NU_QUEUE			  FCP_event_queue; 			// queue of FCP_EVENT_CONTEXT waiting to be handled
	VOID				 *FCP_p_event_queue;		// queue memory
	VOID				 *FCP_p_event_stack;		// event stack memory
	NU_PARTITION_POOL	  FCP_event_context_pool;
	VOID				 *FCP_p_event_context_pool;	// event context pool memory
	NU_TASK		 		  FCP_event_task;

	/*************************************************************************/
	// Buffer globals
	/*************************************************************************/
	NU_QUEUE			  FCP_buffer_queue;			// list of pointers to buffers
	VOID				 *FCP_p_buffer_queue;		// points to buffer queue allocated
	VOID				 *FCP_p_buffer_memory;		// points to buffer memory allocated
	
	/*************************************************************************/
	// ISR globals
	/*************************************************************************/
	UNSIGNED			  FCP_LISR_Registered;		// Flag LISR as installed
	NU_HISR				  FCP_HISR;					// HISR object
	VOID				 *FCP_p_HISR_stack;			// points to HISR stack area
	
	/*************************************************************************/
	// Mailbox globals
	/*************************************************************************/
	FCP_MAILBOX_MESSAGE	  FCP_mailbox_message;
	NU_QUEUE			  FCP_mailbox_queue; 		// queue of mailbox completions
													// waiting to be handled
	VOID				 *FCP_p_mb_queue;			// mailbox queue memory
	
	/*************************************************************************/
	// ISP Chip Data values
	/*************************************************************************/
	VOID				 *ISP_Regs;					// Physical address of registers
	VOID				 *Regs;						// Logical address of registers
	UNSIGNED			  FCP_interrupt;			// ISP 2100 Interrupt vector
	UNSIGNED			  ISP_Type;					// Chip type 2100/2200 etc...
	UNSIGNED			  FCP_loop;					// Loop number
	UNSIGNED			  FCP_chip;					// chip number

	/*************************************************************************/
	// Performance and Status counters (PHS)
	/*************************************************************************/
	struct {
		FCPT_Performance  PHS_Performance;
		FCPT_Status		  PHS_Status;
	} FCPTarget;

#if 0
// We are not implemeting PHS counters for FCP Initiator driver yet.		
	struct {
		FCPI_Performance  PHS_Performance;
		FCPI_Status		  PHS_Status;
	} FCPInitiator;
#endif

	/*************************************************************************/
	// DEBUG Data values
	/*************************************************************************/
#if defined(FCP_DEBUG)	
	UNSIGNED			  CmdsExecuting;			// number of commands left
	UNSIGNED			  MaxCmdsExecuting;			// max # of outstanding cmds
	VOID				 *Last;						// last event started
	VOID				 *LastDone;					// last event deleted
	U32					  Num_low_isr;
	U32					  Num_high_isr;
	U32					  Num_high_isr_entry;
#endif
	
} INSTANCE_DATA, *PINSTANCE_DATA;


// These values must be defined globally since they are referenced in the interrupt code
extern	INSTANCE_DATA		Instance_Data[];				// Instance data

#define	TARGET_INSTANCE		0
#define	INITIATOR_INSTANCE	1
#define	BOTH_INSTANCE		3			// Target and Initiator
#define	INIT_ONLY_INSTANCE	4			// Initiator only
#define	TRGT_ONLY_INSTANCE	5			// Target only

// FCP_flags defines
#define	FCP_TARGET_MODE				0x8000		// set target mode
#define	FCP_INITIATOR_MODE			0x4000		// set initiator mode
#define	FCP_INTERRUPT_IN_PROGRESS	0x0001

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif // FcpData_H