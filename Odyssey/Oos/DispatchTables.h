/* DispatchTables.h -- Request/Signal Dispatch Tables Definitions
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
//  3/12/99 Tom Nelson: Create file
//

#ifndef __DispatchTables_h
#define __DispatchTables_h

#include "Array_t.h"
#include "CtEvent.h"

//***
//*** Request Dispatch Table
//***

class RequestDispatchMethod  {
public:
	DdmServices *pInstance;
	DdmOsServices::RequestCallback pMethod;
		
	RequestDispatchMethod() : pInstance(NULL), pMethod(NULL) {}
		
	BOOL IsValid()			  { return pInstance != NULL; 	}
	ERC Invoke(Message *pMsg) { return (pInstance->*pMethod)(pMsg); }
	
	void Dump()	{
		union {
			DdmOsServices::RequestCallback rc;
			U32 p[3];
		} p;
		p.rc = pMethod;
		Tracef("%x->(%x %x %x) ",pInstance,p.p[0],p.p[1],p.p[2]);
	}
};

class RequestDispatchTable {
	Array_T<RequestDispatchMethod> aRange[REQUEST_RANGE_LAST];	// Range Lookup
public:
	ERC Set(REQUESTCODE reqCode,DdmServices *pInstance,DdmOsServices::RequestCallback pMethod) {
		RequestDispatchMethod rdm;

		rdm.pInstance = pInstance;
		rdm.pMethod = pMethod;
		if ((reqCode >> 16) >= REQUEST_RANGE_LAST)
			return CTS_CHAOS_UNKNOWN_FUNCTION;

		aRange[reqCode >> 16].Set(reqCode & 0xFFFF,rdm);
		
		return OK;
	 }

	RequestDispatchMethod *Get(REQUESTCODE reqCode) {
		static RequestDispatchMethod rdmNull;
		if ((reqCode >> 16) < REQUEST_RANGE_LAST) {
			if ((reqCode & 0xFFFF) < aRange[reqCode >> 16].Size()) {
				return &aRange[reqCode>>16][reqCode & 0xFFFF];
			}
		}
		return &rdmNull;
	}

	void Dump() { 
		for (U32 ii=0; ii<REQUEST_RANGE_LAST; ii++) {
			Tracef("Rg%u: ",ii);
			aRange[ii].Dump();
			Tracef("\n");
		}
	}
};
	
//***
//*** Signal Dispatch Table
//***

class SignalDispatchMethod {
public:
	DdmServices    *pInstance;
	DdmOsServices::SignalCallback pMethod;
		
	SignalDispatchMethod() : pInstance(NULL), pMethod(NULL) {}
		
	BOOL IsValid()			  { return pInstance != NULL; 	}
	ERC Invoke(SIGNALCODE sigCode,void *pPayload) { return (pInstance->*pMethod)(sigCode,pPayload); }
};

class SignalDispatchTable {
	Array_T<SignalDispatchMethod> aTable;	// Range Lookup
public:	
	ERC Set(SIGNALCODE sigCode,DdmServices *pInstance,DdmOsServices::SignalCallback pMethod) {
		SignalDispatchMethod sdm;
		
		sdm.pInstance = pInstance;
		sdm.pMethod = pMethod;
		aTable.Set(sigCode,sdm);
		
		return OK;
	 }

	SignalDispatchMethod *Get(SIGNALCODE sigCode) {
		static SignalDispatchMethod sdmNull;
		
		if (sigCode < aTable.Size())
			return &aTable[sigCode];

		return &sdmNull;
	 }
};

#endif	// __DispatchTables_h
