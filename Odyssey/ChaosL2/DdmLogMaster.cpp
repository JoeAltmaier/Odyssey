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
*	This file contains the definition of the Log Master messages.
* 
* Update Log: 
* 07/02/99 Bob Butler: Rename file - formerly EvtLogHBC.cpp
* 07/21/99 Bob Butler: Updated for changes to Event.h and the Query API
* 08/05/99 Bob Butler: Implemented an in-memory version so the log can be
*						used prior to integration with the PTS
*
*************************************************************************/

#include "LogMasterMessages.h"
#include "DdmLogMaster.h"

#include "BuildSys.h"

#define _TRACEF
#define	TRACE_INDEX		TRACE_SYSTEMLOG
#include "Odyssey_Trace.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


// Class Link Name used by Buildsys.cpp.  Must match CLASSENTRY in buildsys.cpp
CLASSNAME (DdmLogMaster, SINGLE);  


//  Declare the messages served by this DDM
SERVELOCAL(DdmLogMaster, PRIVATE_REQ_ADD_LOGENTRY);
SERVELOCAL(DdmLogMaster, REQ_QUERY_LOG_METADATA);
SERVELOCAL(DdmLogMaster, REQ_QUERY_LOG);
SERVELOCAL(DdmLogMaster, REQ_CANCEL_LOG_LISTEN);



// temporary globals -- just until the PTS has var fields

U32 cEvents = 0;
Event **apEvt;
U32 Seq = 0;


DdmLogMaster::DdmLogMaster (DID did_) : Ddm(did_), apListeners(NULL), cListeners(0)
	 //, pCmdServer(NULL), pCmdSender(NULL)
{
}



STATUS DdmLogMaster::Initialize(Message *pMsg_)
{
	Tracef("DdmLogMaster: Initialized.\n");

	DispatchRequest(PRIVATE_REQ_ADD_LOGENTRY, REQUESTCALLBACK(DdmLogMaster, ProcessAddLogEntry));
	DispatchRequest(REQ_QUERY_LOG_METADATA, REQUESTCALLBACK(DdmLogMaster, ProcessMetadataQuery));
	DispatchRequest(REQ_QUERY_LOG, REQUESTCALLBACK(DdmLogMaster, ProcessQuery));
	DispatchRequest(REQ_CANCEL_LOG_LISTEN, REQUESTCALLBACK(DdmLogMaster, ProcessCancelListen));

	Reply(pMsg_, OK);
	return OK;
}



