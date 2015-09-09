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
*	This file contains the source for DdmEvtLogIOP.
* 
* Update Log: 
* 03/10/99 Bob Butler: Create file
*
*************************************************************************/
#include "Ddm.h"
#include "DdmEvtLogIOP.h"
#include "LogMasterMessages.h"
#include "BuildSys.h"
#include "assert.h"

#define _TRACEF
#define	TRACE_INDEX		TRACE_SYSTEMLOG

#include "Odyssey_Trace.h"

const SIGNALCODE scLOGEVENT = 0;

DdmEventLogIOP *DdmEventLogIOP::log = NULL;

CLASSNAME(DdmEventLogIOP, SINGLE);	// Class Link Name used by Buildsys.cpp


// global function that wraps the static call to DdmEventLogIOP::LogEvent()
void LogEvent (Event *pEvt_)
{
	DdmEventLogIOP::LogEvent(pEvt_);
}


Ddm *DdmEventLogIOP::Ctor(DID did_)
{
	assert (log == NULL);
	log = new DdmEventLogIOP(did_);
	return log;
}

void DdmEventLogIOP::LogEvent(Event *pEvt_)
{ 
	assert (log != NULL);
	log->Signal(scLOGEVENT, pEvt_);
}

STATUS DdmEventLogIOP::Initialize(Message *pMsg_)
{
	TRACEF(1, ("DdmEventLogIOP Initialized\n") );

	DispatchSignal(scLOGEVENT, log, SIGNALCALLBACK(DdmEventLogIOP, SignalLogEvent));
	return Reply(pMsg_, OK);
}


STATUS DdmEventLogIOP::Enable(Message *pMsg_)
{
	return Reply(pMsg_, OK);	
}

STATUS DdmEventLogIOP::Quiesce(Message *pMsg_)
{
	return Reply(pMsg_, OK);	
}


STATUS DdmEventLogIOP::SignalLogEvent(SIGNALCODE /*nSignal_*/,void *pPayload_)
{
	Event *pEvt = (Event *)pPayload_;

	TRACEF(2, ("DdmEventLogIOP::SignalLogEvent: ID=%d, %d parm(s)\n", pEvt->GetEventCode(), pEvt->GetParameterCount()));

	MsgAddLogEntry *pMsg = new MsgAddLogEntry(pEvt);

	Send(pMsg, REPLYCALLBACK(DdmEvtLogIOP,ProcessAddLogEntryReply));

	return OK;
}



STATUS DdmEventLogIOP::ProcessAddLogEntryReply(Message *pMsg_)
{
	MsgAddLogEntry *pMsg = (MsgAddLogEntry*)pMsg_; // cast so d'tor gets called and SGLs get cleaned up
	delete pMsg;
	
	return OK;
}