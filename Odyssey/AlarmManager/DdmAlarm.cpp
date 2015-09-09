/* DdmAlarm.cpp
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History:
// $Log: /Gemini/Odyssey/AlarmManager/DdmAlarm.cpp $  
// 
// 30    10/12/99 7:27p Joehler
// Pulled out Alarm Event Reporting while rethinking event text
// 
// 29    10/12/99 5:58p Joehler
// Added Alarm Master event reporting and Quiesce method
// 
// 28    9/29/99 6:47p Joehler
// Added alarm coalescing
// 
// 27    9/14/99 9:31a Joehler
// Added notes field for remitting an alarm from the user
// 
// 26    9/13/99 3:38p Joehler
// Added functionality for SSAPI to remit an alarm from for the user
// through the command server
// 
// 25    9/12/99 10:50p Joehler
// Changed AlarmTableServices to TableServicesUtil and AlarmQueue to
// CommandProcessingQueue
// 
// 24    9/12/99 5:32p Agusev
// Fixed a "poking at other's internals" bug
// 
// 23    9/11/99 1:08p Agusev
// Added code for marking username and clearing notes on "SUBMIT"
// 
// 22    9/09/99 3:49p Agusev
// Set processingCommand to FALSE initially (wasn't inited)
// 
// 21    9/09/99 3:18p Agusev
// 
// 20    9/07/99 2:05p Joehler
// Changes per code review:
//      called new with tZERO arg
// 
// 19    9/02/99 11:46a Joehler
// added comments
// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include "DdmAlarm.h"
#include "BuildSys.h"

#include "CTEvent.h"

// BuildSys linkage
	
CLASSNAME(DdmAlarm,SINGLE);

SERVELOCAL(DdmAlarm, REQ_ALARM_SUBMIT);
SERVELOCAL(DdmAlarm, REQ_ALARM_REMIT);
SERVELOCAL(DdmAlarm, REQ_ALARM_RECOVER);
SERVELOCAL(DdmAlarm, REQ_ALARM_QUERY);

//SERVEVIRTUAL(DdmAlarm, REQ_ALARM_SUBMIT);
//SERVEVIRTUAL(DdmAlarm, REQ_ALARM_REMIT);
//SERVEVIRTUAL(DdmAlarm, REQ_ALARM_RECOVER);
//SERVEVIRTUAL(DdmAlarm, REQ_ALARM_QUERY);

//  DdmAlarm::Ctor (did)
//	  (static)
//
//  Description:
//    This routine is called by CHAOS when it wants to create
//    an instance of DdmAlarm.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    DdmAlarm::Ctor - Returns a pointer to the new instance, or NULL.
//

Ddm *DdmAlarm::Ctor (DID did)
{
	TRACE_ENTRY(DdmAlarm::Ctor(DID did));
	return (new DdmAlarm (did));
} 
/*	end DdmAlarm::Ctor  *****************************************************/

//  DdmAlarm::Initialize (pMsg)
//    (virtual)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    DdmAlarm::Enable().  Please note that this DDM is not completely
//	  initialized when this procedure returns.  It has spawned several
//    DefineTable messages and a call to Initialize the CmdServer and
//    CmdSender().  When these are complete, m_Initialized will be set
//    to TRUE.
//
//  Inputs:
//    pMsg - Points to message which triggered our DDM's fault-in.  This is
//          always an "initialize" message.
//
//  Outputs:
//    DdmAlarm::Initialize - Returns OK if all is cool, else an error.
//

STATUS DdmAlarm::Initialize (Message *pMsg)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::Initialize(Message*));

	DispatchRequest(REQ_ALARM_SUBMIT, REQUESTCALLBACK (DdmAlarm, ProcessSubmit));
	DispatchRequest(REQ_ALARM_REMIT, REQUESTCALLBACK (DdmAlarm, ProcessRemit));
	DispatchRequest(REQ_ALARM_RECOVER, REQUESTCALLBACK (DdmAlarm, ProcessRecover));
	DispatchRequest(REQ_ALARM_QUERY, REQUESTCALLBACK (DdmAlarm, ProcessQuery));

	m_Initialized = FALSE;
	m_isQuiesced = FALSE;

	m_pQuiesceMessage = NULL;

	m_pAlarmQueue = new CommandProcessingQueue;

	m_pTableService = new TableServicesUtil(this);

	m_pCmdServer = NULL;

	processingCommand = FALSE;

	// This is only for test, only for WIN32 and only to enforce the order 
	// in which tables are defined --> GAI
#ifdef WIN32
	Sleep( 3000 );	// 3 seconds
#endif
	status = DefineAlarmRecordTable(pMsg);

	return status;
		
}
/*	end DdmAlarm::Initialize  *****************************************************/

//  DdmAlarm::Enable (pMsgReq)
//
//  Description:
//   Lets go!

STATUS DdmAlarm::Enable(Message *pMsgReq)
{
	TRACE_ENTRY(DdmAlarm::Enable());

	m_isQuiesced = FALSE;

	// check for any commands to run.
	ProcessNextCommand();
	Ddm::Enable(pMsgReq);
	return OK;
		
}
/* end DdmAlarm::Enable  *****************************************************/

//  DdmAlarm::Quiesce (pMsgReq)
//
//  Description:
//		Quiesce the Alarm Master by stopping the command queue
STATUS	DdmAlarm::Quiesce(Message *pMsg)
{
	// set the Quiesce flag, so that no more commands are
	// processed after this is set
	m_isQuiesced = true;
	m_pQuiesceMessage = pMsg;
	return OS_DETAIL_STATUS_SUCCESS;
}
/* end DdmAlarm::Enable  *****************************************************/

//  DdmAlarm::ProcessSubmit (pMsg)
//
//  Description:
//    Entry function to service a REQ_ALARM_SUBMIT message.  
//
//  Inputs:
//    pMsg - Points to message which triggered submit request.  This is
//          a MsgSubmitAlarm message
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//

STATUS DdmAlarm::ProcessSubmit(Message *pMsgReq) 
{

	STATUS status = OK;
	TRACE_ENTRY(DdmAlarm::ProcessSubmit());

	// cast to correct Message type
	MsgSubmitAlarm* msg = (MsgSubmitAlarm *)pMsgReq;
	
	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;

	pContext->msg = msg;
	
	// get Event Data from message
	msg->GetEventData(&pContext->pData);
	pContext->eventDataSize= msg->GetEventDataSize();
	
	// get virtual device and did from msg
	pContext->did = msg->GetDid();
	pContext->vdn = msg->GetVdn();
	
	// get size of alarm context
	pContext->cbContext = msg->GetAlarmContextSize();
	assert(pContext->cbContext <= ALARM_MAX_SIZE_CONTEXT);
	
	// get alarm context
	msg->GetAlarmContext(&pContext->pAlarmContext);

	SubmitRequest(function_SubmitAlarm, pContext);

	return status;
}
/* end DdmAlarm::ProcessSubmit  ***********************************************/

//  DdmAlarm::ProcessRemit
//
//  Description:
//    Entry function to service a REQ_ALARM_REMIT message.  
//
//  Inputs:
//    pMsg - Points to message which triggered Remit request.  This is
//          a MsgRemitAlarm message
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmAlarm::ProcessRemit(Message *pMsgReq)
{

	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::ProcessRemit());

	// cast message to the correct type
	MsgRemitAlarm* msg = (MsgRemitAlarm*) pMsgReq;

	// allocate and initialize context for remit
	CONTEXT* pContext = new(tZERO) CONTEXT;
	pContext->msg = msg;
	pContext->rid = msg->GetRowId();
	pContext->did = msg->GetDid();
	pContext->vdn = msg->GetVdn();
	
	// calls general alarm remission function
	SubmitRequest(function_RemitAlarm, pContext);

	return status;
}
/* end DdmAlarm::ProcessRemit  ***********************************************/

//  DdmAlarm::ProcessRecover (pMsg)
//
//  Description:
//    Entry function to service a REQ_ALARM_RECOVER message.  
//
//  Inputs:
//    pMsg - Points to message which triggered Recover request.  This is
//          a MsgRecoverAlarms message
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmAlarm::ProcessRecover(Message *pMsgReq) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmAlarm::ProcessRecover());

	// cast message to the correct type
	MsgRecoverAlarms* msg = (MsgRecoverAlarms*) pMsgReq;

	CONTEXT* pContext = new(tZERO) CONTEXT;
	pContext->msg = msg;
	pContext->did = msg->GetDid();
	pContext->vdn = msg->GetVdn();

	// call general alarm recovery routine
	SubmitRequest(function_RecoverAlarms, pContext);

	return status;
}
/* end DdmAlarm::ProcessRecover  ***********************************************/

