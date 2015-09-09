/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopTableInit.cpp
// 
// Description:
// This module is the Loop Monitor Table Initialization Handler
//	Handles:
//		Loop Descriptor Table Create/Read/Write/Update
//		FC Port Descriptor Table Create/Read/Write/Update
//		Export Table Reads
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopTableInit.cpp $
// 
// 7     2/07/00 9:25p Cchan
// Added vdNext reference in the ETState container class.
// 
// 6     1/09/00 7:50p Mpanas
// Set the IDsInUse field and the TargetIDs[] array of the
// LoopDescriptor correctly.
// 
// 5     12/21/99 12:53p Mpanas
// Fix upsteam ghosting problem
// 
// 4     11/03/99 2:51p Jlane
// Make tables persistant.
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
	U32						 chip;				// chip number
	void					*buffer;			// data buffer if used
	Message					*pMsg;				// saved Init message
} LM_TBL_CONTEXT, *PLM_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// LmTableInitialize
// Start of the table initialization state machine, called from the
// Ddm Initialize method
//
// Creates these tables if they do not exist:
//	LoopDescriptor Table
//	FCPortDatabase Table
//
// Reads these tables:
//	LoopDescriptor (adds entries too)
//	Export Table
//	FCPortDatabase
//
/*************************************************************************/
STATUS	LoopMonitorIsm::LmTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(LmTableInitialize);
	
	STATUS			status = ercOK;
	LM_TBL_CONTEXT *pTC = new LM_TBL_CONTEXT;
	
	// save the init message
	pTC->pMsg = pMsg;
	
	// start at the first loop configured
	pTC->chip = FCLOOPCHIP(config.FC_instance[0]);
	
	// initialize the sequence
	m_next = 0;
	pTC->buffer = NULL;
	
	// This is the code to create the Loop Descriptor Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	fieldDef *pciLoopDescTable_FieldDefs = 
				(fieldDef*)new(tPCI) char[cbLoop_Descriptor_Table_FieldDefs];

	memcpy( (char*)pciLoopDescTable_FieldDefs,
			(char*)Loop_Descriptor_Table_FieldDefs,
			cbLoop_Descriptor_Table_FieldDefs
		  ); 
		  
	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		LOOP_DESCRIPTOR_TABLE,				// String64 prgbTableName,
		pciLoopDescTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbLoop_Descriptor_Table_FieldDefs,	// U32 cbrgFieldDefs,
		16,									// U32 cEntriesRsv,
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&LmTblReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	m_pDefineTable->Send();

	return status;
} // LmTableInitialize

/*************************************************************************/
// LmTblReply1
// Reply from creating the Loop Descriptor Table.  Create the 
// FC Port Database Table
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply1(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblReply1);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply1: status = ", status);
	}
	
	// This will create the FC Port Database Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	fieldDef *pciFCPortDBTable_FieldDefs = 
				(fieldDef*)new(tPCI) char[cbFCPortDatabase_FieldDefs];

	memcpy( (char*)pciFCPortDBTable_FieldDefs,
			(char*)FCPortDatabaseTable_FieldDefs,
			cbFCPortDatabase_FieldDefs
		  ); 
		  
	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		FC_PORT_DATABASE_TABLE_NAME,		// String64 prgbTableName,
		pciFCPortDBTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbFCPortDatabase_FieldDefs,			// U32 cbrgFieldDefs,
		150,								// U32 cEntriesRsv,
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&LmTblReply2,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	m_pDefineTable->Send();

	return status;
} // LmTblReply1

/*************************************************************************/
// LmTblReply2
// Create all the LoopDescriptor Records configured if they do not exist.
// This is also the Callback for the Update Table code
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply2(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32				loop;

	TRACE_ENTRY(LmTblReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply2: status = ", status);
	}
	
	loop = config.FC_instance[m_next];
	
	// do the next loop configured
	if (m_next++ < config.num_loops)
	{
		// Create a new record in the LoopDescriptor table
		status = LmLpTableUpdate(NULL, (void *)pTC, 
					loop, 
					(pLMCallback_t)&LmTblReply2);
		
		return(status);
	}
	
	// Done Creating/Updating LoopDescriptor records
	m_next = 0;
	
	status = LmTblReply3(pTC, ercOK);
	
	return status;
	
} // LmTblReply2

