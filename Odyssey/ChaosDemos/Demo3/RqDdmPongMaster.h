/* RqDdmPong.h -- Request Interface to DdmPongMaster
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
// 	 2/12/00 Tom Nelson: Created
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890


#ifndef __RqDdmDemo3Pong_h
#define __RqDdmDemo3Pong_h

#include "OsTypes.h"
#include "RequestCodes.h"
#include "Message.h"

// Derived Request message to the DdmDemo3Pong Ddm.
//
// Ddm Request interfaces are not defined in the same file as the Ddm class
// to keep the Ddms' internal workings as private as possible.
//
class RqDdmPongMasterTick : public Message {
public:
	enum { RequestCode = REQ_OS_DEMO3_PONGTICK };

	// Encapsulate the payload - makes it easy to use the payload as
	// it's own entity.
	struct Payload {
		U32  	nCount;
		TySlot  slot;
		
		Payload(TySlot _slot, U32 _nCount=0) : nCount(_nCount),slot(_slot) {}
		
		U32 GetSlot()				{ return slot;    }
		U32 GetCount() 				{ return nCount; }
		void SetCount(U32 _nCount) 	{ nCount = _nCount; }
	};

	Payload payload;

	RqDdmPongMasterTick(TySlot _slot,U32 _nCount) : Message(RequestCode), payload(_slot,_nCount) {}
	
	DID GetSlot()				{ return payload.GetSlot();   }
	U32 GetCount() 				{ return payload.GetCount(); }
	void SetCount(U32 _nCount) 	{ payload.SetCount(_nCount); }
};

#endif	// __RqDdmDemo3_h


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosDemos/Demo3/RqDdmPongMaster.h $
// 
// 1     2/13/00 7:06p Tnelson
//  Demo shows communication between a SYSTEMENTRY Ddm on an IOP and a
// SYSTEMMASTER Ddm on an HBC.
// 
// 
// 1     2/12/00 9:16p Tnelson
// Demo shows communication between two Ddms using a derived request
// message delivered via SERVELOCAL.  Shows minimum build to run a Ddm on
// a single HBC.
// 

