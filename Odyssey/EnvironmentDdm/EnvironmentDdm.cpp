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
// File: EnvironmentDdm.cpp
// 
// Description:
// 	This file is implementation for the EnvironmentDdm module. 
// 
// $Log: /Gemini/Odyssey/EnvironmentDdm/EnvironmentDdm.cpp $
// 
// 8     12/17/99 7:11p Vnguyen
// Fix problem with a keyswitch message is deleted then replied later.
// Also, make sure keyswitch position info is collected before a keyswitch
// field update to PTS.
// 
// 7     12/14/99 7:43p Jlane
// [ewx] Fixed cut&paste bug in message dispatching.
// 
// 6     12/13/99 7:25p Jlane
// [Changes/check-in by Vuong] Fix bug, change index to EvcRawParameters
// from 1,2 to 0,1 before sending to Reconciler.
// 
// 5     12/13/99 1:33p Vnguyen
// Update for Environment Ddm.
// 
// 4     11/17/99 5:53p Hdo
// Major change to the new model
// 
// 3     11/12/99 4:25p Hdo
// Change name to Environment Ddm
// 
// 2     9/08/99 8:15p Hdo
// Change to match with new SensorCode
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/9/99 Huy Do: Add CommandProcessor
// 
// 8/4/99 Huy Do: Create file
/*************************************************************************/

#include "BuildSys.h"
#include "RequestCodes.h"
#include "CtEvent.h"

#include "Trace_Index.h"
//#define TRACE_INDEX TRACE_ENVIRONMENT
#include "Odyssey_Trace.h"

#include "EnvironmentDdm.h"

#include "DdmCmbMsgs.h"

CLASSNAME(EnvironmentDdm, SINGLE);	// Class Link Name used by Buildsys.cpp

SERVELOCAL( EnvironmentDdm, ENV_POLL_DATA);
SERVELOCAL( EnvironmentDdm, ENV_ACTIVATE);
SERVELOCAL( EnvironmentDdm, ENV_DEACTIVATE);
SERVELOCAL( EnvironmentDdm, ENV_RUN_DIAGNOSTIC);
SERVELOCAL( EnvironmentDdm, ENV_KEYSWITCH_CHANGE_STATE);
SERVELOCAL( EnvironmentDdm, ENV_POWER_EVNT);

#define SAMPLERATE	(15000000)			// Sample rate is 15 second

// EnvironmentDdm - constructor method
//
//
EnvironmentDdm::EnvironmentDdm(DID did) : Ddm( did )
{
	TRACE_ENTRY(EnvironmentDdm::EnvironmentDdm);

	m_pStartTimerMsg = NULL;
	m_pStopTimerMsg = NULL;

	m_pDefineTable = NULL;
	m_pInsertRow = NULL;
	m_pReadRow = NULL;
	m_pModifyRow = NULL;
	memset(&m_rid, 0, sizeof(rowID));
	bStarted = FALSE;
	m_p_EVC_poll_msg = NULL;
	fFirstKeyswitch = FALSE;
	//m_psKeyswitchPayload = (MsgEnvKeyswitch::Payload *)new MsgEnvKeyswitch::Payload();
}

// ~EnvironmentDdm - destructor method
//
//
EnvironmentDdm::~EnvironmentDdm()
{
	TRACE_ENTRY(EnvironmentDdm::~EnvironmentDdm);
}

// Ctor - create a new instance of the EnvironmentDdm Ddm
//
//
Ddm *EnvironmentDdm::Ctor(DID did)
{
	TRACE_ENTRY(EnvironmentDdm::Ctor);

	return (new EnvironmentDdm(did));
}