//  DdmAlarm::ProcessQuery (pMsg)
//
//  Description:
//    Entry function to service a REQ_ALARM_QUERY message.  
//
//  Inputs:
//    pMsg - Points to message which triggered Query request.  This is
//          a MsgQueryAlarms message
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmAlarm::ProcessQuery(Message *pMsgReq) 
{
	STATUS		status = OK;

	TRACE_ENTRY(DdmAlarm::ProcessQuery());

	// cast message to the correct type
	MsgQueryAlarms* msg = (MsgQueryAlarms*) pMsgReq;

	CONTEXT* pContext = new(tZERO) CONTEXT;
	pContext->msg = msg;

	// call general alarm Query routine
	SubmitRequest(function_QueryAlarms, pContext);

	return status;
}
/* end DdmAlarm::ProcessQuery  ***********************************************/

//****************************
// PRIVATE METHODS BEGIN HERE
//****************************

//  DdmAlarm::amstrListenerForCommands (status)
//
//  Description:
//     This is called when any CmdSender objects calls its Execute method.
//     This allows the AlarmManager to receive commands from the SSAPI and 
//     act upon them
//
//  Inputs:
//	  HANDLE handle - handle to indentify request to command sender
//    void* CmdData - command inputs
//
//  Outputs:
//    none
//
void DdmAlarm::amstrListenerForCommands(HANDLE handle, void* pCmdData)
{
	TRACE_ENTRY(DdmAlarm::amstrListenerForCommands(HANDLE, void*));

	STATUS status = OK;
	AMSTR_CMND_INFO *pCmdInfo = (AMSTR_CMND_INFO*)pCmdData;
	AMSTR_CMND_PARAMETERS *pCmdParams = &pCmdInfo->cmdParams;

	// allocate and initialize context
	CONTEXT* pContext = new(tZERO) CONTEXT;
	pContext->handle = handle;
	pContext->pCmdInfo = pCmdInfo;
	pContext->rid = pCmdParams->rid;

	// set notes in pContext

	switch (pCmdInfo->opcode) {
	case AMSTR_CMND_ACKNOWLEDGE_ALARM:
		SubmitRequest(function_AcknowledgeAlarm, pContext);
		break;
	case AMSTR_CMND_UNACKNOWLEDGE_ALARM:
		SubmitRequest(function_UnAcknowledgeAlarm, pContext);
		break;
	case AMSTR_CMND_NOTIFY_ALARM:
		SubmitRequest(function_NotifyAlarm, pContext);
		break;
	case AMSTR_CMND_KILL_ALARM:
		SubmitRequest(function_KillAlarm, pContext);
		break;
	case AMSTR_CMND_REMIT_ALARM_FROM_USER:
		SubmitRequest(function_RemitAlarm, pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmAlarm::amstrListenerForCommands  ************************************/

//  DdmAlarm::DefineAlarmRecordTable
//
//  Description:
//    Defines the AlarmRecord table for use by the AlarmManager
//
//  Inputs:
//	  Message* pMsg - initiating message
//
//  Outputs:
//    status -  return OK if ok
//
STATUS DdmAlarm::DefineAlarmRecordTable(Message* pMsg)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::DefineAlarmRecordTable(Message*));

	// set up TSDefineTable object to define the AlarmTable
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	// allocate and initialize CONTEXT to be used in TSDefineTable
	CONTEXT *pContext = new(tZERO) CONTEXT;
	pContext->state = alarmmstr_RECORD_TABLE_DEFINED;
	pContext->msg = pMsg;

	// Initialize the TSDefineTable Object
	status = pTSDefineTable->Initialize(
		this,
		ALARM_RECORD_TABLE_NAME, 
		AlarmRecordTable_FieldDefs, 
		sizeofAlarmRecordTable_FieldDefs, 
		ALARM_RECORD_TABLE_SIZE,
		TRUE,
		(pTSCallback_t)&DdmAlarm::AlarmInitReplyHandler,
		pContext
		);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();

	return status;
}
/* end DdmAlarm::DefineAlarmRecordTable  ************************************/

//  DdmAlarm::DefineAlarmDidTable
//
//  Description:
//    Defines the DidRecord table for use by the AlarmManager
//
//  Inputs:
//	  none
//
//  Outputs:
//    status -  return OK if ok
//

STATUS DdmAlarm::DefineAlarmDidTable()
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::DefineAlarmDidTable());

	// set up TSDefineTable object to define the AlarmTable
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	// allocate and initialize CONTEXT to be used in TSDefineTable
	CONTEXT *pContext = new(tZERO) CONTEXT;
	pContext->state = alarmmstr_DID_TABLE_DEFINED;

	// Initialize the TSDefineTalble Object
	status = pTSDefineTable->Initialize(
		this,
		ALARM_DID_TABLE_NAME, 
		AlarmDidTable_FieldDefs, 
		sizeofAlarmDidTable_FieldDefs, 
		ALARM_DID_TABLE_SIZE,
		TRUE,
		(pTSCallback_t)&DdmAlarm::AlarmInitReplyHandler,
		pContext
		);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();

	return status;
}
/* end DdmAlarm::DefineAlarmDidTable  ************************************/

//  DdmAlarm::DefineAlarmLogTable
//
//  Description:
//    Defines the AlarmLogRecord table for use by the AlarmManager
//
//  Inputs:
//	  none
//
//  Outputs:
//    status -  return OK if ok
//
STATUS DdmAlarm::DefineAlarmLogTable()
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::DefineAlarmLogTable());

	// set up TSDefineTable object to define the AlarmTable
	TSDefineTable *pTSDefineTable = new TSDefineTable;

	// allocate and initialize CONTEXT to be used in TSDefineTable
	CONTEXT *pContext = new(tZERO) CONTEXT;
	pContext->state = alarmmstr_LOG_TABLE_DEFINED;

	// Initialize the TSDefineTalble Object
	status = pTSDefineTable->Initialize(
		this,
		ALARM_LOG_TABLE_NAME, 
		AlarmLogTable_FieldDefs, 
		sizeofAlarmLogTable_FieldDefs, 
		ALARM_LOG_TABLE_SIZE,
		TRUE,
		(pTSCallback_t)&DdmAlarm::AlarmInitReplyHandler,
		pContext
		);

	if (status == OS_DETAIL_STATUS_SUCCESS)
		pTSDefineTable->Send();

	return status;
}
/* end DdmAlarm::DefineAlarmLogTable  ************************************/

//  DdmAlarm::InitializeCmdServer
//
//  Description:
//    Initializes the command server for use as an interface between 
//    the AlarmManager and the SSAPI
//
//  Inputs:
//	  none
//
//  Outputs:
//    status -  return OK if ok
//
STATUS DdmAlarm::InitializeCmdServer()
{
	TRACE_ENTRY(DdmAlarm::InitializeCmdServer());

	// create the cmdserver object and call its initialize method
	// wait for initialization done reply
	m_pCmdServer = new CmdServer(
		AMSTR_CMD_QUEUE_TABLE,
		sizeof(AMSTR_CMND_INFO),
		sizeof(AMSTR_EVENT_INFO),
		this, 
		(pCmdCallback_t)&DdmAlarm::amstrListenerForCommands);

	m_pCmdServer->csrvInitialize(
		(pInitializeCallback_t)&DdmAlarm::amstrObjectInitializeReply);

	return OK;
}

//  DdmAlarm::amstrObjectIntializeReply (status)
//
//  Description:
//    Waits for initialized reply from command server
//
//  Inputs:
//	  status
//
//  Outputs:
//    none
//
void DdmAlarm::amstrObjectInitializeReply(STATUS status)
{
	TRACE_ENTRY(DdmAlarm::amstrObjectInitializeReply())
	if (!status) {
		// We are initialized successfully, so we should be ready to accept
		// commmands.

		// allocate CONTEXT and go to standard AlarmInitReplyHandler
		CONTEXT *pContext = new(tZERO) CONTEXT;
		pContext->state = alarmmstr_CMD_SERVER_INITIALIZED;
		AlarmInitReplyHandler(pContext, status);
	}
}
/* end DdmAlarm::amstrObjectIntializeReply  ************************************/

//  DdmAlarm::AlarmInitReplyHandler
//
//  Description:
//    This function is the call back routine use through out the initialization
//    process.  It is a state machine which guides the AlarmManager
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the previous call
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmInitReplyHandler(void* _pContext, STATUS status) {
	
	TRACE_ENTRY(DdmAlarm::AlarmInitReplyHandler());
	static Message* initMsg;

	CONTEXT *pContext = (CONTEXT*)_pContext;

	if ((status != OS_DETAIL_STATUS_SUCCESS) &&
		(status != ercTableExists))
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_RECORD_TABLE_DEFINED:
		TRACE_STRING(5, ("DdmAlarm::AlarmInitReplyHandler - RECORD_TABLE_DEFINED\n")); 
		initMsg = pContext->msg;
		status = DefineAlarmDidTable();
		break;
	case alarmmstr_DID_TABLE_DEFINED:
		TRACE_STRING(5, ("DdmAlarm::AlarmInitReplyHandler - DID_TABLE_DEFINED\n")); 
		status = DefineAlarmLogTable();
		break;
	case alarmmstr_LOG_TABLE_DEFINED:
		TRACE_STRING(5, ("DdmAlarm::AlarmInitReplyHandler - LOG_TABLE_DEFINED\n")); 
		status = InitializeCmdServer();
		break;
	case alarmmstr_CMD_SERVER_INITIALIZED:
		TRACE_STRING(5, ("DdmAlarm::AlarmInitReplyHandler - COMMAND_SERVER_INIT\n")); 
		m_Initialized = TRUE;
		Reply(initMsg, OK);
		break;	
	default:
		assert(0);
	}

	delete pContext;
	pContext = NULL;
	
	return status;
}
/* end DdmAlarm::AlarmInitReplyHandler  ********************************/

