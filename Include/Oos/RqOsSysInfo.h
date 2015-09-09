/* RqOsSysInfo.h -- Request Interface to DdmSysInfo.
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 *		Ddm that processes Get System Info requests
 *
**/

// Revision History: 
// 	 3/27/99 Tom Nelson: Created
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __RqOsSysInfo_H
#define __RqOsSysInfo_H

#include "OsTypes.h"
#include "RequestCodes.h"
#include "Message.h"

class RqOsSysInfoGetClassTable : public Message {
public:
	enum { RequestCode = REQ_OS_SYSINFO_GETCLASSTABLE };

	struct Payload {
		char szClassName[sCLASSNAME];// Class Name
		U16  cClass;			// Total Classes Count
		U16  iClass;			// Index of this class.
		U32  cbStack;			// Size of instance stack
		U32  sQueue;			// Size of Class queue
		U32  flags;			// Class flags
		I64  version;			// Version of Class Ddm (0 for now)
		U32  nServeLocal;		// Number of Local Serves
		U32  nServeVirtual;		// Number of Virtual Serves
		U32  nDidInstance;		// Number of current Did instances
		U32  nDdmInstance;		// Number of current Ddm instances
	};

	Payload payload;

	RqOsSysInfoGetClassTable() : Message(RequestCode) {}
};

// Dump Ddm Activity
//
class RqOsSysInfoGetDidActivity : public Message {
public:
	enum { RequestCode = REQ_OS_SYSINFO_GETDIDACTIVITY };

	struct DidActivity {
		DID did;
		VDN vdn;
		char szClassName[sCLASSNAME];
	
		U32 cSend;
		U32 cReply;
		U32 cSignal;
		U32 cAction;
		U32 cDeferred;
		DDMSTATE ddmState;
	
		void Set(const class DidMan *pDidMan);
	};
	
	enum { DIDACTIVITY_RETURNED = 0 };
	
	struct Payload {
		TySlot slot;	// Slot replying
		U32 nDid;		// Number of dids returned
		
		Payload() : slot(SLOTNULL), nDid(0) {}
	};
	
	Payload payload;
	
	RqOsSysInfoGetDidActivity() 
	: Message(RequestCode) {
		AddSgl(DIDACTIVITY_RETURNED, NULL, 0, SGL_DYNAMIC_REPLY);
	}
	void AddReply(const DidActivity *paDa, U32 nDa) {
		payload.slot = Address::GetSlot();
		payload.nDid = nDa;
		CopyToSgl(DIDACTIVITY_RETURNED, 0, (void*) paDa, nDa * sizeof(DidActivity));
	}
	
	U32 GetDidCount() {
		return payload.nDid;
	}
	DidActivity *GetDidActivity(U32 iDid,DidActivity *pDa) {
		CopyFromSgl(DIDACTIVITY_RETURNED,iDid * sizeof(DidActivity), pDa, sizeof(DidActivity));
		return pDa;
	}
};


#endif	// __RqOsSysInfo_H

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Include/Oos/RqOsSysInfo.h $
// 
// 3     12/16/99 3:26p Iowa
// System info support.
// 

