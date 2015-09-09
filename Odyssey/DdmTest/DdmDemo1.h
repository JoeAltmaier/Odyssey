/* DdmDemo1.h -- Demo a basic Ddm
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


#ifndef __DdmDemo1_h
#define __DdmDemo1_h

#include "Ddm.h"

class DdmDemo1 : public Ddm {
public:
	static Ddm *Ctor(DID did) { return new DdmDemo1(did); }

	RqOsTimerStart *pStartMsg;
	
	DdmDemo1(DID did) : Ddm(did) {}

	ERC Enable(Message *pMsg);
	ERC Quiesce(Message *_pQuiesceMsg);
	ERC ProcessReplyStopTimer(Message *_pReply);
	
	ERC ProcessReplyTimer(Message *_pReply);
};

#endif	// __DdmDemo1_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo1/DdmDemo1.h $
// 
// 2     2/12/00 9:08p Tnelson
// 
// 1     2/12/00 6:15p Tnelson
// Demo minimum build to run a Ddm on a single HBC
// 
