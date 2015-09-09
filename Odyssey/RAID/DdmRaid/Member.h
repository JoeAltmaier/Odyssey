/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Member.h
//
// Description:	Header file for Raid Member class
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __Member_h
#define __Member_h

#include "ArrayDescriptor.h"
#include "RAIDMemberTable.h"
#include "RAIDUtilTable.h"

class Member
{

public:

	VDN					Vd;
	rowID				MemberRowId;
	RAID_MEMBER_STATUS 	Health;
	U8					ReadPreference;
	U8					MaxRetryCount;
	U8					QMethod;
	U16					MaxOutstanding;
	U16					NumOutstanding;
	U32					RegeneratedLBA;
	U32					LastAccessedLBA;
	U32					StartLBA;
	U32					EndLBA;
	U32					MemberMask;
	ReqQueue			*pRequestQueue;

	~Member();
	STATUS		Initialize(RAID_ARRAY_MEMBER *pArrayMember);
	STATUS		Initialize(RAID_ARRAY_MEMBER *pArrayMember, Raid *pRaid, U8 QMethod);
};

#endif




