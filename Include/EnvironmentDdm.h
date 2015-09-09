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
// File: EnvironmentDdm.h
// 
// Description:
// 	This file is interface for the EnvironmentDdm module. 
// 
// $Log: /Gemini/Include/EnvironmentDdm.h $
// 
// 6     12/13/99 1:32p Vnguyen
// Update for Environment Ddm.
// 
// 5     11/17/99 5:53p Hdo
// Major change to the new model
// 
// 4     11/12/99 4:24p Hdo
// 
// 3     9/08/99 8:15p Hdo
// Change to match with new SensorCode
// 
// 2     8/25/99 5:01p Hdo
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/9/99 Huy Do: Add the XM_CMD enum type, CommandProcessor()
// 
// 8/4/99 Huy Do: Create file
/*************************************************************************/

#ifndef __EnvironmentDdm_H__
#define __EnvironmentDdm_H__

#include "RqOsTimer.h"
#include "Message.h"
#include "Ddm.h"

#include "Table.h"
#include "Rows.h"
#include "Fields.h"

#include "EVCStatusRecord.h"
#include "EnvDdmMsgs.h"

#include "Reconciler.h"
#include "EventHandler.h"

class EnvironmentDdm : public Ddm {
public:
	EnvironmentDdm(DID did);
	~EnvironmentDdm();
	static	Ddm *Ctor(DID did);
	STATUS	Initialize(Message *pMsg);
	STATUS	Enable(Message *pMsg);
	STATUS	Quiesce(Message *pMsg);

	// The hanlder for the ENV_POLL_DATA request
	STATUS	HandleStartupMsg(Message *pMsg);

	// The hanlder for the ENV_ACTIVATE request
	STATUS	HandleActivateMsg(Message *pMsg);

	// The hanlder for the asynchronous keyswitch event
	STATUS	HandleKeySwitchMsg(Message *pMsg);

	// The hanlder for the Deactivate message
	STATUS	HandleDeActivateMsg(Message *pMsg);

	// The hanlder for the diagnostic message
	STATUS	HandleDiagnosticMsg(Message *pMsg);

	// The hanlder for the asynchronous power event
	STATUS	HandlePowerEventMsg(Message *pMsg);

private:
	// Handle for the timer service reply
	STATUS	HandleTimerSvc(Message *pMsg);
	STATUS	HandleKeyswitchTimerReply(Message *pMsg);

	// The helper method for sending request to the CMB
	void	PollEVCData();

	// The EVCStatusTable helper methods during initialize
	void	EVCStatusTableInitialize(Message *pMsg);
	STATUS	CreateTableReply(void *pContext, STATUS status);
	STATUS	EVCTable_InsertRow(Message *pMsg);
	STATUS	Handle_InsertRow_Reply(void *pContext, STATUS status);
	STATUS	ReadEVCRecord(Message *pMsg);
	STATUS	Handle_ReadEVC_Reply(void *pContext, STATUS status);

	// The EVCStatusTable helper methods in normal operation
	STATUS	UpdateEVCRecord(Message *pMsg);
	STATUS	EVCTable_ModifyRow();
	STATUS	Handle_ModifyRow_Reply(void *pContext, STATUS status);
	STATUS	EVCTable_UpdateKeyswitch(Message *pMsg);


	// All of the TS objects needed to oprate the table
	TSDefineTable	*m_pDefineTable;
	TSInsertRow		*m_pInsertRow;
	TSReadRow		*m_pReadRow;
	TSModifyRow		*m_pModifyRow;
	TSModifyField	*m_pModifyField;


	// Pointer to the timer message for the periodic updating table
	RqOsTimerStart	*m_pStartTimerMsg;
	RqOsTimerStop	*m_pStopTimerMsg;

	// Pointer to the timer message for the keyswitch
	RqOsTimerStart	*m_pKeyswitchStartTimerMsg;
	//RqOsTimerStop	*m_pKeyswitchStopTimerMsg;
	BOOL			fFirstKeyswitch;

	U32				m_ulTargetEVC;
	CT_EVC_KEYPOS	m_eKeyswitchPosition;

	// Pointer to the CMB request message
	Message			*m_p_EVC_poll_msg;

	// RowID of the EVCStatus record
	rowID			m_rid;

	// A flag indicated that we received the ENV_ACTIVATE
	BOOL			bStarted;

	// Our local copy of the EVCStatus record
	EVCStatusRecord m_EVCStatusRecord;

	// The object responsible for the reconcile data
	Reconciler		m_Reconciler;

	// EVC Event handler object
	EventHandler	m_EventHandler;

	// A bitmap for the sensor state returned from the voting algorithm
	// This will be used to start the exception sequence
	I64				m_EVC_Sensor_Bitmap;
} ;

#endif // __EnvironmentDdm_H__
