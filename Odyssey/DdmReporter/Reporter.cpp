/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Reporter.cpp
// 
// Description:
// This file implements the base class for specific Table data reporters. 
// 
// Update Log 
//
// $Log: /Gemini/Odyssey/DdmReporter/Reporter.cpp $
// 
// 19    12/15/99 2:04p Vnguyen
// Move Tracef.  Also increment key counter for each update.  This way we
// can tell when an update just happens and see if the info is propogated
// to GMI GUI or not.
// 
// 18    12/14/99 10:30a Vnguyen
// Remove Tracef.
// 
// 17    12/02/99 1:30p Vnguyen
// Skip EVC Reporter stuffs for now.  When the Environment HDM is ready,
// we can completely remove all traces of EVC Reporter.
// 
// 16    11/17/99 12:50p Vnguyen
// Rearrange time state to skip the timer tick if the Ddm has not
// responded with data yet.
// 
// 15    11/06/99 4:35p Vnguyen
// Temporarily disable CMB Ddm call for now, it is having a lot of assert
// due to temperature beyond thresholds.
// 
// 14    11/06/99 3:14p Vnguyen
// Fix bug, was calling DDM via call by vdn.  Change to call by Did
// because the CMB Ddm does not have a valid vdn.  Also add a few trace
// statements.
// 
// 13    9/01/99 4:47p Vnguyen
// Remove include file ReporterMsgs.h because the define in that file has
// been moved to RqDdmReporter.h
// 
// 12    8/30/99 8:41a Vnguyen
// Add DDM_REPLY_DATA_SGI for AddSgl call.  No impact to other DDM.  They
// can use the new define by including file ../Include/ReporterMsgs.h
// 
// 11    8/23/99 4:51p Vnguyen
// Separate quiesce and pause states as it is possible for the two
// states to overlap.  Quiesce is global to all reporters.  Pause is local
// to one reporter.
// 
// 10    8/17/99 8:31a Vnguyen
// Update TSModifyRow call to accept more parameters.
// 
// 9     8/16/99 1:00p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 8     8/05/99 10:56a Vnguyen
// 
// 6	8/05/99	10:05a Vnguyen -- Add support for ResetReporter and StopReporter
//	Delete TimerService for restart method.  Add support for PauseReporter
// 
// 5     5/19/99 9:02a Jlane
// Miscellaneous changes and bug fixes made during bringup.
// 
// 4     5/13/99 11:38a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 3     5/05/99 10:02a Jlane
// Pass pb/cb data in Handle DdmSampleData instead of pMsg.  make DdmData
// an Sgl item instead of payload.
//
// 10/29/98	JFL	Created.  The day of our first Halloween party.
//
/*************************************************************************/

#include <String.h>
#include "Reporter.h"
#include "RequestCodes.h"
#include "Odyssey_Trace.h"


/**********************************************************************/
// Reporter - This is the contstructor for the Reporter Object.
// Derived classes must call it in their constructors.
/**********************************************************************/
Reporter::Reporter()
{
	m_State = UnInitialized;
}




/**********************************************************************/
// Reporter - This is the destructor for the Reporter Object.
// Derived classes must call it in their destructors.
/**********************************************************************/
Reporter::~Reporter()
{
	// We get here via a delete this from the Stop process.
	// All resources, including timer service, have been deallocated.
	// There is nothing left to do.
	
	// Important:  No one should issue a delete object.  The correct way
	// to stop a Reporter object (and delete) is to issue a PHS_STOP
	// (via call by request code) wih the correct DID and REPORTERCODE.
	// The DdmReporter will issue a stop method to the base Reporter which
	// then will deallocate all resources.  At the end of the stop process,
	// the object kills itself via a delete this.
}


