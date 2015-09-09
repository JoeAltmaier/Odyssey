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
*	This file contains the definition of the LogMaster DDM
* 
* Update Log: 
* 07/02/99 Bob Butler: Create file - formerly EvtLogHbc,h
*
*************************************************************************/

#ifndef DdmLogMaster_h
#define DdmLogMaster_h

#include "ddm.h"
//#include "LogMasterCmds.h"

#include "CmdServer.h"


class DdmLogMaster : public Ddm
{

public:

   //  Constructor
	DdmLogMaster (DID did_);
   //  Create an instance of this DDM
	static Ddm *Ctor (DID did_) { return new DdmLogMaster(did_); }

protected:

	virtual STATUS Initialize (Message *pMsg_);
	virtual STATUS Quiesce (Message *pMsg_);
	virtual STATUS Enable (Message *pMsg_);


	STATUS ProcessQuery (Message *pMsg_);
	STATUS ProcessMetadataQuery (Message *pMsg_);
	STATUS ProcessAddLogEntry (Message *pMsg_);
	STATUS ProcessCancelListen (Message *pMsg_);

private:

	void AddListener (Message *pMsg_);
	void SetMetaData (Message *pMsg_);
	bool IsFilterMatch (Event *pEvt_, Message *pMsg_); 
	void SendListenerReplies(Event *pEvt_);

	U16 cListeners;
	Message **apListeners;
	enum {LISTENER_GROW = 10};

}; 
#endif