/* RqLeak.h -- Request Interface to DdmLeak.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
//  12/114/99 Joe Altmaier: Create file

#ifndef __RqLeak_h
#define __RqLeak_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

// This is the payload structure for the REQ_LEAK_CLEAR message.
class RqLeakClear : public Message {
public:
	enum { requestcode = REQ_LEAK_CLEAR };
	enum { REQ_LEAK_CLEAR_ALLOC=1, REQ_LEAK_CLEAR_MAX=2, REQ_LEAK_CLEAR_THRASH=4, REQ_LEAK_CLEAR_ALL=7 };

	U32 maskClear;

	RqLeakClear(U32 _mask=REQ_LEAK_CLEAR_ALL) : Message(requestcode) {
		maskClear=_mask;
	}
};
	

// This is the payload structure for the REQ_LEAK_DELIVER message.
class RqLeakDeliver : public Message {
public:
	enum { requestcode = REQ_LEAK_DELIVER };

	U32 addrWant;
	
	RqLeakDeliver(U32 _addrWant, void *_pAlloc=NULL, void *_pMax=NULL, void *_pThrash=NULL) : Message(requestcode) {
		addrWant = _addrWant;
		AddSgl(0, _pAlloc, 8192, SGL_REPLY);
		if (_pMax)
			AddSgl(0, _pMax, 8192, SGL_REPLY);
		if (_pThrash)
			AddSgl(0, _pThrash, 8192, SGL_REPLY);
	}
};
	
#endif