STATUS DdmLogMaster::Quiesce(Message *pMsg_)
{
	for (int i = 0; i < cListeners; ++i)
		Reply(apListeners[i], ELOG_CANCEL_LISTEN);

	delete apListeners;
	cListeners = 0;
	apListeners = NULL;

	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmLogMaster::Enable(Message *pMsg_)
{

	cListeners = 0;
	delete apListeners;
	apListeners = NULL;

	Reply(pMsg_, OK);
	return OK;
}


STATUS DdmLogMaster::ProcessQuery (Message *pMsg_)
{
	MsgQueryLog *pMsg = (MsgQueryLog *) pMsg_;


	// Temporary code:  look through the internal array and send back all matches
	if (pMsg->IsListener())
		AddListener(pMsg);
	
	if (pMsg->GetListenMode() == LM_LISTEN_NEW)  // don't query existing entries
		return OK;

	Event **apEvtMatch = new Event *[cEvents];
	U32 cMatches = 0;

	S32 start = pMsg->GetStartingSeqNum();
	S16 step = pMsg->IsForwardDirection() ? 1 : -1;
	S32 count = pMsg->GetMaxCount();
	S32 end;
	if (pMsg->IsForwardDirection())
		end = min(cEvents, start + count);
	else
		end = max(0, start - count);

	for (S32 i = start; pMsg->IsForwardDirection() ? i < end : i > end; i += step)
	{
		if (IsFilterMatch(apEvt[i], pMsg))
			apEvtMatch[cMatches++] = apEvt[i];
	}
	pMsg->SetLogEntries(cMatches, apEvtMatch);
	delete []apEvtMatch;
	

	Reply(pMsg, OK, !pMsg->IsListener());
	return OK;
}

STATUS DdmLogMaster::ProcessMetadataQuery (Message *pMsg_)
{
	MsgQueryLogMetaData *pMsg = (MsgQueryLogMetaData *)pMsg_;
	if (pMsg->IsListener())
		AddListener(pMsg);

	if (pMsg->GetListenMode() == LM_LISTEN_NEW)  // Only reply on updates
		return OK;


	SetMetaData(pMsg);
	Reply(pMsg, OK, !pMsg->IsListener());
	return OK;
}

void DdmLogMaster::AddListener (Message *pMsg_)
{
	if (cListeners % LISTENER_GROW == 0)
	{
		Message **apTemp = new Message*[cListeners + LISTENER_GROW];

		if (cListeners)
			memcpy(apTemp, apListeners, cListeners * sizeof(Message *));
		delete []apListeners;
		apListeners = apTemp;
	}
	apListeners[cListeners++] = pMsg_;

}

bool DdmLogMaster::IsFilterMatch(Event *pEvt_, Message *pMsg_)
{ 
	MsgQueryLog *pMsg = (MsgQueryLog *) pMsg_;
	bool match = pMsg->IsMatchSeverity(pEvt_->GetSeverity())
				&& pMsg->IsMatchFacility(pEvt_->GetFacility())
				&& pMsg->IsMatchSlot(pEvt_->GetSlot())
				&& pMsg->IsMatchTimestamp(pEvt_->GetTimestamp());


	return match;
}

void DdmLogMaster::SendListenerReplies(Event *pEvt_)
{
	for (int i = 0; i < cListeners; ++i)
		if (apListeners[i]->reqCode == REQ_QUERY_LOG && IsFilterMatch(pEvt_, apListeners[i]))
		{
			MsgQueryLog *pMsg = (MsgQueryLog *) apListeners[i];

			
			pMsg->SetLogEntries(1, &pEvt_);
			Reply(pMsg, OK, !pMsg->IsListener());
		}
		else if (apListeners[i]->reqCode == REQ_QUERY_LOG_METADATA)
		{ // this will need to be modifies a little when implemented in the PTS
			MsgQueryLogMetaData *pMsg = (MsgQueryLogMetaData *) apListeners[i];
			pMsg->SetCount(pEvt_->GetSeverity(), pMsg->GetEntryCount(pEvt_->GetSeverity()) + 1);
			pMsg->SetSequenceNumbers(pEvt_->GetSeverity(), pMsg->GetFirstSequenceNumber(pEvt_->GetSeverity()), pEvt_->GetSequenceNum());
			pMsg->SetTimestamps(pEvt_->GetSeverity(), pMsg->GetFirstTimestamp(pEvt_->GetSeverity()), pEvt_->GetTimestamp());
			Reply(pMsg, OK, !pMsg->IsListener());
		}
}

void DdmLogMaster::SetMetaData (Message *pMsg_)
{
	MsgQueryLogMetaData *pMsg = (MsgQueryLogMetaData *)pMsg_;

	// this is all temporary code for Andrey.  A bit messy and slow
	U32 cSev, acSev[Event::SEVERITY_COUNT], aSeq[2][Event::SEVERITY_COUNT];
	Timestamp aTS[2][Event::SEVERITY_COUNT];

	for (cSev = 0; cSev < Event::SEVERITY_COUNT; ++cSev)
	{
		acSev[cSev] = 0;
		aSeq[0][cSev] = -1; // max unsigned
		aSeq[1][cSev] = 0;
		aTS[0][cSev] = (Timestamp)-1;
		aTS[1][cSev] = (Timestamp)0;
	}
	for (int i = 0; i < cEvents; ++i)
	{
		++acSev[apEvt[i]->GetSeverity()];
		aSeq[0][apEvt[i]->GetSeverity()] = min(aSeq[0][apEvt[i]->GetSeverity()], apEvt[i]->GetSequenceNum());
		aSeq[1][apEvt[i]->GetSeverity()] = max(aSeq[1][apEvt[i]->GetSeverity()], apEvt[i]->GetSequenceNum());
		aTS[0][apEvt[i]->GetSeverity()] = min(aTS[0][apEvt[i]->GetSeverity()], apEvt[i]->GetTimestamp());
		aTS[1][apEvt[i]->GetSeverity()] = max(aTS[1][apEvt[i]->GetSeverity()], apEvt[i]->GetTimestamp());
	}
	for (cSev = 0; cSev < Event::SEVERITY_COUNT; ++cSev)
	{
		pMsg->SetCount(cSev, acSev[cSev]);
		pMsg->SetSequenceNumbers(cSev, aSeq[0][cSev], aSeq[1][cSev]);
		pMsg->SetTimestamps(cSev, aTS[0][cSev], aTS[1][cSev]);
	}

}


STATUS DdmLogMaster::ProcessAddLogEntry (Message *pMsg_)
{
	MsgAddLogEntry *pMsg = (MsgAddLogEntry *)pMsg_;
	Event *pEvt;
	pMsg->GetEvent(&pEvt);
	TRACEF(2, ("DdmLogMaster::ProcessAddLogEntry: ID=%d, %d parm(s)\n", pEvt->GetEventCode(), pEvt->GetParameterCount()));


	// temporary code:

	if (cEvents % 20 == 0)
	{
		Event **apTemp = new Event*[cEvents + 20];
		if (cEvents)
			memcpy(apTemp, apEvt, cEvents * sizeof(Event *));
		delete []apEvt;
		apEvt = apTemp;
	}

	pEvt->SetSequenceNum(Seq++);
	pEvt->SetTimeStamp(Kernel::Time_Stamp());
	apEvt[cEvents++] = pEvt;
	SendListenerReplies(pEvt);
	return OK;
}

STATUS DdmLogMaster::ProcessCancelListen (Message *pMsg_)
{
	MsgCancelLogListen *pMsg = (MsgCancelLogListen *)pMsg_;

	for (int i = 0; i < cListeners; ++i)
		if (apListeners[i]->refnum == pMsg->GetRefnum())
		{
			Reply(apListeners[i], ELOG_CANCEL_LISTEN);
			for (int j=i; j<cListeners-1; ++j)
				apListeners[j] = apListeners[j+1];
			--cListeners;
		}

	Reply(pMsg_, OK);
	return OK;
}