//  DdmAlarm::SubmitAlarm
//
//  Description:
//    General alarm submission routine which all requests from SSAPI and 
//    DdmMasters are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::SubmitAlarm(CONTEXT* pContext)
{
	STATUS status = OK;
	U32 sizeOfKey;
	String64 nameOfKey;
	void* valueOfKey;

	TRACE_ENTRY(DdmAlarm::SubmitAlarm(...));

	// Initialize pContext with DidRecord to be used in TSReadRow call
	DidRecord* pDidRecord = new(tZERO) DidRecord;

	pContext->state = alarmmstr_DID_ROW_READ;
	pContext->pDidRecord = pDidRecord;

	SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);

	status = m_pTableService->ReadRowWithKey(
		ALARM_DID_TABLE_NAME, 
		nameOfKey,
		valueOfKey,
		sizeOfKey,
		pContext->pDidRecord,
		sizeof(DidRecord),
		(pTSCallback_t)&DdmAlarm::AlarmSubmitReplyHandler,
		pContext);

	// call back funtion AlarmSubmitReplyHandler calls ReadDidAlarms
	// if there are any outstanding alarms for this device.  Otherwise
	// it calls InsertDidRecord() to insert a row to be associated
	// with this device and keep track of its alarms

	return status;
}
/* end DdmAlarm::SubmitAlarm  ************************************/

//  DdmAlarm::InsertDidRecord
//
//  Description:
//	  If no alarms are outstanding for this device, then a 
//	  DidRecord needs to be created and added to the DidRecord table
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::InsertDidRecord(CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::InsertDidRecord());

	// initialize DidRecord
	pContext->pDidRecord->version = ALARM_DID_TABLE_VERSION;
	pContext->pDidRecord->size = sizeof(DidRecord);
	pContext->pDidRecord->did = pContext->did;
	pContext->pDidRecord->vdn = pContext->vdn;
	pContext->pDidRecord->numberOfAlarms = 1;

	// initialize context state
	pContext->state = alarmmstr_DID_ROW_INSERTED;

	status = m_pTableService->InsertRow(
		ALARM_DID_TABLE_NAME,
		pContext->pDidRecord,
		sizeof(DidRecord),
		&pContext->pDidRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmSubmitReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::InsertDidRecord  ************************************/

//  DdmAlarm::ReadDidAlarms
//
//  Description:
//	  If we enter this function, we have determined there are some
//    outstanding alarms assocaited with this device.  We now need 
//    to examine these alarms to see if any of them are identical
//    to the one being submit.  Implementation of the "coalescing"
//    of alarms is yet to be completed.
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ReadDidAlarms(CONTEXT* pContext)
{
	STATUS status = OK;
	AlarmRecord* pAlarmRecord;
	U32 sizeOfKey;
	String64 nameOfKey;
	void *valueOfKey;

	TRACE_ENTRY(DdmAlarm::ReadDidAlarms(...));

	// Allocate and initialize AlarmRecord to be used in TSReadRow call
	pAlarmRecord = new(tZERO) AlarmRecord[pContext->pDidRecord->numberOfAlarms];

	// initialize context
	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

	SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);

	status = m_pTableService->ReadRowWithKey(
		ALARM_RECORD_TABLE_NAME, 
		nameOfKey,
		valueOfKey,
		sizeOfKey,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord) * pContext->pDidRecord->numberOfAlarms,
		(pTSCallback_t)&DdmAlarm::AlarmSubmitReplyHandler,
		pContext);

	// callback routine will call CheckAlarmMatch() to determine if alarm
	// coalescing can occur

	return status;
}
/* end DdmAlarm::ReadDidAlarms  ************************************/

//  DdmAlarm::CompareAlarms
//
//  Description:
//
//  Inputs:
//
//  Outputs:
//
BOOL DdmAlarm::CompareAlarms(Event* pEvent, Event* pSubmittedEvent)
{
	if (pEvent->GetEventCode() != pSubmittedEvent->GetEventCode())
		return FALSE;

	if (pEvent->GetSignificantParms() != pSubmittedEvent->GetSignificantParms())
		return FALSE;

	U32 sigParams = pEvent->GetSignificantParms();
	U16 maxParamCount, otherParamCount;

	if (pEvent->GetParameterCount() > pSubmittedEvent->GetParameterCount())
	{
		maxParamCount = pEvent->GetParameterCount();
		otherParamCount = pSubmittedEvent->GetParameterCount();
	}
	else
	{
		maxParamCount = pSubmittedEvent->GetParameterCount();
		otherParamCount = pEvent->GetParameterCount();
	}

	for (U16 i = 0; i < maxParamCount; i++)
	{
		if (sigParams & (0x00000001 << i))
		{
			if (otherParamCount <= i)
				return FALSE;
			if (pEvent->GetParameterType(i) != pSubmittedEvent->GetParameterType(i))
				return FALSE;
			switch (pEvent->GetParameterType(i))
			{
			case Event::CHAR_PARM:
			case Event::S8_PARM:
			case Event::U8_PARM:
			case Event::S16_PARM:
			case Event::U16_PARM:
			case Event::S32_PARM:
			case Event::U32_PARM:
			case Event::S64_PARM:
			case Event::U64_PARM:
				if (pEvent->GetPParameter(i) != pSubmittedEvent->GetPParameter(i))
					return FALSE;
				break;
			case Event::HEX_PARM:
				if (pEvent->GetParameterSize(i) != pSubmittedEvent->GetParameterSize(i))
					return FALSE;
				else
					if (memcmp(pEvent->GetPParameter(i), 
						pSubmittedEvent->GetPParameter(i),
						pEvent->GetParameterSize(i)))
						return FALSE;
				break;
			case Event::STR_PARM:
				if (strcmp((char*)pEvent->GetPParameter(i), (char*)pSubmittedEvent->GetPParameter(i)))
					return FALSE;
				break;
			default:
				assert(0);
				break;
			}
		}
	}

	return TRUE;
}
/* end DdmAlarm::CompareAlarms  ************************************/

//  DdmAlarm::CheckAlarmMatch
//
//  Description:
//    Function to check if an alarm is already submitted for this
//    device that matches the one being submitted.  Implementation
//    TBD.  A matching alarm will have the same device, event code
//    same significant parameters
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::CheckAlarmMatch(CONTEXT* pContext)
{
	U16 i;
	STATUS status;
	rowID rid;
	MsgSubmitAlarm* msg = (MsgSubmitAlarm*)pContext->msg;
	Event* pEvent = new Event(pContext->pData);

	TRACE_ENTRY(DdmAlarm::CheckAlarmMatch(...));

	for (i = 0; i < pContext->pDidRecord->numberOfAlarms; i++)
	{
		if (pContext->pAlarmRecord[i].active &&
			CompareAlarms(pEvent, new Event(pContext->pAlarmRecord[i].eventData))) 
		{
			rid = pContext->pAlarmRecord[i].rid;	
			/*if (IsVirtual(pContext))
				LogEvent(CTS_ALARM_VIRTUAL_RESUBMITTED,
					rid.LoPart, 
					pEvent->GetEventCode(), 
					pContext->vdn);
			else
				LogEvent(CTS_ALARM_DEVICE_RESUBMITTED, 
					rid.LoPart,
					pEvent->GetEventCode(), 
					pContext->did);*/
			msg->SetRowId(rid);
			Reply(msg, OK);
			ClearAndDeleteContext(pContext);
			pContext = NULL;
			ProcessNextCommand();
			return OK;
		}
	}

	// if no match was found, insert a new AlarmRecord into the table
	delete pContext->pAlarmRecord;
	pContext->pAlarmRecord = NULL;
	
	status = ModifyIncrementDidRecord(pContext);
	
	return status;
}
/* end DdmAlarm::CheckAlarmMatch  ************************************/

