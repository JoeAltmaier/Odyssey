/* DdmPtsProxyLoader.h -- SystemEntry Ddm to load PTS defaults
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
//  6/30/99 Tom Nelson: Create file


#ifndef __DdmPtsProxyLoader_h
#define __DdmPtsProxyLoader_h

#include "RqOsDdmManager.h"
#include "RqOsPtsProxy.h"
#include "RqOsPtsLoader.h"
#include "Listener_T.h"
#include "PtsTable.h"

//
//  DdmPtsProxyLoader Class
//
class DdmPtsProxyLoader : public Ddm {
	typedef ListenerList_T<RqOsPtsProxyLoaderFindPts>  ListenerFindPts;
	typedef struct _s {
		BOOL fReady;
		_s() : fReady(FALSE) {}
	} System;

	RqOsDdmManagerRouteInstance::Payload ptsOs;
	RqOsDdmManagerRouteInstance::Payload ptsAlt;
	
	RqOsDdmManagerRouteInstance::Payload pts;
	RqOsPtsProxySetIopStateTable::Payload iop;
	ListenerFindPts listenerFindPts;	
	System system;
	
public:
	static Ddm *Ctor(DID did) 	{ return new DdmPtsProxyLoader(did); }
	DdmPtsProxyLoader(DID did);

	ERC Initialize(Message *pArgMsg);
	ERC Enable(Message *pArgMsg);
	ERC ProcessStartPtsReply(Message *pArgMsg);
	ERC ProcessRouteInstanceReply(Message *pArgMsg);
	ERC ProcessAddVddReply(Message *pArgMsg);
	ERC ProcessSetStateTableReply(Message *pArgMsg);
	ERC ProcessFindPtsReply(Message *pArgMsg);
	ERC ProcessSystemReadyReply(Message *pArgMsg);
	ERC ProcessRoutePtsInstanceReply(Message *pArgMsg);
	ERC ProcessFindPts(Message *pArgMsg);

	ERC StartPts(PtsVdd *pPtsVdd, Message *pContext,ReplyCallback pCallback);
	ERC FindPts(PtsVdd *pPtsVdd, Message *pContext,ReplyCallback pCallback);
};

#endif // __DdmPtsProxyLoader_h

