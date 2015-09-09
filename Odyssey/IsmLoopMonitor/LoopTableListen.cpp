/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopTableListen.cpp
// 
// Description:
// This module is the Loop Monitor Table Listen Handler
//	Handles:
//		Export Table Reads
//		Loop Descriptor Table Create/Read/Write/Update
//		Present Port Descriptor Table Create/Read/Write/Update
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopTableListen.cpp $
// 
// 4     2/07/00 9:25p Cchan
// Added vdNext references in the ETState container class.
// 
// 3     8/20/99 7:52p Mpanas
// Changes to support Export Table states
// 
// 2     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 1     7/23/99 1:39p Mpanas
// Add latest versions of the Loop code
// 
// 
// 07/08/99 Michael G. Panas: Create file
/*************************************************************************/

#include "LmCommon.h"


// Table method context
typedef struct _LM_TBL_CONTEXT {
	U32						 LoopInstance;
} LM_TBL_CONTEXT, *PLM_TBL_CONTEXT;


#define	LM_EXPORT_LISTEN_MASK	(ListenOnInsertRow|ListenOnDeleteOneRow|ListenOnModifyOneRowAnyField)
#define	LM_LD_LISTEN_MASK	(ListenOnModifyOneRowAnyField)

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// LmListenInitialize
// Initial listen startup
// Start the listens on all configured loops
/*************************************************************************/
void 	LoopMonitorIsm::LmListenInitialize(void)
{
	TRACE_ENTRY(LmListenInitialize);
	
	// start all listens
	for (int i = 0; i < config.num_loops; i++)
	{
		LmStartListens(config.FC_instance[i], i);
	}
	
} // LmListenInitialize


