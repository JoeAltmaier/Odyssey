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
// This class is the SCSI Target Server (STS) Status Record.   Containing Status
// data used by the Odyssey Performance Health Status System.
// 
// $Log: /Gemini/Include/STSStat.h $
// 
// 8     10/07/99 12:22p Vnguyen
// Change Status counter from #of error sent to # of error internal.  This
// saves some overhead as the SCSI Target Server driver can increment
// these counters separately.  The PHS Reporter will combine them later
// before sending the error counters to the PTS.
// 
// 7     10/07/99 11:29a Vnguyen
// Add Status counters for # of errors received and also for # of errors
// that are generated internally.
// 
// 6     9/09/99 12:07p Vnguyen
// Update perf and status counters to match with what is being reported by
// SCSI Target Server DDM.
// 
// 5     9/03/99 9:12a Vnguyen
// Rename STSStatus to STS_Status to be consistent.
// 
// 4     8/16/99 8:39a Vnguyen
// Update parameter names to match the rest of DdmReporter
// 
// 3     8/02/99 8:59a Vnguyen
// 
// 
// 2     7/27/99  1:20p Vnguyen -- Adapt from BSA to STS structure
// 1     7/20/99 10:21a Jlane
// Initial Checkin.
// 
/*************************************************************************/

#ifndef __STSStatus_h
#define __STSStatus_h

#define		SCSI_PAGE_SIZE				64
#define		DIFFERENTIAL_ERROR_SIZE		64

typedef struct {
	I64	NumTimerTimeout;	// Not being updated as the VC timer is not registered.  However, I 
							// am leaving this in because we may need this counter for debugging
							// in the future.
	I64	NumErrorRepliesReceived;	// The number of received replies that have status != 0.
	I64 NumErrorInternal;			// The # of errors generated internally.
									

// Everything below is not used at this time.  It could be useful in the future if we can pull the
// info from the SCSI mode pages.
	U32	SCSILogPages[SCSI_PAGE_SIZE];
	U32	NumDifferentialError[DIFFERENTIAL_ERROR_SIZE];	// minimum req'd by SCSI
	// we can define any pages we want.
} STS_Status;

#endif  // __STSStatus_h

