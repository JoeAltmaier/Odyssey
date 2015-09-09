/* DdmSSAPIDriver.h 
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
// $Log: /Gemini/Odyssey/AlarmManager/DdmSSAPIDriver.h $
// 
// 10    9/13/99 3:40p Joehler
// Added testing functionality for user remitted alarms
// 
// 9     9/07/99 2:11p Joehler
// Changes to correctly manage memory associated with AlarmContext
// 
// 8     9/02/99 11:46a Joehler
// added comments
//


#ifndef __DdmSsapiDriver_H
#define __DdmSsapiDriver_H

//#include "OsTypes.h"
//#include "Message.h"
//#include "DdmMaster.h"


#include "DdmMaster.h"
#include "CmdSender.h"
#include "CmdServer.h"
//#include "ObjectManager.h"
//#include "ArgumentSet.h"
//#include "ObjectTableElement.h"

struct SSAPIAlarmContext {
	int dummy;
};


class DdmSSAPIDriver : public DdmMaster {
public:
	DdmSSAPIDriver(DID did);
	static Ddm * Ctor(DID did);
	STATUS Enable(Message* pMsg);
	STATUS Initialize(Message* pMsg);

protected:
	STATUS 	AcknowledgeAlarm(void *pAlarmContext_);
	STATUS 	UnAcknowledgeAlarm(void *pAlarmContext_);
	STATUS 	NotifyAlarm(void *pAlarmContext_);
	STATUS RemitAlarmFromUser(rowID rid, UnicodeString userName);

private:

	enum {
		ssapidriver_WAITING_FOR_INIT = 100,
		ssapidriver_INITIALIZED
	};

	CmdSender* m_pCmdSender;

	void cmdsenderInitializedReply(STATUS status);
	void InitReplyHandler(U32 state, Message* msg);

	void cmdsenderEventHandler(STATUS eventCode, void* pStatusData);
	void amstrCommandCompletionReply(STATUS, void*, void*, void*);

	void MessageControl();

	// called by MessageControl function
	void SubmitFirstAlarm();
	void AcknowledgeFirstAlarm();
	void UnAcknowledgeFirstAlarm();
	void NotifyFirstAlarm();
	void SubmitElevenAlarms();
	void SubmitSecondAlarm();
	void RemitFirstAlarm();
	void RecoverAllAlarms() ;
	void DeletePAl();
	void RemitAllAlarms();
	void QueryAllAlarms();
	void QueryActiveAlarms();
	void QueryInactiveAlarms();
	
	// call back functions which call MessageControl
	void cbSubmitAlarm(void * pAlarmContext_, STATUS status_  );
	void cbRemitAlarm(void * pAlarmContext_ , STATUS status_ );
	void cbRecoveryComplete(STATUS status_  );
	void 	cbQueryAlarms(
		U16 numberOfAlarms,
		AlarmRecord* pAlarmRecords,
		U16 numberOfAlarmLogEntries,
		AlarmLogRecord* pAlarmLogEntries,
		STATUS status_);


};


#endif	

