/* DdmPing.h -- Demo a basic Ddm
 *
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
//  2/12/00 Tom Nelson: Created
//


#ifndef __DdmPing_h
#define __DdmPing_h

#include "Ddm.h"

class DdmPing : public Ddm {
public:
	static Ddm *Ctor(DID did) { return new DdmPing(did); }

	U32 count;	// Ping counter
	
	DdmPing(DID did) : Ddm(did), count(0) {}

	ERC Enable(Message *pMsg);
	
	ERC ProcessReplyTimer(Message *_pReply);
	ERC ProcessReplyPong(Message *_pReply);
};

#endif	// __Ping_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo3/DdmPing.h $
// 
// 1     2/13/00 7:05p Tnelson
//  Demo shows communication between a SYSTEMENTRY Ddm on an IOP and a
// SYSTEMMASTER Ddm on an HBC.
// 
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
