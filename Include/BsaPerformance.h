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
// This class is the BsaIsmPerformance Record.   Containing BSA Performance
// data used by the Odyssey Performance Health Status System.
// 
// $Log: /Gemini/Include/BsaPerformance.h $
// 
// 4     9/05/99 3:07p Vnguyen
// Clarify that time unit is in microseconds
// 
// 3     8/26/99 3:50p Vnguyen
// Clean up the number of performance and status counters to be
// calculated and returned to the PHS Reporter.
// 
// 2     5/04/99 9:05p Mpanas
// Remove unused includes
// 
// 1     5/04/99 6:51p Jlane
// Initial Checkin.
//
/*************************************************************************/

#ifndef __BsaPerformance_h
#define __BsaPerformance_h

#include "CTTypes.h"

typedef struct _BSA_ISMPerformance
{
	U32	num_reads;
	U32	num_writes;
	U32	num_other_requests;
	
	I64	num_bytes_read;
	I64	num_bytes_written;
	
	I64	total_read_latency;		// microseconds
	I64	total_write_latency;

// Anything below this line will be removed soon.
	I64	total_latency;
	I64	total_other_latency;
	U32	minimum_read_latency;
	U32	maximum_read_latency;
	U32	minimum_write_latency;
	U32	maximum_write_latency;
} BSA_ISMPerformance;

#endif  // __BsaPerformance_h