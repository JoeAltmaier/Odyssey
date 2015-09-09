/*
 * DdmTriggerMgr.h - Error Injection Trigger Manager DDM
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
 * Revision History:
 *     2/6/1999 Bob Butler: Created
 *
*/

#ifndef __DdmTriggerMgr_H
#define __DdmTriggerMgr_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "Array_T.h"
#include "ErrorTrigger.h"
/*
 * The Trigger Manager DDM is responsible for managing a list of error
 * triggers on a given IOP
*/



class DdmTriggerMgr : public Ddm
{
public:
	DdmTriggerMgr(DID did);
	
	// DDM methods
	static Ddm *Ctor(DID did);
	virtual STATUS Enable(Message *pMsg);
	virtual STATUS Initialize(Message *pMsg);

	static BOOL Trigger(U32 _id, U32 _parameter);
	static U16 ClassClear(U32 _classID);
	static U16 Clear(U32 ID);
	
private:
	static DdmTriggerMgr *pTM;  // this is a singleton class, enforced by the DDM Manager.
								// pTM is a pointer to the single instance on a given IOP.
								
	Message *pArmMsg; // 								
	Array_T<ErrorTrigger> aET;
	bool isArmed;
	
	STATUS LoadScript(Message *pMsg);
	STATUS Disarm(Message *pMsg);
	STATUS Arm(Message *pMsg);
	STATUS Report(Message *pMsg);
	BOOL AddTrigger(class ErrorTrigger &et);
	void CompletionCheck();

};

struct TMReport
{
	U16 iSlot;
	U16 iPhase;
	BOOL isArmed;
	U16 cTriggers;
	U16 cArmed;			
	U16 cDisarmed;			
	U16 cTriggered;			
	U16 cCleared;
};

// temporary
enum { EIS_FORCED_DISARM = 10000,
	   EIS_ALREADY_ARMED };

#endif