/**********************************************************************/
// InitReporter - This is the initializor for the Reporter Object.
/**********************************************************************/
STATUS Reporter::InitReporter
( 

	String64		rgbTableName, 	// The name of the table to refresh.
	rowID			ridTableRow,	// Row ID of table row to refresh.
	U32				RefreshRate,	// The rate at which to refresh the table.
	U32				SampleRate		// The rate at which to sample the Ddm.
)
							  
{

// Tracef("Reporter::InitReporter reset message %d, return data msg %d, SR = %d, RR = %d\n",
//		m_ResetDataMsg, m_ReturnDataMsg, SampleRate, RefreshRate); 

 	// Initialize member variables.
 	strcpy( m_rgbTableName, rgbTableName );
	m_SampleRate = SampleRate; 
	m_RefreshRate = RefreshRate; 
	// Reset m_RefreshCount;
	// This is why refresh rate shoulds be a multiple of sample rate.
	m_RefreshCount = m_RefreshRate / m_SampleRate;
	m_ridTableRow = ridTableRow;
	m_fReStart = false;
	m_pause	= false;
	m_quiesce = false;
	m_StopPending = false;	// So the timer service can initiate stop
	m_SkipOneSample = false;
			
	// Enter the machine.
	return Start();
}


/**********************************************************************/
// Start - The entry point into the Reporter Object state machine.
/**********************************************************************/
STATUS Reporter::Start()
{
STATUS	status;

	// Allocate a message we'll send to the DDM to reset data.
	Message *pDDMResetMsg = new Message(m_ResetDataMsg);
	// Send the message off to the Ddm.
	status = Send(m_didDdm, pDDMResetMsg, (ReplyCallback)&HandleDdmResetReply);
	
	// Check the status from the send.
	if (status != OS_DETAIL_STATUS_SUCCESS)
	{
		// If there was any error delete the message and
		delete pDDMResetMsg;
		
		// Set our state to UnInitialized.
		m_State = UnInitialized;
	}
	else
		// Set our state to WaitingForDDMReset.
		m_State = WaitingForDDMReset;

	// Return our status.
//Tracef("Reporter::Start m_state is %d\n", m_State);
	return status;
}


/**********************************************************************/
// Reporter - This is the ReInitializor for the Reporter Object.
/**********************************************************************/
STATUS Reporter::ReInitReporter( 
	U32				RefreshRate,	// The rate at which to refresh the table.
	U32				SampleRate		// The rate at which to sample the DDM.
)
{

Tracef("Reporter::ReInitReporter\n");

	// This will flag the next transition to abort and restart.
	m_fReStart = true;
	
	// Save the new sample and refresh rates.
	m_NewRefreshRate = RefreshRate;
	m_NewSampleRate = SampleRate;
	
	// Return the status.
	return OS_DETAIL_STATUS_SUCCESS;
}


/**********************************************************************/
// ReStart - 	Used to complete reinitialization.
/**********************************************************************/
STATUS Reporter::ReStart()
{
Tracef("Reporter::ReStart\n");

	// Set saved values into refresh and sample rate member variables.
	m_RefreshRate = m_NewRefreshRate;
	m_SampleRate = m_NewSampleRate;

	// This is why refresh rate shoulds be a multiple of sample rate.
	m_RefreshCount = m_SampleRate / m_RefreshRate;
	
	// Make sure we clear the m_fReStart flag.
	m_fReStart = false;


	// To change the reporting interval, we need to clear all counters and
	// start over.  Also, need to update the timer parameters.
	
	// We need to change (reset) the timeout values for the timer

Tracef("Reporter::ReStart  About to register timer, Sample rate = %d, RR = %d\n", 
        m_SampleRate, m_RefreshRate);	
        
	RqOsTimerReset *pTimerMsg = new RqOsTimerReset(m_pStartTimerMsg, m_SampleRate, m_SampleRate);
	Send(pTimerMsg, &DiscardReply);


	// Set our state to WaitingForTimerSvc.
	m_State = WaitingForTimerSvc;
	
	// Reset derived object counters
	ResetCounters();

	return OK;

}