/*************************************************************************/
// LmTblReply3
// Read how many entries (rows) there currently are in the Export Table.
// Use this count to to build the array for local use.
// TODO: This section of code will be removed later
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply3(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblReply3);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply3: status = ", status);
	}
	
	m_nTableRows = 0;

	// Allocate an Get Table Defs object for the Export Table.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the TSGetTableDef operation.
	m_pTSGetTableDef->Initialize( 
		this,
		EXPORT_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nTableRows,				// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&LmTblReply4,	// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the enumerate table operation.
	m_pTSGetTableDef->Send();
	
	return status;
} // LmTblReply3

/*************************************************************************/
// LmTblReply4
// Number of rows are now read into m_nTableRows.  Use this count + 2 as a
// max to create the local Export table for the current loop instance.
// Use the LoopInstance as a key to read the Export Table for all the
// entries that match.
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply4(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTc = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblReply4);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply3: status = ", status);
	}
	
	// check for a prior table
	if (Lm_TS_Export[pTc->chip])
	{
		delete Lm_TS_Export[pTc->chip];
	}
	
	TRACE_HEX(TRACE_L8, "\n\rLm: num of Export Table rows = ", m_nTableRows);
	
	// Allocate the Local Disk Descriptor Table with enough room for all rows
	Lm_TS_Export[pTc->chip] = new(tPCI) ExportTableEntry[m_nTableRows+2];
		
	// Allocate a ReadRow object for the Export Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		EXPORT_TABLE,						// String64 rgbTableName,
		"FCInstance",						// String64 prgbKeyFieldName,
		&config.FC_instance[pTc->chip],		// void* pKeyFieldValue,
		sizeof(U32),						// U32 cbKeyFieldValue,
		Lm_TS_Export[pTc->chip],			// void* prgbRowData,
		sizeof(ExportTableEntry) * (m_nTableRows +2),	// U32 cbRowDataRetMax, max size
		&m_nTableRowsRead,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&LmTblReply5,		// pTSCallback_t pCallback,
		(void*)pTc							// void* pContext
	);

	// Initiate the ReadRow table operation.
	m_pSRCReadRow->Send();
	
	return status;
} // LmTblReply4

/*************************************************************************/
// LmTblReply5
// Last CallBack to read Export Table, do all configured loops.
// Check the export table entries read to see if any new IDs to configure.
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply5(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblReply5);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply5: status = ", status);
		m_nTableRowsRead = 0;
	}
	
	// save the number of rows read
	nExportRows[pTC->chip] = m_nTableRowsRead;
	
	// check the list for any IDs that need to be exposed
	LmTblReplyCheckExports(config.FC_instance[m_next]);
	
	// free the memory used since we are done with it
	delete Lm_TS_Export[pTC->chip];
	Lm_TS_Export[pTC->chip] = NULL;
	
	// do all the loops configured
	if (++m_next < config.num_loops)
	{
		// do the next loop
		pTC->chip = FCLOOPCHIP(config.FC_instance[m_next]);
		
		status = LmTblReply3(pTC, ercOK);
		return(status);
	}
	
	// start at the first loop configured again
	pTC->chip = FCLOOPCHIP(config.FC_instance[0]);
	m_next = 0;
	
	// read all the FCPortdatabase entries
	status = LmTblReply6(pTC, ercOK);
	
	return status;
	
} // LmTblReply5