/*************************************************************************/
// LmStartListens
// We will listen on the LoopDescriptor Table and the Export Table.
// The LoopDescriptor is listen on Modify
// The Export Table will listen on Modify, Delete, and Add
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmStartListens(U32 LoopInstance, U32 Index)
{
	LM_TBL_CONTEXT *pTC;
	STATUS status;
	
	TRACE_ENTRY(LmStartListens);
	
	//
	// Start the listen on the LoopDescriptor table
	//
	
	pTC = new LM_TBL_CONTEXT;
	pTC->LoopInstance = LoopInstance;
	
	m_LSize = sizeof(LoopDescriptorEntry);
	
	// Allocate a Listen object for the LoopDescriptor table.
	m_Listen = new(tUNCACHED) TSListen;

	// Initialize the modify row operation.
	status = m_Listen->Initialize(
		this,								// DdmServices pDdmServices,
		LM_LD_LISTEN_MASK,					// U32		ListenType,
		LOOP_DESCRIPTOR_TABLE,				// String64	prgbTableName,
		fdLD_LOOP_NUM,						// String64	prgbRowKeyFieldName,
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
		&m_Loop_Desc,						// void**	ppModifiedRecordRet,
		&m_LSize,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&LmLoopListenUpdate,	// pTSCallback_t pCallback,
		pTC									// void*	pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_Listen->Send();
	
	//
	//	Start the listen on the Export Table
	//
	
	pTC = new LM_TBL_CONTEXT;
	pTC->LoopInstance = LoopInstance;

	m_LSize1 = sizeof(ExportTableEntry);
	
	// Allocate a Listen object for the LoopDescriptor table.
	m_Listen1 = new(tUNCACHED) TSListen;

	// Initialize the modify row operation.
	status = m_Listen1->Initialize(
		this,								// DdmServices pDdmServices,
		LM_EXPORT_LISTEN_MASK,				// U32		ListenType,
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
		&m_LType1,							// U32**	ppListenTypeRet,
		&m_Export,							// void**	ppModifiedRecordRet,
		&m_LSize1,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&LmExportListenUpdate,		// pTSCallback_t pCallback,
		pTC									// void*	pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_Listen1->Send();
	
	return status;
	
} // LmStartListens


/*************************************************************************/
// LmLoopListenUpdate
// Called when a listen callback for the LoopDescriptor Table happens.
// Update the LoopDescriptors with the new row data.  Check if the
// DriveMonitor VDN has been set, send a scan message if it is.
/*************************************************************************/
STATUS	LoopMonitorIsm::LmLoopListenUpdate(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->LoopInstance);

	TRACE_ENTRY(LmLoopListenUpdate);
	
	// no work on first reply
	if (*m_LType & ListenInitialReply)
	{
		return(0);
	}
	
	// Check what type of listen reply
	if (*m_LType & ListenOnModifyOneRowAnyField)
	{
		// store the row data that just came in
		memcpy(LM_Loop_Desc[chip], m_Loop_Desc, sizeof(LoopDescriptorEntry));
		memcpy(LM_TS_Loop_Desc[chip], m_Loop_Desc, sizeof(LoopDescriptorEntry));
		
		// check if there is a DriveMonitor VDN and
		// the loop is up
		if (LM_Loop_Desc[chip]->vdnDriveMonitor &&
					LoopFlags[chip] == LM_STS_LOOP_UP)
		{
			// need to send a scan message
			LM_Send_DM_SCAN(chip);
		}
		
	}

	return status;
	
} // LmLoopListenUpdate


/*************************************************************************/
// LmExportListenUpdate
// Called when a listen callback for the Export Table happens.
/*************************************************************************/
STATUS	LoopMonitorIsm::LmExportListenUpdate(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT 	*pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				 chip = FCLOOPCHIP(pTC->LoopInstance);
	LM_ET_STATE		*pETState;
	LM_FCDB_STATE	*pFCDBState;

	TRACE_ENTRY(LmExportListenUpdate);
	
	// no work on first reply
	if (*m_LType1 & ListenInitialReply)
	{
		return(ercOK);
	}
	
	// Check what type of listen reply
	if (*m_LType1 & ListenOnModifyOneRowAnyField)
	{
		// check for the list entry for this Export table entry
		if ((pETS[chip]->Get((CONTAINER_ELEMENT &)pETState,
								(CONTAINER_KEY)m_Export->rid.LoPart)) == FALSE)
		{
			// did not exist, create new list entry
			pETState = new LM_ET_STATE;
			memset(pETState, 0, sizeof(LM_ET_STATE));
			
			pETState->ridET = m_Export->rid;
			pETState->state = m_Export->ReadyState;
			pETState->idlun.LUN = m_Export->ExportedLUN;
			pETState->idlun.id = m_Export->TargetId;
			pETState->idlun.HostId = m_Export->InitiatorId;
			pETState->vdNext = m_Export->vdNext;
			
			// add to the list, key by row id
			pETS[chip]->Add((CONTAINER_ELEMENT)pETState, (CONTAINER_KEY)m_Export->rid.LoPart);
		}
		
		// Row was modified - check the (new) state
		switch(m_Export->ReadyState)
		{
			case StateConfiguredAndExporting:
				// expose this ID (if needed)
				
				// check if ID already exported
				if ((pPDB[chip]->Get((CONTAINER_ELEMENT &)pFCDBState, (CONTAINER_KEY)m_Export->TargetId)) == FALSE)
				{
					if (m_Export->ReadyState == StateConfiguredAndExporting)
					{
						// no such id
						// expose new ID (better be LUN 0)
						m_ids[chip][0] = m_Export->TargetId;
						m_num_IDs[chip] = 1;
		
						LM_Expose_IDs(pTC->LoopInstance);
					}
				}
				else
				{
					// already exported, add this LUN to the count
					pFCDBState->num_LUNs++;
				}
				
				// next state is: StateConfiguredAndExported
				pETState->state = StateConfiguredAndExported;
				LmTblModifyExportState(&pETState->ridET, &pETState->state);
				break;
			
			case StateConfiguredAndExported:
				break;

			case StateConfiguredAndActive:
				break;

			case StateOffLine:
				// turn off ID (if no more LUNs)?
				break;

			case StateQuiesced:
				break;

			default:
			// all other states are NOPs
				break;
		}
	}
	else if (*m_LType1 & ListenOnInsertRow)
	{
		// Row was just inserted
		
		// Should not exist, but check anyway...
		// check for the list entry for this Export table entry
		if ((pETS[chip]->Get((CONTAINER_ELEMENT &)pETState,
								(CONTAINER_KEY)m_Export->rid.LoPart)) == FALSE)
		{
			// did not exist, create new list entry
			pETState = new LM_ET_STATE;
			memset(pETState, 0, sizeof(LM_ET_STATE));
			
			pETState->ridET = m_Export->rid;
			pETState->state = m_Export->ReadyState;
			pETState->idlun.LUN = m_Export->ExportedLUN;
			pETState->idlun.id = m_Export->TargetId;
			pETState->idlun.HostId = m_Export->InitiatorId;
			pETState->vdNext = m_Export->vdNext;
			
			// add to the list, key by row id
			pETS[chip]->Add((CONTAINER_ELEMENT)pETState, (CONTAINER_KEY)m_Export->rid.LoPart);
		}
		
		// check if ID already exported
		if ((pPDB[chip]->Get((CONTAINER_ELEMENT &)pFCDBState, (CONTAINER_KEY)m_Export->TargetId)) == FALSE)
		{
			if (m_Export->ReadyState == StateConfiguredAndExporting)
			{
				// no such id
				// expose new ID
				m_ids[chip][0] = m_Export->TargetId;
				m_num_IDs[chip] = 1;

				LM_Expose_IDs(pTC->LoopInstance);
			}
		}
		else
		{
			// already exported, add this LUN to the count
			pFCDBState->num_LUNs++;
		}
	}
	else if (*m_LType1 & ListenOnDeleteOneRow)
	{
		// Row was deleted
		
		// turn off ID if last LUN removed
		
		// update LoopDescriptor table
	}

	status = ercOK;
	return status;
	
} // LmExportListenUpdate


/*************************************************************************/
// LmExportListenUpdateEnd
// Complete a Listen callback
/*************************************************************************/
STATUS	LoopMonitorIsm::LmExportListenUpdateEnd(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmExportListenUpdateEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmExportListenUpdateEnd: status = ", status);
	}
	
	return ercOK;
	
} // LmExportListenUpdateEnd