//  DdmAlarm::InsertAlarmRecord
//
//  Description:
//    If no outstanding alarm was found to coalesce the alarm
//    being submitted with, then we need to insert a new row
//    in the AlarmRecord table for this alarm
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::InsertAlarmRecord(CONTEXT* pContext)
{
	STATUS status;
	U32 cbEventData;

	TRACE_ENTRY(DdmAlarm::InsertAlarmRecord(...));
		
	MsgSubmitAlarm* msg = (MsgSubmitAlarm*)pContext->msg;

	// allocate an AlarmRecord (row) to insert and initialize
	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord;
	pAlarmRecord->version = ALARM_TABLE_VERSION;
	pAlarmRecord->size = sizeof(AlarmRecord);
	pAlarmRecord->cbContext = pContext->cbContext;
	pAlarmRecord->did = pContext->did;
	pAlarmRecord->vdn = pContext->vdn;
	// copy the value of the alarm context into the alarm record
	memcpy(&(pAlarmRecord->alarmContext), pContext->pAlarmContext, pContext->cbContext);
	pAlarmRecord->active = TRUE;
	pAlarmRecord->acknowledged = FALSE;
	pAlarmRecord->clearable = msg->IsClearable();
	pAlarmRecord->numberOfEvents = 1;
	
	cbEventData = pContext->eventDataSize;	// GAI- : *((char*)pContext->pData);
	// this is an error
	assert(cbEventData <= ALARM_MAX_EVENT_DATA_SIZE);
	memcpy(pAlarmRecord->eventData, pContext->pData, cbEventData);

	// initialize context state
	pContext->state = alarmmstr_ALARM_ROW_INSERTED;
	pContext->pAlarmRecord = pAlarmRecord;

	status = m_pTableService->InsertRow(
		ALARM_RECORD_TABLE_NAME,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		&pContext->pAlarmRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmSubmitReplyHandler,
		pContext);

	// callback routine calls ModifyIncrementDidRecord() to bump up 
	// the alarmCount for this device

	return status;
}
/* end DdmAlarm::InsertAlarmRecord  ************************************/

//  DdmAlarm::ModifyIncrementDidRecord
//
//  Description:
//    Modify the DidRecord to relect that one more alarm is now 
//    associated with this device.
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ModifyIncrementDidRecord(CONTEXT* pContext)
{
	STATUS status;
	U32 sizeOfKey;
	String64 nameOfKey;
	void* valueOfKey;
	U32 numberOfAlarms;

	TRACE_ENTRY(DdmAlarm::ModifyIncrementDidRecord(...));

	// modify DidRecord count
	numberOfAlarms = ++pContext->pDidRecord->numberOfAlarms;

	// initialize context state
	pContext->state = alarmmstr_DID_ROW_MODIFIED;

	SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);

	status = m_pTableService->ModifyField(
		ALARM_DID_TABLE_NAME,
		&pContext->pDidRecord->rid,
		fdNumberOfAlarms,
		&numberOfAlarms,
		sizeof(numberOfAlarms),
		(pTSCallback_t)&DdmAlarm::AlarmSubmitReplyHandler,
		pContext);

	// callback function replies to the original message

	return status;
}
/* end DdmAlarm::ModifyIncrementDidRecord  ********************************/

//  DdmAlarm::AlarmSubmitReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    submission process.  It is a state machine which guides the AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmSubmitReplyHandler(void* _pContext, STATUS status) 
{
	
	TRACE_ENTRY(DdmAlarm::AlarmSubmitReplyHandler());

	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (!((status==OS_DETAIL_STATUS_SUCCESS) ||
		((status==ercKeyNotFound) && (pContext->state == alarmmstr_DID_ROW_READ))))
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_DID_ROW_READ:
		TRACE_STRING(5, ("DdmAlarm::AlarmSubmitReplyHandler - DID_ROW_READ\n")); 
		if (status==ercKeyNotFound) {
			status = InsertDidRecord(pContext);
		}
		else
			status = ReadDidAlarms(pContext);
		break;
	case alarmmstr_DID_ROW_INSERTED:
	case alarmmstr_DID_ROW_MODIFIED:
		TRACE_STRING(5, ("DdmAlarm::AlarmSubmitReplyHandler - DID_ROW_INSERTED or MODIFIED\n"));
		status = InsertAlarmRecord(pContext);
		break;
	case alarmmstr_ALARM_ROWS_READ:
		TRACE_STRING(5, ("DdmAlarm::AlarmSubmitReplyHandler - ALARM_ROWS_READ\n")); 
		status = CheckAlarmMatch(pContext);
		break;
	case alarmmstr_ALARM_ROW_INSERTED:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmSubmitReplyHandler - ALARM_ROW_INSERTED\n"));
		AMSTR_EVENT_INFO* pEvtAlarmSubmitted = new(tZERO) AMSTR_EVENT_INFO;
		pEvtAlarmSubmitted->alarmSubmitted.did = pContext->did;
		pEvtAlarmSubmitted->alarmSubmitted.vdn = pContext->pAlarmRecord->vdn;
		pEvtAlarmSubmitted->alarmSubmitted.rid = pContext->pAlarmRecord->rid;
		m_pCmdServer->csrvReportEvent(
			AMSTR_EVT_ALARM_SUBMITTED,
			pEvtAlarmSubmitted);
		delete pEvtAlarmSubmitted;
		status = LogAlarmEvent(pContext, logaction_SUBMITTED);
		break;
		}
	case alarmmstr_LOG_ROW_INSERTED:
		{
		MsgSubmitAlarm* msg = (MsgSubmitAlarm*)pContext->msg;
		TRACE_STRING(5, ("DdmAlarm::AlarmSubmitReplyHandler - DID_ROW_MODIFIED\n"));
		msg->SetRowId(pContext->pAlarmRecord->rid);
		Event* pEvent = new Event(pContext->pData);
		/*if (IsVirtual(pContext))
			LogEvent(CTS_ALARM_VIRTUAL_SUBMITTED,
				pContext->pAlarmRecord->rid.LoPart, 
				pEvent->GetEventCode(), 
				pContext->vdn);
		else
			LogEvent(CTS_ALARM_DEVICE_SUBMITTED, 
				pContext->pAlarmRecord->rid.LoPart,
				pEvent->GetEventCode(), 
				pContext->did);*/
		Reply(msg, OK);
		ClearAndDeleteContext(pContext);
		pContext = NULL;
		ProcessNextCommand();
		break;
		}
	default:
		assert(0);
		break;
	}
	
	return status;
}
/* end DdmAlarm::AlarmSubmitReplyHandler  ********************************/

//  DdmAlarm::RemitAlarm
//
//  Description:
//    General alarm remission routine which all requests from SSAPI and 
//    DdmMasters are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm remission context
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::RemitAlarm(CONTEXT* pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::RemitAlarm(...));

	// initialize context and allocate AlarmRecord
	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord;

	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

	status = m_pTableService->ReadRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		(pTSCallback_t)&DdmAlarm::AlarmRemitReplyHandler,
		pContext);

	// callback routine calls ModifyAlarmRowStatus to remit this alarm

	return status;
}
/* end DdmAlarm::RemitAlarm  ***************************************************/


//  DdmAlarm::ModifyAlarmRowStatus
//
//  Description:
//    If this is the only alarm associated with this device, then delete the
//    DidRecord row from the table.  If not, then decrement the count of the 
//    alarms associated with the device
//
//  Inputs:
//		CONTEXT* pContext - Alarm remission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//										  
STATUS DdmAlarm::ModifyAlarmRowStatus(
										CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::ModifyAlarmRowStatus(...));

	// initialize AlarmRecord for modification
	pContext->pAlarmRecord->active = FALSE;
	pContext->pAlarmRecord->numberOfEvents++;
	
	// initialize context
	pContext->state = alarmmstr_ALARM_ROW_MODIFIED;

	status = m_pTableService->ModifyRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->pAlarmRecord->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		&pContext->pAlarmRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmRemitReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::ModifyAlarmRowStatus  ********************************/


//  DdmAlarm::AlarmRemitReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    remission process.  It is a state machine which guides the AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmRemitReplyHandler(void* _pContext, STATUS status) 
{

	TRACE_ENTRY(DdmAlarm::AlarmRemitReplyHandler(...));
	
	CONTEXT *pContext = (CONTEXT*)_pContext;
	MsgRemitAlarm* msg = (MsgRemitAlarm*)pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS )
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_ALARM_ROWS_READ:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmRemitReplyHandler - ALARM_ROWS_READ\n")); 
		if (pContext->pCmdInfo) // this is from the SSAPI command queue
			if (pContext->pAlarmRecord->clearable != TRUE)
			{
				// if this is not a user remittable alarm, then this
				// is an error
				m_pCmdServer->csrvReportCmdStatus(
					pContext->handle,
					AMSTR_ERR_NOT_USER_REMITTABLE,
					NULL,
					pContext->pCmdInfo);
				ClearAndDeleteContext(pContext);
				pContext = NULL;
				ProcessNextCommand();
				return AMSTR_ERR_NOT_USER_REMITTABLE;
			}
		ModifyAlarmRowStatus(pContext);
		break;
		}
	case alarmmstr_ALARM_ROW_MODIFIED:
		TRACE_STRING(5, ("DdmAlarm::AlarmRemitReplyHandler - ALARM_ROW_MODIFIED\n"));
		LogAlarmEvent(pContext, logaction_REMITTED);
		break;
	case alarmmstr_LOG_ROW_INSERTED:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmRemitReplyHandler - LOG_ROW_INSERTED\n"));
		AMSTR_EVENT_INFO* pEvtAlarmRemitted = new(tZERO) AMSTR_EVENT_INFO;
		pEvtAlarmRemitted->alarmRemitted.did = pContext->did;
		pEvtAlarmRemitted->alarmRemitted.vdn = pContext->vdn;
		pEvtAlarmRemitted->alarmRemitted.rid = pContext->rid;
		m_pCmdServer->csrvReportEvent(
			AMSTR_EVT_ALARM_REMITTED,
			pEvtAlarmRemitted);
		delete pEvtAlarmRemitted;
		if (pContext->msg)
		{
			/*if (IsVirtual(pContext))
				LogEvent(CTS_ALARM_VIRTUAL_REMITTED,
					pContext->pAlarmRecord->rid.LoPart, 
					pContext->vdn);
			else
				LogEvent(CTS_ALARM_DEVICE_REMITTED, 
					pContext->pAlarmRecord->rid.LoPart,
					pContext->did);*/
			Reply(pContext->msg, OK);
		}
		else
		{
			/*LogEvent(CTS_ALARM_USER_REMITTED, 
				pContext->pAlarmRecord->rid.LoPart);*/
			m_pCmdServer->csrvReportCmdStatus(
				pContext->handle,
				status,
				NULL,
				pContext->pCmdInfo);
		}
		ClearAndDeleteContext(pContext);
		pContext = NULL;
		ProcessNextCommand();
		break;
		}
	default:
		assert(0);
		break;
	}
	
	return status;
}
/* end DdmAlarm::AlarmRemitReplyHandler  ********************************/

