/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidPerformance.h
//
// Description:
//
// Raid DDM Performance Structure used with the Performance / Health / Status
// system
//
// Update Log: 
//	5/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidPerformance_h
#define __RaidPerformance_h


#include "CTTypes.h"
#include "RaidDefs.h"

#define	NUM_MEASURE_SIZES	12

// Count reads and writes by the number of blocks requested
// [0]   = 512
// [1]   = 1k
// [2]   > 1k   <= 2k
// [3]   > 2k   <= 4k
// [4]   > 4k   <= 8k
// [5]   > 8k   <= 16k
// [6]   > 16k  <= 32k
// [7]   > 32k  <= 64k
// [8]   > 64k  <= 128k
// [9]   > 128k <= 256k
// [10]  > 256k <= 512k
// [11]  > 512k

typedef struct
{
	U32		NumReads[NUM_MEASURE_SIZES];
	U32		NumWrites[NUM_MEASURE_SIZES];
	U32		NumBlocksRead;
	U32		NumBlocksWritten;
	U32		NumSGCombinedReads;
	U32		NumSGCombinedWrites;
	U32		NumOverwrites;
} RAID_PERFORMANCE;


#endif