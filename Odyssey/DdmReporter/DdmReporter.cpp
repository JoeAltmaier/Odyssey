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
// This file is the implementation of the null device.
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/DdmReporter.cpp $
// 
// 17    11/06/99 3:07p Vnguyen
// Add trace to display when a new reporter is registered.
// 
// 16    8/27/99 7:54a Vnguyen
// Fix up missing status return code to compile.
// 
// 15    8/23/99 4:51p Vnguyen
// Separate quiesce and pause states as it is possible for the two
// states to overlap.  Quiesce is global to all reporters.  Pause is local
// to one reporter.
// 
// 14    8/16/99 12:59p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 13    8/05/99 10:52a Vnguyen
// 
// 12	 8/04/99 1:04p Vnguyen
// Many major changes.  Implement Ddm call by request code.
// Implement support for Quiesce mode.  Remove StorageRollCall stuffs.
//
// 11    5/19/99 9:02a Jlane
// Miscellaneous changes and bug fixes made during bringup.
// 
// 10    5/13/99 11:38a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 9     5/07/99 11:14a Jlane
// Added VerifySRCTableExists to create the StorageRollCallTable if
// necessary.
// 
// 8     5/05/99 4:16p Jlane
// Supply Listen with sizeof returned record.
// 
// 7     5/05/99 4:08p Jlane
// Miscellaneous integration changes.
//
// 8/17/98 Joe Altmaier: Create file
// 10/05/98	JFL	Created DdmReporter.cpp/h from DdmNull.cpp/h.  Thanks Joe!
// 10/06/98 Jerry Lane: 
/*************************************************************************/

#include <stdio.h>

#include "BuildSys.h"
#include "Odyssey_Trace.h"
#include "DdmReporter.h"

#include "RqDdmReporter.h"

#include "DiskPerformanceReporter.h"
#include "DiskStatusReporter.h"

#include "ArrayPerformanceReporter.h"
#include "ArrayStatusReporter.h"

#include "STSPerfReporter.h"
#include "STSStatReporter.h"

#include "FCPTargetPerformanceReporter.h"
#include "FCPTargetStatusReporter.h"

#include "SSDPerformanceReporter.h"
#include "SSDStatusReporter.h"

#include "EVCStatusReporter.h"

#include "SglLinkedList.h"

CLASSNAME(DdmReporter,SINGLE);

SERVELOCAL(DdmReporter, PHS_START);
SERVELOCAL(DdmReporter, PHS_PAUSE);
SERVELOCAL(DdmReporter, PHS_CONTINUE);
SERVELOCAL(DdmReporter, PHS_RESET);
SERVELOCAL(DdmReporter, PHS_STOP);

/*************************************************************************/
// Declare and initialize static variables and other global data.
/*************************************************************************/
DdmReporter*	DdmReporter::m_pDdmReporter = NULL;


/*************************************************************************/
// DdmReporter
// Constructor method for the class DdmReporter
/*************************************************************************/
DdmReporter::DdmReporter(DID did): Ddm(did)
{

	TRACE_ENTRY(DdmReporter::DdmReporter);
	
	TRACE_HEX(TRACE_L6, "DdmReporter::DdmReporter this = ", (U32)this);
	TRACE_HEX(TRACE_L6, "DdmReporter::DdmReporter vd = ", (U32)vd);
	
	m_pDdmReporter = this;		// save the first one
	SetConfigAddress(NULL,0);	// point the base class to our non-existant config data

	}	// DdmReporter


/*************************************************************************/
// Ctor
// Create a new instance of the DdmReporter Ddm.
/*************************************************************************/
Ddm *DdmReporter::Ctor(DID did) { 

	TRACE_ENTRY(DdmReporter::Ctor);

	return new DdmReporter(did); 
}	// Ctor


/*************************************************************************/
// ~DdmReporter
// Destructor method for the class DdmReporter
/*************************************************************************/
DdmReporter::~DdmReporter()
{
	// Need to go through the list of reporters and call 
	// Reporter.StopReporter() method.  They will delete their own object
	// when they are ready to die.
	
	Reporter *pItem;

	pItem = (Reporter *) m_Reporters.RemoveFromHead();
	while (pItem)
	{
   		pItem->StopReporter();
   		pItem = (Reporter *) m_Reporters.RemoveFromHead();
   	}

}	// ~DdmReporter


