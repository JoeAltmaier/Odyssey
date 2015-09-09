/* DdmPtsLoader.h -- SystemEntry Ddm to load PTS defaults
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 * Copyright (C) Dell Computer, 2000
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
//  6/30/99 Tom Nelson: Create file
// ** Log at end-of-file **

#ifndef __DdmPtsLoader_h
#define __DdmPtsLoader_h

#include "RqOsDdmManager.h"
#include "Listener_T.h"

// BuildSys related
#include "VirtualTable.h"
#include "CtTypes.h"

// DdmSvcVirtualLoader
//
class DdmSvcVirtualLoader : public DdmServices {
	
	void *pContext;
	ActionCallback callback;
	
	ERC status;

public:
	DdmSvcVirtualLoader(DdmServices *_pDdmParent) 
	:  DdmServices(_pDdmParent),status(OK) {}

	ERC Status()			 		{ return status; }
	void *GetContext() 		 		{ return pContext; }
	
	// ActionCallback receives pointer to this DdmServices object.	
	ERC Execute(ActionCallback _callback) {
		return Execute(NULL, _callback);
	}
	ERC Execute(void *pContext, ActionCallback _callback);

private:
	ERC ProcessLoadVirtualDeviceReply(Message *_pReply);
	ERC ProcessDefineTableReply(Message *_pReply);
	BOOL LoadVirtualData(VirtualEntry *_pEntry);
	ERC ProcessCfgInsertReply(Message *_pReply);
	ERC ProcessVdtInsertReply(Message *_pReply);
	ERC InsertConfigData(VirtualEntry *pEntry, void* pContext, ReplyCallback callback);
	ERC InsertVirtualDevice(VirtualEntry *_pEntry,void *payload, RowId &ridCfg, ReplyCallback callback);

};

//
//  DdmPtsLoader Class
//
class DdmPtsLoader : public Ddm {

public:
	static Ddm *Ctor(DID did) 	{ return new DdmPtsLoader(did); }
	DdmPtsLoader(DID did);

	ERC Initialize(Message *_pMsgInit);
	ERC Enable(Message *_pMsgEnable);
	ERC ProcessLoadVirtualComplete(void *_pContext);
};


#endif // __DdmPtsLoader_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmPtsLoader.h $
// 
// 8     2/15/00 6:06p Tnelson
// Removed references to obsolete files
// 
// 7     2/09/00 3:08p Tnelson
// Removed references to unused include files.  No code changes.
// 
// 6     2/08/00 8:57p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 7     2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
