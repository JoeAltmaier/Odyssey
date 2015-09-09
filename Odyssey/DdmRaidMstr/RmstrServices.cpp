/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: RmstrServices.cpp
// 
// Description:
// Helper methods for PTS operations and other general service methods
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrServices.cpp $
// 
// 15    8/27/99 5:24p Dpatel
// added event code..
// 
// 14    8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 13    8/03/99 5:25p Dpatel
// Removed the service method to modify SRC, used table services..
// 
// 12    7/28/99 6:35p Dpatel
// Added capability code, table services, add/remove members, preferred
// member and source member, hot copy etc...
// 
// 11    7/26/99 3:04p Dpatel
// removed return status from Send()
// 
// 10    7/23/99 5:47p Dpatel
// Added internal cmds, hotcopy, changed commit spare etc.
// 
// 9     7/22/99 6:43p Dpatel
// Added unicode string names, changed validation
// 
// 8     7/20/99 6:49p Dpatel
// Some bug fixes and changed arrayName in ArrayDescriptor to rowID.
// 
// 7     7/16/99 10:56a Dpatel
// Changes for PTS Interface change for GetTableDef.
// 
// 6     7/16/99 10:28a Dpatel
// Added DownMember and Commit Spare code. Also removed the reads for
// validation.
// 
// 5     7/09/99 5:26p Dpatel
// 
// 4     7/06/99 4:56p Dpatel
// Added flag for Updating the SRC "fUsed" field.
// 
// 3     6/28/99 5:16p Dpatel
// Implemented new methods, changed headers.
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/


#include "DdmRaidMgmt.h"



//************************************************************************
//	rmstrServiceCheckSize
//		Check if SRC record not already used
//		Check for member size > array definition's smallest member size
//
//	RETURN:
//		RMSTR_SUCCESS		success
//		RMSTR_ERR_INSUFFICIENT_MEMBER_CAPACITY
//		RMSTR_ERR_INSUFFICIENT_SPARE_CAPACITY
//		RMSTR_ERR_STORAGE_ELEMENT_IN_USE;
//
//************************************************************************
STATUS DdmRAIDMstr::
rmstrServiceCheckSize(
			BOOL						isMember,
			StorageRollCallRecord		*pSRCRecord,
			I64							smallestMemberCapacity)
{
	STATUS							status = RMSTR_SUCCESS;

	if (pSRCRecord->fUsed){
		status = RMSTR_ERR_STORAGE_ELEMENT_IN_USE;
	} else {
		// check the capacity of the member against the 
		// array's smallest member cap
		if (pSRCRecord->Capacity < smallestMemberCapacity){
			if (isMember){
				status = RMSTR_ERR_INSUFFICIENT_MEMBER_CAPACITY;
			} else {
				status = RMSTR_ERR_INSUFFICIENT_SPARE_CAPACITY;
			}
		}
	}
	return status;
}



//*****************************************************************
//	rmstrServiceDeleteMemberSpareUtilityFromADT
//		Deletes the rowID corresponding to member,spare,util from ADT
//		Will update the count and fill the holes in the corresponding
//		array[] (if any)
//	type		- RAID_MEMBER, RAID_SPARE, RAID_UTIL
//	pRowId		- the row Id of the member/spare/util to be deleted
//	pADTRecord	= the array
//
//*****************************************************************
BOOL DdmRAIDMstr
::rmstrServiceDeleteMemberSpareUtilityFromADT(
			U32						type,
			rowID					*pRowId,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord)
{
	U32		x=0, y=0;
	BOOL	found = false;

	U32		maxEntries = 0;
	rowID	*pADTRows = NULL;
	U32		*pADTCount = NULL;

	switch (type){
		case RAID_MEMBER:
			maxEntries = MAX_ARRAY_MEMBERS;
			pADTRows = pADTRecord->members, 
			pADTCount = &pADTRecord->numberMembers;
			break;

		case RAID_SPARE:
			maxEntries = MAX_ARRAY_SPARES;
			pADTRows = pADTRecord->spares;
			pADTCount = &pADTRecord->numberSpares;
			break;
		case RAID_UTILITY:
			maxEntries = MAX_UTILS;
			pADTRows = pADTRecord->utilities; 
			pADTCount = &pADTRecord->numberUtilities;
			break;
		default:
			return found;
	}

	for (x=0; x < maxEntries; x++){
		if (found)
			break;
		if (memcmp(
				&pADTRows[x], 
				pRowId,
				sizeof(rowID)) == 0){
			// decrement the numberOfUtilities count.
			memset(&pADTRows[x],0,sizeof(rowID));
			(*pADTCount)--;
			// now fill the hole if one was created.
			for (y=x; y < *pADTCount; y++){
				pADTRows[y] = 
					pADTRows[y+1]; 
			}
			// clear the last util row id
			memset(
				&pADTRows[*pADTCount],
				0,
				sizeof(rowID));
			found = true;
		}
	}
	return found;
}



//************************************************************************
//	PrepareMemberInformation
//		This is a convinience method which prepares the MDT record
//	before inserting it into the table.
//
//************************************************************************
void DdmRAIDMstr::
rmstrServicePrepareMemberInformation(
			RAID_ARRAY_MEMBER			*pMember,
			rowID						*pArrayRowId,
			rowID						*pSRCRowId,
			RAID_MEMBER_STATUS			memberHealth,
			U32							memberIndex,
			I64							endLBA,
			I64							startLBA,
			VDN							memberVD,
			U32							maxRetryCnt,
			RAID_QUEUE_METHOD			queueMethod,
			U32							maxOutstanding,
			RAID_MEMBER_POLICIES		policy)
{
	pMember->version = RAID_MEMBER_DESCRIPTOR_TABLE_VERSION;
	pMember->size = sizeof(RAID_ARRAY_MEMBER);

	pMember->arrayRID = *pArrayRowId;			// array descriptor id
	pMember->memberRID = *pSRCRowId;				// rowid in SRCTbl

	pMember->memberVD = memberVD;				// VD of member -- 
	pMember->memberHealth = memberHealth;
	pMember->memberIndex = memberIndex;			// relative to this array
	pMember->maxRetryCnt = maxRetryCnt;			// (default=3)set by System Master
	pMember->queueMethod = queueMethod;			// (default=RAID_QUEUE_ELEVATOR)
	pMember->startLBA = startLBA;				// where host data begins
	pMember->endLBA = endLBA;					// where host data ends
	pMember->maxOutstanding = maxOutstanding;	// (default=5)request queue depth
	pMember->policy = policy;
}