/*************************************************************************/
// Initialize()
// Called after construction.
// Note this code must call the base class' Initialize method at its end.
/*************************************************************************/
STATUS DdmReporter::Initialize(Message *pMsg)
{
int status;

	
	DispatchRequest(PHS_START, (RequestCallback) &ProcessStartRequest);
	DispatchRequest(PHS_PAUSE, (RequestCallback) &ProcessRequest);
	DispatchRequest(PHS_CONTINUE, (RequestCallback) &ProcessRequest);
	DispatchRequest(PHS_RESET, (RequestCallback) &ProcessRequest);
	DispatchRequest(PHS_STOP, (RequestCallback) &ProcessRequest);
	
	status = Ddm::Initialize(pMsg);
		
	return status;
}


/*************************************************************************/
// Enable()
// Called at enable time.
//
// Also called when quiesce is over.
/*************************************************************************/
STATUS DdmReporter::Enable(Message *pMsg)
{
   Tracef("DdmReporter::Enable() entered.\n");
   // We need to tell all Reporters that it is okay to resume.
   

	Reporter *pItem;
	SglLinkedListIterator ReporterList(&m_Reporters);

	pItem = (Reporter *) ReporterList.CurrentItem();
	while (pItem)
	{
   		pItem->EnableReporter();
		pItem = (Reporter *) ReporterList.NextItem();
	}   		
   
   
   //  let base class know that we're done with enable phase
   //  (allows new requests to be processed by our DDM instance --
   //   until this call, CHAOS was holding them off from us to let
   //   us finish initialization / reply processing)
   Reply (pMsg, OS_DETAIL_STATUS_SUCCESS);
   return (OS_DETAIL_STATUS_SUCCESS);      // (must *always* return success)

}  /* end of DdmReporter::Enable */

	



/*************************************************************************/
// Quiesce()
// Called to enter quiesce state
//
/*************************************************************************/
STATUS DdmReporter::Quiesce(Message *pMsg)
{
	
	Tracef("DdmReporter::Quiesce() entered.\n");

   //  * * *  do local Quiesce stuff here.  * * *

	// We need to tell all Reporters to pause data collection.
	// Important:  We need to separate Quiesce from pause because it is 
	// possible for the two to overlap.


	Reporter *pItem;
	SglLinkedListIterator ReporterList(&m_Reporters);

	pItem = (Reporter *) ReporterList.CurrentItem();
	while (pItem)
	{
   		pItem->QuiesceReporter();
		pItem = (Reporter *) ReporterList.NextItem();
	}   		
   

   //  signal CHAOS that our DDM instance is finished quiescing.
   Reply (pMsg, OS_DETAIL_STATUS_SUCCESS);

   return (OS_DETAIL_STATUS_SUCCESS);      // (must *always* return success)

}



