/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiEvent.h
// 
// Description:
// The HSCSI driver is event driven.  Each time an event occurs that
// requires an action, an event is posted in HSCSI_event_queue.
// This file describes the interface to the HSCSI_event_queue.
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiEvent.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiEvent_H)
#define HscsiEvent_H

#include "HscsiIOCB.h"
#include "HscsiISP.h"
#include "Scsi.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


#pragma pack(1)

STATUS	HSCSI_Event_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Event_Destroy();


// Name assigned to event task
#define HSCSI_EVENT_TASK_NAME "EvenTask"

/*************************************************************************/
//	Context event Action values
//	The action field determines what method will be called when
// 	a HSCSI_TARGET_CONTEXT is received in HSCSI_target_queue.
/*************************************************************************/
typedef enum
{
    TARGET_HANDLE_NEW_IOCB				=  1,
    TARGET_HANDLE_I2O_RESPONSE			=  2, 
    TARGET_HANDLE_CTIO_WRITE			=  3,
    TARGET_HANDLE_CTIO_FINAL			=  4,
    INITIATOR_HANDLE_SCSI_REQUEST		=  5,
    INITIATOR_HANDLE_COMMAND_RESPONSE	=  6,
    HSCSI_ACTION_HANDLE_OTHER_AE		=  7,
    HSCSI_ACTION_HANDLE_THROW_EVENT		=  8
} HSCSI_EVENT_ACTION;

/*************************************************************************/
//	Context structure defines the context of a thread 
//	of execution.
/*************************************************************************/
typedef struct _HSCSI_EVENT_CONTEXT {
//	IOCB_ATIO_TYPE_2				iocb;
	IOCB_STATUS_ENTRY				status_iocb;
	
	void							*message;		// 4 bytes
	HSCSI_DATA_SEGMENT_DESCRIPTOR	data_segment;	// 8 bytes
	REQUEST_SENSE					sense_data;		// 19 bytes
	U8								CDB_length;		// need it to build IOCB
	U8								endfill[6];		// to fill entire struct
													// to 64 byte boundary
	// flags to send back with CTIO if command is successful
	U16								CTIO_flags;		// 2 bytes
	
	// saved pointer for delete
	void							*buffer;		// 4 bytes
	
	// move these enums to the end to maintain alignment on a 4 byte boundary
	HSCSI_EVENT_ACTION				action;			// 4 bytes
	
	// Pointer to our instance data
	PHSCSI_INSTANCE_DATA			Id;				// 4 bytes
	
	// need to save the class "this" pointer for the current HDM
	void							*p_this_hdm;	// 4 bytes
	
	// DEBUG Flags
	U32								flags;			// 4 bytes
	U8								resp_index;
	U8								req_index;
	U8								ctio_cnt;
	U8								spare;
	
}  HSCSI_EVENT_CONTEXT, *PHSCSI_EVENT_CONTEXT;

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

#endif //   HscsiEvent_H