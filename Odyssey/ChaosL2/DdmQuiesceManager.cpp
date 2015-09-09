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
*	This file contains the C++ source for the QuiesceManager DDM
*	This DDM lives on the non-HBC IOPs and reroutes quiesce commands
*	to IOP Local DDMs
* 
* Update Log: 
* 10/12/99 Bob Butler: Create file
*
*************************************************************************/

#include "DdmQuiesceManager.h"
#include "QuiesceMasterMsgs.h"
#include "BuildSys.h"
#include "Odyssey_Trace.h"


// Class Link Name used by Buildsys.cpp.  Must match CLASSENTRY in buildsys.cpp
CLASSNAME (DdmQuiesceManager, SINGLE);  


//  Declare the messages served by this DDM

SERVELOCAL(DdmQuiesceManager, REQ_QUIESCE_ROUTED_IOPLOCAL);


DdmQuiesceManager::DdmQuiesceManager (DID did_) : Ddm(did_)
{
}

STATUS DdmQuiesceManager::Initialize(Message *pMsg_)
{
	Tracef("DdmQuiesceMaster: Initialized.\n");

	DispatchRequest(REQ_QUIESCE_ROUTED_IOPLOCAL, REQUESTCALLBACK(DdmQuiesceManager, ProcessRqRoutedQuiesceIopLocal));

	Reply(pMsg_, OK);
	return OK;
}



STATUS DdmQuiesceManager::Quiesce(Message *pMsg_)
{
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmQuiesceManager::Enable(Message *pMsg_)
{
	Reply(pMsg_, OK);
	return OK;
}

STATUS DdmQuiesceManager::ProcessRqRoutedQuiesceIopLocal(Message *pMsg_)
{
	RqRoutedQuiesceIopLocal *pMsg = (RqRoutedQuiesceIopLocal *)pMsg_;
	Message *pQuiesceMsg = new Message(REQ_OS_DDM_QUIESCE);
	Send(pMsg->GetVdn(), pQuiesceMsg, pMsg_, REPLYCALLBACK(DdmQuiesceManager, ProcessQuiesceReply));

	return OK;
}

STATUS DdmQuiesceManager::ProcessQuiesceReply(Message *pMsg_)
{
	Message *pQIopLocalMsg = (Message *)pMsg_->GetContext();
	Reply(pQIopLocalMsg, pMsg_->DetailedStatusCode);
	delete pMsg_;
	return OK;
}