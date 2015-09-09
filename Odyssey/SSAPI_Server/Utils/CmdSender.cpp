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
// File: CmdSender.cpp
// 
// Description:
// Client-side of Odyssey inter-DDM request queue machinery.
// 
// $Log: /Gemini/Odyssey/CmdQueues/CmdSender.cpp $
// 
// 17    11/17/99 3:24p Joehler
// Add variable size status and command data to command queues
// 
// 16    8/23/99 2:22p Dpatel
// added the enum routines to cmd sender.
// 
// 15    8/20/99 3:02p Dpatel
// Added CheckAndExecute() to check for duplicate cmds.
// 
// 14    8/16/99 2:42p Dpatel
// ReplyContinuous needs reply with row flag OR'd.
// 
// 13    7/27/99 6:07p Dpatel
// checked the return status for initialize...
// 
// 12    7/23/99 5:53p Dpatel
// fixed the return of Send from STATUS to void.
// 
// 11    7/22/99 4:03p Dpatel
// removed the return type for Send() according to PTS changes.
// 
// 10    6/24/99 7:14p Ewedel
// Fixed result callback handler to supply NULL status data ptr when
// interface status data size is zero.  Also added standard file header
// (from Odyssey/template.c).
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/

#include "CmdSender.h"



//************************************************************************
//	CONSTRUCTOR:
//
//	Registers an internal listener on SQ for any inserts
//************************************************************************
CmdSender::CmdSender(
					String64			queueName,
					U32					sizeofCQParams,
					U32					sizeofSQParams,
					DdmServices			*pParentDdm) : DdmServices( pParentDdm )
{
	m_isInitialized		= false;
#ifdef VAR_CMD_QUEUE
	m_varSizeData = false;
#endif
	m_pDdmServices		= pParentDdm;

	// min size of Cmd data = 4, since opcode required at min
	assert(sizeofCQParams > 3);

	strcpy(m_CQTableName,(char *)queueName);
	m_sizeofCQParams	= sizeofCQParams;
	strcpy(m_SQTableName,(char *)queueName);
	strcat(m_SQTableName, "_STATUS");
 	m_sizeofSQParams	= sizeofSQParams;

	m_totalStatusDataSize = sizeof(StatusQueueRecord) + sizeofSQParams;
	m_SQListenerType = 0;
	m_pSQModifiedRecord	= NULL;


	// Initialize outstanding cmd list's head/tail
	pHead = NULL;
	pTail = NULL;

	m_useTerminate = 0;
	m_pSQListenObject = NULL;

	m_eventCallback =NULL;
	m_objectInitializedCallback = NULL;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	CONSTRUCTOR:
//
//	Registers an internal listener on SQ for any inserts
//************************************************************************
CmdSender::CmdSender(
					String64			queueName,
					DdmServices			*pParentDdm) : DdmServices( pParentDdm )
{
	m_isInitialized		= false;
	m_varSizeData = true;
	m_pDdmServices		= pParentDdm;

	strcpy(m_CQTableName,(char *)queueName);
	strcpy(m_SQTableName,(char *)queueName);
	strcat(m_SQTableName, "_STATUS");

	m_SQListenerType = 0;
	m_pSQModifiedRecord	= NULL;


	// Initialize outstanding cmd list's head/tail
	pHead = NULL;
	pTail = NULL;

	m_useTerminate = 0;
	m_pSQListenObject = NULL;

	m_eventCallback =NULL;
	m_objectInitializedCallback = NULL;
}
#endif

//************************************************************************
//	DESTRUCTOR:
//
//		Delete any memory allocated
//
//************************************************************************

CmdSender::~CmdSender()
{
	assert(m_useTerminate != 0);
// don't delete or allocate this.  taken care of by listener
//	if (m_pSQModifiedRecord)
//		delete m_pSQModifiedRecord;
}

#ifndef VAR_CMD_QUEUE
//************************************************************************
//	PUBLIC:
//		csndrInitialize
//
//	Will Define the Command Queue and Status Queues
//
//************************************************************************
STATUS CmdSender
::csndrInitialize(pInitializeCallback_t	objectInitializedCallback)
{
	STATUS			status;
	m_objectInitializedCallback = objectInitializedCallback;
	// Define the tables even if they are already defined.
	status = csndrCQDefineTable();
	if (status)
		return status;
	status = csndrSQDefineTable();
	return status;
}
#else
//************************************************************************
//	PUBLIC:
//		csndrInitialize
//
//	Will Define the Command Queue and Status Queues
//
//************************************************************************
STATUS CmdSender
::csndrInitialize(pInitializeCallback_t	objectInitializedCallback)
{
	STATUS			status;
	m_objectInitializedCallback = objectInitializedCallback;
	// Define the tables even if they are already defined.
	if (m_varSizeData==false)
		status = csndrCQDefineTable();
	else
		status = csndrCQDefineVLTable();
	if (status)
		return status;
	if (m_varSizeData==false)
		status = csndrSQDefineTable();
	else
		status = csndrSQDefineVLTable();
	return status;
}
#endif


//************************************************************************
//	PUBLIC:
//		csndrExecute:
//
//************************************************************************
STATUS CmdSender
::csndrExecute(
			void						*pCmdData,			// including param data
			pCmdCompletionCallback_t	completionCallback,
			void						*pCmdContext)
{
	STATUS				status;
	U32					sizeofCommandData;

#ifdef VAR_CMD_QUEUE
	assert(m_varSizeData==false);
#endif

	if (m_isInitialized != TRUE){
		return !OK;
	}
	TSInsertRow *pTSInsertRow = new TSInsertRow;

	CONTEXT	*pContext = new CONTEXT;
	memset(pContext, 0, sizeof(CONTEXT));

	pContext->state = csndrCQ_ROW_INSERTED_STATE;
	sizeofCommandData = m_sizeofCQParams;
			
	
	// save this data so it can be added to the outstanding
	// cmd list on row insert reply
	pContext->pCallback = completionCallback;
	pContext->pCmdContext = pCmdContext;

	pContext->pData = new char[sizeofCommandData + sizeof(CommandQueueRecord)];
	if (pCmdData){
		// copy the original cmd's CQ data here
		memcpy((void *)((char *)pContext->pData + sizeof(CommandQueueRecord)), 
					pCmdData,
					sizeofCommandData);
	}
	status = pTSInsertRow->Initialize(
		this,
		m_CQTableName,
		pContext->pData,
		sizeof(CommandQueueRecord) + sizeofCommandData,
		&pContext->newRowId,
		(pTSCallback_t)&CmdSender::csndrReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSInsertRow->Send();
	return status;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	PUBLIC:
//		csndrExecute:
//
//************************************************************************
STATUS CmdSender
::csndrExecute(
			void						*pCmdData,			// including param data
			U32							cbCmdData,
			pCmdCompletionCallback_t	completionCallback,
			void						*pCmdContext)
{
	STATUS				status;

	assert (m_varSizeData==true);

	if (m_isInitialized != TRUE){
		return !OK;
	}

	TSInsertVLRow *pTSInsertRow = new TSInsertVLRow;

	CONTEXT	*pContext = new (tZERO) CONTEXT;

	pContext->state = csndrCQ_ROW_INSERTED_STATE;
	
	// save this data so it can be added to the outstanding
	// cmd list on row insert reply
	pContext->pCallback = completionCallback;
	pContext->pCmdContext = pCmdContext;

	// temporary restriction on size of variable pts fields
	assert(cbCmdData<512);
	assert(!(cbCmdData%4));

	pContext->pData = new CQRecord(cbCmdData, (U8*)pCmdData);

	status = pTSInsertRow->Initialize(
		this,
		m_CQTableName,
		(CQRecord*)pContext->pData,
		&pContext->newRowId,
		(pTSCallback_t)&CmdSender::csndrReplyHandler,
		pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSInsertRow->Send();
	return status;
}
#endif

//************************************************************************
//	PUBLIC:
//		csndrCheckAndExecute:
//			Executes a cmd only if we dont find such a cmd already existing
//			in the CQ PTS table.
//			Assumption: The sender has to make sure that his cmd packets
//			are unique (e.g by putting some unique row id etc). Otherwise
//			the cmd may not be executed.
//
//************************************************************************
STATUS CmdSender
::csndrCheckAndExecute(
			void						*pCmdData,			// including param data
			pCmdCompletionCallback_t	completionCallback,
			void						*pCmdContext)
{
	STATUS				status;

#ifdef VAR_CMD_QUEUE
	assert(m_varSizeData==false);
#endif

	if (m_isInitialized != TRUE){
		return !OK;
	}

	CONTEXT		*pContext= new CONTEXT;
	memset(pContext,0,sizeof(CONTEXT));

	// save our cmd data
	pContext->pData1 = new char[m_sizeofCQParams];
	memcpy(pContext->pData1, pCmdData, m_sizeofCQParams);

	pContext->pCallback = completionCallback;
	pContext->pCmdContext = (CONTEXT *)pCmdContext;

	status = utilEnumTable(
				m_CQTableName,
				&pContext->pData,				// table data returned
				&pContext->value1,				// data returned size
				&pContext->value,				// number of rows returned here
				pContext,						// context
				(pTSCallback_t)&CmdSender::csndrEnumReplyHandler);
	return status;
}




//************************************************************************
//	PUBLIC:
//		csndrRegisterForEvents:
//			Will report any events reported in the status queue
//
//		eventCallback		callback to be called on any event
//
//************************************************************************
STATUS CmdSender
::csndrRegisterForEvents(pEventCallback_t	eventCallback)
{
	// just save the event callback
	m_eventCallback = eventCallback;
	return OK;
}


//************************************************************************
//	PUBLIC:
//		csndrTerminate
//	Functionality:
//		Stop Listening for Inserts into SQ
//
//************************************************************************
void CmdSender
::csndrTerminate()
{
	// Stop listening on inserts into SQ
	if (m_pSQListenObject)
		m_pSQListenObject->Stop();
	// Remove any outstanding cmds from the list
	csndrDeleteAllOutstandingCmds();
	
	// call the destructor
	// set a flag to distinguish local destructor call from 
	// a call by the user. (call by user is not allowed)
	m_useTerminate = 1;	
	delete this;
}


//************************************************************************
//	PRIVATE METHODS BEGIN HERE
//************************************************************************

//************************************************************************
//	PRIVATE:
//		csndrCQDefineTable:
//	
//			Defines the command queue table
//
//************************************************************************
STATUS CmdSender::csndrCQDefineTable()
{
	STATUS			status;
	U32				sizeofFieldDef;
	fieldDef		*pFieldDef;

#ifdef VAR_CMD_QUEUE
	assert(m_varSizeData==false);
#endif

	CONTEXT			*pContext = new CONTEXT;
	memset(pContext, 0, sizeof(CONTEXT));

	// change the size of the binary data in field defs.
	pFieldDef = &CommandQueueTable_FieldDefs[0];
	sizeofFieldDef = sizeofCommandQueueTable_FieldDefs;

	pFieldDef->cbField = m_sizeofCQParams;
	pContext->pData = new char[sizeofFieldDef];
	memcpy(pContext->pData,CommandQueueTable_FieldDefs,sizeofFieldDef);
	

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csndrCQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_CQTableName,
		(fieldDef *)pContext->pData,		// fieldDef* prgFieldDefsRet,
		sizeofFieldDef,						// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdSender::csndrReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	PRIVATE:
//		csndrCQDefineVLTable:
//	
//			Defines the command queue table
//
//************************************************************************
STATUS CmdSender::csndrCQDefineVLTable()
{
	STATUS			status;

	assert(m_varSizeData==true);

	CONTEXT			*pContext = new(tZERO) CONTEXT;

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csndrCQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_CQTableName,
		aCQTable_FieldDefs,		// fieldDef* prgFieldDefsRet,
		cbCQTable_FieldDefs,						// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdSender::csndrReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}
#endif

//************************************************************************
//	PRIVATE:
//		csndrSQDefineTable:
//	
//			Defines the status queue table
//
//************************************************************************
STATUS CmdSender::csndrSQDefineTable()
{
	STATUS			status;
	U32				sizeofFieldDef;
	fieldDef		*pFieldDef;
	

#ifdef VAR_CMD_QUEUE
	assert(m_varSizeData==false);
#endif

	CONTEXT			*pContext = new CONTEXT;
	memset(pContext, 0, sizeof(CONTEXT));

	if (m_sizeofSQParams == 0){
		// change the size of the binary data in field defs.
		pFieldDef = &StatusQueueTableNoResultData_FieldDefs[2];
		pFieldDef->cbField = sizeof(CommandQueueRecord) + m_sizeofCQParams;

		sizeofFieldDef = sizeofStatusQueueTableNoResultData_FieldDefs;
		pContext->pData = new char[sizeofFieldDef];
		memcpy(
				pContext->pData,
				StatusQueueTableNoResultData_FieldDefs,
				sizeofFieldDef);
	} else {
		// change the size of the binary data in field defs.
		pFieldDef = &StatusQueueTable_FieldDefs[2];
		pFieldDef->cbField = m_sizeofSQParams;
		pFieldDef ++;
		pFieldDef->cbField = sizeof(CommandQueueRecord) + m_sizeofCQParams;

		sizeofFieldDef = sizeofStatusQueueTable_FieldDefs;
		pContext->pData = new char[sizeofFieldDef];
		memcpy(pContext->pData,StatusQueueTable_FieldDefs,sizeofFieldDef);
	}


	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	pContext->state = csndrSQ_TABLE_DEFINED_STATE;
	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_SQTableName,
		(fieldDef *)pContext->pData,		// fieldDef* prgFieldDefsRet,
		sizeofFieldDef,						// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdSender::csndrReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}

#ifdef VAR_CMD_QUEUE
//************************************************************************
//	PRIVATE:
//		csndrSQDefineVLTable:
//	
//			Defines the status queue table
//
//************************************************************************
STATUS CmdSender::csndrSQDefineVLTable()
{
	STATUS			status;	

	assert(m_varSizeData==true);

	CONTEXT			*pContext = new (tZERO) CONTEXT;
	pContext->state = csndrSQ_TABLE_DEFINED_STATE;

	// Allocate an Define Table object 
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pTSDefineTable->Initialize(
		this,								// DdmServices* pDdmServices,
		m_SQTableName,
		aSQTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
		cbSQTable_FieldDefs,				// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		true,								// bool* pfPersistant,
		(pTSCallback_t)&CmdSender::csndrReplyHandler,// pTSCallback_t pCallback,
		pContext								// void* pContext
	);	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();
	return status;
}
#endif

//**************************************************************************
//	PRIVATE:
//		csrvRegisterListener
//	Registers for any "listenerType" for a given table
//
//**************************************************************************
STATUS CmdSender
::csndrRegisterListener(
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

	m_pSQListenObject = new TSListen;
	status = m_pSQListenObject->Initialize(
		this,								// DdmServices* pDdmServices,
		listenerType,						// ListenTypeEnum ListenType,
		tableName,
		NULL,									// String64 prgbRowKeyFieldName,
		NULL,									// void* prgbRowKeyFieldValue,
		0,										// U32 cbRowKeyFieldValue,
		NULL,									// String64 prgbFieldName,
		NULL,									// void* prgbFieldValue,
		0,										// U32 cbFieldValue,
		ReplyContinuous | ReplyWithRow,			// ReplyModeEnum ReplyMode - continuous
		NULL,
		NULL,
		pListenerId,							// U32* pListenerIDRet,
		(U32**)ppListenTypeRet,					// U32**ppListenTypeRet
		ppModifiedRecord,						// void**ppModifiedRecordRet
		pSizeofModifiedRecord,
		(pTSCallback_t)pCallback,				// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pSQListenObject->Send();
	return status;
}


//**************************************************************************
//
//	PRIVATE:
//		csndrReplyHandler
//
//	Queue Operations Reply Handler
//
//**************************************************************************
STATUS CmdSender
::csndrReplyHandler(void* _pContext, STATUS status )
{
	if (status != OS_DETAIL_STATUS_SUCCESS){
		// if table exists, still continue to register listeners
		if (status != ercTableExists)
			return status;
	}
	OUTSTANDING_CMD		*pOutstandingCmd;
	CONTEXT				*pContext = (CONTEXT *)_pContext;

	switch (pContext->state){
	case	csndrCQ_TABLE_DEFINED_STATE:
		if (pContext->pData){
			delete pContext->pData;	// field defs
			pContext->pData = NULL;
		}
		delete pContext;
		pContext = NULL;
		break;
	case	csndrSQ_TABLE_DEFINED_STATE:
		if (pContext->pData){
			delete pContext->pData;	// field defs
			pContext->pData = NULL;
		}
		delete pContext;
		pContext = NULL;
		// Register a listener for SQ inserts
		pContext = new CONTEXT;
		memset(pContext, 0, sizeof(CONTEXT));

		pContext->state = csndrSQ_LISTENER_REGISTERED_STATE;
		//m_SQSizeOfModifiedRecord = sizeof(StatusQueueRecord) + m_sizeofSQParams +
		//	sizeof(CommandQueueRecord) + m_sizeofCQParams;
		// don't allocate. taken care of by table service
		//m_pSQModifiedRecord = new char[m_SQSizeOfModifiedRecord];
		//memset(m_pSQModifiedRecord,0,m_SQSizeOfModifiedRecord);

		csndrRegisterListener(
					m_SQTableName,
					ListenOnInsertRow,
					&m_SQListenerId,			// to stop listening
					&m_pSQListenTypeRet,
					&m_pSQModifiedRecord,	// for listeners to stuff
					&m_SQSizeOfModifiedRecord,
					pContext,
					(pTSCallback_t)&CmdSender::csndrReplyHandler);
		if (m_isInitialized != TRUE){
			// as far as we are concerned, init is done
			m_isInitialized = TRUE;
			// now call the object initialized callback
			if (m_objectInitializedCallback)
				(m_pDdmServices->*m_objectInitializedCallback)(OK);
		}
		break;
	case	csndrCQ_ROW_INSERTED_STATE:
		// Also, the listener, previously registered by the cmdServer 
		// will be called, so it can process the cmd.

		// Here we add the cmd to outstanding cmd list
		pOutstandingCmd = new OUTSTANDING_CMD;
		memset(pOutstandingCmd, 0, sizeof(OUTSTANDING_CMD));

		pOutstandingCmd->cmdRowId = pContext->newRowId;
		pOutstandingCmd->completionCallback = pContext->pCallback;
		pOutstandingCmd->pCmdContext = pContext->pCmdContext;
		csndrAddOutstandingCmd(pOutstandingCmd);
		// clean up cmd data
		if (pContext){
			if (pContext->pData){
				delete pContext->pData;
				pContext->pData = NULL;
			}
			delete pContext;
			pContext = NULL;
		}
		break;
	case	csndrSQ_LISTENER_REGISTERED_STATE:
		if ((*m_pSQListenTypeRet & ListenInitialReply)){
			// should never be non NULL
			/*if (pContext->pData){
				delete pContext->pData;		
				pContext->pData = NULL;
			}*/
		}
		if (*m_pSQListenTypeRet & ListenOnInsertRow){
#ifndef VAR_CMD_QUEUE
			// call the SQ insert row callback
			// need to copy the row because it goes away after cb exited
			void* copyOfRow = new char[m_SQSizeOfModifiedRecord];
			memcpy(copyOfRow, m_pSQModifiedRecord, m_SQSizeOfModifiedRecord);
			csndrProcessStatus(copyOfRow);
			status = status;
#else
			if (m_varSizeData == false)
			{
				// call the SQ insert row callback
				// need to copy the row because it goes away after cb exited
				void* copyOfRow = new char[m_SQSizeOfModifiedRecord];
				memcpy(copyOfRow, m_pSQModifiedRecord, m_SQSizeOfModifiedRecord);
				csndrProcessStatus(copyOfRow);
				status = status;
			}
			else
			{
				// need to read VL row and then call csndrProcessStatus()
				CONTEXT* pReadContext = new (tZERO) CONTEXT;
				pReadContext->state = csndrSQ_VL_ROW_READ_STATE;
				pReadContext->newRowId = ((SQRecord*)m_pSQModifiedRecord)->rid;
				TSReadVLRow* pReadRow = new TSReadVLRow;
				status = pReadRow->Initialize(
					this,
					m_SQTableName,
					CT_PTS_RID_FIELD_NAME,
					&pReadContext->newRowId,
					sizeof(rowID),
					(CPtsRecordBase**)&pReadContext->pData,
					&pReadContext->value1, // size of row data
					&pReadContext->value, // number of rows returned
					(pTSCallback_t)&CmdSender::csndrReplyHandler,
					pReadContext);
				assert(status==OK);
				pReadRow->Send();
			}
#endif
		}
		break;
#ifdef VAR_CMD_QUEUE
	case csndrSQ_VL_ROW_READ_STATE:
		{
			assert(pContext->value==1); // number of rows returned
			void* copyOfRow = new char[pContext->value1]; // size of row data returned
			memcpy(copyOfRow, pContext->pData, pContext->value1);
			csndrProcessStatus(copyOfRow);
			delete pContext;
			break;
		}
#endif
	default:
		break;
	}
	return status;
}


//**************************************************************************
//
//	PRIVATE:
//		csndrEnumReplyHandler
//			For checking if the same cmd exists in our CQ
//
//**************************************************************************
STATUS CmdSender
::csndrEnumReplyHandler(void* _pContext, STATUS status )
{
	CONTEXT						*pContext = (CONTEXT *)_pContext;
	BOOL						isError = false;
	void						*pNewCmdData = NULL;
	void						*pExistingCmdData = NULL;
	CONTEXT						*pOriginalContext = NULL;
	pCmdCompletionCallback_t	pCompletionCallback = NULL;
	U32							numberOfRows = 0;
	U32							totalCQSize = 0;
	BOOL						sameCmdExists = false;
	U32							i = 0;
	U32							pos;

	pOriginalContext	= (CONTEXT *)pContext->pCmdContext;
	pNewCmdData			= pContext->pData1;
	pCompletionCallback = pContext->pCallback;

	// we check for the same command
	numberOfRows = pContext->value;
	totalCQSize = sizeof(CommandQueueRecord) +  m_sizeofCQParams;
	// pContext->pData = enum data		
	// read the data that was enumerated
	pExistingCmdData = pContext->pData;
	pExistingCmdData = (char *)pExistingCmdData + sizeof(CommandQueueRecord); 
	for (i=0; i < numberOfRows; i++){
		pExistingCmdData = 
			((char *)pExistingCmdData+ (totalCQSize*i));
		if (pExistingCmdData){
			if ((pos = memcmp(
				pExistingCmdData,
				pNewCmdData,
				m_sizeofCQParams) )== 0){
				sameCmdExists = true;
			}
		}
	}
	if (sameCmdExists){
		// the existing cmd will reply to the cb
	} else {
		csndrExecute(pNewCmdData,pCompletionCallback,pOriginalContext);
	}
	delete pContext;
	return status;
}


#ifndef VAR_CMD_QUEUE
//**************************************************************************
//
//	PRIVATE:
//		csndrProcessStatus
//
//	Checks if SQ entry was a cmd status or an event and processes
//	accordingly
//
//**************************************************************************
STATUS CmdSender
::csndrProcessStatus(void *pModifiedSQRecord)
{
	StatusQueueRecord		*pSQRecord;

	char					*pResultData;

	pSQRecord = (StatusQueueRecord *)pModifiedSQRecord;

	switch (pSQRecord->type){
		case SQ_COMMAND_STATUS:
			// Now match up the status with the cmd
			// if status is for one of our cmds, then reply to
			// completionCallback. Also remove cmd from the
			// outstanding cmd list.
			csndrCheckIfCmdOutstanding(pSQRecord);
			break;
		case SQ_EVENT_STATUS:
			// call the eventCallback to notify them for event
			if (m_eventCallback){
				pResultData = new char[m_sizeofSQParams];
				memcpy(
					pResultData,
					(void *)((char *)pSQRecord+sizeof(StatusQueueRecord)),
					m_sizeofSQParams);
				(m_pDdmServices->*m_eventCallback)(
						pSQRecord->statusCode,	// SUCCESS, Failure
						pResultData);		// additional result data
				delete pResultData;
				// we copied the row, so we need to delete it
				delete pSQRecord;
			}
			break;
		default:
			break;
	}
	return OK;
}

#else
//**************************************************************************
//
//	PRIVATE:
//		csndrProcessStatus
//
//	Checks if SQ entry was a cmd status or an event and processes
//	accordingly
//
//**************************************************************************
STATUS CmdSender
::csndrProcessStatus(void *pModifiedSQRecord)
{

	if (m_varSizeData == false)
	{
		StatusQueueRecord		*pStatusQueueRecord = (StatusQueueRecord *)pModifiedSQRecord;
		char					*pResultData;
		switch (pStatusQueueRecord->type){
			case SQ_COMMAND_STATUS:
				// Now match up the status with the cmd
				// if status is for one of our cmds, then reply to
				// completionCallback. Also remove cmd from the
				// outstanding cmd list.
				csndrCheckIfCmdOutstanding(pStatusQueueRecord);
				break;
			case SQ_EVENT_STATUS:
				// call the eventCallback to notify them for event
				if (m_eventCallback){
					pResultData = new char[m_sizeofSQParams];
					memcpy(
						pResultData,
						(void *)((char *)pStatusQueueRecord+sizeof(StatusQueueRecord)),
						m_sizeofSQParams);
					(m_pDdmServices->*m_eventCallback)(
							pStatusQueueRecord->statusCode,	// SUCCESS, Failure
							pResultData);		// additional result data
					delete pResultData;
					// we copied the row, so we need to delete it
					delete pStatusQueueRecord;
				}
				break;
			default:
				break;
		}	
	}
	else
	{
		SQRecord	*pSQRecord = (SQRecord*)pModifiedSQRecord;
		char					*pResultData;
		switch (pSQRecord->type){
			case SQ_COMMAND_STATUS:
				// Now match up the status with the cmd
				// if status is for one of our cmds, then reply to
				// completionCallback. Also remove cmd from the
				// outstanding cmd list.
				csndrCheckIfCmdOutstanding(pSQRecord);
				break;
			case SQ_EVENT_STATUS:
				// call the eventCallback to notify them for event
				if (m_eventCallback){
					pResultData = new char[pSQRecord->statusData.Size()];
					memcpy(pResultData,
						pSQRecord->statusData.ConstPtr(),
						pSQRecord->statusData.Size());
					(m_pDdmServices->*m_eventCallback)(
							pSQRecord->statusCode,	// SUCCESS, Failure
							pResultData);		// additional result data
					delete pResultData;
					// we copied the row, so we need to delete it
					delete pSQRecord;
				}
				break;
			default:
				break;
		}	
	}

	return OK;
}
#endif

//**************************************************************************
//
//	PRIVATE:
//		csndrCheckIfCmdOutstanding
//
//	- Extract the Command Queue Record from the Status Queue Record
//	- Search the outstanding cmd list to see if this status was for
//		a cmd in our list
//	- if status for out cmd, call the cmd completion callback and 
//		remove the entry from our list
//
//**************************************************************************
void CmdSender
::csndrCheckIfCmdOutstanding(StatusQueueRecord *pSQRecord)
{
	CommandQueueRecord			*pCQRecord;
	rowID						cmdRowId;
	pCmdCompletionCallback_t	cb;
	char						*pResultData;
	void						*pCmdData;


	pCQRecord = (CommandQueueRecord *)
		((char *)pSQRecord + sizeof(StatusQueueRecord) + m_sizeofSQParams);
	cmdRowId = pCQRecord->thisRID;
	OUTSTANDING_CMD *pOutstandingCmd = NULL;
	csndrGetOutstandingCmd(&cmdRowId, &pOutstandingCmd);
	if (pOutstandingCmd){
		// reply to callback
		cb = pOutstandingCmd->completionCallback; 
		if (cb){
         if (m_sizeofSQParams > 0){
            //  got result data, so extract it
		   	pResultData = new char[m_sizeofSQParams];
		   	memcpy(
		   		pResultData,
		   		(void *)((char *)pSQRecord+sizeof(StatusQueueRecord)),
		   		m_sizeofSQParams);
         } else {
            //  no result data, so provide a nice, tidy NULL instead
            pResultData = NULL;
         }

			pCmdData = ((char *)pCQRecord + sizeof(CommandQueueRecord));

			(m_pDdmServices->*cb)(
				pSQRecord->statusCode,
				pResultData,
				pCmdData,
				(void *)pOutstandingCmd->pCmdContext);

         //  dispose of result data temp buffer, if any
			delete [] pResultData;
			// we allocated a copy of the row, so we need to delete it
			delete pSQRecord;
		}
		csndrDeleteOutstandingCmd(pOutstandingCmd);
	}
}

#ifdef VAR_CMD_QUEUE
//**************************************************************************
//
//	PRIVATE:
//		csndrCheckIfCmdOutstanding
//
//	- Extract the Command Queue Record from the Status Queue Record
//	- Search the outstanding cmd list to see if this status was for
//		a cmd in our list
//	- if status for out cmd, call the cmd completion callback and 
//		remove the entry from our list
//
//**************************************************************************
void CmdSender
::csndrCheckIfCmdOutstanding(SQRecord *pSQRecord)
{
	rowID						cmdRowId;
	pCmdCompletionCallback_t	cb;
	char						*pResultData;
	void						*pCmdData;

	cmdRowId = pSQRecord->cmdRowId;
	OUTSTANDING_CMD *pOutstandingCmd = NULL;
	csndrGetOutstandingCmd(&cmdRowId, &pOutstandingCmd);
	if (pOutstandingCmd)
	{
		
		// reply to callback
		cb = pOutstandingCmd->completionCallback; 
		if (cb)
		{
			if (pSQRecord->statusData.Size() > 0)
			{
				//  got result data, so extract it
		   		pResultData = new char[pSQRecord->statusData.Size()];
		   		memcpy(pResultData,
					pSQRecord->statusData.ConstPtr(),
					pSQRecord->statusData.Size());
			} 
			else 
			{
				//  no result data, so provide a nice, tidy NULL instead
				pResultData = NULL;
			}

			pCmdData = new char[pSQRecord->cmdData.Size()];
			memcpy(pCmdData, pSQRecord->cmdData.ConstPtr(), pSQRecord->cmdData.Size());

			(m_pDdmServices->*cb)(
				pSQRecord->statusCode,
				pResultData,
				pCmdData,
				(void *)pOutstandingCmd->pCmdContext);

			//  dispose of result data temp buffer, if any
			delete [] pResultData;
			delete [] pCmdData;
			// we allocated a copy of the row, so we need to delete it
			delete pSQRecord;
		}
		csndrDeleteOutstandingCmd(pOutstandingCmd);
	}
}
#endif


//**************************************************************************
//
//	PRIVATE:
//		csndrAddOutstandingCmd
//
//	Adds a command to the outstanding list so that it can be matched
//	up later when a status queue insert row reply is received for
//	the particular cmd.
//
//**************************************************************************
void CmdSender
::csndrAddOutstandingCmd(OUTSTANDING_CMD	*pOutstandingCmd)
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
//		csndrGetOutStandingCmd
//
//	- Search the outstanding cmd list to see if this status was for
//		a cmd in our list
//	- if found, return the outstanding cmd 
//
//**************************************************************************
void CmdSender
::csndrGetOutstandingCmd(rowID	*pCmdRowId, OUTSTANDING_CMD **ppOutstandingCmd)
{
	OUTSTANDING_CMD	*pTemp;

	pTemp = pHead;
	while (pTemp != NULL){
		if (memcmp(&pTemp->cmdRowId, pCmdRowId, sizeof(rowID)) == 0){
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
//		csndrDeleteOutstandingCmd
//
//	- Search the outstanding cmd , if found remove it and update list
//
//**************************************************************************
void CmdSender
::csndrDeleteOutstandingCmd(OUTSTANDING_CMD *pOutstandingCmd)
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
			delete pOutstandingCmd;
			return;
		}
		pTemp = pTemp->pNextCmd;
	}
}


void CmdSender
::csndrDeleteAllOutstandingCmds()
{
	OUTSTANDING_CMD	*pTemp;

	pTemp = pHead;
	while (pHead != NULL){
		pHead = pTemp->pNextCmd;
		delete pTemp;
	}
}






//*********************************************************
//	utilEnumTable (utility method)
//		Enums the table specified and returns the number of rows
//		and the data in those rows.
//
//	tableName		- table whose rows are to be enumed
//	ppTableDataRet	- where you want data - caller is supposed to delete
//	pSizeofTableDataRet	- where you want size of ret data
//	pNumberOfRows		- where you want no.of rows returned
//
//	pOriginalContext	- any context data
//	pCallback			- the callback on completion of enum
//
//********************************************************
STATUS CmdSender::
utilEnumTable(
			char			*tableName,
			void			**ppTableDataRet,		// where you want data
			U32				*pSizeofTableDataRet,	// where you want size of ret data
			U32				*pNumberOfRows,
			void			*pOriginalContext,
			pTSCallback_t	pCallback)
{
	TSGetTableDef		*pTableDef = new TSGetTableDef;
	STATUS				status;

	TABLE_CONTEXT		*pContext = new TABLE_CONTEXT;
	
	pContext->pParentContext		= pOriginalContext;			
	pContext->pCallback			= pCallback;
	pContext->pData				= new String64;
	strcpy((char *)pContext->pData,tableName);
	pContext->pData1			= ppTableDataRet;
	pContext->pData2			= (void *)pSizeofTableDataRet;
	pContext->pData3			= (void *)pNumberOfRows;
	
	status = pTableDef->Initialize(	
							this,
							(char *)pContext->pData,
							NULL,		
							0,
							&pContext->value,	// num cols
							&pContext->value1,	// bytes/row
							&pContext->value2,	// no of rows
							&pContext->value3,	// count of fields in a row
							NULL,
							(pTSCallback_t)&CmdSender::utilEnumerate,
							pContext );
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTableDef->Send();
	return status;
}




//*********************************************************
//	utilEnumerate (utility method)
//		This method is called after we get the table def for
//		the table and determine the number of rows and bytes per
//		row. We allocate the total memory required to hold the table
//		data and call the PTS enum table method to actually read
//		the table data.
//
//********************************************************
STATUS CmdSender::
utilEnumerate(void	*_pContext, STATUS status)
{
	U32					numberOfRows = 0;
	U32					bytesPerRow = 0;
	void				**ppTableDataRet = NULL;
	U32					*pSizeofTableDataRet =NULL;
	U32					*pNumberOfRows = NULL;
	char				*pTableName;
	TABLE_CONTEXT		*pContext = (TABLE_CONTEXT *)_pContext;
	
	if (status != OS_DETAIL_STATUS_SUCCESS){
		// we report back to the caller with the error.
		utilEnumReplyHandler (pContext,status);
	} else {
		numberOfRows = pContext->value2;
		bytesPerRow = pContext->value1;

		// Fill in the information for the original context
		pNumberOfRows = (U32 *)pContext->pData3;
		*pNumberOfRows = numberOfRows;


		ppTableDataRet = (void **)pContext->pData1;
		pSizeofTableDataRet = (U32 *)pContext->pData2;

		// this is where the user had wanted his data, caller will delete
		*ppTableDataRet	= new char[numberOfRows * bytesPerRow];
		memset(*ppTableDataRet, 0, numberOfRows * bytesPerRow );

		pTableName				= new String64;
		strcpy(pTableName, (char *)pContext->pData);

		// the handler will return the enum data to the caller
		// with the state of the orignalContext.
		TSEnumTable			*pEnumTable = new TSEnumTable();
		status = pEnumTable->Initialize(
							this,
							(char *)pTableName,		
							0,						// start row
							*ppTableDataRet,
							numberOfRows * bytesPerRow,
							(U32 *)pSizeofTableDataRet,
							(pTSCallback_t)&CmdSender::utilEnumReplyHandler,
							pContext);
		if (status == OS_DETAIL_STATUS_SUCCESS)
			pEnumTable->Send();
		// pEnumTable->Initialize() does a strcpy, so we can delete.
		delete pTableName;
	}
	return status;
}


//*********************************************************
//	utilReplyHandler (utility method)
//		The reply for the EnumTable->Send(). Here we just call
//		the users callback with their originalContext and the
//		status.
//
//********************************************************
STATUS CmdSender::
utilEnumReplyHandler(void *_pContext, STATUS status)
{
	TABLE_CONTEXT		*pOriginalContext = NULL;
	pTSCallback_t		cb;

	TABLE_CONTEXT		*pContext = (TABLE_CONTEXT *)_pContext;

	pOriginalContext = (TABLE_CONTEXT *)pContext->pParentContext;
	cb = (pTSCallback_t)pContext->pCallback,	// callback
	(this->*cb)(pOriginalContext,status);
	// now you can delete pContext, but we dont need to delete
	// any data that we allocated since user will delete it.
	pContext->pData1 = NULL;
	pContext->pData2 = NULL;
	pContext->pData3 = NULL;
	delete pContext;
	pContext = NULL;
	return status;
}

