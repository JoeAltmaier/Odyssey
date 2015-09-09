/* RqOsPong.h -- Request Interface to DdmPongMaster/Slave Ddm.
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
//  7/02/99 Tom Nelson: Create file


#ifndef __RqOsPong_h
#define __RqOsPong_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

// Check if all SystemEntries have started
//
class RqOsPong : public Message {
public:
	enum { RequestCode = REQ_OS_TEST_PONG };
	
	struct Payload {
		U32 count;
		
		Payload(U32 _c=0) : count(_c) {}
	};
	Payload payload;
	
	RqOsPong(U32 count=0) : Message(RequestCode), payload(count) {}
};


#endif	// __RqOsPong_h