/**********************************************************************/
// PauseReporter - 	Pause reporter by setting m_pause flag
/**********************************************************************/
STATUS Reporter::PauseReporter()
{

Tracef("Reporter::PauseReporter\n");
	m_pause = true;
	return OK;
}

/**********************************************************************/
// ContinueReporter - 	
/**********************************************************************/
STATUS Reporter::ContinueReporter()
{

Tracef("Reporter::ContinueReporter\n");

	// This flag is to skip one sampling data before resuming collection.
	// The purpose is to align the collection period to one sampling interval.
	// This flag is set when the Reporter is exitting pause or quiesce state.
	// And it is cleared by the timer service
	m_SkipOneSample = true; 

	m_pause = false;
		
	return OK;
}


/**********************************************************************/
// QuiesceReporter - 	Quiesce reporter by setting m_quiesce flag
/**********************************************************************/
STATUS Reporter::QuiesceReporter()
{

Tracef("Reporter::QuiesceReporter\n");
	m_quiesce = true;
	return OK;
}

/**********************************************************************/
// EnableReporter - 	
/**********************************************************************/
STATUS Reporter::EnableReporter()
{
Tracef("Reporter::EnableReporter\n");

	// This flag is to skip one sampling data before resuming collection.
	// The purpose is to align the collection period to one sampling interval.
	// This flag is set when the Reporter is exitting pause or quiesce state.
	// And it is cleared by the timer service
	m_SkipOneSample = true; 

	m_quiesce = false;
		
	return OK;
}





/**********************************************************************/
// ResetReporter - 	Important:  This method is for resetting performance
// counters only.  Its effect is similar to just starting the reporter.
// To reset status counters, we need another method to communicate with
// the Ddm.  Note:  The PHS_RESET code will not be issued to the Ddm
// again.  The PHS_RESET code is only issued once when the Reporter
// is started with PHS_START.
/**********************************************************************/
STATUS Reporter::ResetReporter()
{
Tracef("Reporter::ResetReporter\n");

    return ReInitReporter(m_RefreshRate, m_SampleRate); 
}


/**********************************************************************/
// StopReporter - Stop the Reporter and deallocate allresources.  Also,
// perform a delete this.
/**********************************************************************/
STATUS Reporter::StopReporter()
{
Tracef("Reporter::StopReporter\n");

	m_StopPending = true;
	return OK;
}


/**********************************************************************/
// DoStopReporter - Used to complete reinitialization.
// Note:  The derived class can hook in here to do their own clean up
// if needed.  But they need to call this routine at the end.
// e.g.  Reporter::DoStopReporter();
/**********************************************************************/
STATUS Reporter::DoStopReporter()
{
Tracef("Reporter::DoStopReporter\n");

	// We need to stop the Timer Service
	RqOsTimerStop *pTimerMsg = new RqOsTimerStop(m_pStartTimerMsg);
	Send(pTimerMsg, &DiscardReply);


	// Take care of any pending Listen, too.
	
	

	delete this;  // must be the last thing before return
						
	return OK;
}


