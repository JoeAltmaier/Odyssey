/* DdmMasterDriver.h -- Test Timer DDM
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
// $Log: /Gemini/Odyssey/AlarmManager/DdmMasterDriver.h $
// 
// 9     9/29/99 6:47p Joehler
// modified drivers to test alarm coalescing
// 
// 8     9/13/99 3:39p Joehler
// Added testing functionality for user remitted alarms
// 
// 7     9/07/99 2:11p Joehler
// Changes to correctly manage memory associated with AlarmContext
// 
// 6     9/02/99 11:46a Joehler
// added comments
//


#ifndef __DdmMasterDriver_H
#define __DdmMasterDriver_H

#include "OsTypes.h"
#include "Message.h"
#include "DdmMaster.h"

struct AlarmContext {
	int dummy;
};

struct DummyContext {
	char dummy[1000];
};

class DdmMasterDriver: public DdmMaster {
public:
	DdmMasterDriver(DID did_) : DdmMaster(did_) {}
	static Ddm *Ctor(DID did);

	STATUS Enable(Message *pMsg);
	
private:

	// main MessageControl function
	void MessageControl();

	// called by MessageControl function
	void SubmitFirstAlarm();
	void SubmitCoalesceAlarmSigParam();
	void SubmitCoalesceAlarmNonSigParam();
	void SubmitUserRemittableAlarm();
	void SubmitNonUserRemittableAlarm();
	void SubmitSecondAlarm();
	void RemitFirstAlarm();
	void DeletePAl();
	void RecoverAllAlarms() ;
	void RemitMostAlarms();
	void SubmitBigAlarmContext();

	// call back functions which call MessageControl
	void cbSubmitAlarm(void * pAlarmContext_, STATUS status_  );
	void cbRemitAlarm(void * pAlarmContext_ , STATUS status_ );
	void cbRecoveryComplete(STATUS status_  );

};
#endif	// __DdmMasterDriver_H

