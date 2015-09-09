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
// This class is the SCSI Target Server (STS) Performance Record.   Containing
// performance data used by the Odyssey Perfoirmance Health Status System.
// 
// $Log: /Gemini/Include/STSPerf.h $
// 
// 7     9/09/99 4:21p Vnguyen
// Add performance counters for the number of bytes for BSA read and
// write.
// 
// 6     9/09/99 1:45p Vnguyen
// Remove duplicate variable NumSCSICmds.
// 
// 5     9/09/99 12:07p Vnguyen
// Update perf and status counters to match with what is being reported by
// SCSI Target Server DDM.
// 
// 4     8/16/99 8:39a Vnguyen
// Update parameter names to match the rest of DdmReporter
// 
// 3     8/02/99 8:59a Vnguyen
// 
// 2     7/27/99  3:22p Vnguyen -- Adapt from BSA structure to STS
// 1     7/20/99 10:21a Jlane
// Initial Checkin.
// 
/*************************************************************************/

#ifndef __STSPerformance_h
#define __STSPerformance_h

typedef struct  {
	U32	NumBSAReads;
	U32	NumBSAWrites;
	U32	NumSCSICmds;

	I64	NumBSABytesRead;
	I64 NumBSABytesWritten;
	
// Everything below this line is not used.	
	U32	NumMessageIn;
	U32	NumReplyOut;
	U32	NumBSAMessageSent;
	U32	NumBSAMessageReceived;
	U32	NumSCSIOtherCmds;			// less than 20 of these Mike has the list. Scsi.h
	U32	NumSCSICmdsNotSupported;
} STS_Performance;

#endif  // __STSPerformance_h
