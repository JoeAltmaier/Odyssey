#include "ddm.h"
#include "logmastermessages.h"
#include "assert.h"
#include "buildsys.h"

#include "ctevent.h"

#include <stdlib.h>

#define _TRACEF
#define	TRACE_INDEX		TRACE_SYSTEMLOG
#include "Odyssey_Trace.h"


class DdmTestLM : public Ddm
{
public:
	static Ddm *Ctor (DID did_) ;
	DdmTestLM (DID did_) : Ddm(did_)  {}

	virtual STATUS Enable (Message *pMsg_);
private:
	REFNUM refnumQ;

	STATUS StartTest(void *);

	STATUS ProcessMDQReply(Message *pMsg_);
	STATUS ProcessQueryReply(Message *pMsg_);
	STATUS ProcessCancelReply(Message *pMsg_);


};

// Class Link Name used by Buildsys.cpp.  Must match CLASSENTRY in buildsys.cpp
CLASSNAME (DdmTestLM, SINGLE);  

// test various creation conditions.  There are enough files here to cause the partition table page to grow. If
// file 30 doesn't fail (out of space), something went wrong.


STATUS DdmTestLM::Enable (Message *pMsg_)
{
	
	Reply(pMsg_, OK);
	STATUS status = Action(ACTIONCALLBACK(DdmTestLM, StartTest), NULL);
	return OK;
}


Ddm *DdmTestLM::Ctor(DID did)
{
	return new DdmTestLM(did);
}


STATUS DdmTestLM::StartTest(void *context) 
{	
	LogEvent(ELOG_TEST_INFO1, "An arg");
	LogEvent(ELOG_TEST_INFO2);
	LogEvent(ELOG_TEST_WARNING1, 5);
	LogEvent(ELOG_TEST_WARNING2);
	LogEvent(ELOG_TEST_ERROR1, 12.2);
	LogEvent(ELOG_TEST_ERROR2);
	LogEvent(ELOG_TEST_INTERNAL1, "Another Arg");
	LogEvent(ELOG_TEST_INTERNAL2, "This has", 3, "Args");

	MsgQueryLogMetaData *pMsg = new MsgQueryLogMetaData(LM_LISTEN_NEW);
	Send(pMsg, REPLYCALLBACK(DdmTestLM, ProcessMDQReply));

	
	// query for all events
	MsgQueryLog *pMsgql = new MsgQueryLog(0, 10, true, LM_LISTEN_ALL);
	Send(pMsgql, REPLYCALLBACK(DdmTestLM, ProcessQueryReply));
	refnumQ = pMsgql->refnum;

	return OK;
}

// This will probably be called several times, since it was a listen and
// the query above probably happens before all the log entries actually 
// make it into the log.
STATUS DdmTestLM::ProcessMDQReply(Message *pMsg_)
{
	MsgQueryLogMetaData *pMsg = (MsgQueryLogMetaData*)pMsg_;

	if (pMsg->DetailedStatusCode == ELOG_CANCEL_LISTEN)
	{
		TRACEF(2, ("Got cancelled listen response to MD Query\n"));
	}
	else
	{

		TRACEF(2, ("Got Metadata reply.\n"));
		TRACEF(2, ("There are %d events in the log\n", pMsg->GetEntryCount()));

		for (int i = 0; i < Event::SEVERITY_COUNT; ++i)
			TRACEF(2, ("There are %d events in the log at severity %d\n", pMsg->GetEntryCount(i), i));
		if (pMsg->GetEntryCount() == 4) // try another query now that the log is partly populated
		{
			MsgCancelLogListen *pMsgcan = new MsgCancelLogListen(pMsg_); // cancel the metadata query
			Send(pMsgcan, REPLYCALLBACK(DdmTestLM, ProcessCancelReply));

			pMsgcan = new MsgCancelLogListen(refnumQ); // cancel the query
			Send(pMsgcan, REPLYCALLBACK(DdmTestLM, ProcessCancelReply));

			// should return 4 events: 2,3,4,5
			MsgQueryLog *pMsgql = new MsgQueryLog(0, 10, true, LM_LISTEN_NEW);  // get the last four, one at a time
			pMsgql->QuerySeverity(1,2); 
			Send(pMsgql, REPLYCALLBACK(DdmTestLM, ProcessQueryReply));

			// should return 5 events: 6,5,4,3,2
			MsgQueryLog *pMsgql2 = new MsgQueryLog(6, 10, false, LM_LISTEN_ALL);
			pMsgql2->QuerySeverity(1,3); 
			Send(pMsgql2, REPLYCALLBACK(DdmTestLM, ProcessQueryReply));

		}
	}
	delete pMsg;  

	return OK;
}

STATUS DdmTestLM::ProcessCancelReply(Message *pMsg_)
{
	delete pMsg_;
	return OK;
}


STATUS DdmTestLM::ProcessQueryReply(Message *pMsg_)
{
	MsgQueryLog *pMsg = (MsgQueryLog *)pMsg_;

	if (pMsg->DetailedStatusCode == ELOG_CANCEL_LISTEN)
	{
		TRACEF(2, ("Got cancelled listen response to Query\n"));
	}
	else
	{

		TRACEF(2, ("Got Query reply.\n"));
		Event *apEvt;

		U32 count = pMsg->GetLogEntries(&apEvt);
		TRACEF(2, ("There are %d events in the result set\n", count));

		for (int i = 0; i < count; ++i)
		{
			TRACEF(2, ("Event #%d: ID=%d, Seq=%d, Sev=%d\n",i, apEvt[i].GetEventCode(), apEvt[i].GetSequenceNum(), apEvt[i].GetSeverity()));
			TRACEF(2, ("  This event has %d parameters.\n", apEvt[i].GetParameterCount()));
			if (apEvt[i].GetParameterCount() > 0)
			{
				TRACEF(2, ("    They are: "));
				for (int ii = 0; ii < apEvt[i].GetParameterCount(); ++ii)
				{
					if (apEvt[i].GetParameterType(ii) == Event::STR_PARM)
					{
						TRACEF(2, (" #%d (string): %s ", ii+1, apEvt[i].GetPParameter(ii)));
					}
					else if (apEvt[i].GetParameterType(ii) == Event::U32_PARM)
					{
						TRACEF(2, (" #%d (U32): %d ", ii+1, *((U32 *)apEvt[i].GetPParameter(ii))));
					}
				}
				TRACEF(2, ("\n"));
			}
		}
		delete []apEvt;
	}
	delete pMsg; // delete the cast message so the d'tor gets called
	return OK;
}