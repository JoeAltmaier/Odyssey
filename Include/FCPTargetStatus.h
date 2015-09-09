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
// This class is the Fcp Target Status Record.   Containing Fcp Target
// Status data used by the Odyssey Performance Health Status System.
// 
// $Log: /Gemini/Include/FCPTargetStatus.h $
// 
// 2     10/11/99 6:03p Vnguyen
// Add timestamp for performance counters and # of errors received for
// Status counters.
// 
// 1     8/16/99 4:21p Vnguyen
// Initial check-in.
// 
/*************************************************************************/

#ifndef __FCPTargetStatus_h
#define __FCPTargetStatus_h

#include "CTTypes.h"

typedef struct {
	I64	NumErrorRepliesReceived;	// The number of received replies that are in error
									// It could be either read or write.
	
// The counters below are not being updated yet.  We need to wait for
// the loop monitor driver.
	I64 FCPTargetStateTable;
	I64 DriverReadyStateFlag;	// Reset, Ready, Not Ready
	I64 Errors;
	I64 LoopDown;				// We can force the loop down if we want.
	I64 TransferIncomplete;
} FCPT_Status;

#endif  // __FCPTargetStatus_h

