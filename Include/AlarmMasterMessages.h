/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
*	This file contains the definition of the Alarm Master messages.
* 
* Update Log: 
* 07/02/99 Bob Butler: Create file
*
* 07/30/99 Jaymie Oehler: 
*			Added TRACE_ALARM
*			Fixed initialization of message type in MsgRemitAlarm()
*			Added GetVdn() to class MsgRecoverAlarms
* 08/08/99 Bob Butler: Updates to work with Event.h changes, fixed SGL cleanup.
* 08/11/99 Jaymie Oehler: Added DID to classes MsgSumbitAlarm and
*           MsgRecoverAlarms
* 08/25/99 Jaymie Oehler: added userRemittable flag to MsgSubmitAlarm and
*			fromUser flag to MsgRemitAlarm
* 08/30/99 Jaymie Oehler: added MsgQueryAlarms class to support
*			Query functionality for the SSAPI
* 09/01/99 Jaymie Oehler: userName in MsgRemitAlarm should be a
*			UnicodeString16
* 09/07/99 Jaymie Oehler: modified constructor for MsgSubmitAlarm
*			to copy alarm context and then call AddSgl()
*************************************************************************/

#ifndef AlarmMasterMessages_h
#define AlarmMasterMessages_h

#include "Trace_Index.h"		// include the trace module numbers

#ifdef TRACE_INDEX	// GAI
#undef TRACE_INDEX
#endif	// TRACE_INDEX

#define	TRACE_INDEX		TRACE_ALARM	// set this modules index to your index	
#include "Odyssey_Trace.h"			// include trace macros

#include "CTTypes.h"
#include "message.h"
#include "AlarmRecordTable.h"
#include "AlarmLogTable.h"
#include "UnicodeString.h"


class MsgSubmitAlarm : public Message
{
public:
	// SGL indexes for the data being sent
	enum {CONTEXT_SGL, EVTDATA_SGL};
	
	// Ctor: add the SGLs for the context data, and the event data and parms	
	MsgSubmitAlarm(VDN vdn_, DID  did_, U16 cbContext_, void *pAlarmContext_, 
		const Event *pEvt_, BOOL userRemittable_)
	 : Message(REQ_ALARM_SUBMIT), vdn(vdn_), did(did_), cbContext(cbContext_),
	 userRemittable(userRemittable_)
	{
		void *pData;
		// need to make a copy of the alarm context to add as a sgl
		// because this memory will be deleted when the the message
		// is destructed by CleanAllSgl()
		void* pAlarmContext = new char[cbContext_];
		memcpy(pAlarmContext, pAlarmContext_, cbContext_);
		cbEventData = pEvt_->GetEventData(&pData);
		AddSgl(CONTEXT_SGL, pAlarmContext, cbContext_);
		AddSgl(EVTDATA_SGL, pData, cbEventData); 
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgSubmitAlarm() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
	U16 GetAlarmContextSize() { return cbContext; }

	// allocate storage for the alarm context, and copy the data.  The
	// caller supplies the pointer and is responsible for deleting it.
	void GetAlarmContext(void **ppAlarmContext_) 
	{
		*ppAlarmContext_ = new char[cbContext];
		CopyFromSgl(CONTEXT_SGL, 0, *ppAlarmContext_, cbContext);
	}
	
	// Create the Event object with the event and parm data from the
	// SGLs.  The caller supplies the pointer and is responsible for 
	// deleting it.
	void GetEvent(Event **ppEvt_)
	{
		void *pData = new char[cbEventData];
		
		CopyFromSgl(EVTDATA_SGL, 0, pData, cbEventData);
		*ppEvt_ = new Event(pData);
		delete pData;
	}
	
	// allocate storage for the event  blob, and copy the data.  
	// The caller supplies the pointer and is responsible for deleting it.
	void GetEventData(void **ppEvtData_)
	{
		*ppEvtData_ = new char[cbEventData];
		CopyFromSgl(EVTDATA_SGL, 0, *ppEvtData_, cbEventData);
	}
	
	U32 GetEventDataSize() const { return cbEventData; }
	
	VDN GetVdn() { return vdn; }
	DID GetDid() { return did; }

	BOOL IsClearable() { return userRemittable; }

	rowID GetRowId() { return rowId; }
	
	// Set the Row ID prior to replying
	void SetRowId(rowID rowId_)
	{
		rowId = rowId_;
	}
		
private:
	// send data:
	U16 cbContext;
	U32 cbEventData;
	VDN vdn;
	DID did;
	BOOL userRemittable;
	// reply data:
	rowID rowId;
		
};

class MsgRemitAlarm : public Message
{
public:
	MsgRemitAlarm(rowID rowId_, VDN vdn_, DID did_, UnicodeString32 userName_)
		: Message(REQ_ALARM_REMIT), rowId(rowId_), vdn(vdn_), did(did_)
	{
		memcpy(userName, userName_, 32);
	}
	