/*************************************************************************/
// LmTblReplyCheckExports
// Scan the entire set of Export table entries for any that can be exposed.
// Count the number of LUNs for each ID, update local tables and update
// the Export table entry ReadyState with any changed state.
/*************************************************************************/
void LoopMonitorIsm::LmTblReplyCheckExports(U32 loop)
{
	U8				ids[32], num_luns[32], num_ids, index;
	U32				chip = FCLOOPCHIP(loop);
	ExportTableEntry	*pEt;
	LM_ET_STATE			*pETState;
	LM_FCDB_STATE		*pFCDBState;
	U8					*p;

	TRACE_ENTRY(LmTblReplyCheckExports);
	
	if (m_nTableRowsRead == 0)
	{
		// no configured exports
		return;
	}

	// check the list for any IDs that need to be exposed
	// clear the local arrays
	memset(num_luns, 0, 32);
	memset(ids, 0, 32);
	num_ids = 0;
	
	pEt = Lm_TS_Export[chip];
	
	// scan all entries just read, build the ID/LUN lists
	for (int i = 0; m_nTableRowsRead--; )
	{
		// check for the list entry for this Export table entry
		if ((pETS[chip]->Get((CONTAINER_ELEMENT &)pETState,
								(CONTAINER_KEY)pEt->rid.LoPart)) == FALSE)
		{
			// did not exist, create new list entry
			pETState = new LM_ET_STATE;
			memset(pETState, 0, sizeof(LM_ET_STATE));
			
			pETState->ridET = pEt->rid;
			pETState->state = pEt->ReadyState;
			pETState->idlun.LUN = pEt->ExportedLUN;
			pETState->idlun.id = pEt->TargetId;
			pETState->idlun.HostId = pEt->InitiatorId;
			pETState->vdNext = pEt->vdNext;
			
			// add to the list, key by row id
			pETS[chip]->Add((CONTAINER_ELEMENT)pETState, (CONTAINER_KEY)pEt->rid.LoPart);
		}
		
		// check if ID already exported
		if ((pPDB[chip]->Get((CONTAINER_ELEMENT &)pFCDBState, (CONTAINER_KEY)pEt->TargetId)) == FALSE)
		{
			// no such id exposed
			// is this ID/LUN at a state we can export?  remember, this is
			// a startup case only...
			if ((pEt->ReadyState == StateConfiguredAndExported) ||
					(pEt->ReadyState == StateConfiguredAndActive))
			{				
				if (num_ids && 
					(p = (U8*)memchr(&ids[0], pEt->TargetId, num_ids)))
				{
					// found ID, get index
					index = p - &ids[0];
					num_luns[index]++;		// add a new LUN
				}
				else
				{
					// not found, add new ID/LUN
					num_luns[i]++;
					ids[i++] = (U8)pEt->TargetId;
					num_ids++;
				}
				
				if (pEt->ReadyState == StateConfiguredAndActive)
				{				
					// next state is: StateConfiguredAndExported
					pETState->state = StateConfiguredAndExported;
					LmTblModifyExportState(&pETState->ridET, &pETState->state);
				}
			}
		}
		else
		{
			// already exported, add this LUN to the count
			pFCDBState->num_LUNs++;
		}

		// next
		pEt++;
	}
	// update member data with the number and count of the new IDs found
	m_num_IDs[chip] = num_ids;
	memcpy(m_ids[chip], ids, 32);
	memcpy(m_num_luns[chip], num_luns, 32);
	
	// expose the IDs found that are not already exposed
	if (num_ids)
	{
		LM_Expose_IDs(loop);
	}
	
} // LmTblReplyCheckExports


/*************************************************************************/
// LmTblReply6
// Read how many entries (rows) there currently are in the FCPortDatabase Table.
// Use this count to to build the array for local use.
// TODO: This section of code will be removed later
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply6(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTC = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblReply6);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply6: status = ", status);
	}
	
	m_nTableRows = 0;

	// Allocate a Get Table Defs object for the FCPortDatabase Table.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the TSGetTableDef operation.
	status = m_pTSGetTableDef->Initialize( 
		this,
		FC_PORT_DATABASE_TABLE_NAME,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nTableRows,				// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&LmTblReply7,	// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pTSGetTableDef->Send();
	
	return status;
} // LmTblReply6

