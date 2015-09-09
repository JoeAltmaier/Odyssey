/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: HscsiConfig.h
// 
// Description:
// This file defines the configuration data for the HSCSI DDM. 
// 
// Update Log
//	$Log: /Gemini/Include/CTTables/ConfigRecords/HscsiConfig.h $
// 
// 2     10/12/99 5:11p Cchan
// Changes made to support PTS and HscsiConfig.cpp
// 
// 1     9/15/99 11:12a Cchan
// Include necessary for HSCSI library (QL1040B) support
// 
/*************************************************************************/
#if !defined(HscsiConfig_H)
#define HscsiConfig_H

#ifdef          __cplusplus
#include "CTtypes.h"
#include "PtsCommon.h"

extern  "C" {                               /* C declarations in C++     */

extern const fieldDef	HscsiConfig_FieldDefs[];
extern const U32		cbHscsiConfig_FieldDefs;

#endif

/*************************************************************************/
//    HSCSI_CONFIG
//    defines configuration structure
/*************************************************************************/
#define HSCSI_CONFIG_TABLE_NAME "HSCSI_CONFIG_TABLE"
#define HSCSI_CONFIG_VERSION 3

typedef struct _HSCSI_CONFIG {

#ifdef	__cplusplus
	static const fieldDef *FieldDefs()		{ return HscsiConfig_FieldDefs; }
	static const U32 FieldDefsSize()		{ return cbHscsiConfig_FieldDefs; }
	static const char* const TableName()	{ return HSCSI_CONFIG_TABLE_NAME; }
#endif
	
	rowID			rid;
	UNSIGNED 		version;
	UNSIGNED		size;
	UNSIGNED		enable_initiator_mode;
	UNSIGNED		enable_target_mode;
	UNSIGNED		enable_fairness;
	UNSIGNED		enable_fast_posting;
	UNSIGNED		enable_hard_address;
	
	// event_queue_size specifies the number of events that can be 
	// queued at any one time for the event task to handle.
	UNSIGNED		event_queue_size;
   
	// mb_queue_size specifies the number of mailboxes that can be 
	// queued at any one time for handling later.
	UNSIGNED		mb_queue_size;
   
	UNSIGNED		task_stack_size;   
	UNSIGNED		task_priority;
	   
	// ISP_FIFO_request_size specifies the number of entries in the ISP FIFO
	// for requests.
	UNSIGNED		ISP_FIFO_request_size;
   
	// ISP_FIFO_response_size specifies the number of entries in the ISP FIFO
	// for responses.
	UNSIGNED		ISP_FIFO_response_size;
   
	// hard_address is FCP specific for now
	UNSIGNED		hard_address;
      
	// size of stack to allocate to high level interrupt service routine
	UNSIGNED		HISR_stack_size;
   
	// num_LUNs is the number of LUNs that we will bring up in target mode
	// starting with first_LUN
	UNSIGNED		num_LUNs;
	
	// number of buffers allocated in PCI window memory
	UNSIGNED		num_buffers;
	
	// physical start address of the ISP chip
	UNSIGNED		base_ISP_address;
	
	// physical interrupt of the ISP chip
	UNSIGNED		interrupt;
	
	// instance number used to configure this DDM
	UNSIGNED		config_instance;
	
	// virtual device number used to throw messages for this DDM
	VDN				virtual_device;
	
}  HSCSI_CONFIG, *PHSCSI_CONFIG;

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif // HscsiConfig_H