	rowID GetRowId() { return rowId; }

	DID GetDid() { return did; }
	VDN GetVdn() { return vdn; }

	void GetUserName(UnicodeString32* user) 
	{ 
		memcpy(*user, userName, sizeof(userName));
	}

private:
	rowID rowId;
	VDN vdn;
	DID did;
	UnicodeString32 userName;
};

class MsgRecoverAlarms : public Message
{
public:
	enum {CONTEXT_SGL};
	MsgRecoverAlarms(VDN vdn_, DID did_) : Message(REQ_ALARM_RECOVER), vdn(vdn_), did(did_)
	{
		// zero the context so that if no alarms need to be recovered,
		// DdmMaster::ProcessRecoverReply will work correctly
		cbContext = 0;
		// allocate the SGL for reply
		AddSgl(CONTEXT_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgRecoverAlarms() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs

	// Used by the AlarmMaster to set the persistant context and rowId 
	// for each outstanding alarm submitted by the VDN.  Reply last with 
	// the final alarm.  If there are no outstanding alarms, reply 
	// last without calling this
	void SetAlarmContext(rowID rowId_, U16 cbContext_, const void *pAlarmContext_)
	{
		cbContext = cbContext_;
		rowId = rowId_;
		AllocateSgl(CONTEXT_SGL, cbContext);
		// need to cast away const even though it really is 
		CopyToSgl(CONTEXT_SGL, 0, (void *)pAlarmContext_, cbContext);
	}		
			
	U16 GetAlarmContextSize() { return cbContext; }

	// Allocate storage for the alarm context, and copy the data.  The
	// caller supplies the pointer and is responsible for deleting it.
	void GetAlarmContext(void **ppAlarmContext_) 
	{
		*ppAlarmContext_ = new char[cbContext];
		CopyFromSgl(CONTEXT_SGL, 0, *ppAlarmContext_, cbContext);
	}
	
	rowID GetRowId() { return rowId; }

	VDN GetVdn() { return vdn; }
	DID GetDid() { return did; }
	
private:
	// send data:
	VDN vdn;
	DID did;
	// reply data:
	U16 cbContext;
	rowID rowId;
};

enum { ALL_ALARMS, ACTIVE_ALARMS, INACTIVE_ALARMS };

class MsgQueryAlarms : public Message
{
public:
	// SGL indexes for the data being sent
	enum {ALARM_SGL, ALARM_HISTORY_SGL};
	
	// Ctor
	MsgQueryAlarms(U16 action)
		: Message(REQ_ALARM_QUERY), actionFlag(action) 
	{
		// allocate the SGLS for reply
		AddSgl(ALARM_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
		AddSgl(ALARM_HISTORY_SGL, NULL, 0, SGL_DYNAMIC_REPLY);
	}

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgQueryAlarms() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs
	
	void AddAlarms(U16 cbAlarms_, void* pAlarms)
	{
		cbAlarms = cbAlarms_;
		AllocateSgl(ALARM_SGL, cbAlarms);
		CopyToSgl(ALARM_SGL, 0, pAlarms, cbAlarms);
	}

	void GetAlarms(void** ppAlarms)
	{
		*ppAlarms = new char[cbAlarms];
		CopyFromSgl(ALARM_SGL, 0, *ppAlarms, cbAlarms);
	}

	U16 GetNumberOfAlarms()
	{
		return cbAlarms / sizeof(AlarmRecord);
	}

	void AddAlarmHistory(U16 cbAlarmHistory_, void* pAlarmHistory) 
	{
		cbAlarmHistory = cbAlarmHistory_;
		AllocateSgl(ALARM_HISTORY_SGL, cbAlarmHistory);
		CopyToSgl(ALARM_HISTORY_SGL, 0, pAlarmHistory, cbAlarmHistory);
	}

	void GetAlarmHistory(void** ppAlarmHistory)
	{
		*ppAlarmHistory = new char[cbAlarmHistory];
		CopyFromSgl(ALARM_HISTORY_SGL, 0, *ppAlarmHistory, cbAlarmHistory);
	}

	U16 GetNumberOfAlarmLogEntries() 
	{
		return cbAlarmHistory / sizeof(AlarmLogRecord);
	}

	U16 GetActionFlag() { return actionFlag; }
		
private:
	// send data:
	U16 actionFlag;
	// reply data:
	U16 cbAlarms;
	U16 cbAlarmHistory;
};

#endif