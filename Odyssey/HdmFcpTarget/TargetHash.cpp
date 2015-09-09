/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: TargetHash.cpp
// 
// Description:
// This module has the FCP Target Hash Table methods
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTarget/TargetHash.cpp $
// 
// 1     8/20/99 7:49p Mpanas
// Changes to support Export Table states
// 
// 
// 08/17/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "FC_Loop.h"
#include "CTIdLun.h"
#include "Scsi.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_FCP_TARGET
#include "Odyssey_Trace.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

// Tables referenced
#include "ExportTable.h"

#include "HdmFcpTarget.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpTarget.h"

//
//	FCP Library Hooks
//
/*************************************************************************/
// FCP_Find_Next_Virtual_Device
// Lookup the next Virtual Device number in the route based on the
// Initiator ID and LUN.  If no virtual circuit for this pair is configured,
// return a -1 (which can not be a valid VirtualDevice).
/*************************************************************************/
VDN HdmFcpTarget::FCP_Find_Next_Virtual_Device(void *pCtx, U32 key)
{
	VDN				 vdNext;
	Xlt				*pX;
	pHandler_t 		 Handler;
	
	TRACE_ENTRY(HdmFcpTarget::FCP_Find_Next_Virtual_Device);

	if ((pX = (Xlt *)Find_Hash_Entry(key)) == NULL)
	{
		TRACE_HEX(TRACE_L2, "\n\rFailed Key = ", key);
		TRACE_HEX(TRACE_L2, " Hash ", (U32)Hash(key));
		
		// send an error bach to the host
		Handle_Error_Resp(pCtx, NULL);
		
		return ((VDN)-1);	// no such ID/LUN
	}
	
	Handler = pX->Handler;
	
	// Invoke the Handler, if there is one
	if (Handler)
		vdNext = (this->*Handler)((void *)pCtx, (void *)pX);
	else
	{
		Handle_Error_Resp(pCtx, NULL);

		vdNext = (VDN)-1;
	}
	
	//vdNext = pX->vd; // the old way
	
	TRACE_HEX(TRACE_L8, "\n\rKey = ", key);
	TRACE_HEX16(TRACE_L8, " Next VD = ", vdNext);
	
	return(vdNext);
	
}	// FCP_Find_Next_Virtual_Device


/*************************************************************************/
// Hash
// returns the hash index of key
// this version uses h(k) = [m (k A mod 1], where m is a power of two
/*************************************************************************/
U32 HdmFcpTarget::Hash(U32 key)
{
	U32				A = 618033988;  // set A to a 32 bit int constant
	//U32				A = 618033 * size;  // set A to a 32 bit int
	U32				index;
	long long		tmp;
		
 	TRACE_ENTRY(HdmFcpTarget::Hash);
 	
 	tmp = key * A;
 	
 	// need only the upper 14 bits of the lower 32bits of the result
 	index = (tmp & 0x00000000fffc0000) >> 18;
 	
	return (index);
} // Hash

/*************************************************************************/
// Add_Hash_Entry
// Add key to the _LunXlate table pointed to by pXlt. returns 0 if OK.
/*************************************************************************/
U32 HdmFcpTarget::Add_Hash_Entry(U32 key, void *entry)
{
	U32			 hash;
	Xlt			*pX;
	
 	TRACE_ENTRY(HdmFcpTarget::Add_Hash_Entry);
 	
 	hash = Hash(key);
 
#if 0
	TRACE_HEX(TRACE_L2, "\n\rhash key = ", (U32)key);
	TRACE_HEX(TRACE_L2, "  index = ", (U32)hash);
#endif

 	if (pXlt[hash] == NULL)
 	{
 		pXlt[hash] = (Xlt *)entry;
 		return (0);
 	}
 	else
 	{
 		pX = pXlt[hash];
 		
 		// find end of list
 		while(pX->pNext)
 		{
 			pX = pX->pNext;
 		}
 		
 		// add current entry to end of list
 		pX->pNext = (Xlt *)entry;
 	}
 	
	return(1);			// error, already allocated

} // Add_Hash_Entry


/*************************************************************************/
// Remove_Hash_Entry
// Remove the key from the _LunXlate table pointed to by pXlt.  Leave a
// NULL in its place.  Return a pointer to the deleted entry or NULL if none.
/*************************************************************************/
void *HdmFcpTarget::Remove_Hash_Entry(U32 key)
{
	Xlt			*pX, *pXlast;

 	TRACE_ENTRY(HdmFcpTarget::Add_Hash_Entry);

 	// get the first entry for this key or NULL
 	pX = pXlt[Hash(key)];
 	
 	if (pX == NULL)
 		return(NULL);	// bad key
 		
 	if (pX->pNext == NULL)
 		// exact match and no children
 		pXlt[Hash(key)] = NULL;
 		
 		return(pX);
 
 	// in case the first entry matches
	pXlast = NULL;
	
	// shuffle through list looking for our key
	while(pX->pNext)
	{
		if (key == pX->key.l)
			break;
		
		pXlast = pX;
		pX = pX->pNext;
	}
	
	if (pX)
	{
		// not NULL, we found it at last.
		// remove our match from the list
		if (pXlast)
		{
			// not first
			pXlast->pNext = pX->pNext;
		}
		else
		{
			// first in list
			pXlt[Hash(key)] = pX->pNext;
		}
	}
	
	// pX could be NULL if we did not find our key
	return(pX);
	
} // Remove_Hash_Entry


