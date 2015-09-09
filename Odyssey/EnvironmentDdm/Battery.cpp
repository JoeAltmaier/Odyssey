/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: Battery.cpp
// 
// Description:
// 	This file is implementation for the Battery module. 
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/Battery.cpp $
// 
// 2     9/08/99 8:14p Hdo
// Integration with AlarmMaster
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/4/99 Huy Do: Create file
/*************************************************************************/

#include "Battery.h"

// performTask - 
//
//
STATUS Battery::performTask(XM_CONTEXT	*pXMContext)
{
	TRACE_ENTRY(Battery::performTask);
	STATUS status;

	switch( pXMContext->Level )
	{
	case NORMAL:
		if( state == NORMAL )
			break;
		// The alarm event wentback to normal.

		// Log the event
		p_Event = new Event(Event::ALARM_EVT, m_ParentVdn, m_ParentDid);
		pAlamrContext->Data = *pXMContext;
		status = SubmitAlarm(p_Event, sizeof(AlarmContext), pAlamrContext);

		// Remit that alarm
		state = NORMAL;
		break;
	case WARNING:
		if( state == WARNING )
			break;
		// Check if this is the first time

		// If it's the second time, is the value change?

		// Log the event
		p_Event = new Event(Event::ALARM_EVT, m_ParentVdn, m_ParentDid);
		pAlamrContext->Data = *pXMContext;
		status = SubmitAlarm(p_Event, sizeof(AlarmContext), pAlamrContext);

		// Raise the Alarm
		state = WARNING;
		break;
	case ALARM:
		// Posible Policy: how many time the event occur that I should make a big
		// deal and do something dramastic?

		// Log the event
		p_Event = new Event(Event::ALARM_EVT, m_ParentVdn, m_ParentDid);
		pAlamrContext->Data = *pXMContext;
		status = SubmitAlarm(p_Event, sizeof(AlarmContext), pAlamrContext);
		// Doesn't matter it's the 1st, or second time, submit an alarm right away
		state = ALARM;
		break;
	}

	return status;
}

// Charge - Charge the battery
//
//
void Battery::Charge()
{
	TRACE_ENTRY(Battery::Charge);
	// Do nothing now
}