// Initialize - called after the constructor
// Call the TableOperation to start a state machine for EVCStatusTable.
// After the TableInitialize, there will be one entry in the table either
// with the old data or a dummy record.
// Initialize all sensors' state.
// 
// Inputs
// 
// Outputs
//
STATUS EnvironmentDdm::Initialize(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::Initialize);

	DispatchRequest(ENV_POLL_DATA, (RequestCallback) &HandleStartupMsg);
	DispatchRequest(ENV_ACTIVATE, (RequestCallback) &HandleActivateMsg);
	DispatchRequest(ENV_DEACTIVATE, (RequestCallback) &HandleDeActivateMsg);
	DispatchRequest(ENV_RUN_DIAGNOSTIC, (RequestCallback) &HandleDiagnosticMsg);
	DispatchRequest(ENV_KEYSWITCH_CHANGE_STATE, (RequestCallback) &HandleKeySwitchMsg);
	DispatchRequest(ENV_POWER_EVNT, (RequestCallback) &HandlePowerEventMsg);

	// Start the state machine for EVCStatusTable
	EVCStatusTableInitialize(pMsg);

	// Initialize all sensor state by calling the CMB, after getting the
	// data, update the sensor state accordingly
	m_EventHandler.InitSensorState();

	return CTS_SUCCESS;
}

// Enable - called at enable time as well as when the quiesce is over
// If it's called after the quiesce, we need to start a new timer and
// set the bStarted flag
STATUS EnvironmentDdm::Enable(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::Enable);

	// This is this message after quiesce
	if ( m_pStopTimerMsg )
	{
		// Set the bStarted flag to enable the polling
		bStarted = TRUE;

		// Start the timer again
		m_pStartTimerMsg = new RqOsTimerStart(SAMPLERATE, SAMPLERATE);

		// Send the message off to the timer service.
		STATUS status = Send(m_pStartTimerMsg, this, (ReplyCallback)&HandleTimerSvc);

		if ( status )
		{
			TRACEF(TRACE_L8, "Cannot register RqOSTimerStart.\n");
			delete m_pStartTimerMsg;
			m_pStartTimerMsg = NULL;

			Reply(pMsg, status);

			return status;
		}
	}

	Reply(pMsg, CTS_SUCCESS);

	return CTS_SUCCESS;
}


// Quiesce - Enter the quiesce state
// Stop the timer and clear the bStarted flag
//
STATUS EnvironmentDdm::Quiesce(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::Quiesce);

	// Clear the bStarted to disable the polling
	bStarted = FALSE;

	// Stop the timer
	m_pStopTimerMsg = new RqOsTimerStop(m_pStartTimerMsg);
	STATUS status = Send(m_pStopTimerMsg, &DiscardReply);

	if ( status )
	{
		TRACEF(TRACE_L8, "Cannot register RqOSTimerStop.\n");
		delete m_pStartTimerMsg;
		m_pStartTimerMsg = NULL;
	}

	Reply(pMsg, status);

	return status;
}


// HandleStartupMsg
// Call the CMB to update the EVC record with no exception handling
// 
// Inputs
// 
// Outputs
//
//
STATUS EnvironmentDdm::HandleStartupMsg(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleStartupMsg);

	// TODO: Check the battery for operational status (80% charge level)

	// TODO: Run diagnostic to check for all the sensors
	// Also check the config record

	// Everything is in place, now it's time to start the process
	// Get raw data from the CMB and start the table operation state machine
	PollEVCData();

	Reply(pMsg, OK);

	return CTS_SUCCESS;
}


// HandleActivateMsg
// Register a new timer.
// Call the CMB to update the EVC record
// 
// Inputs
// 
// Outputs
//
//
STATUS EnvironmentDdm::HandleActivateMsg(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleActivateMsg);

	// If the timer started return right away
	if ( bStarted )
		return CTS_SUCCESS;

	// Allocate a message we'll send to the timer service to start a timer.
	// Initialize the payload structure for the start timer message with the
	// specified sample rate.
TRACEF(TRACE_L8, "Registering the timer.\n");
	m_pStartTimerMsg = new RqOsTimerStart(SAMPLERATE, SAMPLERATE);

	// Send the message off to the timer service.
	STATUS status = Send(m_pStartTimerMsg, this, (ReplyCallback)&HandleTimerSvc);
	if ( status )
	{
TRACEF(TRACE_L8, "Cannot register RqOSTimerStart.\n");
		delete m_pStartTimerMsg;
		m_pStartTimerMsg = NULL;

		return status;
	}

	bStarted = TRUE;

	// Get raw data from the CMB and start the process
	PollEVCData();

	Reply(pMsg, OK);

	return status;
}


