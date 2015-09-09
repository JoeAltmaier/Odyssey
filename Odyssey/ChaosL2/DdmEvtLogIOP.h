//*************************************************************************
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the declaration of the DdmEventLogIOP class. 
// 
// Update Log: 
// 03/10/99	RJB	Created.
//*************************************************************************

#ifndef _DdmEventLogIOP_h_
#define _DdmEventLogIOP_h_

#include "OsTypes.h"
#include "Message.h"
#include "Event.h"



class DdmEventLogIOP : public Ddm
{
  public:

	  // Create and instance of this DDM
	static Ddm *Ctor(DID did_);
	static void LogEvent(Event *pEvt_);

	// Constructor
	DdmEventLogIOP(DID did_) : Ddm(did_) {};

	// Ddm Methods
	virtual STATUS Initialize(Message *pMsg_);
	virtual STATUS Enable(Message *pMsg_);
	virtual STATUS Quiesce(Message *pMsg_);

//	static DdmEventLogIOP *GetIOPLog() { return log; }
	
  private:
	static DdmEventLogIOP *log;	
	
	STATUS SignalLogEvent(SIGNALCODE nSignal_,void *pPayload_);
	STATUS ProcessAddLogEntryReply(Message *pMsg_);
};
#endif	