//  DdmAlarm::KillAlarm
//
//  Description:
//    General alarm killing routine which all requests from SSAPI  
//    are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm killing context
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::KillAlarm(CONTEXT* pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::KillAlarm(...));

	// initialize context and allocate AlarmRecord
	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord;

	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

	status = m_pTableService->ReadRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		(pTSCallback_t)&DdmAlarm::AlarmKillReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::KillAlarm  ***************************************************/

//  DdmAlarm::DeleteAlarmRecord
//
//  Description:
//
//  Inputs:
//		CONTEXT* pContext - Alarm remission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//										  
STATUS DdmAlarm::DeleteAlarmRecord(CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::DeleteAlarmRecord(...));

	pContext->state = alarmmstr_ALARM_ROW_DELETED;

	status = m_pTableService->DeleteRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->pAlarmRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmKillReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::DeleteAlarmRecord  ********************************/


//  DdmAlarm::ReadDidRecord
//
//  Description:
//
//  Inputs:
//		CONTEXT* pContext - Alarm killing context
//
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ReadDidRecord(CONTEXT* pContext)
{
	STATUS status = OK;
	U32 sizeOfKey;
	String64 nameOfKey;
	void* valueOfKey;

	TRACE_ENTRY(DdmAlarm::ReadDidRecord(...));

	// initialize context and allocate DidRecord
	DidRecord* pDidRecord = new(tZERO) DidRecord;

	pContext->state = alarmmstr_DID_ROW_READ;
	pContext->pDidRecord = pDidRecord;

	SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);


	status = m_pTableService->ReadRowWithKey(
		ALARM_DID_TABLE_NAME, 
		nameOfKey,
		valueOfKey,
		sizeOfKey,
		pContext->pDidRecord,
		sizeof(DidRecord),
		(pTSCallback_t)&DdmAlarm::AlarmKillReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::ReadDidRecord  ***************************************************/

//  DdmAlarm::ModifyDecrementDidRecord
//
//  Description:
//    Modify the DidRecord to relect that one less alarm is now 
//    associated with this device.
//
//  Inputs:
//		CONTEXT* pContext - Alarm submission context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ModifyDecrementDidRecord(CONTEXT* pContext)
{
	STATUS status;
	U32 sizeOfKey;
	String64 nameOfKey;
	void* valueOfKey;
	U32 numberOfAlarms;

	TRACE_ENTRY(DdmAlarm::ModifyDecrementDidRecord(...));

	// modify DidRecord count
	numberOfAlarms = --pContext->pDidRecord->numberOfAlarms;

	// initialize context state
	pContext->state = alarmmstr_DID_ROW_MODIFIED;

	SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);

	status = m_pTableService->ModifyField(
		ALARM_DID_TABLE_NAME,
		&pContext->pDidRecord->rid,
		fdNumberOfAlarms,
		&numberOfAlarms,
		sizeof(numberOfAlarms),
		(pTSCallback_t)&DdmAlarm::AlarmKillReplyHandler,
		pContext);

	// callback function replies to the original message

	return status;
}
/* end DdmAlarm::ModifyDecrementDidRecord  ********************************/

//  DdmAlarm::AlarmKillReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    killing process.  It is a state machine which guides the AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmKillReplyHandler(void* _pContext, STATUS status) 
{

	TRACE_ENTRY(DdmAlarm::AlarmKillReplyHandler(...));
	
	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS )
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_ALARM_ROWS_READ:
		TRACE_STRING(5, ("DdmAlarm::AlarmKillReplyHandler - ALARM_ROWS_READ\n")); 
		pContext->did = pContext->pAlarmRecord->did;
		pContext->vdn = pContext->pAlarmRecord->vdn;
		DeleteAlarmRecord(pContext);
		break;
	case alarmmstr_ALARM_ROW_DELETED:
		TRACE_STRING(5, ("DdmAlarm::AlarmKillReplyHandler - ALARM_ROW_DELETED\n"));
		ReadDidRecord(pContext);
		break;
	case alarmmstr_DID_ROW_READ:
		TRACE_STRING(5, ("DdmAlarm::AlarmKillReplyHandler - DID_ROW_READ\n"));
		ModifyDecrementDidRecord(pContext);
		break;
	case alarmmstr_DID_ROW_MODIFIED:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmKillReplyHandler - DID_ROW_MODIFIED\n"));
		AMSTR_EVENT_INFO* pEvtKilled = new(tZERO) AMSTR_EVENT_INFO;
		pEvtKilled->alarmKilled.rid = pContext->rid;
		m_pCmdServer->csrvReportEvent(
			AMSTR_EVT_ALARM_KILLED,
			pEvtKilled);
		delete pEvtKilled;			
		/*LogEvent(CTS_ALARM_KILLED, 
			pContext->rid.LoPart);*/
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			status,
			NULL,
			pContext->pCmdInfo);
		ClearAndDeleteContext(pContext);
		pContext = NULL;
		ProcessNextCommand();
		break;
		}
	default:
		assert(0);
		break;
	}
	return status;
}
/* end DdmAlarm::AlarmKillReplyHandler  ********************************/

//  DdmAlarm::AcknowledgeAlarm
//
//  Description:
//    General alarm acknowledgement routine which all requests from SSAPI  
//     are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm acknowledgement context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AcknowledgeAlarm(CONTEXT* pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::AcknowledgeAlarm(...));
		
	// allocate and initialize an AlarmRecord 
	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord;

	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

	status = m_pTableService->ReadRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->rid,
		pContext->pAlarmRecord, 
		sizeof(AlarmRecord),
		(pTSCallback_t)&DdmAlarm::AlarmAcknowledgeReplyHandler,
		pContext);

	// callback routine calls ModifyAlarmRowAcknowledge to acknowledge this alarm

	return status;
}
/* end DdmAlarm::AcknowledgeAlarm  ***************************************************/

//  DdmAlarm::ModifyAlarmRowAcknowledge
//
//  Description:
//		Acknowledge the alarm
//
//  Inputs:
//		CONTEXT* pContext - Alarm acknowledgement context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ModifyAlarmRowAcknowledge(
										   CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::ModifyAlarmRowAcknowledge(...));

	// initialize AlarmRecord and context state
	pContext->pAlarmRecord->acknowledged = TRUE;
	pContext->pAlarmRecord->numberOfEvents++;

	pContext->state = alarmmstr_ALARM_ROW_MODIFIED;

	status = m_pTableService->ModifyRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->pAlarmRecord->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		&pContext->pAlarmRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmAcknowledgeReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::ModifyAlarmRowAcknowledge  ********************************/

//  DdmAlarm::AlarmAcknowledgeReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    acknowledgement process.  It is a state machine which guides the 
//    AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmAcknowledgeReplyHandler(
		void* _pContext, 
		STATUS status)
{
	TRACE_ENTRY(DdmAlarm::AlarmAcknowledgeReplyHandler(...));
	
	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS )
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_ALARM_ROWS_READ:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmAcknowledgeReplyHandler - ALARM_ROWS_READ\n")); 
		LogAlarmEvent(pContext, logaction_ACKNOWLEDGED);
		break;
		}
	case alarmmstr_LOG_ROW_INSERTED:
		ModifyAlarmRowAcknowledge(pContext);
		break;
	case alarmmstr_ALARM_ROW_MODIFIED:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmAcknowledgeReplyHandler - ALARM_ROW_MODIFIED\n"));
		AMSTR_EVENT_INFO* pEvtAcknowledged = new(tZERO) AMSTR_EVENT_INFO;
		pEvtAcknowledged->alarmAcknowledged.rid = pContext->rid;
		m_pCmdServer->csrvReportEvent(
			AMSTR_EVT_ALARM_ACKNOWLEDGED,
			pEvtAcknowledged);
		delete pEvtAcknowledged;
		/*LogEvent(CTS_ALARM_ACKNOWLEDGED, 
			pContext->rid.LoPart);*/
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			status,
			NULL,
			pContext->pCmdInfo);
		ClearAndDeleteContext(pContext);
		pContext = NULL;
		ProcessNextCommand();
		break;
		}
	default:
		assert(0);
		break;
	}

	return status;
}
/* end DdmAlarm::AlarmAcknowledgeReplyHandler  ********************************/

