/* DdmDemo2Pong.h -- Demo a basic Ddm
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


#ifndef __DdmDemo2Pong_h
#define __DdmDemo2Pong_h

#include "Ddm.h"

class DdmDemo2Pong : public Ddm {
public:
	static Ddm *Ctor(DID did) { return new DdmDemo2Pong(did); }

	DdmDemo2Pong(DID did);

	ERC Enable(Message *pMsg);
	
	ERC ProcessRqDemo2Pong(Message *_pReply);
};

#endif	// __DdmDemo2Pong_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo2/DdmDemo2Pong.h $
// 
// 1     2/12/00 9:15p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 