/*************************************************************************/
// ProcessStartRequest
// 
// 
/*************************************************************************/
STATUS DdmReporter::ProcessStartRequest(Message *pArgMsg)
{
	RqDdmReporter *pMsg;
	
	TRACE_ENTRY(DdmReporter::ProcessStartRequest);
		
	pMsg = (RqDdmReporter*) pArgMsg;

	Tracef("Processing PHS_START, ReporterCode is %d, did = %u, vdn = %d\n", 
			pMsg->ReporterCode, pMsg->did, pMsg->vdn);
	
	// We may want to check if the Reporter is already installed for
	// this vdn.  We can scan the m_Reporters list and check for vdn.
	// If we find the same vdn and ReporterCode, this is a bug from the 
	// calling Ddm.
	switch(pMsg->ReporterCode)
	{
		case PHS_DISK_PERFORMANCE:
			DiskPerformanceReporter *pDiskPerformanceReporter;
			pDiskPerformanceReporter = new DiskPerformanceReporter;
			m_Reporters.AddToTail(pDiskPerformanceReporter);
			pDiskPerformanceReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_DISK_STATUS:
			DiskStatusReporter *pDiskStatusReporter;
			pDiskStatusReporter = new DiskStatusReporter;
			m_Reporters.AddToTail(pDiskStatusReporter);
			pDiskStatusReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_ARRAY_PERFORMANCE:
			ArrayPerformanceReporter *pArrayPerformanceReporter;
			pArrayPerformanceReporter = new ArrayPerformanceReporter;
			m_Reporters.AddToTail(pArrayPerformanceReporter);
			pArrayPerformanceReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_ARRAY_STATUS:
			ArrayStatusReporter *pArrayStatusReporter;
			pArrayStatusReporter = new ArrayStatusReporter;
			m_Reporters.AddToTail(pArrayStatusReporter);
			pArrayStatusReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_SCSI_TARGET_SERVER_PERFORMANCE:
			STSPerfReporter *pSTSPerfReporter;
			pSTSPerfReporter = new STSPerfReporter;
			m_Reporters.AddToTail(pSTSPerfReporter);
			pSTSPerfReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_SCSI_TARGET_SERVER_STATUS:
			STSStatusReporter *pSTSStatusReporter;
			pSTSStatusReporter = new STSStatusReporter;
			m_Reporters.AddToTail(pSTSStatusReporter);
			pSTSStatusReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_FCP_TARGET_PERFORMANCE:
			FCPTargetPerformanceReporter *pFCPTargetPerformanceReporter;
			pFCPTargetPerformanceReporter = new FCPTargetPerformanceReporter;
			m_Reporters.AddToTail(pFCPTargetPerformanceReporter);
			pFCPTargetPerformanceReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;
			
		case PHS_FCP_TARGET_STATUS:
			FCPTargetStatusReporter *pFCPTargetStatusReporter;
			pFCPTargetStatusReporter = new FCPTargetStatusReporter;
			m_Reporters.AddToTail(pFCPTargetStatusReporter);
			pFCPTargetStatusReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;

		case PHS_SSD_PERFORMANCE:
			SSDPerformanceReporter *pSSDPerformanceReporter;
			pSSDPerformanceReporter = new SSDPerformanceReporter;
			m_Reporters.AddToTail(pSSDPerformanceReporter);
			pSSDPerformanceReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;

		case PHS_SSD_STATUS:
			SSDStatusReporter *pSSDStatusReporter;
			pSSDStatusReporter = new SSDStatusReporter;
			m_Reporters.AddToTail(pSSDStatusReporter);
			pSSDStatusReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;

		case PHS_EVC_STATUS:
			EVCStatusReporter *pEVCStatusReporter;
			pEVCStatusReporter = new EVCStatusReporter;
			m_Reporters.AddToTail(pEVCStatusReporter);
			pEVCStatusReporter->Initialize(this, pMsg->did, pMsg->vdn, pMsg->ReporterCode);
			break;

		default:
			// This is a bug.  Do an event log here
			break;
	} /* switch */

	Reply(pArgMsg, OS_DETAIL_STATUS_SUCCESS);
	return OK;	
}


/*************************************************************************/
// ProcessRequest
// 
// 
/*************************************************************************/
STATUS DdmReporter::ProcessRequest(Message *pArgMsg)
{
	RqDdmReporter *pMsg;
	Reporter *pItem;
	
	TRACE_ENTRY(DdmReporter::ProcessRequest);
		
	pMsg = (RqDdmReporter*) pArgMsg;
	
	// We need to run through the m_Reporters list to find the reporter
	// for the calling did, then call the Reporter object method to carry out
	// the request
	
	SglLinkedListIterator ReporterList(&m_Reporters);

	pItem = (Reporter *) ReporterList.CurrentItem();
	
	while (pItem)
	{
		if (pItem->m_didDdm == pMsg->did &&
			pItem->m_ReporterCode == pMsg->ReporterCode)
		{
			switch(pMsg->reqCode)
			{
				case PHS_PAUSE:
						pItem->PauseReporter();
						break;
				case PHS_CONTINUE:
						pItem->ContinueReporter();
						break;
				case PHS_RESET:
						pItem->ResetReporter();
						break;
				case PHS_STOP:
						pItem->StopReporter();
						// Ideally we should delete the Reporter object here.  However,
						// since the StopRequest operation is asynchronous, I don't want
						// to delete the object just yet.  The way to stop a reporter is
						// to call Reporter.StopReporter() method.  It will detele itself
						// when it is ready to die.  Our job is to clean up the linked list.
						ReporterList.RemoveCurrentItem();
						break;
				default:
						break;
			} /* switch */
			break; // exit the while loop.  We assume there is only one match.

		} /* if */
		
		pItem = (Reporter *) ReporterList.NextItem();

	} /* while */

	Reply(pArgMsg, OS_DETAIL_STATUS_SUCCESS);
	return OK;
}
