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
* File: DdmMaster.h
*
* Description:
*	This file contains the class declaration of DdmMaster.  This class should
*   be used as the base class for all System Master DDMs.
* 
* Update Log: 
* 07/01/99 Bob Butler: Create file
* 08/16/99 Jaymie Oehler: moved AlarmList from DdmMaster.cpp to here
* 08/30/99 Jaymie Oehler: added QueryAlarms() functionality for SSAPI
* 08/30/99 Jaymie Oehler: moved QueryAlarms enum { ALL_ALARMS,
*			ACTIVE_ALARMS, INACTIVE_ALARMS } to AlarmMasterMessages.h
*
*************************************************************************/

// $Log: /Gemini/Include/Oos/DdmMaster.h $
// 
// 11    9/12/99 10:30p Joehler
// Made default userName for alarm remission "System"
// 
// 10    9/02/99 11:46a Joehler
// added comments

#ifndef DdmMaster_h
#define DdmMaster_h

#include "ddm.h"
#include "Rows.h"
#include "AlarmMasterMessages.h"
#include "UnicodeString.h"

class AlarmList 
{
public:
	enum State { NONE, SUBMITTING, REMIT_PENDING, SUBMITTED, REMITTING, REMITTED, RECOVERED };
	
	AlarmList(U16 cbAlarmContext_, void *pAlarmContext_, AlarmList *pNext_ = NULL)
	: cbAlarmContext(cbAlarmContext_), pAlarmContext(pAlarmContext_), pNext(pNext_), state(NONE)
		{
		memset(&rowId, 0, sizeof(rowID));
		}

	State GetState() { return state; }
	void SetState(State state_) { state = state_; }	

	U16 	GetAlarmContextSize() { return cbAlarmContext; }
	void 	*GetAlarmContext() { return pAlarmContext; }
	
	AlarmList *GetNext() { return pNext; }
	void SetNext(AlarmList *pNext_) { pNext = pNext_; }
	
	void SetRowId(rowID rowId_) { rowId = rowId_; }
	rowID GetRowId() { return rowId; }
	
	// Find an AlarmList starting with this and continuing to 
	// the end of the list, given a pointer to an alarm context
	AlarmList *Find(void *pAlarmContext_)
	{

		AlarmList *pALWork = this;
		while (pALWork && pALWork->GetAlarmContext() != pAlarmContext_)
			pALWork = pALWork->GetNext();
		return pALWork;
	}

	// Find and unlink  an AlarmList starting with this
	// and continuing to the end of the list, given a pointer to an
	// alarm context
	AlarmList *FindAndUnlink(void *pAlarmContext_)
	{
		// find this alarm in our list
		AlarmList *pALWork = this;
		if (pALWork->GetAlarmContext() == pAlarmContext_) // head condition
			return this;  // caller has to move the head pointer
		else // find it
			while (pALWork->GetNext() && pALWork->GetNext()->GetAlarmContext() != pAlarmContext_)
				pALWork = pALWork->GetNext();
		AlarmList *pAL = pALWork->GetNext();
		if (pAL)
			pALWork->SetNext(pAL->GetNext());
		
		return pAL;
	}

	void SetUserName(UnicodeString user) { userName = user; }

	UnicodeString GetUserName() { return userName; } 
	
private:
	AlarmList();
	State state;
	U16 cbAlarmContext;
	void *pAlarmContext;	
	rowID rowId;
	AlarmList *pNext;
	// temporary used to pass userName from a REMIT_PENDING alarm 
	// to a REMITTING alarm
	UnicodeString userName;
};

class DdmMaster : public Ddm
{
public:
	DdmMaster(DID did_) : Ddm(did_), pALhead(NULL) {}

protected:
	// submit an alarm:  The event describes the alarm, and the alarm context is 
	// persistant data about the alarm.  If failover occurs, the alarm context
	// must contain enough information for the Master to recover the alarm state.
	STATUS 	SubmitAlarm(const Event *pEvt_, U16 cbContext_, void *pAlarmContext_,
		BOOL userRemittable = FALSE);
	
	// remit an alarm:  The alarm context must be the same pointer as was originally
	// passed to SubmitAlarm.  For recovered alarms, the pointer must be the same
	// pointer that was passed back to cbRecoverAlarm(
	STATUS 	RemitAlarm(void *pAlarmContext_, UnicodeString userName = 
		UnicodeString(StringClass("System")));
	
	// remit all alarms.
	STATUS 	RemitAll();
	
	// recover any alarms that were submitted prior to failover
	STATUS 	RecoverAlarms();

	// Query Alarms - to be used ONLY by the SSAPI
	STATUS QueryAlarms(U16 action);

	// Below are callbacks for the above functions.  Derived classes should override
	// them if they need more than the default behaviour
	
	// Callback after an alarm has been submitted and the AlarmMaster has
	// replied.  By default, the function does nothing.  Passes back the same pointer
	// that was originally passed to SubmitAlarm()
	virtual void 	cbSubmitAlarm(void *pAlarmContext_, STATUS status);
	
	// Callback after an alarm has been remitted and the AlarmMaster has replied.
	// by default, the alarm context is removed from the list of open alarms 
	// and the context is deleted.  Returns the same pointer that was passed to 
	// the orginal SubmitAlarm() unless this is a recovered alarm, in which case it 
	// passes the same pointer that was passed to cbRecoverAlarm() when the alarm
	// was recovered
	virtual void 	cbRemitAlarm(void *pAlarmContext_, STATUS status);
	
	// This callback is called once per recovered alarm.  By default, the 
	// function does nothing.  The pointer is created in the reply processor.
	virtual void 	cbRecoverAlarm(void *pAlarmContext_, STATUS status);

	// This callback is called when all alarms have been recovered and the AlarmMaster 
	// has sent the last reply to a MsgRecoverAlarms message.  By default, all recovered
	// alarms are remitted.
	virtual void 	cbRecoveryComplete(STATUS status_);

	// Callback after all alarms and alarm history have been replied
	virtual void 	cbQueryAlarms(
		U16 numberOfAlarms,
		AlarmRecord* pAlarmRecords,
		U16 numberOfAlarmLogEntries,
		AlarmLogRecord* pAlarmLogEntries,
		STATUS status_);

private:
	// Handle the replies from the alarm master and manage the list of outstanding
	// alarms
	STATUS ProcessRecoverReply(Message *pMsg);
	STATUS ProcessSubmitReply(Message *pMsg_);
	STATUS ProcessRemitReply(Message *pMsg_);
	STATUS ProcessQueryReply(Message *pMsg_);

	// jlo temp to test - need to access pALhead to delete it
protected:
	class AlarmList *pALhead;

}; 


#endif