/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: CmdServer.cpp
// 
// Description:
// Server-side of Odyssey inter-DDM request queue machinery.
// 
// $Log: /Gemini/Odyssey/CmdQueues/CmdServer.cpp $
// 
// 21    11/17/99 3:24p Joehler
// Add variable size status and command data to command queues
// 
// 20    10/29/99 2:14p Sgavarre
// DeleteRow, rowid was being used on the stack
// 
// 19    8/20/99 3:02p Dpatel
// Added CheckAndExecute() to check for duplicate cmds.
// 
// 18    8/16/99 6:42p Dpatel
// used the address of rowid as handle, instead of outstanding cmd..
// 
// 17    8/16/99 2:42p Dpatel
// ReplyContinuous needs reply with row flag OR'd.
// 
// 16    8/15/99 12:08p Jlane
// Added parameters for new TS interface changes to delete row operation.
// 
// 15    8/02/99 2:40p Dpatel
// return status was not being checked for csrvDeleteRow.
// 
// 14    7/27/99 6:07p Dpatel
// checked the return status for initialize...
// 
// 13    7/23/99 5:53p Dpatel
// fixed the return of Send from STATUS to void.
// 
// 12    7/22/99 4:03p Dpatel
// removed the return type for Send() according to PTS changes.
// 
// 11    7/14/99 4:22p Dpatel
// DeleteRow interface changed to take U32 instead of rowid *.
// 
// 10    6/24/99 7:12p Ewedel
// Fixed incorrect assert(), added standard file header (from
// Odyssey/template.c).
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "CmdServer.h"

