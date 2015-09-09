/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpEvent.h
// 
// Description:
// The FCP driver is event driven.  Each time an event occurs that
// requires an action, an event is posted in FCP_event_queue.
// This file describes the interface to the FCP_event_queue.
// 
// Update Log 
// 6/3/98 Jim Frandeen: Create file
// 8/17/98 Michael G. Panas: Force FCP_EVENT_CONTEXT struct to be a multiple
//                           of 64 bytes by rearranging and filling.
// 9/2/98 Michael G. Panas: Convert FCP_EVENT_CONTEX to DDM  use
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/18/98 Michael G. Panas: add new actions for FCP interrupts
// 11/20/98 Michael G. Panas: add DEBUG info, clean unused fields
// 11/30/98 Michael G. Panas: New memory allocation methods
// 02/17/99 Michael G. Panas: convert to new Message format and remove I2O
/*************************************************************************/

#if !defined(FcpEvent_H)
#define FcpEvent_H

#include "FcpIOCB.h"
#include "FcpISP.h"
#include "Scsi.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


#pragma pack(1)

STATUS	FCP_Event_Create(PINSTANCE_DATA Id);
void	FCP_Event_Destroy();


// Name assigned to event task
#define FCP_EVENT_TASK_NAME "EvenTask"

/*************************************************************************/
//	Context event Action values
//	The action field determines what method will be called when
// 	a FCP_TARGET_CONTEXT is received in FCP_target_queue.
/*************************************************************************/
typedef enum
{
    TARGET_ACTION_HANDLE_NEW_IOCB				=  1,
    TARGET_ACTION_HANDLE_I2O_RESPONSE			=  2, 
    TARGET_ACTION_HANDLE_CTIO_WRITE				=  3,
    TARGET_ACTION_HANDLE_CTIO_FINAL				=  4,
    INITIATOR_ACTION_HANDLE_SCSI_REQUEST		=  5,
    INITIATOR_ACTION_HANDLE_COMMAND_RESPONSE	=  6,
    FCP_ACTION_HANDLE_LOOP_UP					=  7,
    FCP_ACTION_HANDLE_LOOP_DOWN					=  8,
    FCP_ACTION_HANDLE_LIP						=  9,
    FCP_ACTION_HANDLE_LIP_RESET					= 10,
    FCP_ACTION_HANDLE_OTHER_AE					= 11,
    FCP_ACTION_HANDLE_THROW_EVENT				= 12,
    FCP_ACTION_HANDLE_LOOP_EVENT				= 13,
    FCP_ACTION_HANDLE_LOOP_IOCB_COMPL			= 14,
    FCP_ACTION_HANDLE_IMMEDIATE_NOTIFY			= 15,
	TARGET_ACTION_HANDLE_CACHE_AVAILABLE		= 16
} FCP_EVENT_ACTION;

#define FCP_FIRST_BLOCK_BIT		0x01
#define FCP_LAST_BLOCK_BIT		0x02

// request type - stored in context
#define NO_DATA_XFER		0
#define SMALL_READ_XFER		1
#define READ_BUFFER_XFER	2
#define SMALL_WRITE_XFER 	3
#define WRITE_BUFFER_XFER	4

// Context state
// all cache blocks locked - use 1st free one
#define	FCP_NO_CACHE			0x01
// requested cache block is in use and locked - wait for it to be released
#define	FCP_LOCKED_CACHE		0x02
// filling cache (writes from host or reads from media)  
#define	FCP_FILLING_CACHE		0x03
// sending cache to host (read) or writing cache to media (write)
#define	FCP_VALID_CACHE			0x04
// when all portions of the request completes - the last context sends status to the host
#define	FCP_LAST_REQUEST	0x05
// after processing context complete the context is released i.e. free state
// when all contexts associated with the orig IO request are complete, the status IOCB is sent to the host
#define	FCP_FREE_CONTEXT		0x00


typedef struct	_REQUEST_ADJUSTMENTS {
	U8		first_last_flags ;
	U8		reserved;
	U16		data_segment_count ;
	U32		first_request_offset ;
	U32		last_request_count ;
	U32		transfer_offset ;
} FCP_REQUEST_ADJUSTMENTS, *PFCP_REQUEST_ADJUSTMENTS ;

typedef struct	_CONTEXT_CHAIN {
	void		*pPrevContext ;
	void		*pNextContext ;
} FCP_CONTEXT_CHAIN, *PFCP_CONTEXT_CHAIN ;


/*************************************************************************/
//	Context structure defines the context of a thread 
//	of execution.
/*************************************************************************/
typedef struct _FCP_EVENT_CONTEXT {
	IOCB_ATIO_TYPE_2				iocb; 			/* original IOCB received at target */
	IOCB_STATUS_TYPE0				status_iocb;	/* status to return for the original IOCB */
	void							*message;		/* ptr to message if any */
	DATA_SEGMENT_DESCRIPTOR			data_segment;	// 8 bytes
	REQUEST_SENSE					sense_data;		// 19 bytes
	U8								req_state ;		// current state of this part of the total request

	DATA_SEGMENT_DESCRIPTOR			*p_dsd;

	U8								fill[2];		
													
	// flags to send back with CTIO if command is successful
	U16								CTIO_flags;		// 2 bytes
	
	// saved pointer for delete
	void							*buffer;		// 4 bytes - for cache io points to head cache block chain
	
	// move these enums to the end to maintain alignment on a 4 byte boundary
	FCP_EVENT_ACTION				action;			// 4 bytes
	
	// Pointer to our instance data
	PINSTANCE_DATA					Id;				// 4 bytes

	void							*p_this_hdm;	// 4 bytes
	
	// DEBUG Flags
	U32								flags;			// 4 bytes
	void							*pStatusIOCB;
	U8								resp_index;
	U8								req_index;
	U8								ctio_cnt;
	U8								req_type;
	FCP_REQUEST_ADJUSTMENTS			req_adjust ;
	FCP_CONTEXT_CHAIN				req_link ;
	U8								fill2[4];		// makes struct 224 bytes - multiple of 8 bytes
	
}  FCP_EVENT_CONTEXT, *PFCP_EVENT_CONTEXT;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


// flags used for debug
#define		EV_DONE					0x0001		// done with operation
#define		EV_CDB_IN				0x0002		// CDB received
#define		EV_SENT_MESSAGE			0x0004		// sent I2O message to next
#define		EV_REPLY_RECEIVED		0x0008		// message reply received
#define		EV_CTIO_SENT  			0x0010
#define		EV_CTIO_CHECK_SENT		0x0020
#define		EV_CTIO_RESPONSE_IN		0x0040
#define		EV_CTIO_WRITE_SENT		0x0080
#define		EV_CTIO_WRITE_RESP_IN	0x0100
#define		EV_CTIO_ERROR_SENT  	0x0200
#define		EV_CTIO_SENSE_SENT  	0x0400

#define		EV_MESSAGE_IN			0x1000
#define		EV_REPLY_SENT			0x2000
#define		EV_CMD_SENT				0x4000
#define		EV_CMD_RESP_IN			0x8000

#endif //   FcpEvent_H