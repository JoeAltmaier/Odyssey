/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Member.cpp
//
// Description:	Raid Member class
//
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "Ddm.h"
#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "RaidStructs.h"
#include "Raidq.h"
#include "Member.h"
#include "OsStatus.h"


/*************************************************************************/
// Initialize
// Initialize method for the class Raid Member
/*************************************************************************/

STATUS	Member::Initialize(RAID_ARRAY_MEMBER *pArrayMember)
{ 
	Vd = pArrayMember->memberVD;		// rid same as vd ??
	MemberRowId = pArrayMember->thisRID;
	Health = pArrayMember->memberHealth;
	StartLBA = pArrayMember->startLBA;
EndLBA = pArrayMember->endLBA;
	MaxOutstanding = pArrayMember->maxOutstanding;
	NumOutstanding = 0;
	MaxRetryCount = pArrayMember->maxRetryCnt;
// set from config
	QMethod = RAIDQ_ELEVATOR;
	MemberMask = ( (U32) 1 << pArrayMember->memberIndex);

	ReadPreference = pArrayMember->policy.ReadPreference;
// need to set from util table
//	RegeneratedLBA = pArrayMember->RegeneratedLBA;
	LastAccessedLBA = 0;
	pRequestQueue = NULL;

	return OS_DETAIL_STATUS_SUCCESS;
}	

/*************************************************************************/
// Initialize
// Overloaded Initialize method. Also allocates a Request Queue
/*************************************************************************/

STATUS	Member::Initialize(RAID_ARRAY_MEMBER *pArrayMember, Raid *pRaid, U8 QMethod)
{ 
	Member::Initialize(pArrayMember);

	if ((pRequestQueue = new ReqQueue(pRaid, QMethod)) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	return OS_DETAIL_STATUS_SUCCESS;

}	

/*************************************************************************/
// ~Member
// Destructor method for the class Raid Member
/*************************************************************************/

Member::~Member()
{ 
	if (pRequestQueue)
		delete pRequestQueue;
}