/**********************************************************************/
// HandleDdmResetReply - Throw away the reply from the DDM, change state 
// to WaitingForTimerSvc and fire off the timer service.
/**********************************************************************/
STATUS Reporter::HandleDdmResetReply(Message *pMsg)
{
STATUS	status = pMsg->DetailedStatusCode;

//Tracef("Reporter::HandleDdmResetReply\n");

	// Always delete the message.
	delete pMsg;
	
	// Verify that we're in the right state.
	if (m_State != WaitingForDDMReset) {
		Tracef("Reporter::HandleDdmResetReply():  m_State is not WaitingForDDMReset.\n");
		return 1;			// TODO: need an error code here!!!!!!!!
	}


	// Reset derived object counters
	ResetCounters();

	// Allocate a message we'll send to the timer service to start a timer.
	// Initialize the payload structure for the start timer message with the
	// specified sample rate. 
//Tracef("Reporter::Start  About to register timer, Sample rate = %d, RR = %d\n", 
//        m_SampleRate, m_RefreshRate);	
	m_pStartTimerMsg = new RqOsTimerStart(m_SampleRate, m_SampleRate);

	// Set our state to WaitingForTimerSvc.
	m_State = WaitingForTimerSvc;
	
	// Send the message off to the timer service.
	status = Send(m_pStartTimerMsg, this, (ReplyCallback)&HandleTimerSvcReply);
	if (status)
	{
		Tracef("Reporter::HandleDdmResetReply():  Cannot register RqOSTimerStart.\n");
		delete m_pStartTimerMsg;
		m_pStartTimerMsg = NULL;
	}
	
	return status;
}


/**********************************************************************/
// HandleTimerSvcReply - Handle the fact that the timer service has
// replied and told us it's time to refresh table data.  This code
// just sends a message to the DDM requesting data
/**********************************************************************/
STATUS Reporter::HandleTimerSvcReply(Message *pMsg)
{
STATUS	status = pMsg->DetailedStatusCode;
//Tracef("Reporter::HandleTimerSvcReply, State = %d, SRate = %d, RRate = %d\n", 
//		m_State, m_SampleRate, m_RefreshRate);

	// Always delete the message.
	delete pMsg;

	// for now we skip the EVC Status Reporter because the Environment HDM is
	// supposed to handle EVC stuffs.  We will remove support for EVC when the
	// envrionment HDM is ready.
	if (m_ReporterCode == PHS_EVC_STATUS)
		return OK; //~~
		
	// Check for quiesce here.  For now we skip the timer tick.  
	// Important:  We need to check for quiesce before anything else.  Otherwise, we
	// may send a timer message (or stop) during a quiesce.
	if	(m_quiesce)
		return OK;

	// Check for stop.  need to check for Stop before pause.  This allows Stop message
	// to go through even when the Reporter is in pause state.
	if (m_StopPending)
		return DoStopReporter();

	// Check for pause here.  For now we skip the timer tick.  
	if	(m_pause)
		return OK;
	
	// Finally, check for ReStart (to change Refresh/Sample rates).
	if (m_fReStart)
		return ReStart();
			
	// Verify that we're in the right state.
	if (m_State != WaitingForTimerSvc) {
		// Tracef("Reporter::HandleTimerSvcReply():  m_State is %d (4 means waiting for PTS write.\n", m_State);
		return OK;			// TODO: need an error code here!!!!!!!!
	}
	// Allocate a message we'll send to the DDM to get data.
	Message *pDdmRequestMsg = new Message(m_ReturnDataMsg);

	// Add the return Table data SGL item
	// NULL pb indicates data buffer to be alloc'd by transport.
	// NUll cb will tell transport to get allocate size when the
	// PTS calls GetSGL.  This requires SGL_DYNAMIC_REPLY too.
	pDdmRequestMsg->AddSgl(DDM_REPLY_DATA_SGI, NULL, NULL, SGL_DYNAMIC_REPLY);

	// Send the message off to the DDM.
	status = Send(m_didDdm, pDdmRequestMsg, (ReplyCallback)&HandleDdmDataReply);
	if (status)
	{
		delete pDdmRequestMsg;
		m_State = WaitingForTimerSvc;
	}
	else	
		// Set our state to WaitingForDDMData.
		m_State = WaitingForDDMData;
	return OK;
}