//  DdmAlarm::UnAcknowledgeAlarm
//
//  Description:
//    General alarm unacknowledgement routine which all requests from SSAPI  
//     are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm acknowledgement context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::UnAcknowledgeAlarm(CONTEXT* pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::UnAcknowledgeAlarm(...));

	// allocate and initialize an AlarmRecord 
	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord;

	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

	m_pTableService->ReadRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		(pTSCallback_t)&DdmAlarm::AlarmUnAcknowledgeReplyHandler,
		pContext);

	// callback routine calls ModifyAlarmRowUnAcknowledge to acknowledge this alarm

	return status;
}
/* end DdmAlarm::AcknowledgeAlarm  ***************************************************/

//  DdmAlarm::ModifyAlarmRowUnAcknowledge
//
//  Description:
//		Unacknowledge the alarm
//
//  Inputs:
//		CONTEXT* pContext - Alarm acknowledgement context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ModifyAlarmRowUnAcknowledge(
										   CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::ModifyAlarmRowUnAcknowledge(...));

	// initialize AlarmRecord and context state
	pContext->pAlarmRecord->acknowledged = FALSE;
	pContext->pAlarmRecord->numberOfEvents++;

	pContext->state = alarmmstr_ALARM_ROW_MODIFIED;

	status = m_pTableService->ModifyRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->pAlarmRecord->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		&pContext->pAlarmRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmAcknowledgeReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::ModifyAlarmRowUnAcknowledge  ********************************/

//  DdmAlarm::AlarmUnAcknowledgeReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    unacknowledgement process.  It is a state machine which guides the 
//    AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmUnAcknowledgeReplyHandler(
		void* _pContext, 
		STATUS status)
{
	TRACE_ENTRY(DdmAlarm::AlarmUnAcknowledgeReplyHandler(...));
	
	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS )
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_ALARM_ROWS_READ:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmUnAcknowledgeReplyHandler - ALARM_ROWS_READ\n")); 
		LogAlarmEvent(pContext, logaction_UNACKNOWLEDGED);
		break;
		}
	case alarmmstr_LOG_ROW_INSERTED:
		ModifyAlarmRowUnAcknowledge(pContext);
		break;
	case alarmmstr_ALARM_ROW_MODIFIED:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmUnAcknowledgeReplyHandler - ALARM_ROW_MODIFIED\n"));
		AMSTR_EVENT_INFO* pEvtUnAcknowledged = new(tZERO) AMSTR_EVENT_INFO;
		pEvtUnAcknowledged->alarmUnacknowledged.rid = pContext->rid;
		m_pCmdServer->csrvReportEvent(
			AMSTR_EVT_ALARM_UNACKNOWLEDGED,
			pEvtUnAcknowledged);
		delete pEvtUnAcknowledged;
		/*LogEvent(CTS_ALARM_UNACKNOWLEDGED, 
			pContext->rid.LoPart);*/
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			status,
			NULL,
			pContext->pCmdInfo);
		ClearAndDeleteContext(pContext);
		pContext = NULL;
		ProcessNextCommand();
		break;
		}
	default:
		assert(0);
		break;
	}

	return status;
}
/* end DdmAlarm::AlarmUnAcknowledgeReplyHandler  ********************************/

//  DdmAlarm::NotifyAlarm
//
//  Description:
//    General alarm notify routine which all requests from SSAPI  
//     are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm notify context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::NotifyAlarm(CONTEXT* pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::NotifyAlarm(...));

	// allocate and initialize an AlarmRecord 
	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord;

	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

	status = m_pTableService->ReadRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		(pTSCallback_t)&DdmAlarm::AlarmNotifyReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::AcknowledgeAlarm  ***************************************************/

//  DdmAlarm::ModifyAlarmRowNotify
//
//  Description:
//		Notify the alarm
//
//  Inputs:
//		CONTEXT* pContext - Alarm acknowledgement context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ModifyAlarmRowNotify(
										   CONTEXT* pContext)
{
	STATUS status;

	TRACE_ENTRY(DdmAlarm::ModifyAlarmRowNotify(...));

	// initialize AlarmRecord and context state
	pContext->pAlarmRecord->numberOfEvents++;

	pContext->state = alarmmstr_ALARM_ROW_MODIFIED;

	status = m_pTableService->ModifyRow(
		ALARM_RECORD_TABLE_NAME,
		&pContext->pAlarmRecord->rid,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord),
		&pContext->pAlarmRecord->rid,
		(pTSCallback_t)&DdmAlarm::AlarmNotifyReplyHandler,
		pContext);

	return status;
}
/* end DdmAlarm::ModifyAlarmRowNotify  ********************************/

//  DdmAlarm::AlarmNotifyReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    notify process.  It is a state machine which guides the 
//    AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmNotifyReplyHandler(
		void* _pContext, 
		STATUS status)
{
	TRACE_ENTRY(DdmAlarm::AlarmNotifyReplyHandler(...));
	
	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS )
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_ALARM_ROWS_READ:
		{
		TRACE_STRING(5, ("DdmAlarm::AlarmNotifyReplyHandler - ALARM_ROWS_READ\n")); 
		LogAlarmEvent(pContext, logaction_NOTIFIED);
		break;
		}
	case alarmmstr_LOG_ROW_INSERTED:
		ModifyAlarmRowNotify(pContext);
		break;
	case alarmmstr_ALARM_ROW_MODIFIED:
		{
		AMSTR_EVENT_INFO* pEvtNotified = new(tZERO) AMSTR_EVENT_INFO;
		pEvtNotified->alarmNotified.rid = pContext->rid;
		m_pCmdServer->csrvReportEvent(
			AMSTR_EVT_ALARM_NOTIFIED,
			pEvtNotified);
		delete pEvtNotified;		
		/*LogEvent(CTS_ALARM_NOTED, 
			pContext->rid.LoPart);*/
		m_pCmdServer->csrvReportCmdStatus(
			pContext->handle,
			status,
			NULL,
			pContext->pCmdInfo);
		ClearAndDeleteContext(pContext);
		pContext = NULL;
		ProcessNextCommand();
		break;
		}
	default:
		assert(0);
		break;
	}

	return status;
}
/* end DdmAlarm::AlarmNotifyReplyHandler  ********************************/

//  DdmAlarm::RecoverAlarms
//
//  Description:
//    General alarm recovery routine which all requests from SSAPI and 
//    DdmMasters are directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm recovery context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::RecoverAlarms(CONTEXT *pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::RecoverAlarms(...));

	U32 sizeOfKey;
	String64 nameOfKey;
	void* valueOfKey;
	// Allocate and initialize CONTEXT to be used in TSReadRow call to
	// determine if there are any alarms active for this device
	DidRecord* pDidRecord = new(tZERO) DidRecord;

	pContext->state = alarmmstr_DID_ROW_READ;
	pContext->pDidRecord = pDidRecord;

	SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);

	status = m_pTableService->ReadRowWithKey(
		ALARM_DID_TABLE_NAME, 
		nameOfKey,
		valueOfKey,
		sizeOfKey,
		pContext->pDidRecord,
		sizeof(DidRecord),
		(pTSCallback_t)&DdmAlarm::AlarmRecoverReplyHandler,
		pContext);
 
	// call back routine calls RecoverDidAlarms to read the alarms associated
	// with this device from the AlarmRecord table if any exist.
	// if none exist, then it simply replies to the original message

	return status;
}
/* end DdmAlarm::RecoverAlarms  ********************************/

