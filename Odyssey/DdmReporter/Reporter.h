/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class is the base class for a table data reporter.
//
// NOTE: REFRESH RATE SHOULD BE AS CLOSE AS POSSIBLE TO AN EVEN MULTIPLE
// OF THE SAMPLE RATE.
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/Reporter.h $
// 
// 12    10/08/99 4:10p Vnguyen
// Comment out unused method "TimerResetReply"
// 
// 11    8/23/99 4:51p Vnguyen
// Separate quiesce and pause states as it is possible for the two
// states to overlap.  Quiesce is global to all reporters.  Pause is local
// to one reporter.
// 
// 10    8/16/99 1:00p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 9     8/05/99 10:56a Vnguyen
// 
// 8	8/05/99	10:05a Vnguyen -- Add support for ResetReporter and 
// StopReporter.  Add support for PauseReporter
// 
// 7     8/02/99 8:59a Vnguyen
// 
// 6     5/19/99 9:02a Jlane
// Miscellaneous changes and bug fixes made during bringup.
// 
// 5     5/05/99 4:12p Jlane
// Remove VerifyTableExists prorotype.
// 
// 4     5/05/99 4:08p Jlane
// Miscellaneous integration changes.
// 
// 3     5/05/99 10:02a Jlane
// Pass pb/cb data in Handle DdmSampleData instead of pMsg.  make DdmData
// an Sgl item instead of payload.
//
// 10/26/98	JFL	Created.
/*************************************************************************/

#ifndef __Reporter_h
#define __Reporter_h

#include "CTTypes.h"
//#include "DdmTimer.h"
#include "Table.h"
#include "Rows.h"
#include "RqOsTimer.h"
#include "RqDdmReporter.h"
#include "DdmReporter.h"
#include "Message.h"

//#include "StorageRollCallTable.h"
//#include "SRCandExportClass.h"

#define NULLROWID(rid) ((!rid.Table)&&(!rid.HiPart)&&(!rid.LoPart))

class Reporter : public DdmServices
{

public:
			Reporter();
	virtual	~Reporter();  // Watch out for the Timer Service

	virtual	STATUS	InitReporter(	
									String64		rgbTableName, 	// The name of the table to refresh.
									rowID			ridTableRow,	// Row ID of table row to refresh.
									U32				RefreshRate,	// The rate at which to refresh the table.
									U32				SampleRate		// The rate at which to sample the DDM.
								 );
				 

	virtual	STATUS	ReInitReporter(	U32				RefreshRate,	// The rate at which to refresh the table.
									U32				SampleRate		// The rate at which to sample the DDM.
									);
									
	virtual	STATUS PauseReporter();
	virtual STATUS ContinueReporter();
	virtual	STATUS QuiesceReporter();
	virtual STATUS EnableReporter();
	virtual STATUS ResetReporter();
	virtual STATUS StopReporter();
	virtual	STATUS DoStopReporter();
				 
	// The handlers for the reply callbacks.  These are 
	virtual STATUS	HandleDdmResetReply(Message *pMsg);
	virtual STATUS	HandleTimerSvcReply(Message *pMsg);
	virtual STATUS	HandleDdmDataReply(Message *pMsg);
	virtual STATUS	HandlePTSWriteReply(void *pContext, STATUS status);
//	virtual STATUS	HandleTimerStartReply(Message *pMsg);
//	virtual STATUS	HandleTimerResetReply(Message *pMsg);

	// These are functions that are supplied by the derived classes if needed, otherwise,
	// they are do nothing.
	virtual void ResetCounters() {};

	// These are abstract functions supplied by the derived classes.
	virtual STATUS	HandleDdmSampleData(void* pDdmData, U32 cbDdmData) = 0;
	virtual STATUS	ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData) = 0;

	// These are our state values.
	 enum ReporterStateEnum {
		UnInitialized		= 0,	// The constructor leaves us in this state
		WaitingForDDMReset	= 1,	// Initialize sends a reset to the DDM and leaves us in this state.
		WaitingForTimerSvc	= 2,	// HandleDDMResetReply and HandleTimerResetReply & HandlePTSWriteReply leave us here.
		WaitingForDDMData	= 3,	// HandleTimerStartReply leaves us in this state.
		WaitingForPTSWrite	= 4		// HandleDDMDataReply leaves us in this state
	};	
	
// protected:
	// Protected member variable declarations:
	U32				m_State;
	DdmReporter*	m_pPHSReporter;		// Pointer to our owner PHSReporter object.
	String64		m_rgbTableName;		// This is the name of the table to update.  Why do I need this?
	rowID			m_ridTableRow;		// rowID of table data to update.
	I64				m_RefreshRate;		// Refresh rate.  Currently in Microseconds.
	I64				m_SampleRate;		// Sample rate.  Currently in Microseconds.
	I64				m_RefreshCount;		// # of samples between refresh operations.
	I64				m_NewRefreshRate;	// New Refresh rate used by ReInit.  Currently in Microseconds.
	I64				m_NewSampleRate;	// New Sample rate use by ReInit.  Currently in Microseconds.
	bool			m_fReStart;			// Set = 1 by reinitialize to restart state machine.
	bool			m_pause;			// Set to true if in pause state
	bool			m_quiesce;			// Set to true if in quiesce state.
	bool			m_StopPending;		// The Reporter is about to stop
	bool			m_SkipOneSample;	// Align the sampling windows to skip any pause or quiesce event
	RqOsTimerStart*	m_pStartTimerMsg;	// Message I use to start the Timer service.
	RqOsTimerStart*	m_pResetTimerMsg;	// Message I use to start the Timer service.
	VDN				m_vdnDdm;			// Virtual Device to look up in SRC/Export table
	DID				m_didDdm;			// DID to querry for data
	REQUESTCODE		m_ResetDataMsg;		// PHS_RESET_STATUS or PHS_RESET_PEREFORMANCE
	REQUESTCODE		m_ReturnDataMsg;	// PHS_RETURN_STATUS or PHS_RETURN_PERFORMANCE
	TSModifyRow*	m_pModifyMsg;		// Msg we send to the PTS to refresh records.

	REPORTERCODE	m_ReporterCode;		// Type of reporter for this object

	// Our own internal entry point into the state machine.
	STATUS Start();
	
	// A private routine used to complete reinitialization.
	STATUS ReStart();

//private:	

};



#endif	// __Reporter_h
