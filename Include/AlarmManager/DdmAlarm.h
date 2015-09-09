/* DdmAlarm.h
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
// $Log: /Gemini/Include/AlarmManager/DdmAlarm.h $
// 
// 19    10/12/99 6:00p Joehler
// Added event logging and Quiesce method
// 
// 18    9/29/99 6:34p Joehler
// Added coalesce functionality to alarm master
// 
// 17    9/13/99 3:28p Joehler
// Added error code for attempting to user remit an unclearable alarm
// 
// 16    9/12/99 10:25p Joehler
// Generalized AlarmTableServices and AlarmQueue to TableServicesUtil and
// MessageProcessingQueue
// 
// 15    9/12/99 5:33p Agusev
// Fixed a "poking at other's internals" bug
// 
// 14    9/02/99 11:45a Joehler
// added comments
//			



#ifndef __DdmAlarm_H
#define __DdmAlarm_H

#include "Ddm.h"
#include "CTtypes.h"
#include "CmdServer.h"
#include "CmdSender.h"

#include "AlarmMasterMessages.h"
#include "AlarmDidRecordTable.h"
#include "AlarmLogTable.h"
#include "AlarmRecordTable.h"
#include "AlarmEvents.h"
#include "AlarmCmdQueue.h"
#include "CommandProcessingQueue.h"
#include "TableServicesUtil.h"
#include "DdmMaster.h"

class DdmAlarm: public Ddm 
{
public:
	DdmAlarm(DID did) : Ddm(did) {}
	static Ddm *Ctor(DID did);

	STATUS Initialize (Message *pMsg);

	STATUS Quiesce(Message *pMsg);

	STATUS Enable(Message *pMsg);
	
	STATUS ProcessSubmit(Message *pMsg);
	STATUS ProcessRemit(Message *pMsg);
	STATUS ProcessRecover(Message *pMsg);
	STATUS ProcessQuery(Message *pMsg);

	enum {
		logaction_SUBMITTED = 1,
		logaction_ACKNOWLEDGED,
		logaction_UNACKNOWLEDGED,
		logaction_REMITTED,
		logaction_NOTIFIED
	};

	enum
	{
		AMSTR_ERR_NOT_USER_REMITTABLE = 200
	};

private:


	// enum for state of CONTEXT
	enum {
		alarmmstr_RECORD_TABLE_DEFINED = 100,
		alarmmstr_DID_TABLE_DEFINED,
		alarmmstr_LOG_TABLE_DEFINED,
		alarmmstr_CMD_SERVER_INITIALIZED,
		alarmmstr_DID_ROW_READ,
		alarmmstr_DID_ROW_MODIFIED,
		alarmmstr_DID_ROW_INSERTED,
		alarmmstr_DID_ROW_DELETED,
		alarmmstr_ALARM_ROWS_READ,
		alarmmstr_ALARM_ROW_INSERTED,
		alarmmstr_ALARM_ROW_MODIFIED,
		alarmmstr_ALARM_ROW_DELETED,
		alarmmstr_LOG_ROWS_READ,
		alarmmstr_LOG_ROW_INSERTED
	};

	typedef struct _CONTEXT {
		U32 state; // state as defined by the above enum
		Message* msg; // message associated with this request
		HANDLE handle; // handle associated with cmd from SSAPI
		AMSTR_CMND_INFO* pCmdInfo; // command info submitted from SSAPI
		DID did; // did of the device associated with this message
		VDN vdn; // vdn of the device associated with this message (if virtual)
		rowID rid; // rowId associated with this AlarmRecord or DidRecord
		U32 cRows; // count of rows read or modified
		U32 cbContext; // size of AlarmContext associated with this alarm
		U32 numberOfAlarms;
		void* pAlarmContext; // pointer to AlarmContext associated with this alarm
		DidRecord* pDidRecord; // DidRecord row associated with this device
		AlarmRecord* pAlarmRecord; // AlarmRecord rows associated with this device.
		AlarmLogRecord* pLogRecord; // LogRecord inserted
		void* pData; // used to initialize AlarmManager tables and hold
					 // event data during an alarm submission
		U32	eventDataSize;	// GAI+:used to store the size of the serialzed 
							// 		event object (to avoid poking at its internals)	
	} CONTEXT;

	BOOL IsVirtual(CONTEXT* pContext) { return (pContext->vdn!=NULL); }

	void ClearAndDeleteContext(CONTEXT* pContext);

	void SetKeyValues(CONTEXT* pContext,
		U32* sizeOfKey,
		char* nameOfKey,
		void** valueOfKey)
	{
		if (IsVirtual(pContext))
		{
			*sizeOfKey = sizeof(VDN);
			strcpy(nameOfKey,fdVdn);
			*valueOfKey = &pContext->vdn;
		}
		else
		{
			*sizeOfKey = sizeof(DID);
			strcpy(nameOfKey,fdDid);
			*valueOfKey = &pContext->did;
		}
	}


	CmdServer* m_pCmdServer;

	enum
	{
		function_SubmitAlarm = 1,
		function_RemitAlarm,
		function_RecoverAlarms,
		function_AcknowledgeAlarm,
		function_UnAcknowledgeAlarm,
		function_NotifyAlarm,
		function_QueryAlarms,
		function_KillAlarm
	};

	CommandProcessingQueue* m_pAlarmQueue;
	BOOL processingCommand;

	BOOL NotProcessingCommand() { return (processingCommand == FALSE); }
	void ProcessCommand() { processingCommand = TRUE; }
	void FinishCommand() { processingCommand = FALSE; }

	TableServicesUtil *m_pTableService;

	BOOL m_Initialized;
	BOOL m_isQuiesced;

	Message* m_pQuiesceMessage;

private:
	// generic action methods
	STATUS SubmitAlarm(
		CONTEXT* pContext);

	STATUS RemitAlarm(
		CONTEXT* pContext);

	STATUS KillAlarm(
		CONTEXT* pContext);

	STATUS AcknowledgeAlarm(
		CONTEXT* pContext);

	STATUS UnAcknowledgeAlarm(
		CONTEXT* pContext);

	STATUS NotifyAlarm(
		CONTEXT* pContext);

	STATUS RecoverAlarms(
		CONTEXT* pContext);

	STATUS QueryAlarms(
		CONTEXT* pContext);

	// Command Queue listener
	void amstrListenerForCommands(HANDLE handle, 
		void* pCmdData);

	// initialization methods
	STATUS DefineAlarmRecordTable(
		Message* msg);

	STATUS DefineAlarmDidTable();

	STATUS DefineAlarmLogTable();

	STATUS InitializeCmdServer();
	
	void amstrObjectInitializeReply(
		STATUS status);

	STATUS AlarmInitReplyHandler(
		void* _pContext, 
		STATUS status);
	
	// submit methods
	STATUS InsertAlarmRecord(
		CONTEXT* pContext);
	
	STATUS InsertDidRecord(
		CONTEXT* pContext);
	
	STATUS ReadDidAlarms(
		CONTEXT* pContext);
	
	BOOL CompareAlarms(
		Event* pEvent, 
		Event* pSubmittedEvent);

	STATUS CheckAlarmMatch(
		CONTEXT* pContext);
		
	STATUS ModifyIncrementDidRecord(
		CONTEXT* pContext);
	
	STATUS AlarmSubmitReplyHandler(
		void* _pContext, 
		STATUS status);
	
	// remit methods
	STATUS ModifyAlarmRowStatus(
		CONTEXT* pContext);

	STATUS AlarmRemitReplyHandler(
		void* _pContext, 
		STATUS status);
	
	// kill methods
	STATUS DeleteAlarmRecord(
		CONTEXT* pContext);

	STATUS ReadDidRecord(
		CONTEXT* pContext);

	STATUS ModifyDecrementDidRecord(
		CONTEXT* pContext);

	STATUS AlarmKillReplyHandler(
		void* _pContext, 
		STATUS status);
	
	// acknowledge methods
	STATUS ModifyAlarmRowAcknowledge(
		CONTEXT* pContext);

	STATUS AlarmAcknowledgeReplyHandler(
		void* _pContext, 
		STATUS status);

	// unacknowledge methods
	STATUS ModifyAlarmRowUnAcknowledge(
		CONTEXT* pContext);

	STATUS AlarmUnAcknowledgeReplyHandler(
		void* _pContext, 
		STATUS status);

	// notify methods
	STATUS ModifyAlarmRowNotify(
		CONTEXT* pContext);
	
	STATUS AlarmNotifyReplyHandler(
		void* _pContext, 
		STATUS status);

	// recover methods
	STATUS RecoverDidAlarms(
		CONTEXT* pContext);
	
	STATUS ReplyWithRecoveredAlarms(
		CONTEXT* pContext);
	
	STATUS AlarmRecoverReplyHandler(
		void* _pContext, 
		STATUS status);
	
	//query methods
	STATUS InitReplyWithAlarms(
		CONTEXT* pContext);

	STATUS ReplyWithAlarmHistory(
		CONTEXT* pContext);

	STATUS AlarmQueryReplyHandler(
		void* _pContext, 
		STATUS status);

	void SubmitRequest(U32 functionToCall, CONTEXT* pContext);
	void ExecuteFunction(U32 functionToCall, CONTEXT* pContext);
	void ProcessNextCommand();

	// interface to alarm log
	STATUS LogAlarmEvent(
		CONTEXT* pContext,
		U32 action);
};

#endif	// __DdmAlarm_H
