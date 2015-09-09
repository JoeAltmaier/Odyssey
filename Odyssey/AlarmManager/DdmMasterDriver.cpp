/* DdmMaserDriver.cpp -- Test SysInfo DDM
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
// $Log: /Gemini/Odyssey/AlarmManager/DdmMasterDriver.cpp $
// 
// 11    9/29/99 6:47p Joehler
// modified drivers to test alarm coalescing
// 
// 10    9/13/99 3:39p Joehler
// Added testing functionality for user remitted alarms
// 
// 9     9/07/99 2:11p Joehler
// Changes to correctly manage memory associated with AlarmContext
// 
// 8     9/02/99 11:46a Joehler
// added comments

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"

#include "DdmMasterDriver.h"
#include "AlarmMasterMessages.h"
#include "BuildSys.h"
#include "Event.h"
#include "CTEvent.h"

// BuildSys linkage
	
CLASSNAME(DdmMasterDriver,SINGLE);

// temp
AlarmContext* pAlarmContext1;

extern void QuitTest();

Ddm *DdmMasterDriver::Ctor (DID did)
{

	TRACE_ENTRY(DdmMasterDriver::Ctor(DID));
	return (new DdmMasterDriver (did));

} 

// .Enable -- Start-it-up ---------------------------------------------------DdmMasterDriver-
//
STATUS DdmMasterDriver::Enable(Message *pMsgReq)
{
	TRACE_ENTRY(DdmMasterDriver::Enable(Message*));

	MessageControl();

	Reply(pMsgReq, OK);

	return OK;
}

void DdmMasterDriver::SubmitFirstAlarm()
{
	TRACE_ENTRY(DdmMasterDriver::SubmitFirstAlarm());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_CRITICAL, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter(1);
	p_Evt->SetSignificantParms(0xFFFFFFFE);
	pAlarmContext1 = new AlarmContext;
	pAlarmContext1->dummy = 33;
	SubmitAlarm(p_Evt, sizeof(AlarmContext), pAlarmContext1);
}

void DdmMasterDriver::SubmitCoalesceAlarmSigParam()
{
	TRACE_ENTRY(DdmMasterDriver::SubmitCoalesceAlarmSigParam());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_CRITICAL, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter(1);
	p_Evt->SetSignificantParms(0xFFFFFFFE);
	pAlarmContext1 = new AlarmContext;
	pAlarmContext1->dummy = 33;
	SubmitAlarm(p_Evt, sizeof(AlarmContext), pAlarmContext1);
}

void DdmMasterDriver::SubmitCoalesceAlarmNonSigParam()
{
	TRACE_ENTRY(DdmMasterDriver::SubmitCoalesceAlarmNonSigParam());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_CRITICAL, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter(3);
	p_Evt->SetSignificantParms(0xFFFFFFFE);
	pAlarmContext1 = new AlarmContext;
	pAlarmContext1->dummy = 33;
	SubmitAlarm(p_Evt, sizeof(AlarmContext), pAlarmContext1);
}

void DdmMasterDriver::SubmitUserRemittableAlarm()
{
	TRACE_ENTRY(DdmMasterDriver::SubmitUserRemittableAlarm());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_CRITICAL, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter("string");
	AlarmContext* pAlarmContext = new AlarmContext;
	pAlarmContext->dummy = 69;
	SubmitAlarm(p_Evt, sizeof(AlarmContext), pAlarmContext, TRUE);
}

void DdmMasterDriver::SubmitNonUserRemittableAlarm()
{
	TRACE_ENTRY(DdmMasterDriver::SubmitUserRemittableAlarm());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_CRITICAL, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter("list");
	AlarmContext* pAlarmContext = new AlarmContext;
	pAlarmContext->dummy = 69;
	SubmitAlarm(p_Evt, sizeof(AlarmContext), pAlarmContext);
}

void DdmMasterDriver::SubmitBigAlarmContext()
{
	TRACE_ENTRY(DdmMasterDriver::SubmitBigAlarmContext());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_OFFLINE, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter(4);
	p_Evt->AddEventParameter("tree");
	DummyContext* pDummyContext = new DummyContext;
	memset(pDummyContext, 1, sizeof(DummyContext));
	SubmitAlarm(p_Evt, sizeof(DummyContext), pDummyContext);
}

void DdmMasterDriver::SubmitSecondAlarm() 
{
	TRACE_ENTRY(DdmMasterDriver::SubmitSecondAlarm());

	Event* p_Evt = new Event(CTS_RMSTR_ARRAY_CRITICAL, this->GetVdn(), this->GetDid());
	p_Evt->AddEventParameter(2);
	AlarmContext* pAlarmContext = new AlarmContext;
	pAlarmContext->dummy = 66;
	SubmitAlarm(p_Evt, sizeof(AlarmContext), pAlarmContext);
}

void DdmMasterDriver::RemitFirstAlarm() 
{
	TRACE_ENTRY(DdmMasterDriver::RemitFirstAlarm());

	RemitAlarm(pAlarmContext1);
}

void DdmMasterDriver::DeletePAl()
{
	AlarmList* current;
	while (current = pALhead->GetNext())
	{
		delete pALhead->GetAlarmContext();
		delete pALhead;
		pALhead = current;
	}
	delete pALhead->GetAlarmContext();
	delete pALhead;
	pALhead = NULL;
}

void DdmMasterDriver::RecoverAllAlarms() 
{
	TRACE_ENTRY(DdmMasterDriver::RecoverAllAlarms());
	RecoverAlarms();
}

void DdmMasterDriver::RemitMostAlarms() 
{
	TRACE_ENTRY(DdmMasterDriver::RemitAllAlarms());
	AlarmList *pAL = pALhead;
	while (pAL->GetNext()->GetNext()) {
		RemitAlarm(pAL->GetAlarmContext());
		pAL = pAL->GetNext();
	}
}

void DdmMasterDriver::MessageControl()
{
	TRACE_ENTRY(DdmMasterDriver::MessageControl());

	static int i = 0;

	switch (i)
	{
	case 0:
	SubmitUserRemittableAlarm();
	SubmitNonUserRemittableAlarm();
	SubmitFirstAlarm();
	SubmitCoalesceAlarmSigParam();
	SubmitCoalesceAlarmNonSigParam();
	SubmitSecondAlarm();
	break;
	case 6:
	RemitFirstAlarm();
	break;
	case 7:
	SubmitFirstAlarm();
	RemitFirstAlarm();
	break;
	case 9:
	SubmitFirstAlarm();
	break;
	case 10:
	DeletePAl();
	RecoverAllAlarms();
	break;
	case 11:
	RemitMostAlarms();
	break;
	case 13:
	RecoverAllAlarms();
	break;
	case 14:
	SubmitBigAlarmContext();
	break;
	case 15:
	DeletePAl();
	RecoverAllAlarms();
	break;
	case 16:
	Tracef("\nDdmMasterDriver - DONE!!!\n");
	QuitTest();
	break;
	}
	i++;
}


void DdmMasterDriver::cbSubmitAlarm(void * pAlarmContext_, STATUS status_  ) 
{
	TRACE_ENTRY(DdmMasterDriver::cbSubmitAlarm(STATUS, void*));
	MessageControl();
}

void DdmMasterDriver::cbRemitAlarm( void * pAlarmContext_, STATUS status_ ) 
{
	TRACE_ENTRY(DdmMasterDriver::cbRemitAlarm(STATUS, void*));
	delete pAlarmContext_;
	MessageControl();
}

void DdmMasterDriver::cbRecoveryComplete( STATUS status_ ) 
{
	TRACE_ENTRY(DdmMasterDriver::cbRecoveryComplete(STATUS, void*));
	MessageControl();
}
