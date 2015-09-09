/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopTable.cpp
// 
// Description:
// This module is the Loop Monitor Table Handler
//	Handles:
//		Export Table Reads
//		Loop Descriptor Table Create/Read/Write/Update
//		Present Port Descriptor Table Create/Read/Write/Update
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopTable.cpp $
// 
// 7     1/09/00 5:00p Mpanas
// Fix LoopDescriptor ghosting problem
// 
// 6     9/14/99 9:00p Mpanas
// Minor trace level changes
// 
// 5     9/10/99 3:15p Mpanas
// Fix problems
// - portType
// - FCDB table update
// - More debug
// 
// 4     8/16/99 7:56p Mpanas
// One more change to use the new PTS
// Must check for Key Not Found also
// 
// 3     8/16/99 1:53p Mpanas
// Update to latest PTS model
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
#include "fields.h"

// Local Table method context
typedef struct _LM_TBL_CONTEXT {
	void							*pContext;			// saved context
	LM_FCDB_STATE					*pDBState;			// pointer to FCDB PTS entry
	Message							*pMsg;				// saved message
	LoopMonitorIsm::pLMCallback_t	 Callback;			// saved Callback address
	U32								 key;				// search key
	U32								 flags;				// 0 = no add, 1 = added
} LM_TBL_CONTEXT, *PLM_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// Table Update statemachines
//
// Updates these tables:
//	LoopDescriptor
//	FCPortDatabase
//
/*************************************************************************/

/*************************************************************************/
// LmLpTableUpdate
// Updates a table row entry in the LoopDescriptor table that has changed
// or been created.
// Build the context and call the first state in the state machine
/*************************************************************************/
STATUS	LoopMonitorIsm::LmLpTableUpdate(Message *pMsg,
					 void *pContext, U32 LoopId, pLMCallback_t Callback)
{
	TRACE_ENTRY(LmLpTableUpdate);
	
	STATUS			status;
	LM_TBL_CONTEXT *pTC = new LM_TBL_CONTEXT;
	
	// build a local context
	pTC->pMsg = pMsg;
	pTC->pDBState = NULL;
	pTC->pContext = pContext;
	pTC->Callback = Callback;
	pTC->key = LoopId;
	pTC->flags = 0;
		
	status = LmTblUpdReadLpDesc(pTC, ercOK);
	
	return status;

} // LmLpTableUpdate

/*************************************************************************/
// LmTblUpdReadDesc
// Read the LoopDescriptor row that needs to be updated into the
// LM_TS_Loop_Desc[chip] array
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmTblUpdReadLpDesc(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->key);

	TRACE_ENTRY(LmTblUpdReadLpDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rLmTblUpdReadLpDesc: status = ", status);
		if (status == ercEOF)
		{
			status = LmTblUpdAddLpDesc(pTC, ercOK);
			return status;
		}
	}
	
	// Allocate a ReadRow object for the LoopDescriptor Table.
	m_pSRCReadRow = new TSReadRow;

	// Use the LoopInstance number as the key to read in the row desired
	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		LOOP_DESCRIPTOR_TABLE,				// String64 rgbTableName,
		fdLD_LOOP_NUM,						// String64 prgbKeyFieldName,
		(void*)&pTC->key,					// void* pKeyFieldValue,
		sizeof(U32),						// U32 cbKeyFieldValue,
		LM_TS_Loop_Desc[chip],				// void* prgbRowData,
		sizeof(LoopDescriptorEntry),		// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&LmTblUpdModifyLpDesc, // pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
	
} // LmTblUpdReadLpDesc