// HandleTimerSvc
// Periodic message from the Timer service
// It will call the CMB to get the data and update the EVCStatus table
// 
// Inputs
// 
// Outputs
//
STATUS EnvironmentDdm::HandleTimerSvc(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleTimerSvc);

	// Always delete the message.
	delete pMsg;

	// If the timer has not started, return right away
	if ( bStarted )
		// Get raw data from the CMB
		PollEVCData();

	return CTS_SUCCESS;
}

// PollEVCData
// Create a new CMB_POLL message and send it to DdmCMB. Set the callback
// handler to UpdateEVCRecord
// 
// Inputs
// 
// Outputs
//
void EnvironmentDdm::PollEVCData()
{
	TRACE_ENTRY(EnvironmentDdm::PollEVCData);

	m_p_EVC_poll_msg = new Message(CMB_POLL_ENVIRONMENT);

	Send(m_p_EVC_poll_msg, (ReplyCallback)&UpdateEVCRecord );
}

// UpdateEVCRecord
// Update EVCStatusRecord by polling the CMB for raw data then reconcile
// it. The raw data is send back in the CPayload class.
// bStarted is the flag that indicate if it needs to act on exception.
// 
// Inputs
// 
// Outputs
//
STATUS	EnvironmentDdm::UpdateEVCRecord(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::UpdateEVCRecord);

	if ( !pMsg->IsReply() )
	{
		delete pMsg;
		return CTS_MSG_NOT_IMPLEMENTED;
	}

	MsgCmbPollEnvironment *p_EVC_Poll_Data = (MsgCmbPollEnvironment *)pMsg;
	MsgCmbPollEnvironment::CPayload *pPayload;

	// Get raw data from the payload
	p_EVC_Poll_Data->GetPayload(pPayload);

	// Reconcile data here
	//m_Reconciler.Set48V();
	m_Reconciler.SetRawData(pPayload->EVCRawParameters[0], pPayload->EVCRawParameters[1]);
	m_Reconciler.GetDistilledData(&m_EVCStatusRecord, &m_EVC_Sensor_Bitmap);

	// If the activte message has not been received then it won't act
	// on exception
	if ( bStarted )
		m_EventHandler.CheckEVCStatus(m_EVCStatusRecord, m_EVC_Sensor_Bitmap);

	// Update the EVCStatus record
	EVCTable_ModifyRow();

	delete pMsg;

	return CTS_SUCCESS;
}

// HandleDeActivateMsg
// This is the handler method for DEACTIVATE message.
// It will send a RqTimerStop and clear the bStarted flag.
// 
// Inputs
// 
// Outputs
//
STATUS EnvironmentDdm::HandleDeActivateMsg(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleDeActivateMsg);

	return CTS_SUCCESS;
}

// HandleDiagnosticMsg
// This is the handler for the DIAGNOSTIC message. This message is sent
// from the SSAPI to run test on all sensors
// It starts exercising all the sensors and report back the sensor's status
// 
// Inputs
// 
// Outputs
//
STATUS EnvironmentDdm::HandleDiagnosticMsg(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleDiagnosticMsg);

	return CTS_SUCCESS;
}