/*************************************************************************/
// Find_Hash_Entry
// Return the index to the key in the _LunXlate table pointed to by pXlt.
/*************************************************************************/
void * HdmFcpTarget::Find_Hash_Entry(U32 key)
{
	Xlt			*pX;

 	TRACE_ENTRY(HdmFcpTarget::Find_Hash_Entry);
 	
 	// get the first entry for this key or NULL
 	pX = pXlt[Hash(key)];
 	
 	if (pX == NULL || pX->pNext == NULL)
 		// no entry or exact match
 		return((void *)pX);
 	
	// shuffle through list looking for our key
	while(pX->pNext)
	{
		if (key == pX->key.l)
			break;
			
		pX = pX->pNext;
	}
	
 	return((void*)pX);

} // Find_Hash_Entry


/*************************************************************************/
// Target_Build_Tables
// Build local LUN translation Hash tables
// This section will be executed after the PTS has been read to get all
// the Export Table entries that match our FCInstance.
/*************************************************************************/
void HdmFcpTarget::Target_Build_Tables() {

	U32				 loop, id;
	_LunXlate		*pX;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;
	ExportTableEntry	*pExport;

	TRACE_ENTRY(HdmFcpTarget::Target_Build_Tables);
		
	// Scan the Export Table to read all the entries that belong to us
	// and get the data we need to build the translation table
	pExport = Tar_TS_Export;
	
	for (loop = 0; loop < m_nTableRows; loop++)
	{
		// use entries on our loop only
		if (pExport->FCInstance != config.loop_instance)
		{
    		pExport++;	// next entry
			continue;
		}
			
	    // use the LUN and id fields to build key
	    sig.idlun.HostId = (U8)pExport->InitiatorId;
	    sig.idlun.id = (U8)pExport->TargetId;
	    //sig.idlun.id = 0;
	    sig.idlun.LUN = (U16)pExport->ExportedLUN;
	    
	    if (sig.idlun.HostId == 0xff)
	    {
	    	// allow all hosts to access this LUN
	    	for (id = 0; id < MAX_FC_IDS; id++)
	    	{
	    		pX = new Xlt;				// each entry must have a new Xlt
    			pX->pNext = NULL;
	    		pX->vd = pExport->vdNext;
	    		pX->Row = pExport->rid;		// save this entries row ID
	    		
	    		// set same state as the Export entry
	    		pX->state = pExport->ReadyState;
	    		Set_Handler(pX);
	    		
	    		sig.idlun.HostId = (U8)id;
	    		pX->key.l = sig.l;
	    		
		    	if (Add_Hash_Entry(sig.l, (void *)pX))
		    	{
					TRACE_HEX(TRACE_L2, "\n\rAdd_Hash_Entry: key extended = ", (U32)sig.l);
					TRACE_HEX(TRACE_L2, " hash ", (U32)Hash(sig.l));
		    	}
	    	}
    	}
    	else
    	{
	   		// make a new Xlt record and fill it in
    		pX = new Xlt;
    		pX->pNext = NULL;
    		pX->vd = pExport->vdNext;
    		pX->Row = pExport->rid;		// save this entries row ID
    		pX->key.l = sig.l;
    		
    		// set temporary state
    		pX->state = pExport->ReadyState;
    		Set_Handler(pX);
	    		
	    	if (Add_Hash_Entry(sig.l, (void *)pX))
	    	{
				TRACE_HEX(TRACE_L2, "\n\rAdd_Hash_Entry: key extended = ", (U32)sig.l);
				TRACE_HEX(TRACE_L2, " hash ", (U32)Hash(sig.l));
	    	}
    		num_valid_exports++;
    	}
    	
    	// check the Export table entry state, if it is: StateConfigured,
    	// change the export table entry state to: StateConfiguredAndExporting.
    	if (pExport->ReadyState == StateConfigured)
    	{
    		// use the last Xlt to send the state change
    		pX->state = StateConfiguredAndExporting;
    		
    		TarTblModifyExportState(&pX->Row, &pX->state);
    	}
    	
    	// next entry
    	pExport++;
	}
	
} // FCP_Build_Tables