//  DdmAlarm::RecoverDidAlarms
//
//  Description:
//    This function should only be called if there are alarms associated
//    with this device.  It then reads the alarms from the AlarmRecord
//    Table
//
//  Inputs:
//		CONTEXT* pContext - Alarm recovery context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::RecoverDidAlarms(CONTEXT* pContext)
{
	STATUS status = OK;
	U32 sizeOfKey;
	String64 nameOfKey;
	void* valueOfKey;

	TRACE_ENTRY(DdmAlarm::RecoverDidAlarms(...));

	// if we got this far, there should be alarms associated with this did
	assert(pContext->pDidRecord->numberOfAlarms);

	AlarmRecord* pAlarmRecord = new(tZERO) AlarmRecord[pContext->pDidRecord->numberOfAlarms];

	pContext->state = alarmmstr_ALARM_ROWS_READ;
	pContext->pAlarmRecord = pAlarmRecord;

		SetKeyValues(pContext, &sizeOfKey, nameOfKey, &valueOfKey);

	status = m_pTableService->ReadRowWithKey(
		ALARM_RECORD_TABLE_NAME, 
		nameOfKey,
		valueOfKey,
		sizeOfKey,
		pContext->pAlarmRecord,
		sizeof(AlarmRecord) * pContext->pDidRecord->numberOfAlarms,
		(pTSCallback_t)&DdmAlarm::AlarmRecoverReplyHandler,
		pContext); 

	// call back routine of this function calls ReplyWithRecoveredAlarms to
	// reply each of the alarms back to the calling message with the last
	// alarm marked last

	return status;
}
/* end DdmAlarm::RecoverDidAlarms  ********************************/

//  DdmAlarm::ReplyWithRecoveredAlarms
//
//  Description:
//    This function replies once for each alarm that is active for
//    this device.  It marks the last reply as last to let the 
//    requesting client know it is done
//
//  Inputs:
//		CONTEXT* pContext - Alarm recovery context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ReplyWithRecoveredAlarms(CONTEXT* pContext)
{
	U16 i, j=0, k=0;
	TRACE_ENTRY(DdmAlarm::ReplyWithRecoveredAlarms(...));
	
	MsgRecoverAlarms* msg = (MsgRecoverAlarms*) pContext->msg;
	for(i=0; i<pContext->pDidRecord->numberOfAlarms; i++)
	{
		if (pContext->pAlarmRecord[i].active) 
			j++;
	}
	if (j!=NULL)
	{
		/*if (IsVirtual(pContext))
			LogEvent(CTS_ALARMS_VIRTUAL_RECOVERED,
				pContext->vdn);
		else
			LogEvent(CTS_ALARMS_DEVICE_RECOVERED,
				pContext->did);*/
		for (i=0; i< pContext->pDidRecord->numberOfAlarms; i++)
		{
			// for each alarm in the array, call method SetAlarmContext() of class
			// MsgRecoverAlarms and then reply.  The last reply should have last
			// set to TRUE
			if (pContext->pAlarmRecord[i].active)
			{
				k++;
				msg->SetAlarmContext(pContext->pAlarmRecord[i].rid, 
					pContext->pAlarmRecord[i].cbContext, 
//					pContext->pAlarmRecord[i].pAlarmContext);
					&pContext->pAlarmRecord[i].alarmContext);
				Reply(msg, OK, (j==k));
			}
		}
	}
	else
		Reply(msg, OK, 1);
	ClearAndDeleteContext(pContext);
	pContext = NULL;
	ProcessNextCommand();
	return OK;
}
/* end DdmAlarm::ReplyWithRecoveredAlarms  ********************************/

//  DdmAlarm::AlarmRecoverReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    recovery process.  It is a state machine which guides the AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmRecoverReplyHandler(void* _pContext, STATUS status)
{
	
	TRACE_ENTRY(DdmAlarm::AlarmRecoverReplyHandler());

	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (!((status==OS_DETAIL_STATUS_SUCCESS) ||
		((status==ercKeyNotFound) && (pContext->state == alarmmstr_DID_ROW_READ))))
	{
		assert(0);
		return status;
	}

	switch (pContext->state) {
	case alarmmstr_DID_ROW_READ:
		TRACE_STRING(5, ("DdmAlarm::AlarmRecoverReplyHandler - DID_ROW_READ\n")); 
		if (status==ercKeyNotFound) {
			Reply(pContext->msg, OK, 1);
			ClearAndDeleteContext(pContext);
			pContext = NULL;
			ProcessNextCommand();
		}
		else
			status = RecoverDidAlarms(pContext);
		break;
	case alarmmstr_ALARM_ROWS_READ:
		TRACE_STRING(5, ("DdmAlarm::AlarmRecoverReplyHandler - ALARM_ROWS_READ\n")); 
		status = ReplyWithRecoveredAlarms(pContext);
		break;
	default:
		assert(0);
		break;
	}

	return status;
}
/* end DdmAlarm::AlarmRecoveryReplyHandler  ********************************/

//  DdmAlarm::QueryAlarms
//
//  Description:
//    General alarm query routine which all requests from SSAPI and 
//    is directed to 
//
//  Inputs:
//		CONTEXT* pContext - Alarm recovery context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::QueryAlarms(CONTEXT *pContext)
{
	STATUS status = OK;

	TRACE_ENTRY(DdmAlarm::QueryAlarms(...));

	pContext->state = alarmmstr_ALARM_ROWS_READ;

	m_pTableService->EnumTable(
		ALARM_RECORD_TABLE_NAME,
		(void**)&pContext->pAlarmRecord,
		&pContext->cbContext,
		&pContext->cRows,
		(pTSCallback_t)&DdmAlarm::AlarmQueryReplyHandler,
		pContext); 


	return status;
}
/* end DdmAlarm::QueryAlarms  ********************************/

//  DdmAlarm::InitReplyWithAlarms
//
//  Description:
//
//  Inputs:
//		CONTEXT* pContext - Alarm recovery context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::InitReplyWithAlarms(CONTEXT *pContext)
{
	STATUS status = OK;
	U16 i, j=0;
	BOOL allAlarms = TRUE;
	BOOL activeState;
	AlarmRecord* pAlarms = new AlarmRecord[pContext->cRows];

	TRACE_ENTRY(DdmAlarm::InitReplyWithAlarms(...));

	MsgQueryAlarms* msg = (MsgQueryAlarms*) pContext->msg;

	switch (msg->GetActionFlag())
	{
	case ALL_ALARMS:
		break;
	case ACTIVE_ALARMS:
		activeState = TRUE;
		allAlarms = FALSE;
		break;
	case INACTIVE_ALARMS:
		activeState = FALSE;
		allAlarms = FALSE;
		break;
	default:
		assert(0);
	}

	if (allAlarms == TRUE)
		msg->AddAlarms(pContext->cbContext, pContext->pAlarmRecord);
	else
	{
		for (i=0; i < pContext->cRows; i++)
		{
			if (pContext->pAlarmRecord[i].active == activeState)
				pAlarms[j++] = pContext->pAlarmRecord[i];
		}
		msg->AddAlarms(j * sizeof(AlarmRecord), pAlarms);
		delete pContext->pAlarmRecord;
		//pContext->pAlarmRecord = new AlarmRecord[j];
		pContext->pAlarmRecord = pAlarms;
		pContext->numberOfAlarms = j;
	}

	pContext->state = alarmmstr_LOG_ROWS_READ;

	m_pTableService->EnumTable(
		ALARM_LOG_TABLE_NAME,
		(void**)&pContext->pLogRecord,
		&pContext->cbContext,
		&pContext->cRows,
		(pTSCallback_t)&DdmAlarm::AlarmQueryReplyHandler,
		pContext); 

	return status;
}
/* end DdmAlarm::InitReplyWithAlarms  ********************************/

class Element
{ 
public:

	Element(rowID rid_, Element* pNext_) : rid(rid_), pNext(pNext_) {}

	void SetRowId(rowID rid_) { rid = rid_; }
	rowID GetRowId() { return rid; }

	void SetNext(Element* pNext_) { pNext = pNext_; }
	Element* GetNext() { return pNext; }

private:
	rowID rid;
	Element* pNext;
};

class LinkedList
{
public:

	LinkedList() : pHead(NULL) {}
	
	~LinkedList() 
	{
		Element* element, *next;
		for (element = pHead; element != NULL; element = next)
		{
			next = element->GetNext();
			delete element;
		}
	}

	void AddElement(rowID rid) 
	{
		Element* element = new Element(rid, pHead);
		pHead = element;
	}
	
	BOOL InList(rowID rid) 
	{
		Element* element;
		for (element = pHead; element!=NULL; element = element->GetNext())
		{
			if (element->GetRowId() == rid)
				return TRUE;
		}
		return FALSE;
	}

private:
	Element* pHead;
};

//  DdmAlarm::ReplyWithAlarmHistory
//
//  Description:
//
//  Inputs:
//		CONTEXT* pContext - Alarm recovery context
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::ReplyWithAlarmHistory(CONTEXT *pContext)
{
	STATUS status = OK;
	U16 i, j=0;
	AlarmLogRecord* pEntries = new AlarmLogRecord[pContext->cRows];

	TRACE_ENTRY(DdmAlarm::ReplyWithAlarmHistory(...));

	MsgQueryAlarms* msg = (MsgQueryAlarms*) pContext->msg;

	if (msg->GetActionFlag() == ALL_ALARMS)
		msg->AddAlarmHistory(pContext->cbContext, pContext->pLogRecord);
	else
	{
		LinkedList* list = new LinkedList();
		for (i = 0; i < pContext->numberOfAlarms; i++)
			list->AddElement(pContext->pAlarmRecord[i].rid);
		for (i = 0; i < pContext->cRows; i++)
		{
			if (list->InList(pContext->pLogRecord[i].alarmRid))
				pEntries[j++] = pContext->pLogRecord[i];
		}
		msg->AddAlarmHistory(j * sizeof(AlarmLogRecord), pEntries);
		delete list;
	}

	Reply(msg, OK);
	ClearAndDeleteContext(pContext);
	pContext = NULL;
	ProcessNextCommand();


	return status;
}
/* end DdmAlarm::ReplyWithAlarmHistory  ********************************/

