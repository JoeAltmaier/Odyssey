/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FcpConfig.h
// 
// Description:
// This file defines the configuration data for the FCP DDM. 
// 
// Update Log 
// 
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 8/27/98 Michael G. Panas: Add new fields for config, rearrange
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: add new fields for memory allocation, remove
//                            first_LUN since all LUNs start at 0
// 12/24/98 Michael G. Panas: add new field interrupt to pass the interrupt
//                            number of the ISP chip
/*************************************************************************/
#if !defined(FcpConfig_H)
#define FcpConfig_H

#pragma	pack(4)

#ifdef          __cplusplus
#include "CTtypes.h"
#include "PtsCommon.h"

extern  "C" {                               /* C declarations in C++     */

extern const fieldDef	FcpConfig_FieldDefs[];
extern const U32		cbFcpConfig_FieldDefs;

#endif

/*************************************************************************/
//    FCP_CONFIG
//    defines configuration structure
/*************************************************************************/
#define FCP_CONFIG_TABLE_NAME "FCP_CONFIG_TABLE"
#define FCP_CONFIG_VERSION 3

typedef struct _FCP_CONFIG {

#ifdef          __cplusplus
	static const fieldDef *FieldDefs()		{ return FcpConfig_FieldDefs;   }
    static const U32 FieldDefsSize()		{ return cbFcpConfig_FieldDefs;  }
    static const char* const TableName()	{ return FCP_CONFIG_TABLE_NAME; }
#endif

    rowID	rid;
	U32		version;
	U32		size;
	U32		enable_initiator_mode;
	U32		enable_target_mode;
	U32		enable_fairness;
	U32		enable_fast_posting;
	U32		enable_hard_address;
	
	// for all the flags outlined in the 2100 Addendum, this value is used
	// as the initial options value
	U32		options;

	// event_queue_size specifies the number of events that can be 
	// queued at any one time for the event task to handle.
	U32		event_queue_size;
   
	// mb_queue_size specifies the number of mailboxes that can be 
	// queued at any one time for handling later.
	U32		mb_queue_size;
   
	U32		task_stack_size;   
	U32		task_priority;
	
	// maximum size of a FC frame
	// Due to a bug in the 2100e firmware, the max is: 2110 not 2112
	// for performance this size should be set in multiples of 1024
	U32		max_frame_size;
   
	// ISP_FIFO_request_size specifies the number of entries in the ISP FIFO
	// for requests.
	U32		ISP_FIFO_request_size;
   
	// ISP_FIFO_response_size specifies the number of entries in the ISP FIFO
	// for responses.
	U32		ISP_FIFO_response_size;
   
	// If enable_hard_address is not zero, then the firmware attempts to
	// acquire the ISP2100 AL_PA during the LIHA (hard address) phase
	// of loop initialization using hard_address as the address.
	U32		hard_address;
   
	// node_name specifies a worldwide unique name that is used during the
	// login phase of port initialization.
	UNSIGNED_CHAR	node_name[8];
   
	// size of stack to allocate to high level interrupt service routine
	U32		HISR_stack_size;
   
	// num_IDs is the number of target IDs that we will bring up in target mode.
	// the actual target IDs to configure will be determined later (by the
	// LoopMonitor)
	U32		num_IDs;
	
	// num_LUNs is the number of LUNs that we will bring up in target mode
	// starting with first_LUN
	U32		num_LUNs;
	
	// number of buffers allocated in PCI window memory
	U32		num_buffers;
	
	// instance number used to configure this DDM
	U32		config_instance;
	
	// the loop instance number used to access the loop for this DDM
	U32		loop_instance;
	
	// virtual device number used to throw messages for this DDM
	VDN				virtual_device;
	
}  FCP_CONFIG, *PFCP_CONFIG;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif // FcpConfig_H