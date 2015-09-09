/* RqOsTimer.h -- Request Interface to DdmTimer.
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
//  8/17/98 Joe Altmaier: Create file
//  3/04/99 Tom Nelson:	  Change NU_Create_Timer to Kernel::Create_Timer
//  3/18/99 Tom Nelson:   Changed to use Timer.h and SERVE macros
//  3/27/99 Tom Nelson:   Support derived timer request classes
//  6/27/99 Tom Nelson:	  Added RequestCode enums

#ifndef __RqProfile_h
#define __RqProfile_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

// This is the payload structure for the REQ_PROFILE_START message.
class RqProfileStart : public Message {
public:
	enum { requestcode = REQ_PROFILE_START };

	I64	timeInterval;

	RqProfileStart(I64 _timeInterval) : Message(requestcode) {
		timeInterval = _timeInterval;
	}
};
	

// This is the payload structure for the REQ_PROFILE_STOP message.
class RqProfileStop : public Message {
public:
	enum { requestcode = REQ_PROFILE_STOP };

	RqProfileStop() : Message(requestcode) {}
};
	

// This is the payload structure for the REQ_PROFILE_CLEAR message.
class RqProfileClear : public Message {
public:
	enum { requestcode = REQ_PROFILE_CLEAR };

	RqProfileClear() : Message(requestcode) {}
};
	

// This is the payload structure for the REQ_PROFILE_DELIVER message.
class RqProfileDeliver : public Message {
public:
	enum { requestcode = REQ_PROFILE_DELIVER };

	U32 addrWant;
	
	RqProfileDeliver(U32 _addrWant) : Message(requestcode) {
		addrWant = _addrWant;
	}
};
	
#endif