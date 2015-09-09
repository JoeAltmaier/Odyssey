/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopMessages.h
// 
// Description:
// This file describes the Message formats used by the LoopMonitor.
// 
// Update Log:
//	$Log: /Gemini/Include/Fcp/LoopMessages.h $
// 
// 1     9/15/99 6:39p Mpanas
// New header for Loop Monitor message formats
// 
// 
// 09/15/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __LoopMessages_h
#define __LoopMessages_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"


// Loop Monitor Loop Up Message
class LmLoopUp : public Message {
public:
	enum { RequestCode = LM_LOOP_UP };

	typedef struct _payload {
		U32		instance;		// Loop Instance Number
	} Payload;
	
	Payload payload;
	
	LmLoopUp() : Message(RequestCode) {}
};


// Loop Monitor Loop LIP Message
class LmLoopLIP : public Message {
public:
	enum { RequestCode = LM_LOOP_LIP };

	typedef struct _payload {
		U32		instance;		// Loop Instance Number
	} Payload;
	
	Payload payload;
	
	LmLoopLIP() : Message(RequestCode) {}
};


// Loop Monitor Loop Down Message
class LmLoopDown : public Message {
public:
	enum { RequestCode = LM_LOOP_DOWN };

	typedef struct _payload {
		U32		instance;		// Loop Instance Number
	} Payload;
	
	Payload payload;
	
	LmLoopDown() : Message(RequestCode) {}
};


// Loop Monitor Loop Quiesce Message
class LmLoopQuiesce : public Message {
public:
	enum { RequestCode = LM_LOOP_QUIESCE };

	typedef struct _payload {
		U32		instance;		// Loop Instance Number
	} Payload;
	
	Payload payload;
	
	LmLoopQuiesce() : Message(RequestCode) {}
};


#endif
