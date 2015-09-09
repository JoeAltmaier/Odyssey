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
// This class is the FCP Target Performance Record.   Containing
// performance data used by the Odyssey Performance Health Status System.
// 
// $Log: /Gemini/Include/FCPTargetPerformance.h $
// 
// 5     10/11/99 6:03p Vnguyen
// Add timestamp for performance counters and # of errors received for
// Status counters.
// 
// 4     9/15/99 9:58a Vnguyen
// Change byte counters from U32 to I64
// 
// 3     9/13/99 5:32p Vnguyen
// Fix spellings for NumTotalPackets.
// 
// 2     9/13/99 4:48p Vnguyen
// Update performance counters to match what is being reported by FCP
// Target Driver.
// 
// 1     8/16/99 4:21p Vnguyen
// Initial check-in.
// 
/*************************************************************************/

#ifndef __FCPTargetPerformance_h
#define __FCPTargetPerformance_h

typedef struct {
	U32	NumReadPackets;
	U32	NumWritePackets;
	U32 NumTotalPackets; // Read + Write + Other packets.
	
	I64 NumBytesRead;
	I64 NumBytesWritten;

	I64	TotalReadLatency;		// microseconds
	I64	TotalWriteLatency;


// Everything below this line is not used.
#if 0	
	I64	TotalPacketsRcvd;
	I64 ReadPacketsRcvd;
	I64	WritePacketsRcvd;
	I64 OtherPacketsRcvd;

	I64 TotalResponses;
	I64 PacketsResponsesSent;
	I64 MessagesSent;

	I64 sesRcvd;
	I64 GoodResponsesRcvd;
	I64 ErrorResponsesRcvd;

	I64 BufferUsage;
	I64 FreeBuffers;
	I64 InUseBuffers;
#endif

} FCPT_Performance;

#endif  // __FCPTargetPerformance_h