/*************************************************************************/
// LmTblUpdModifyLpDesc
// LoopDescriptor row entry has been read into the LM_TS_Loop_Desc[chip]
// array.  Modify this row entry with the table data in the
// LM_Loop_Desc[chip] that may have changed.
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmTblUpdModifyLpDesc(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->key);
	LoopDescriptorEntry			*pLD, *pLD1;

	TRACE_ENTRY(LmTblUpdModifyLpDesc);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L3, "\n\rLmTblUpdModifyLpDesc: status = ", status);
		if ((status == ercEOF) || (status == ercKeyNotFound))
		{
			// row did not exist, add it
			status = LmTblUpdAddLpDesc(pTC, ercOK);
			return status;
		}
	}
	
	// Modify fields that could have changed
	pLD = LM_Loop_Desc[chip];
	pLD1 = LM_TS_Loop_Desc[chip];
	pLD1->ActualLoopState = pLD->ActualLoopState;
	pLD1->DesiredLoopState = pLD->DesiredLoopState;
	pLD1->IDsInUse = pLD->IDsInUse;
	memcpy(&pLD1->TargetIDs[0], &pLD->TargetIDs[0], 32);
	
	// update the local version with the modified row data
	// to make sure they are insync
	//*pLD = *pLD1; // don't work?
	memcpy(pLD, pLD1, sizeof(LoopDescriptorEntry));
	
	// Allocate an Modify Row object for the LoopDescriptor Table
	m_ModifyRow = new TSModifyRow;

	// use the RowID read as a key to find the row
	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		LOOP_DESCRIPTOR_TABLE,				// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&LM_TS_Loop_Desc[chip]->rid,	// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		LM_TS_Loop_Desc[chip],				// void* prgbRowData,
		sizeof(LoopDescriptorEntry),		// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// U32 cbMaxRowID,
		(pTSCallback_t)&LmLpTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // LmTblUpdModifyLpDesc

/*************************************************************************/
// LmTblUpdAddLpDesc
// Tried to modify a row that did not exist, so now we must add it to
// the LoopDescriptor table.  The data in the LM_Loop_Desc[chip] array is
// already filled out.  All we need to do is get the row id when it is
// returned and then copy the whole struct to the LM_TS_Loop_Desc[chip]
// array when we are done
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblUpdAddLpDesc(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->key);

	TRACE_ENTRY(LmTblUpdAddLpDesc);
	
	// row will be added
	pTC->flags = 1;
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskDescTable.
	m_pInsertRow = new TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,								// Ddm* ClientDdm
		LOOP_DESCRIPTOR_TABLE,				// prgbTableName
		LM_Loop_Desc[chip],					// prgbRowData
		sizeof(LoopDescriptorEntry),		// cbRowData
		(rowID *)LM_Loop_Desc[chip],		// *pRowIDRet, back into inserted record
		(pTSCallback_t)&LmLpTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
	
} // LmTblUpdAddLpDesc


/*************************************************************************/
// LmLpTableUpdateEnd
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS LoopMonitorIsm::LmLpTableUpdateEnd(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->key);
	pLMCallback_t 	Callback = pTC->Callback;
	
	TRACE_ENTRY(LmLpTableUpdateEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmLpTableUpdateEnd: status = ", status);
	}
	else
	{
		// if row was added, we need to copy it to the LM_TS_Loop_Desc[chip]
		// array, so we have the rowID and we can compare changed fields later
		if (pTC->flags)
		{
			memcpy(LM_TS_Loop_Desc[chip], LM_Loop_Desc[chip],
					sizeof(LoopDescriptorEntry));
		}
	}
			
	// don't reply to the message unless there is one
	if (pTC->pMsg)
		Reply(pTC->pMsg, 0);
		
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pContext, status);
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // LmLpTableUpdateEnd




/*************************************************************************/
// LmDBTableUpdate
// Updates/Creates a table row entry in the FC Port Database table.
// Build the context and call the first state in the state machine
/*************************************************************************/
STATUS	LoopMonitorIsm::LmDBTableUpdate(LM_FCDB_STATE *pDBState,
					 void *pContext, pLMCallback_t Callback)
{
	TRACE_ENTRY(LmDBTableUpdate);
	
	STATUS			status;
	LM_TBL_CONTEXT *pTC = new LM_TBL_CONTEXT;
	
	// build a local context
	pTC->pDBState = pDBState;
	pTC->pMsg = NULL;
	pTC->pContext = pContext;
	pTC->Callback = Callback;
	pTC->key = 0;
	pTC->flags = 0;
	
	TRACEF(TRACE_L3, ("\n\rLmDBTableUpdate: device Id = %d, type:%d",
							pDBState->pDBR->id, pDBState->pDBR->portType));

	// Check for an add or update
	if (pDBState->state == FCDB_STATE_ADD)
	{	
		status = LmTblUpdAddDBEntry(pTC, ercOK);
	}
	else
	{
		status = LmTblUpdModifyDBEntry(pTC, ercOK);
	}
	
	return status;

} // LmDBTableUpdate