/*************************************************************************/
// Set_Handler
// Set the Handler address based on the Ready State
/*************************************************************************/
void HdmFcpTarget::Set_Handler(void *p) {
	Xlt			*pX = (Xlt *)p;

 	TRACE_ENTRY(HdmFcpTarget::Set_Handler);
 	
 	// select the correct handler
 	switch (pX->state)
 	{
 	case StateConfiguredAndActive:
	    pX->Handler = &Handle_Cmd_Resp;
	    break;
 	
 	case StateQuiesced:
	    pX->Handler = &Handle_Busy_Resp;
	    break;
 	
 	case StateOffLine:
	    pX->Handler = &Handle_Error_Resp;
	    break;
 	
 	case StateConfiguredAndExported:
	    pX->Handler = &Handle_Trigger_Resp;
	    break;
 	
 	case StateConfigured:
 	case StateConfiguredAndExporting:
 	default:
	    pX->Handler = &Handle_Error_Resp;
	    break;
 	}

} // Set_Handler



// Handlers
// The handler is selected by the Handler field in the Xlt struct.  Each
// handler implies a specific response function to the host.  Some (all)
// of these handlers make calls directly into the FCP Library.  Specifically,
// to do a CTIO call, we must call: 
//	STATUS FCP_Send_CTIO(FCP_EVENT_CONTEXT *p_context, 
//		FCP_EVENT_ACTION next_action, 
//		U8 SCSI_status, UNSIGNED flags)


/*************************************************************************/
// Handle_Busy_Resp
// Always return a BUSY SCSI status to the host.
/*************************************************************************/
U32 HdmFcpTarget::Handle_Busy_Resp(void *pContext, void *pX)
{
	STATUS			status;

 	TRACE_ENTRY(HdmFcpTarget::Handle_Busy_Resp);
 	
     // Send Continue Target I/O with BUSY condition 
	status = FCP_Send_CTIO((FCP_EVENT_CONTEXT *)pContext, 
    	TARGET_ACTION_HANDLE_CTIO_FINAL,
    	SCSI_STATUS_BUSY,
		ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
		| ISP_CTIO_FLAGS_FAST_POST
		| ISP_CTIO_FLAGS_NO_DATA_XFR); 

	return(-1);
	
} // Handle_Busy_Resp


/*************************************************************************/
// Handle_Error_Resp
// Always return a SCSI check status with an error to the host.  In most
// cases this will be the: ASC_LOGICAL_UNIT_NOT_SUPPORTED error.
/*************************************************************************/
U32 HdmFcpTarget::Handle_Error_Resp(void *pContext, void *pX)
{
	STATUS			status;
	REQUEST_SENSE	abort_sense = {RESPONSE_CODE, 0, SENSE_NOT_READY, 0,0,0,0,
							ADDITIONAL_LENGTH, 0,0,0,0, ASC_LOGICAL_UNIT_NOT_SUPPORTED,
							0,0,0,0,0 };

 	TRACE_ENTRY(HdmFcpTarget::Handle_Error_Resp);
 	
	status = FCP_Send_CTIO_Check((FCP_EVENT_CONTEXT *)pContext, 
		TARGET_ACTION_HANDLE_CTIO_FINAL,
		(U8 *) &abort_sense,
		sizeof(REQUEST_SENSE),
		  ISP_CTIO_FLAGS_SEND_SCSI_STATUS 
		| ISP_CTIO_FLAGS_INC_RESOURCE_COUNT
		| ISP_CTIO_FLAGS_NO_DATA_XFR);

 	return(-1);
 	
} // Handle_Error_Resp


/*************************************************************************/
// Handle_Cmd_Resp
// handle as a normal command, return VDN next or -1 if none.
/*************************************************************************/
U32 HdmFcpTarget::Handle_Cmd_Resp(void *pContext, void *p)
{
	Xlt			*pX = (Xlt *)p;

 	TRACE_ENTRY(HdmFcpTarget::Handle_Cmd_Resp);
 	
 	return(pX->vd);
 	
} // Handle_Cmd_Resp


/*************************************************************************/
// Handle_Trigger_Resp
// This is the first state after ConfiguredandExporting.  Pass the command
// through, set next handler as: Handle_Cmd_Resp and Quiesce to failover
// partner.
/*************************************************************************/
U32 HdmFcpTarget::Handle_Trigger_Resp(void *pContext, void *p)
{
	Xlt			*pX = (Xlt *)p;

 	TRACE_ENTRY(HdmFcpTarget::Handle_Trigger_Resp);
 	
 	// change the Export table entry state
 	
 	// set new handler
    pX->Handler = &Handle_Cmd_Resp;
 	
 	return(pX->vd);
 	
} // Handle_Trigger_Resp


