/* DdmStatus.h -- Display Device (DDM) Status
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
//  12/07/99 Tom Nelson: Create file


#ifndef __DdmStatus_h
#define __DdmStatus_h

#include "Ddm.h"
#include "DidMan.h"
#include "VirtualDeviceTable.h"

#include "RqOsVirtualMaster.h"
#include "RqOsSysInfo.h"
#include "RqOsStatus.h"

//
// DdmStatus Class
//
class DdmStatus : public Ddm {
	typedef RqOsVirtualMasterListenIopState::States States;
	typedef RqOsSysInfoGetDidActivity::DidActivity DidActivity;
	
	static Ddm *pThis;
	
	DidActivity *aPDidActivity[DIDMAX];
	
public:
	static Ddm *Ctor(DID did) 	{ return new DdmStatus(did); }
	
	static ERC DumpIst();
	static ERC DumpVdt();
	static ERC DumpDidActivity();
	static ERC DumpAllDidActivity();
	
	States iop;

	DdmStatus(DID did);
	ERC Enable(Message *pArgMsg);

	ERC ProcessDumpIst(Message *_pRequest);
	ERC ProcessDumpVdt(Message *_pRequest);
	ERC ProcessDumpDidActivity(Message *_pRequest);
	ERC ProcessDumpAllDidActivity(Message *_pRequest);
	ERC ProcessGetDidActivityReply(Message *_pReply);

	ERC ProcessGetDidActivity(Message *_pRequest);
	ERC ProcessIOPStatusTableEnumerateReply(Message *_pReply);
	ERC ProcessVirtualDeviceTableEnumerateReply(Message *_pReply);
	ERC DumpStates(States &iop);
	ERC ProcessIOPStatusTableListenReply(Message *_pReply);
};

#endif // __DdmStatus_h

