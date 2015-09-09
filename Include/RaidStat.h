/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidStatus.h
//
// Description:
//
// Raid DDM Status Structure used with the Performance / Health / Status
// system
//
// Update Log: 
//	5/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidStatus_h
#define __RaidStatus_h


#include "CTTypes.h"
#include "RaidDefs.h"


typedef struct
{
	U32		NumRetries[MAX_ARRAY_MEMBERS];
	U32		NumRecoveredErrors[MAX_ARRAY_MEMBERS];
	U32		NumReassignedSuccess[MAX_ARRAY_MEMBERS];
	U32		NumReassignedFailed[MAX_ARRAY_MEMBERS];
	U32		NumRaidReassignedSuccess;
	U32		NumRaidReassignedFailed;
} RAID_STATUS;

#endif