// HandleKeySwitchMsg
// This is the handler for the async message from lower level to inform
// about the state's change in keyswitch position
// 
// Inputs
// 
// Outputs
//
STATUS EnvironmentDdm::HandleKeySwitchMsg(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleKeySwitchMsg);

	MsgEnvKeyswitch		*pKeyswitchMsg = (MsgEnvKeyswitch *)pMsg;

	// Get the EVC that report the event
	STATUS	status;
	U32		ulTargetEVC = pKeyswitchMsg->m_Payload.ulTargetEVC;

	if ( fFirstKeyswitch )
	{
		// This is the second keyswitch message, forward to the
		// HandleKeyswitchTimerReply routine
		// First set the fFirstKeyswitch
		//fFirstKeyswitch = FALSE;
		
		// ~~VN Change begins
		// status = HandleKeyswitchTimerReply(pMsg);
		// This is a short term fix to eliminate the condition in which
		// a message is deleted in the HandleKeyswitchTimerReply routine
		// and then replied later (see below).
		// We need to revisit how the keyswitch messages are being
		// handled to take care of the outstanding timer.
		
		// Get the new keyswitch position
		m_eKeyswitchPosition = pKeyswitchMsg->m_Payload.ePosition;
		fFirstKeyswitch = FALSE;
		status = EVCTable_UpdateKeyswitch(pMsg);
		// ~~VN Change ends.
		
	}
	else
	{
		// Validate with the local EVCStatusRecord
		if ( m_EVCStatusRecord.afEvcReachable[ulTargetEVC] )
		{
			U32 ulReverseTargetEVC = ulTargetEVC? 0: 1;

			if ( m_EVCStatusRecord.afEvcReachable[ulReverseTargetEVC] )
			{
				fFirstKeyswitch = TRUE;
				// Start the timer for 1 second, with only one reply
				m_pKeyswitchStartTimerMsg = new RqOsTimerStart(1000000, 0);

				// Send the message off to the timer service.
				status = Send(m_pKeyswitchStartTimerMsg, this, 
						(ReplyCallback)&HandleKeyswitchTimerReply);
				if ( status )
				{
					delete m_pKeyswitchStartTimerMsg;
					m_pKeyswitchStartTimerMsg = NULL;
				}
			}
			else
			{
				// If only one EVC is available then we don't need to wait for
				// the second message
				fFirstKeyswitch = FALSE;
				// Update the table
				m_eKeyswitchPosition = pKeyswitchMsg->m_Payload.ePosition;
				status = EVCTable_UpdateKeyswitch(pMsg);
			}
		}
		else
		{
			// Should not happend, this is BAD
		}
	}

	Reply(pMsg, status);

	return CTS_SUCCESS;
}

// HandleKeyswitchTimerReply
// 
// 
// Inputs
// 
// Outputs
//
// 
STATUS EnvironmentDdm::HandleKeyswitchTimerReply(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandleKeyswitchTimerReply);
	STATUS	status;

	switch ( pMsg->reqCode )
	{
	case ENV_KEYSWITCH_CHANGE_STATE:	// This is the second keyswitch message
		if ( fFirstKeyswitch )
		{
			// Update table with the new keyswitch
			status = EVCTable_UpdateKeyswitch(pMsg);
			fFirstKeyswitch = FALSE;
		}
		else	// This should not happen
		{
			// TODO: log the event
		}
		break;

	case REQ_OS_TIMER_START:	// This is the timer message
		if ( fFirstKeyswitch )
		{
			// The second message doesn't arrive, start query the CMB
			// TODO: Eric's Msg class to query keyswitch
		}
		else
		{
			// TODO: log the event
			fFirstKeyswitch = FALSE;
		}
	}

	// Always delete the message.
	delete pMsg;

	return CTS_SUCCESS;
}

// HandleKeySwitchMsg
// This is the handler for the async message from lower level to inform
// about the power event
// 
// Inputs
// 
// Outputs
//
STATUS EnvironmentDdm::HandlePowerEventMsg(Message *pMsg)
{
	TRACE_ENTRY(EnvironmentDdm::HandlePowerEventMsg);

	MsgEnvPowerEvent	*pPowerEventMsg = (MsgEnvPowerEvent *)pMsg;

	// Get the EVC that report the event
	U32		ulTargetEVC = pPowerEventMsg->m_Payload.ulTargetEVC;

	// Validate with the local EVCStatusRecord
	if ( m_EVCStatusRecord.afEvcReachable[ulTargetEVC] )
	{
		U32 ulReverseTargetEVC = ulTargetEVC? 0: 1;

		// If only one EVC is available then we don't need to wait for
		// the second message
		if ( m_EVCStatusRecord.afEvcReachable[ulReverseTargetEVC] )
		{
		}
		else
		{
			// Update the table
		}
	}
	else
	{
		// Should not happend, this is BAD
	}

	return CTS_SUCCESS;
}