/**********************************************************************/
// HandleDdmDataReply - This method although overridden by the derived
// classes MUST be called by them at their exit.  This code will delete
// the message and set the next state.  Unless they do it themselves.
/**********************************************************************/
STATUS Reporter::HandleDdmDataReply(Message *pMsg)
{
void*	pDdmReplyData;
U32		cbDdmReplyData;
STATUS	status = pMsg->DetailedStatusCode;
	
//Tracef("Reporter::HandleDdmDataReply\n");

	m_State = WaitingForTimerSvc;
	
	// If there's an error abort???
	if (status != OS_DETAIL_STATUS_SUCCESS){
		delete pMsg;
		return status;
	}
	
	if (m_SkipOneSample)
	{	// This flag is to skip one sampling data before resuming collection.
		// The purpose is to align the collection period to one sampling interval.
		// This flag is set when the Reporter is exiting pause or quiesce states.
		// And it is cleared by the timer service (here).
		m_SkipOneSample = false;
		// delete the message.
		delete pMsg;
		return OK;
	}
	
	// Get the reply SGL item pointer and size in our local variables.
	pMsg->GetSgl(DDM_REPLY_DATA_SGI, &pDdmReplyData, &cbDdmReplyData );
	
	// Call the derived class to do whatever it wants with the data.
	status = HandleDdmSampleData( pDdmReplyData, cbDdmReplyData );

	// delete the message.
	delete pMsg;

	// If the refresh counter has expired send refresh data to the PTS.
	if (--m_RefreshCount == 0)
	{
		void*	pRecordRefreshData;
		U32		cbRecordRefreshData;
		
		// Call the derived class to compute PTS Update data pb/cb.
		status = ReturnTableRefreshData( pRecordRefreshData, cbRecordRefreshData );
		
		// If there was no error send the data off to the table service.
		if (status == OS_DETAIL_STATUS_SUCCESS)
		{
			m_pModifyMsg = new TSModifyRow;
			
			status = m_pModifyMsg->Initialize(	
				this,								// DdmServices		*pDdmServices,
				m_rgbTableName,						// String64		rgbTableName,
				CT_PTS_RID_FIELD_NAME,				// String64		rgbKeyFieldName,
				&m_ridTableRow,						// void			*pKeyFieldValue,
				sizeof(m_ridTableRow),				// U32			cbKeyFieldValue,
				pRecordRefreshData,					// void			*prgbRowData,
				cbRecordRefreshData,				// U32			cbRowData,
				1,									// U32			cRowsToModify,
				NULL,								// U32			*pcRowsModifiedRet,
				NULL,								// rowID		*pRowIDRet,
				0,									// U32			cbMaxRowID,
				(pTSCallback_t)&HandlePTSWriteReply, // pTSCallback_t	pCallback,
				NULL								// void			*pContext 
			);
			
			if (status == OS_DETAIL_STATUS_SUCCESS)
			{
				m_pModifyMsg->Send();
				// Set our state to WaitingForPTSWrite.
				m_State = WaitingForPTSWrite;
			}
			else
				delete m_pModifyMsg;
		} /* if */	
		// Reset m_RefreshCount;
		// This is why refresh rate shoulds be a multiple of sample rate.
		m_RefreshCount = m_RefreshRate / m_SampleRate;
	} /* if */
	return status;
}


/**********************************************************************/
// HandlePTSWriteReply - This method although overridden by the derived
// classes must be called by them at their entrance.  
// 
/**********************************************************************/
STATUS Reporter::HandlePTSWriteReply(void *pContext, STATUS status)
{
#pragma unused(pContext)
//Tracef("Reporter::HandlePTSWriteReply\n");

	// Set our state to WaitingForTimerSvc.
	m_State = WaitingForTimerSvc;
	
	return status;
}


#if 0
/**********************************************************************/
// HandleTimerResetReply - Casts the pContext pointer to a pointer to a
// Reporter (which it is) and then calls the HandlePTSWriteReply() 
// method of that object. 
/**********************************************************************/
STATUS Reporter::HandleTimerResetReply(Message *pMsg)
{
STATUS 	status = pMsg->DetailedStatusCode;

	// delete the message.
	delete pMsg;
	
	return status;	
}

#endif