//  DdmAlarm::AlarmQueryReplyHandler
//
//  Description:
//    This function is the call back routine used through out the alarm
//    query process.  It is a state machine which guides the AlarmManager.
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    STATUS - status of the calling function
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::AlarmQueryReplyHandler(void* _pContext, STATUS status)
{
	
	TRACE_ENTRY(DdmAlarm::AlarmQueryReplyHandler());

	CONTEXT *pContext = (CONTEXT*)_pContext;

	if (status!=OS_DETAIL_STATUS_SUCCESS) 
	{
		assert(0);
		return status;
	}

	switch (pContext->state) 
	{
	case alarmmstr_ALARM_ROWS_READ:
		InitReplyWithAlarms(pContext);
		break;
	case alarmmstr_LOG_ROWS_READ:
		ReplyWithAlarmHistory(pContext);
		break;
	default:
		assert(0);
		break;
	}

	return status;
}
/* end DdmAlarm::AlarmQueryReplyHandler  ********************************/

//  DdmAlarm::LogAlarmEvent
//
//  Description:
//		This function inserts a log entry into the AlarmLog table
//
//  Inputs:
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    U32 action - action to submit into log
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
STATUS DdmAlarm::LogAlarmEvent(CONTEXT* pContext, U32 action)
{
	STATUS status;
	pTSCallback_t cbFunction;
	
	// allocate a LogRecord (row) to insert and initialize
	AlarmLogRecord* pLogRecord = new AlarmLogRecord;
	pLogRecord->version = ALARM_LOG_TABLE_VERSION;
	pLogRecord->size = sizeof(AlarmLogRecord);
	pLogRecord->timeStamp = Time();
	pLogRecord->didPerformedBy = pContext->did;
	pLogRecord->vdnPerformedBy = pContext->vdn;
	pLogRecord->action = action;
	pLogRecord->alarmRid = pContext->pAlarmRecord->rid;

	// allocate and initialize a context used to pass information to the 
	// callback routine
	pContext->state = alarmmstr_LOG_ROW_INSERTED;
	pContext->pLogRecord = pLogRecord;

	switch (action)
	{
	case logaction_SUBMITTED:
		{
			// GAI: a hacky approach until we have a server-side localizer
			UnicodeString	us( StringClass("System") );
			us.CString( pLogRecord->userName, us.GetSize() );
			memset( pLogRecord->notes, 0, sizeof(pLogRecord->notes) );
			cbFunction = (pTSCallback_t)&DdmAlarm::AlarmSubmitReplyHandler;
		}
		break;
	case logaction_REMITTED:
		{
			MsgRemitAlarm* msg = (MsgRemitAlarm*) pContext->msg;
			if (msg)
			{
				msg->GetUserName(&pLogRecord->userName);
				memset( pLogRecord->notes, 0, sizeof(pLogRecord->notes) );
			}
			else
			{
				memcpy(pLogRecord->userName, pContext->pCmdInfo->cmdParams.userName,
				sizeof(pLogRecord->userName));
				memcpy(pLogRecord->notes, pContext->pCmdInfo->cmdParams.notes, sizeof(pLogRecord->notes));
			}
			cbFunction = (pTSCallback_t)&DdmAlarm::AlarmRemitReplyHandler;
		}
		break;
	case logaction_ACKNOWLEDGED:
		memcpy(pLogRecord->notes, pContext->pCmdInfo->cmdParams.notes, sizeof(pLogRecord->notes));
		memcpy(pLogRecord->userName, pContext->pCmdInfo->cmdParams.userName, 
			sizeof(pLogRecord->userName));
		cbFunction = (pTSCallback_t)&DdmAlarm::AlarmAcknowledgeReplyHandler;
		break;
	case logaction_UNACKNOWLEDGED:
		memcpy(pLogRecord->notes, pContext->pCmdInfo->cmdParams.notes, sizeof(pLogRecord->notes));
		memcpy(pLogRecord->userName, pContext->pCmdInfo->cmdParams.userName, 
			sizeof(pLogRecord->userName));
		cbFunction = (pTSCallback_t)&DdmAlarm::AlarmUnAcknowledgeReplyHandler;
		break;
	case logaction_NOTIFIED:
		memcpy(pLogRecord->notes, pContext->pCmdInfo->cmdParams.notes, sizeof(pLogRecord->notes));
		memcpy(pLogRecord->userName, pContext->pCmdInfo->cmdParams.userName, 
			sizeof(pLogRecord->userName));
		cbFunction = (pTSCallback_t)&DdmAlarm::AlarmNotifyReplyHandler;
		break;
	default:
		assert(0);
		break;
	}
	
	status = m_pTableService->InsertRow(
		ALARM_LOG_TABLE_NAME,
		pContext->pLogRecord,
		sizeof(AlarmLogRecord),
		&pContext->pLogRecord->rid,
		cbFunction,
		pContext);

	return status;
}
/* end DdmAlarm::LogAlarmEvent  ********************************/

//  DdmAlarm::SubmitRequest
//
//  Description:
//		This function either enqueues a new function call
//		entry or makes the call if the queue is presently
//		empty and the AlarmMaster is not processing a command
//  Inputs:
//	  U32 functionToCall - enumerated value of function to call
//    void* pContext - information passed from the calling DdmAlarm
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmAlarm::SubmitRequest(U32 functionToCall, CONTEXT* pContext)
{
	if (m_pAlarmQueue->Empty() && NotProcessingCommand())
		ExecuteFunction(functionToCall, pContext);
	else
		m_pAlarmQueue->AddFunctionCall(functionToCall, ((void*)pContext));
}
/* end DdmAlarm::SubmitRequest  ********************************/

//  DdmAlarm::ExecuteFunction
//
//  Description:
//		Sets the ProcessingCommand flag in the Alarm Master and
//		calls the appropriate function
//
//  Inputs:
//    U32 functionToCall - enumerated value which specifies 
//		function to call
//    void* _pContext - information passed from the calling DdmAlarm
//      method
//    
//  Outputs:
//    STATUS - returns OK if everything is ok.
//
void DdmAlarm::ExecuteFunction(U32 functionToCall, CONTEXT* pContext)
{
	ProcessCommand();
	switch (functionToCall)
	{
	case function_SubmitAlarm:
		SubmitAlarm(pContext);
		break;
	case function_RemitAlarm:
		RemitAlarm(pContext);
		break;
	case function_RecoverAlarms:
		RecoverAlarms(pContext);
		break;
	case function_AcknowledgeAlarm:
		AcknowledgeAlarm(pContext);
		break;
	case function_UnAcknowledgeAlarm:
		UnAcknowledgeAlarm(pContext);
		break;
	case function_NotifyAlarm:
		NotifyAlarm(pContext);
		break;
	case function_QueryAlarms:
		QueryAlarms(pContext);
		break;
	case function_KillAlarm:
		KillAlarm(pContext);
		break;
	default:
		assert(0);
		break;
	}
}
/* end DdmAlarm::ExecuteFunction  ********************************/

//  DdmAlarm::ProcessNextCommand
//
//  Description:
//		Checks the queue for next function call and executes
//		or sets the Alarm Master to a non-busy state if there
//		are no more commands to process
//
void DdmAlarm::ProcessNextCommand()
{
	FunctionCallElement* pElement;

	// If Quiesce request, then dont send any further cmds,
	// wait for outstanding cmds to finish...
	if (m_isQuiesced == TRUE)
	{
		// Reply to Quiese, that we are done quiescing..
		Reply(m_pQuiesceMessage);
	} 
	else 
	{
		pElement = m_pAlarmQueue->PopFunctionCall();

		if (pElement)
		{
			ExecuteFunction(pElement->GetFunction(), (CONTEXT*)pElement->GetContext());
			delete pElement;
		}
		else
			FinishCommand();
	}
}
/* end DdmAlarm::ProcessNextCommand  ********************************/

//  DdmAlarm::ClearAndDeleteContext
//
//  Description:
//		Clears CONTEXT and deletes the memory associated with it
//
void DdmAlarm::ClearAndDeleteContext(CONTEXT* pContext)
{
	delete pContext->pAlarmContext;
	pContext->pAlarmContext = NULL;
	delete pContext->pDidRecord;
	pContext->pDidRecord = NULL;
	delete pContext->pAlarmRecord;
	pContext->pAlarmRecord = NULL;
	delete pContext->pLogRecord;
	pContext->pLogRecord = NULL;
	delete pContext;
}
/* end DdmAlarm::ProcessNextCommand  ********************************/
