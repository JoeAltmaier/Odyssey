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
*	This file contains the class declaration for the QuiesceManager DDM
*	This DDM lives on the non-HBC IOPs and reroutes quiesce commands
*	to IOP Local DDMs
* 
* Update Log: 
* 10/12/99 Bob Butler: Create file
*
*************************************************************************/


#ifndef DdmQuiesceManager_h
#define DdmQuiesceManager_h

#include "ddm.h"


class DdmQuiesceManager : public Ddm
{

public:

   //  Constructor
	DdmQuiesceManager (DID did_);
   //  Create an instance of this DDM
	static Ddm *Ctor (DID did_) { return new DdmQuiesceManager(did_); }

	STATUS Initialize(Message *pMsg_);
	STATUS Enable(Message *pMsg_);
	STATUS Quiesce(Message *pMsg_);
	
private: 

	STATUS ProcessRqRoutedQuiesceIopLocal(Message  *pMsg_);
	STATUS ProcessQuiesceReply(Message  *pMsg_);
};


#endif