/*************************************************************************/
// LmTblUpdModifyDBEntry
// Modify the FCPortDatabaseRecord row entry with the
// table data in the Container.
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmTblUpdModifyDBEntry(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblUpdModifyDBEntry);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rLmTblUpdModifyDBEntry: status = ", status);
	}
	
	// Use the data in the Container to modify the row
	// Fields should already be changed prior to the Modify.
	
	// Allocate an Modify Row object for the DiskStatusTable.
	m_ModifyRow = new TSModifyRow;

	// Initialize the modify row operation.
	status = m_ModifyRow->Initialize(
		this,								// DdmServices pDdmServices,
		FC_PORT_DATABASE_TABLE_NAME,		// String64 rgbTableName,
		"rid",								// String64 prgbKeyFieldName,
		(void*)&pTC->pDBState->ridDBR,		// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTC->pDBState->pDBR,				// void* prgbRowData,
		sizeof(FCPortDatabaseRecord),		// U32 cbRowData,
		0,									// U32 cRowsToModify,
		NULL,								// U32 *pcRowsModifiedRet,
		NULL,								// rowID *pRowIDRet,
		0,									// cbMaxRowID,
		(pTSCallback_t)&LmDBTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_ModifyRow->Send();
	
	return status;
	
} // LmTblUpdModifyDBEntry

/*************************************************************************/
// LmTblUpdAddDBEntry
// Tried to modify a row that did not exist, so now we must add it to
// the LoopDescriptor table.  The data in the LM_Loop_Desc[chip] array is
// already filled out.  All we need to do is get the row id when it is
// returned and then copy the whole struct to the LM_TS_Loop_Desc[chip]
// array when we are done
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblUpdAddDBEntry(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->key);

	TRACE_ENTRY(LmTblUpdAddDBEntry);
	
	// row will be added
	pTC->flags = 1;
	
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskDescTable.
	m_pInsertRow = new TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,								// Ddm* ClientDdm
		FC_PORT_DATABASE_TABLE_NAME,		// prgbTableName
		pTC->pDBState->pDBR,				// prgbRowData
		sizeof(FCPortDatabaseRecord),		// cbRowData
		(rowID *)&pTC->pDBState->ridDBR,	// *pRowIDRet, back into inserted record
		(pTSCallback_t)&LmDBTableUpdateEnd,	// pTSCallback_t pCallback,
		(void*)pTC							// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
	
} // LmTblUpdAddDBEntry


/*************************************************************************/
// LmDBTableUpdateEnd
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS LoopMonitorIsm::LmDBTableUpdateEnd(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				chip = FCLOOPCHIP(pTC->key);
	pLMCallback_t 	Callback = pTC->Callback;
	
	TRACE_ENTRY(LmDBTableUpdateEnd);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmDBTableUpdateEnd: status = ", status);
	}
	else
	{
		// if row was added, we need to copy the rid to the
		// rid field of the FCPortDatabaseRecord in pTC->pDBState->pDBR
		if (pTC->flags)
		{
			memcpy(pTC->pDBState->pDBR, &pTC->pDBState->ridDBR,
					sizeof(rowID));
			
			// state change to FCDB_STATE_UPDATE for next try
			pTC->pDBState->state = FCDB_STATE_UPDATE;
		}
	}
			
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pContext, status);
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // LmDBTableUpdateEnd


