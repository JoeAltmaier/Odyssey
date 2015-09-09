/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: TargetTableListen.cpp
// 
// Description:
// This module is the Export Table Listen Handler for the
// FCP Target.
//
//	Handles:
//		Export Table changes
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTarget/TargetTableListen.cpp $
// 
// 1     8/20/99 7:50p Mpanas
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


// Table method context
typedef struct _TL_TBL_CONTEXT {
	U32						 LoopInstance;
} TL_TBL_CONTEXT, *PTL_TBL_CONTEXT;


#define	EXPORT_LISTEN_MASK	(ListenOnInsertRow|ListenOnDeleteOneRow|ListenOnModifyOneRowAnyField)

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// TarStartListen
// We will listen on the Export Table.
// The Export Table will listen on Modify, Delete, and Add
/*************************************************************************/
STATUS 	HdmFcpTarget::TarStartListen(U32 LoopInstance)
{
	TL_TBL_CONTEXT *pTC;
	STATUS status;
	
	TRACE_ENTRY(TarStartListen);
	
	//
	//	Start the listen on the Export Table
	//
	
	pTC = new TL_TBL_CONTEXT;
	pTC->LoopInstance = LoopInstance;

	m_LSize = sizeof(ExportTableEntry);
	
	// Allocate a Listen object for the Export table.
	m_Listen = new(tUNCACHED) TSListen;

	// Initialize the modify row operation.
	status = m_Listen->Initialize(
		this,								// DdmServices pDdmServices,
		EXPORT_LISTEN_MASK,					// U32		ListenType,
		EXPORT_TABLE,						// String64	prgbTableName,
		"FCInstance",						// String64	prgbRowKeyFieldName,
		(void*)&pTC->LoopInstance,			// void*	prgbRowKeyFieldValue,
		sizeof(U32),						// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyWithRow|ReplyContinuous),		// U32		ReplyMode, (1=once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_LType,							// U32**	ppListenTypeRet,
		&m_Export,							// void**	ppModifiedRecordRet,
		&m_LSize,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&TarExportListenUpdate,	// pTSCallback_t pCallback,
		pTC									// void*	pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_Listen->Send();
	
	return status;
	
} // TarStartListen


/*************************************************************************/
// TarExportListenUpdate
// Called when a listen callback for the Export Table happens.
/*************************************************************************/
STATUS	HdmFcpTarget::TarExportListenUpdate(void *pClientContext, STATUS status)
{
	TL_TBL_CONTEXT 	*pTC = (TL_TBL_CONTEXT *)pClientContext;
	U32				 id;
	Xlt				*pX;
	union {
		IDLUN		 idlun;
		long		 l;
	} sig;

	TRACE_ENTRY(TarExportListenUpdate);
	
	// no work on first reply
	if (*m_LType & ListenInitialReply)
	{
		return(ercOK);
	}
	
    // get the LUN and id fields to build key
    sig.idlun.id = (U8)m_Export->TargetId;
    sig.idlun.LUN = (U16)m_Export->ExportedLUN;
    
    if (m_Export->InitiatorId != 0xffffffff)
    	sig.idlun.HostId = (U8)m_Export->InitiatorId;
    else
    	sig.idlun.HostId = 0;
    
	// Check what type of listen reply
	if (*m_LType & ListenOnModifyOneRowAnyField)
	{
	    if (m_Export->InitiatorId == 0xffffffff)
	    {
	    	// do all hosts ids
	    	for (id = 0; id < MAX_FC_IDS; id++)
	    	{
	    		sig.idlun.HostId = (U8)id;
	    		
				// check for the Hash Table entry for this Export table entry
			    pX = (Xlt *)Find_Hash_Entry(sig.l);
				
				// set the (possibly new) state
				pX->state = m_Export->ReadyState;
				
				// set the handler based on the new state
				Set_Handler(pX);
	    	}
	    }
	    else
	    {
			// check for the Hash Table entry for this Export table entry
		    pX = (Xlt *)Find_Hash_Entry(sig.l);
			
			// set the (possibly new) state
			pX->state = m_Export->ReadyState;
			
			// set the handler based on the new state
			Set_Handler(pX);
		}

    	// check the Export table entry state, if it is: StateConfigured,
    	// change the export table entry state to: StateConfiguredAndExporting.
    	if (m_Export->ReadyState == StateConfigured)
    	{
    		// use the last Xlt to send the state change
    		pX->state = StateConfiguredAndExporting;
    		
    		TarTblModifyExportState(&pX->Row, &pX->state);
    	}
    	
	}
	else if (*m_LType & ListenOnInsertRow)
	{
		// Row was just inserted
		// check for all hosts...
	    if (m_Export->InitiatorId == 0xffffffff)
	    {
	    	// allow all hosts to access this LUN
	    	for (id = 0; id < MAX_FC_IDS; id++)
	    	{
	    		pX = new Xlt;				// each entry must have a new Xlt
    			pX->pNext = NULL;
	    		pX->vd = m_Export->vdNext;
	    		pX->Row = m_Export->rid;		// save this entries row ID
	    		
	    		// set same state as the Export entry
	    		pX->state = m_Export->ReadyState;
	    		Set_Handler(pX);
	    		
	    		sig.idlun.HostId = (U8)id;
	    		pX->key.l = sig.l;
	    		
		    	if (Add_Hash_Entry(sig.l, (void *)pX))
		    	{
					TRACE_HEX(TRACE_L2, "\n\rAdd_Hash_Entry: key extended = ", (U32)sig.l);
					TRACE_HEX(TRACE_L2, " hash ", (U32)Hash(sig.l));
		    	}
	    	}
 			num_valid_exports++;
   	}
    	else
    	{
	  		// make a new Xlt record and fill it in
			pX = new Xlt;
			pX->pNext = NULL;
			pX->vd = m_Export->vdNext;
			pX->Row = m_Export->rid;		// save this entries row ID
			pX->key.l = sig.l;
			
			// set temporary state
			pX->state = m_Export->ReadyState;
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
    	if (m_Export->ReadyState == StateConfigured)
    	{
    		// use the last Xlt to send the state change
    		pX->state = StateConfiguredAndExporting;
    		
    		TarTblModifyExportState(&pX->Row, &pX->state);
    	}
    	
	}
	else if (*m_LType & ListenOnDeleteOneRow)
	{
		// Row was deleted
		// remove Hash Table entry(s)
		
	    if (m_Export->InitiatorId == 0xffffffff)
	    {
	    	// remove all host entries
	    	for (id = 0; id < MAX_FC_IDS; id++)
	    	{
	    		sig.idlun.HostId = (U8)id;
	    		
	    		// remove next entry
	    		pX = (Xlt *)Remove_Hash_Entry(sig.l);
	    		if (pX)
	    			delete pX;
	    	}
    	}
    	else
    	{
    		// remove just one entry
    		pX = (Xlt *)Remove_Hash_Entry(sig.l);
	    	if (pX)
	    		delete pX;
		}
	}

	status = ercOK;
	return status;
	
} // TarExportListenUpdate