//************************************************************************
//	CONSTRUCTOR:
//
//************************************************************************
CmdServer::CmdServer(
					String64			cmdQueueName,
					U32					sizeofCmdData,
					U32					sizeofStatusData,
					DdmServices			*pParentDdm,
					pCmdCallback_t		cmdInsertionCallback) : DdmServices( pParentDdm )
{
	m_isInitialized		= false;
#ifdef VAR_CMD_QUEUE
	m_varSizeData		= false;
#endif
	m_pDdmServices		= pParentDdm;

	// min size of Cmd data = 4, since opcode required at min
	assert(sizeofCmdData > 3);

	strcpy(m_CQTableName,(char *)cmdQueueName);
	m_sizeofCmdData	= sizeofCmdData;
	strcpy(m_SQTableName,(char *)cmdQueueName);
	strcat(m_SQTableName,"_STATUS");
	m_sizeofStatusData	= sizeofStatusData;
	m_objectInitializedCallback = NULL;

	// save the cmd insertion cb
	m_CQInsertRowCallback	= cmdInsertionCallback;

	// Listener stuff
	m_CQListenerType = 0;
	m_pCQModifiedRecord	= NULL;
	m_pCQTableDataRet = NULL;
	m_sizeofCQTableDataRet = 0;
	m_pCQListenObject = NULL;

	m_useTerminate = 0;
	
	// Initialize outstanding cmd list's head/tail
	pHead = NULL;
	pTail = NULL;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	CONSTRUCTOR:
//
//************************************************************************
CmdServer::CmdServer(
					String64			cmdQueueName,
					DdmServices			*pParentDdm,
					pCmdCallback_t		cmdInsertionCallback) : DdmServices( pParentDdm )
{
	m_isInitialized		= false;
	m_varSizeData		= true;
	m_pDdmServices		= pParentDdm;

	strcpy(m_CQTableName,(char *)cmdQueueName);
	strcpy(m_SQTableName,(char *)cmdQueueName);
	strcat(m_SQTableName,"_STATUS");
	m_objectInitializedCallback = NULL;

	// save the cmd insertion cb
	m_CQInsertRowCallback	= cmdInsertionCallback;

	// Listener stuff
	m_CQListenerType = 0;
	m_pCQModifiedRecord	= NULL;
	m_pCQTableDataRet = NULL;
	m_sizeofCQTableDataRet = 0;
	m_pCQListenObject = NULL;

	m_useTerminate = 0;
	
	// Initialize outstanding cmd list's head/tail
	pHead = NULL;
	pTail = NULL;
}
#endif

//************************************************************************
//	DESTRUCTOR:
//
//		Delete any memory allocated
//
//************************************************************************

CmdServer::~CmdServer()
{
	assert(m_useTerminate != 0);
	// don't delete these.  they are allocated and deleted by the table
	// service
	/*if (m_pCQModifiedRecord)
		delete m_pCQModifiedRecord;
	if (m_pCQTableDataRet)
		delete m_pCQTableDataRet;*/
}

#ifndef VAR_CMD_QUEUE
//************************************************************************
//	PUBLIC:
//		csrvInitialize
//
//	Will Define the Command Queue and Status Queues
//	Also a listener will be registered on the CQ for any inserts
//	into the CQ
//
//************************************************************************
STATUS CmdServer
::csrvInitialize(pInitializeCallback_t	objectInitializedCallback)
{
	STATUS		status;

	if (m_isInitialized == TRUE){
		// we are already initialized once..
		return !OK;
	}

	m_objectInitializedCallback = objectInitializedCallback;
	status = csrvCQDefineTable();
	if (status)
		return status;
	status = csrvSQDefineTable();
	return status;
}
#else
//************************************************************************
//	PUBLIC:
//		csrvInitialize
//
//	Will Define the Command Queue and Status Queues
//	Also a listener will be registered on the CQ for any inserts
//	into the CQ
//
//************************************************************************
STATUS CmdServer
::csrvInitialize(pInitializeCallback_t	objectInitializedCallback)
{
	STATUS		status;

	if (m_isInitialized == TRUE){
		// we are already initialized once..
		return !OK;
	}

	m_objectInitializedCallback = objectInitializedCallback;
	if (m_varSizeData==false)
		status = csrvCQDefineTable();
	else
		status = csrvCQDefineVLTable();
	if (status)
		return status;
	if (m_varSizeData==false)
		status = csrvSQDefineTable();
	else
		status = csrvSQDefineVLTable();
	return status;
}
#endif



//************************************************************************
//	PUBLIC:
//		csrvReportStatus:
//	Functionality:
//		Will insert the status record into the SQ
//
//	Operation:
//			Append the CQ Record to the SQ record
//			Insert the SQ record	
//			Delete the SQ record
//			Delete the CQ record
//
//************************************************************************
STATUS CmdServer
::csrvReportStatus(
			HANDLE			handle,
			SQ_STATUS_TYPE	type,	
			STATUS			statusCode, 
			void			*pResultData, 
			void			*pCmdData)
{
	STATUS				status;
	U32					sizeofStatusRecord;
	char				*pStatusRecordResultData;
	char				*pStatusRecordCommandData;
	CommandQueueRecord	*pCQRecord;

#ifdef VAR_CMD_QUEUE
	assert(m_varSizeData==false);
#endif

	if (m_sizeofStatusData == 0){
		assert(pResultData == NULL);
	}

	// Before calling this method, everything should be initialized
	if (m_isInitialized != TRUE){
		return !OK;
	}


	TSInsertRow *pTSInsertRow	= new TSInsertRow;
	pCQRecord					= NULL;

	CONTEXT	*pContext			= new CONTEXT;
	memset(pContext, 0, sizeof(CONTEXT));

	pContext->state				= csrvSQ_ROW_INSERTED_STATE;
	sizeofStatusRecord			= sizeof(StatusQueueRecord) + m_sizeofStatusData +
									sizeof(CommandQueueRecord) + m_sizeofCmdData;

	pContext->pData				= new char[sizeofStatusRecord];
	memset(pContext->pData,0,sizeofStatusRecord);

	StatusQueueRecord *pSQRecord = (StatusQueueRecord *) pContext->pData;
	pStatusRecordResultData = (char *)((char *)pContext->pData + sizeof(StatusQueueRecord));
	pStatusRecordCommandData = (char *)((char *)pStatusRecordResultData + m_sizeofStatusData);

	pSQRecord->type = type;
	pSQRecord->statusCode = statusCode;

	// if handle == NULL, make sure it is an event
	if (handle == NULL){
		if (pSQRecord->type != SQ_EVENT_STATUS){
			return !OK;
		}
		if (pCmdData){
			return !OK;
		}
	} else {
		// now match up handle, with row id and insert the row id for the
		// command queue record
		OUTSTANDING_CMD		*pOutstandingCmd = NULL;
		csrvGetOutstandingCmd(handle, &pOutstandingCmd);
		if (pOutstandingCmd != NULL){
			// prepare the CQ Record data by appending row id to CQ Data
			if (pCmdData){
				pCQRecord = (CommandQueueRecord *)new char[sizeof(CommandQueueRecord) + m_sizeofCmdData];
				pCQRecord->thisRID = pOutstandingCmd->cmdRowId;
				memcpy(((char *)pCQRecord + sizeof(CommandQueueRecord)),pCmdData,m_sizeofCmdData);
			}
			csrvDeleteOutstandingCmd(pOutstandingCmd);
		}
	}

	if (pResultData){
		memcpy(pStatusRecordResultData, pResultData, m_sizeofStatusData);
	}

	if (pCQRecord){
		// copy the original cmd's CQ data here
		memcpy((void *)pStatusRecordCommandData, 
					(void *)pCQRecord,
					sizeof(CommandQueueRecord) + m_sizeofCmdData);
	}


	status = pTSInsertRow->Initialize(
		this,
		m_SQTableName,
		pSQRecord,
		sizeofStatusRecord,
		&pContext->newRowId,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSInsertRow->Send();

	// Also delete the CQ record now (for events no cmd record exists)
	if (pCmdData){
		CONTEXT			*pDeleteCQContext = new (tZERO) CONTEXT;

		pDeleteCQContext->state = csrvCQ_ROW_DELETED_STATE;
		pDeleteCQContext->newRowId = pCQRecord->thisRID;
		csrvDeleteRow(m_CQTableName,&pDeleteCQContext->newRowId, pDeleteCQContext);
		delete pCQRecord;
	}
	return status;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	PUBLIC:
//		csrvReportStatus:
//	Functionality:
//		Will insert the status record into the SQ
//
//	Operation:
//			Append the CQ Record to the SQ record
//			Insert the SQ record	
//			Delete the SQ record
//			Delete the CQ record
//
//************************************************************************
STATUS CmdServer
::csrvReportStatus(
			HANDLE			handle,
			SQ_STATUS_TYPE	type,	
			STATUS			statusCode, 
			void			*pResultData, 
			U32				cbResultData,
			void			*pCmdData,
			U32				cbCmdData)
{
	STATUS				status;
	RowId				cmdRowId;
	assert(m_varSizeData = true);

	if (cbResultData == 0){
		assert(pResultData == NULL);
	}

	// Before calling this method, everything should be initialized
	if (m_isInitialized != TRUE){
		return !OK;
	}

	TSInsertVLRow *pTSInsertRow	= new TSInsertVLRow;

	CONTEXT	*pContext			= new (tZERO) CONTEXT;
	pContext->state				= csrvSQ_ROW_INSERTED_STATE;

	// if handle == NULL, make sure it is an event
	if (handle == NULL)
	{
		if (type != SQ_EVENT_STATUS)
			return !OK;
		if (pCmdData)
			return !OK;

		// temporary restriction on size of variable pts fields
		assert(cbResultData<512);
		assert(!(cbResultData%4));

		pContext->pData = new SQRecord(type, statusCode, cbResultData, (U8*)pResultData);
	} 
	else 
	{
		// now match up handle, with row id and insert the row id for the
		// command queue record
		OUTSTANDING_CMD		*pOutstandingCmd = NULL;
		csrvGetOutstandingCmd(handle, &pOutstandingCmd);
		assert (pOutstandingCmd);
		cmdRowId = pOutstandingCmd->cmdRowId;

		// temporary restriction on size of variable pts fields
		assert(cbResultData<512);
		assert(!(cbResultData%4));
		assert(cbCmdData<512);
		assert(!(cbCmdData%4));

		pContext->pData = new SQRecord(type, statusCode, pOutstandingCmd->cmdRowId,
			cbCmdData, (U8*)pCmdData, cbResultData, (U8*)pResultData);
		csrvDeleteOutstandingCmd(pOutstandingCmd);
	}
	
	status = pTSInsertRow->Initialize(
		this,
		m_SQTableName,
		(SQRecord*)pContext->pData,
		&pContext->newRowId,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSInsertRow->Send();

	// Also delete the CQ record now (for events no cmd record exists)
	if (pCmdData){
		CONTEXT			*pDeleteCQContext = new (tZERO) CONTEXT;

		pDeleteCQContext->state = csrvCQ_ROW_DELETED_STATE;
		pDeleteCQContext->newRowId = cmdRowId;
		csrvDeleteRow(m_CQTableName,&pDeleteCQContext->newRowId, pDeleteCQContext);
	}
	return status;
}
#endif
//************************************************************************
//	PUBLIC:
//		csrvReportCmdStatus:
//	Functionality:
//		Will report the cmd status
//
//************************************************************************
STATUS CmdServer
::csrvReportCmdStatus(
		HANDLE				cmdHandle,	
		STATUS				statusCode, 
		void				*pResultData, 
		void				*pCmdData)
{
	return csrvReportStatus(
			cmdHandle,
			SQ_COMMAND_STATUS,
			statusCode,
			pResultData,
			pCmdData);
}
#ifdef VAR_CMD_QUEUE
STATUS CmdServer
::csrvReportCmdStatus(
		HANDLE				cmdHandle,	
		STATUS				statusCode, 
		void				*pResultData, 
		U32					cbResultData,
		void				*pCmdData,
		U32					cbCmdData)
{
	return csrvReportStatus(
			cmdHandle,
			SQ_COMMAND_STATUS,
			statusCode,
			pResultData,
			cbResultData,
			pCmdData,
			cbCmdData);
}
#endif


//************************************************************************
//	PUBLIC:
//		csrvReportEvent:
//	Functionality:
//		Will report the Event to status queue
//
//************************************************************************
STATUS CmdServer
::csrvReportEvent(
		STATUS				statusCode, 
		void				*pResultData)
{
	return csrvReportStatus(
			NULL,
			SQ_EVENT_STATUS,
			statusCode,
			pResultData,
			NULL);
}
#ifdef VAR_CMD_QUEUE
STATUS CmdServer
::csrvReportEvent(
		STATUS				statusCode, 
		void				*pResultData,
		U32					cbResultData)
{
	return csrvReportStatus(
			NULL,
			SQ_EVENT_STATUS,
			statusCode,
			pResultData,
			cbResultData,
			NULL,
			NULL);
}
#endif

//************************************************************************
//	PUBLIC:
//		csrvTerminate
//	Functionality:
//		Stop Listening for Inserts into CQ
//
//************************************************************************
void CmdServer
::csrvTerminate()
{
	// Stop listening on inserts into CQ
	if (m_pCQListenObject)
		m_pCQListenObject->Stop();
	m_pCQListenObject = NULL;
	// Remove any outstanding cmds from the list
	csrvDeleteAllOutstandingCmds();

	m_useTerminate = 1;
	delete this;
}



//************************************************************************
//	PRIVATE METHODS BEGIN HERE
//************************************************************************

//************************************************************************
//	PRIVATE:
//		csrvDeleteRow
//	
//
//************************************************************************
STATUS CmdServer
::csrvDeleteRow(
			String64	tableName,
			rowID		*pRowId,
			void		*_pContext)
{
	STATUS		status;
	CONTEXT		*pContext;

	TSDeleteRow *pTSDeleteRow = new TSDeleteRow; 

	pContext = (CONTEXT *)_pContext;
	if (pContext->pData){
		// delete the SQ record data
		delete pContext->pData;
		pContext->pData = NULL;
	}
	pContext->state = pContext->state;
	
	status = pTSDeleteRow->Initialize(
		this,
		tableName,
		CT_PTS_RID_FIELD_NAME,
		pRowId,
		sizeof(rowID),
		1,
		&pContext->rowsDeleted,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDeleteRow->Send();
	return status;
}


//************************************************************************
//	PRIVATE:
//		csrvCQDefineTable:
//	
//			Defines the command queue table
//
//************************************************************************
STATUS CmdServer::csrvCQDefineTable()
{
	STATUS			status;
	U32				sizeofFieldDef;
	fieldDef		*pFieldDef;

	CONTEXT			*pContext = new CONTEXT;
	memset(pContext, 0, sizeof(CONTEXT));

	// change the size of the binary data in field defs.
	pFieldDef = &CommandQueueTable_FieldDefs[0];
	sizeofFieldDef = sizeofCommandQueueTable_FieldDefs;

	pFieldDef->cbField = m_sizeofCmdData;
	pContext->pData = new char[sizeofFieldDef];
	memcpy(pContext->pData,CommandQueueTable_FieldDefs,sizeofFieldDef);
	

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csrvCQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_CQTableName,
		(fieldDef *)pContext->pData,		// fieldDef* prgFieldDefsRet,
		sizeofFieldDef,						// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	PRIVATE:
//		csrvCQDefineVLTable:
//	
//			Defines the command queue table
//
//************************************************************************
STATUS CmdServer::csrvCQDefineVLTable()
{
	STATUS			status;

	CONTEXT			*pContext = new(tZERO) CONTEXT;

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csrvCQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_CQTableName,
		aCQTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
		cbCQTable_FieldDefs,					// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}
#endif


//************************************************************************
//	PRIVATE:
//		csrvSQDefineTable:
//	
//			Defines the status queue table
//
//************************************************************************
STATUS CmdServer::csrvSQDefineTable()
{
	STATUS			status;
	U32				sizeofFieldDef;
	fieldDef		*pFieldDef;
	

	CONTEXT			*pContext = new CONTEXT;
	memset(pContext, 0, sizeof(CONTEXT));

	if (m_sizeofStatusData == 0){
		pFieldDef = &StatusQueueTableNoResultData_FieldDefs[2];
		pFieldDef->cbField = sizeof(CommandQueueRecord) + m_sizeofCmdData;

		sizeofFieldDef = sizeofStatusQueueTableNoResultData_FieldDefs;
		pContext->pData = new char[sizeofFieldDef];
		memcpy(
			pContext->pData,
			StatusQueueTableNoResultData_FieldDefs,
			sizeofFieldDef);
	} else {
		// change the size of the binary data in field defs.
		pFieldDef = &StatusQueueTable_FieldDefs[2];
		pFieldDef->cbField = m_sizeofStatusData;
		pFieldDef ++;
		pFieldDef->cbField = sizeof(CommandQueueRecord) + m_sizeofCmdData;

		sizeofFieldDef = sizeofStatusQueueTable_FieldDefs;
		pContext->pData = new char[sizeofFieldDef];
		memcpy(pContext->pData,StatusQueueTable_FieldDefs,sizeofFieldDef);
	}

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csrvSQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_SQTableName,
		(fieldDef *)pContext->pData,		// fieldDef* prgFieldDefsRet,
		sizeofFieldDef,						// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	PRIVATE:
//		csrvSQDefineVLable:
//	
//			Defines the status queue table
//
//************************************************************************
STATUS CmdServer::csrvSQDefineVLTable()
{
	STATUS status;

	CONTEXT			*pContext = new (tZERO) CONTEXT;

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csrvSQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_SQTableName,
		aSQTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
		cbSQTable_FieldDefs,				// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdServer::csrvReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}
#endif

//**************************************************************************
//
//	PRIVATE:
//		csrvReplyHandler
//
//	Queue Operations Reply Handler
//
//**************************************************************************
STATUS CmdServer
::csrvReplyHandler(void* _pContext, STATUS status )
{
	if (status != OS_DETAIL_STATUS_SUCCESS){
		// if table exists then continue to register listeners
		if (status != ercTableExists)
			return status;
	}
	void					*pCmdData;
#ifdef VAR_CMD_QUEUE
	CQRecord				*pCQRecord;
#endif
	CommandQueueRecord		*pTempCQRecord;
	CONTEXT *pContext = (CONTEXT *)_pContext;

	switch (pContext->state){
	case	csrvCQ_TABLE_DEFINED_STATE:
		if (pContext->pData){
			delete pContext->pData;		// field defs		
			pContext->pData = NULL;
		}
		pContext->state = csrvCQ_LISTENER_REGISTERED_STATE;

		// this memory should be deleted on destruction or
		// stop listen
		// don't allocate this, it is allocated and deleted by the table service
		/*m_CQSizeOfModifiedRecord = sizeof(CommandQueueRecord) + m_sizeofCmdData;
		m_pCQModifiedRecord = new char[m_CQSizeOfModifiedRecord];
		memset(m_pCQModifiedRecord,0,m_CQSizeOfModifiedRecord);*/

		csrvRegisterListener(
				m_CQTableName,
				ListenOnInsertRow,			// Listen on cmd inserts 
				&m_CQListenerId,			// to stop listening
				&m_pCQListenTypeRet,
				&m_pCQModifiedRecord,		// for listeners to stuff
				&m_CQSizeOfModifiedRecord,
				pContext,
				(pTSCallback_t)&CmdServer::csrvReplyHandler);
		break;
	case	csrvSQ_TABLE_DEFINED_STATE:
		if (pContext->pData){
			delete pContext->pData;		// field defs		
			pContext->pData = NULL;
		}
		delete pContext;
		pContext = NULL;
		break;
#ifndef VAR_CMD_QUEUE
	case	csrvCQ_LISTENER_REGISTERED_STATE:
		if ((*m_pCQListenTypeRet & ListenInitialReply)){
			// listeners are registered
			if (pContext->pData)
				delete pContext->pData;		// field defs

			// as far as we are concerned, init is done
			if (m_isInitialized != TRUE){
				m_isInitialized = TRUE;
				if (m_objectInitializedCallback)
					(m_pDdmServices->*m_objectInitializedCallback)(OK);		
			}

			// check for any data before we started listening and
			// call the insert row callback.
			// This will handle the failover case.
			pTempCQRecord = (CommandQueueRecord *)m_pCQTableDataRet;
			if (pTempCQRecord != NULL){
				for (U32 i=0; 
					i < m_sizeofCQTableDataRet;
						i+= (sizeof(CommandQueueRecord) + m_sizeofCmdData)){
					pTempCQRecord = (CommandQueueRecord *)((char *)m_pCQTableDataRet+i);
					if (RowId(pTempCQRecord->thisRID) != NULL){
						OUTSTANDING_CMD		*pOutstandingCmd = new OUTSTANDING_CMD;
						memset(pOutstandingCmd, 0, sizeof(OUTSTANDING_CMD));

						pCmdData = (void *)((char *)pTempCQRecord + sizeof(CommandQueueRecord));
						pOutstandingCmd->cmdRowId	= pTempCQRecord->thisRID;
						pOutstandingCmd->pCmdData = new char[m_sizeofCmdData];
						memcpy(pOutstandingCmd->pCmdData,pCmdData,m_sizeofCmdData);
						csrvAddOutstandingCmd(pOutstandingCmd);
						(m_pDdmServices->*m_CQInsertRowCallback)( 
							(void *)&pOutstandingCmd->cmdRowId, pOutstandingCmd->pCmdData);
					}
				}
			}
		}
		if (*m_pCQListenTypeRet & ListenOnInsertRow){
			// call the CQ insert row callback
			OUTSTANDING_CMD		*pOutstandingCmd = new OUTSTANDING_CMD;
			memset(pOutstandingCmd, 0, sizeof(OUTSTANDING_CMD));

			pTempCQRecord = (CommandQueueRecord *)m_pCQModifiedRecord,	
			pCmdData = (void *)((char *)pTempCQRecord + sizeof(CommandQueueRecord));
			pOutstandingCmd->cmdRowId	= pTempCQRecord->thisRID;
			pOutstandingCmd->pCmdData = new char[m_sizeofCmdData];
			memcpy(pOutstandingCmd->pCmdData,pCmdData,m_sizeofCmdData);
			csrvAddOutstandingCmd(pOutstandingCmd);
			(m_pDdmServices->*m_CQInsertRowCallback)(
				(void *)&pOutstandingCmd->cmdRowId, pOutstandingCmd->pCmdData);
		}
		break;
#else
	case	csrvCQ_LISTENER_REGISTERED_STATE:
		if ((*m_pCQListenTypeRet & ListenInitialReply)){
			// listeners are registered
			if (pContext->pData)
				delete pContext->pData;		// field defs

			// as far as we are concerned, init is done
			if (m_isInitialized != TRUE){
				m_isInitialized = TRUE;
				if (m_objectInitializedCallback)
					(m_pDdmServices->*m_objectInitializedCallback)(OK);		
			}

			if (m_varSizeData == false)
			{
				// check for any data before we started listening and
				// call the insert row callback.
				// This will handle the failover case.
				pTempCQRecord = (CommandQueueRecord *)m_pCQTableDataRet;
				if (pTempCQRecord != NULL){
					for (U32 i=0; 
						i < m_sizeofCQTableDataRet;
							i+= (sizeof(CommandQueueRecord) + m_sizeofCmdData)){
						pTempCQRecord = (CommandQueueRecord *)((char *)m_pCQTableDataRet+i);
						if (RowId(pTempCQRecord->thisRID) != NULL){
							OUTSTANDING_CMD		*pOutstandingCmd = new OUTSTANDING_CMD;
							memset(pOutstandingCmd, 0, sizeof(OUTSTANDING_CMD));

							pCmdData = (void *)((char *)pTempCQRecord + sizeof(CommandQueueRecord));
							pOutstandingCmd->cmdRowId	= pTempCQRecord->thisRID;
							pOutstandingCmd->pCmdData = new char[m_sizeofCmdData];
							memcpy(pOutstandingCmd->pCmdData,pCmdData,m_sizeofCmdData);
							csrvAddOutstandingCmd(pOutstandingCmd);
							(m_pDdmServices->*m_CQInsertRowCallback)( 
								(void *)&pOutstandingCmd->cmdRowId, pOutstandingCmd->pCmdData);
						}
					}
				}
			}
			else
			{
				// check for any data before we started listening and
				// call the insert row callback.
				// This will handle the failover case.
				pCQRecord = (CQRecord *)m_pCQTableDataRet;
				if (pCQRecord != NULL)
				{
					for (U32 i=0; i < m_sizeofCQTableDataRet; i += pCQRecord->TotalRecSize())
					{
						pCQRecord = (CQRecord *)((char *)m_pCQTableDataRet+i);
						OUTSTANDING_CMD		*pOutstandingCmd = new (tZERO) OUTSTANDING_CMD;
						pOutstandingCmd->cmdRowId	= pCQRecord->rid;
						pOutstandingCmd->pCmdData = new char[pCQRecord->cmdData.Size()];
						memcpy(pOutstandingCmd->pCmdData,
							pCQRecord->cmdData.ConstPtr(),
							pCQRecord->cmdData.Size());
						csrvAddOutstandingCmd(pOutstandingCmd);
						(m_pDdmServices->*m_CQInsertRowCallback)( 
							(void *)&pOutstandingCmd->cmdRowId, pOutstandingCmd->pCmdData);
					}
				}
			}	
		}
		if (*m_pCQListenTypeRet & ListenOnInsertRow){
			if (m_varSizeData == false)
			{
				// call the CQ insert row callback
				OUTSTANDING_CMD		*pOutstandingCmd = new OUTSTANDING_CMD;
				memset(pOutstandingCmd, 0, sizeof(OUTSTANDING_CMD));

				pTempCQRecord = (CommandQueueRecord *)m_pCQModifiedRecord;	
				pCmdData = (void *)((char *)pTempCQRecord + sizeof(CommandQueueRecord));
				pOutstandingCmd->cmdRowId	= pTempCQRecord->thisRID;
				pOutstandingCmd->pCmdData = new char[m_sizeofCmdData];
				memcpy(pOutstandingCmd->pCmdData,pCmdData,m_sizeofCmdData);
				csrvAddOutstandingCmd(pOutstandingCmd);
				(m_pDdmServices->*m_CQInsertRowCallback)(
					(void *)&pOutstandingCmd->cmdRowId, pOutstandingCmd->pCmdData);
			}
			else
			{
				// need to read vl row and then insert outstanding cmd
				CONTEXT* pReadContext = new (tZERO) CONTEXT;
				pReadContext->state = csrvCQ_VL_ROW_READ_STATE;
				pReadContext->newRowId = ((CQRecord*)m_pCQModifiedRecord)->rid;
				TSReadVLRow* pReadRow = new TSReadVLRow;
				status = pReadRow->Initialize(
					this,
					m_CQTableName,
					CT_PTS_RID_FIELD_NAME,
					&pReadContext->newRowId,
					sizeof(rowID),
					(CPtsRecordBase**)&pReadContext->pData,
					&pReadContext->value1, // size of row data
					&pReadContext->value, // number of rows returned
					(pTSCallback_t)&CmdServer::csrvReplyHandler,
					pReadContext);
				assert(status==OK);
				pReadRow->Send();
			}
		}
		break;
#endif
//	case	csrvSQ_LISTENER_REGISTERED_STATE:
//		break;
#ifdef VAR_CMD_QUEUE
	case	csrvCQ_VL_ROW_READ_STATE:
		{
			assert(pContext->value==1);
			OUTSTANDING_CMD		*pOutstandingCmd = new (tZERO) OUTSTANDING_CMD;
			pCQRecord = (CQRecord *)pContext->pData;	
			pOutstandingCmd->cmdRowId	= pCQRecord->rid;
			pOutstandingCmd->pCmdData = new char[pCQRecord->cmdData.Size()];
			memcpy(pOutstandingCmd->pCmdData,
				pCQRecord->cmdData.ConstPtr(),
				pCQRecord->cmdData.Size());
			csrvAddOutstandingCmd(pOutstandingCmd);
			(m_pDdmServices->*m_CQInsertRowCallback)(
				(void *)&pOutstandingCmd->cmdRowId, pOutstandingCmd->pCmdData);
			delete pContext;
			break;
		}
#endif
	case	csrvSQ_ROW_INSERTED_STATE:
		if (pContext->pData)
		{
			delete pContext->pData;
			pContext->pData = NULL;
		}
#ifndef VAR_CMD_QUEUE
		pContext->state = csrvSQ_ROW_DELETED_STATE;
		csrvDeleteRow(m_SQTableName,&pContext->newRowId, pContext);
#else
		if (m_varSizeData == false)
		{
			pContext->state = csrvSQ_ROW_DELETED_STATE;
			csrvDeleteRow(m_SQTableName,&pContext->newRowId, pContext);
		}
#endif
		break;
	case	csrvSQ_ROW_DELETED_STATE:
		if (pContext){
			delete pContext;
			pContext = NULL;
		}
		break;
	case	csrvCQ_ROW_DELETED_STATE:
/*		if (pContext){
			if (pContext->pData){
				delete pContext->pData;
				pContext->pData = NULL;
			}
			delete pContext;
			pContext = NULL;
		}*/
		delete pContext;
		pContext = NULL;
		break;
	default:
		break;
	}
	return status;
}

//**************************************************************************
//	PRIVATE:
//		csrvRegisterListener
//	Registers for any "listenerType" for a given table
//
//**************************************************************************
STATUS CmdServer
::csrvRegisterListener(
			String64		tableName,
			U32 			listenerType, 
			U32				*pListenerId,
			U32**			ppListenTypeRet,
			void**			ppModifiedRecord,
			U32*			pSizeofModifiedRecord,
			void			*_pContext,
			pTSCallback_t	pCallback)
{
	STATUS			status;

	CONTEXT *pContext = (CONTEXT *)_pContext;
	m_pCQTableDataRet = NULL;
	m_sizeofCQTableDataRet = 0;

	m_pCQListenObject = new TSListen;
	status = m_pCQListenObject->Initialize(
		this,					// DdmServices* pDdmServices,
		listenerType,			// ListenTypeEnum ListenType,
		tableName,
		NULL,					// String64 prgbRowKeyFieldName,
		NULL,					// void* prgbRowKeyFieldValue,
		0,						// U32 cbRowKeyFieldValue,
		NULL,					// String64 prgbFieldName,
		NULL,					// void* prgbFieldValue,
		0,						// U32 cbFieldValue,
		ReplyContinuous | ReplyWithRow,		// ReplyModeEnum ReplyMode - continuous
		&m_pCQTableDataRet,
		&m_sizeofCQTableDataRet,
		pListenerId,			// U32* pListenerIDRet,
		(U32**)ppListenTypeRet,	// U32**ppListenTypeRet
		ppModifiedRecord,		// void**ppModifiedRecordRet
		pSizeofModifiedRecord,
		(pTSCallback_t)pCallback,	// pTSCallback_t pCallback,
		pContext					// void* pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pCQListenObject->Send();
	return status;
}








//**************************************************************************
//
//	PRIVATE:
//		csrvAddOutstandingCmd
//
//	Adds a command to the outstanding list so that it can be matched
//	up later when a status queue insert row reply is received for
//	the particular cmd.
//
//**************************************************************************
void CmdServer
::csrvAddOutstandingCmd(OUTSTANDING_CMD	*pOutstandingCmd)
{
	// check if first node
	if ((pHead == NULL) && (pTail == NULL)){
		// add node
		pHead = pOutstandingCmd;
		pTail = pOutstandingCmd;
		pOutstandingCmd->pNextCmd = NULL;
	} else {
		// add node to tail
		pTail->pNextCmd = pOutstandingCmd;
		pOutstandingCmd->pNextCmd = NULL;
		pTail = pOutstandingCmd;
	}
}

//**************************************************************************
//
//	PRIVATE:
//		csrvGetOutStandingCmd
//
//	- Search the outstanding cmd list to see if this status was for
//		a cmd in our list
//	- if found, return the outstanding cmd 
//
//**************************************************************************
void CmdServer
::csrvGetOutstandingCmd(HANDLE	handle, OUTSTANDING_CMD **ppOutstandingCmd)
{
	OUTSTANDING_CMD	*pTemp;

	pTemp = pHead;
	while (pTemp != NULL){
		//if (&pTemp->cmdRowId == handle){
		if (memcmp(&pTemp->cmdRowId, handle, sizeof(rowID)) == 0){
			//found a match
			*ppOutstandingCmd = pTemp;
			break;
		}
		pTemp = pTemp->pNextCmd;
	}
}


//**************************************************************************
//
//	PRIVATE:
//		csrvDeleteOutstandingCmd
//
//	- Search the outstanding cmd , if found remove it and update list
//
//**************************************************************************
void CmdServer
::csrvDeleteOutstandingCmd(OUTSTANDING_CMD *pOutstandingCmd)
{
	OUTSTANDING_CMD	*pTemp;

	if (pHead == NULL){
		return;
	}

	// check first entry
	if (pHead == pOutstandingCmd){
		pHead = pOutstandingCmd->pNextCmd;
		if (pOutstandingCmd == pTail){
			// if it happens to be last entry
			pTail = NULL;
		}
		if (pOutstandingCmd->pCmdData){
			delete pOutstandingCmd->pCmdData;
		}
		delete pOutstandingCmd;
		return;
	}
	pTemp = pHead;
	while (pTemp->pNextCmd != NULL){
		if (pTemp->pNextCmd == pOutstandingCmd){
			//found a match
			pTemp->pNextCmd = pOutstandingCmd->pNextCmd;
			if (pTemp->pNextCmd == NULL){
				pTail = pTemp;
			}
			if (pOutstandingCmd->pCmdData){
				delete pOutstandingCmd->pCmdData;
			}
			delete pOutstandingCmd;
			return;
		}
		pTemp = pTemp->pNextCmd;
	}
}


void CmdServer
::csrvDeleteAllOutstandingCmds()
{
	OUTSTANDING_CMD	*pTemp;

	pTemp = pHead;
	while (pHead != NULL){
		pHead = pTemp->pNextCmd;
		if (pTemp->pCmdData){
			delete pTemp->pCmdData;
		}
		delete pTemp;
	}
}