/*************************************************************************/
// LmTblReply7
// Number of rows are now read into m_nTableRows.  Use this count + 2 as a
// max to create the local FCPortDatabase table for the current loop instance.
// Use the LoopDescriptor rid as a key to read the FCPortDatabase Table
// for all the entries that match.
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReply7(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT *pTc = (LM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(LmTblReply7);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReply7: status = ", status);
	}
	
	// check for a prior table
	if (pTc->buffer)
	{
		delete pTc->buffer;
	}
	
	TRACE_HEX(TRACE_L8, "\n\rLm: num of FCPortDatabase Table rows = ", m_nTableRows);
	
	// Allocate the Local FCPortDatabase Table with enough room for all rows
	pTc->buffer = new(tPCI) FCPortDatabaseRecord[m_nTableRows+2];
		
	// Allocate a ReadRow object for the FCPortDatabase Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		FC_PORT_DATABASE_TABLE_NAME,		// String64 rgbTableName,
		"ridLoopDescriptor",				// String64 prgbKeyFieldName,
		&LM_TS_Loop_Desc[pTc->chip]->rid,	// void* pKeyFieldValue,
		sizeof(rowID),						// U32 cbKeyFieldValue,
		pTc->buffer,						// void* prgbRowData,
		sizeof(FCPortDatabaseRecord) * (m_nTableRows +2), // U32 cbRowDataRetMax, max size
		&m_nTableRowsRead,					// U32 *pcRowsReadRet,
		(pTSCallback_t)&LmTblReplyLast,		// pTSCallback_t pCallback,
		(void*)pTc							// void* pContext
	);

	// Initiate the ReadRow table operation.
	m_pSRCReadRow->Send();
	
	return status;
} // LmTblReply7

/*************************************************************************/
// LmTblReplyLast
// Last CallBack, start the listens needed and return the Initialize
// message we saved way back when.
/*************************************************************************/
STATUS LoopMonitorIsm::LmTblReplyLast(void *pClientContext, STATUS status)
{
	LM_TBL_CONTEXT 			*pTC = (LM_TBL_CONTEXT *)pClientContext;
	U32						 chip = pTC->chip;  // faster
	U32						 rows;
	FCPortDatabaseRecord	*pDb, *pDbX;
	LM_FCDB_STATE			*pFCDBState;

	TRACE_ENTRY(LmTblReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rLmTblReplyLast: status = ", status);
		m_nTableRowsRead = 0;
	}
	
	// any rows read?
	if (m_nTableRowsRead)
	{
		pDbX = (FCPortDatabaseRecord *) pTC->buffer;
	
		// check the list for any IDs that need to be added to our container list
		for (rows = m_nTableRowsRead ;rows--; pDbX++)
		{
			if ((pPDB[chip]->Get((CONTAINER_ELEMENT &)pFCDBState, (CONTAINER_KEY)pDbX->id)) == FALSE)
			{
				// did not exist
				pFCDBState = new(tPCI) LM_FCDB_STATE;
				memset(pFCDBState, 0, sizeof(LM_FCDB_STATE));
				pDb = new(tPCI) FCPortDatabaseRecord;
				pFCDBState->pDBR = pDb;
				pFCDBState->state = FCDB_STATE_UPDATE;
				
				// Add new entry
				memcpy(pDb, pDbX, sizeof(FCPortDatabaseRecord));
												
				memcpy(pFCDBState->pDBR, pDb, sizeof(rowID));
			
				// add to the list
				pPDB[chip]->Add((CONTAINER_ELEMENT)pFCDBState, (CONTAINER_KEY)pDbX->id);
			}
		}
	}
	
	// do all the loops configured
	if (++m_next < config.num_loops)
	{
		// do the next loop
		pTC->chip = FCLOOPCHIP(config.FC_instance[m_next]);
		
		status = LmTblReply6(pTC, ercOK);
		return(status);
	}
	
	if (pTC->buffer)
	{
		delete pTC->buffer;
	}

	// Start Listens needed for the LoopMonitor
	// Listen on Export table, FCPortDatabase table and the
	// LoopDescriptor table.
	LmListenInitialize();
	
	// don't reply to the init message until the last TS message
	if (pTC->pMsg)
		Reply(pTC->pMsg); 
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // LmTblReplyLast