/*************************************************************************/
// LmTblModifyExportState
// Modify the ExportTableEntry ReadyState field with the new state passed.
// no callback or context are used. pState and pRow must point to static data!
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmTblModifyExportState(rowID *pRow, CTReadyState *pState)
{
	STATUS			 status;
	TSModifyField 	*pModifyField;
	
	TRACE_ENTRY(LmTblModifyExportState);
	
	// Allocate an Modify Field object for the Export table
	pModifyField = new TSModifyField;

	// Initialize the modify row operation.
	status = pModifyField->Initialize(
		this,							// DdmServices pDdmServices,
		EXPORT_TABLE,					// String64 rgbTableName,
		"rid",							// String64 prgbKeyFieldName,
		(void*)pRow,					// void* pKeyFieldValue,
		sizeof(rowID),					// U32 cbKeyFieldValue,
		"ReadyState",					// String64 prgbFieldName,
		(void*)pState,					// void* pbFieldValue,
		sizeof(CTReadyState),			// U32 cbFieldValue,
		0,								// U32 cRowsToModify,
		NULL,							// U32 *pcRowsModifiedRet,
		NULL,							// rowID *pRowIDRet,
		0,								// U32 cbMaxRowID,
		(pTSCallback_t)NULL,			// pTSCallback_t pCallback,
		(void*)NULL						// void* pContext
	);

	// Initiate the Modify Field operation.
	if (status == ercOK)
		pModifyField->Send();
	
	return status;
	
} // LmTblModifyExportState


/*************************************************************************/
// LmTblModifyAllExportState
// Modify the ExportTableEntry ReadyState fields of all the records that
// match the LoopInstance number with the new state passed.
// no callback or context are used.
// pLoopInstance and pStatus must point to static data!
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmTblModifyAllExportState(U32 *pLoopInstance, CTReadyState *pState)
{
	STATUS			 status;
	TSModifyField 	*pModifyField;
	
	TRACE_ENTRY(LmTblModifyAllExportState);
	
	// Allocate an Modify Field object for the Export table
	pModifyField = new TSModifyField;

	// Initialize the modify row operation.
	status = pModifyField->Initialize(
		this,							// DdmServices pDdmServices,
		EXPORT_TABLE,					// String64 rgbTableName,
		"FCInstance",					// String64 prgbKeyFieldName,
		(void*)pLoopInstance,			// void* pKeyFieldValue,
		sizeof(U32),					// U32 cbKeyFieldValue,
		"portStatus",					// String64 prgbFieldName,
		(void*)pState,					// void* pbFieldValue,
		sizeof(CTReadyState),			// U32 cbFieldValue,
		0,								// U32 cRowsToModify,
		NULL,							// U32 *pcRowsModifiedRet,
		NULL,							// rowID *pRowIDRet,
		0,								// U32 cbMaxRowID,
		(pTSCallback_t)NULL,			// pTSCallback_t pCallback,
		(void*)NULL						// void* pContext
	);

	// Initiate the Modify Field operation.
	if (status == ercOK)
		pModifyField->Send();
	
	return status;
	
} // LmTblModifyAllExportState


/*************************************************************************/
// LmTblModifyPortDBStatus
// Modify the FCPortDatabase portStatus fields of all the records that
// match the LoopDescriptors row ID with the new status passed.
// no callback or context are used.
// pLoopDesc and pStatus must point to static data!
/*************************************************************************/
STATUS 	LoopMonitorIsm::LmTblModifyPortDBStatus(rowID *pLoopDesc, U32 *pStatus)
{
	STATUS			 status;
	TSModifyField 	*pModifyField;
	
	TRACE_ENTRY(LmTblModifyPortDBStatus);
	
	// Allocate an Modify Field object for the FCPortDatabase table
	pModifyField = new TSModifyField;

	// Initialize the modify row operation.
	status = pModifyField->Initialize(
		this,							// DdmServices pDdmServices,
		FC_PORT_DATABASE_TABLE_NAME,	// String64 rgbTableName,
		"rid",							// String64 prgbKeyFieldName,
		(void*)pLoopDesc,				// void* pKeyFieldValue,
		sizeof(rowID),					// U32 cbKeyFieldValue,
		"portStatus",					// String64 prgbFieldName,
		(void*)pStatus,					// void* pbFieldValue,
		sizeof(U32),					// U32 cbFieldValue,
		0,								// U32 cRowsToModify,
		NULL,							// U32 *pcRowsModifiedRet,
		NULL,							// rowID *pRowIDRet,
		0,								// U32 cbMaxRowID,
		(pTSCallback_t)NULL,			// pTSCallback_t pCallback,
		(void*)NULL						// void* pContext
	);

	// Initiate the Modify Field operation.
	if (status == ercOK)
		pModifyField->Send();
	
	return status;
	
} // LmTblModifyPortDBStatus

