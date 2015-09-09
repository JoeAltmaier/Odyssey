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
// File: RmstrCapabilityData.cpp
// 
// Description:
// Data for display
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrCapabilityData.cpp $
// 
// 5     12/09/99 4:49p Agusev
// all capabilities now used as 512K byte blocks
// 
// 4     11/23/99 11:09a Dpatel
// Check for Data block sizes
// 
// 3     8/14/99 1:37p Dpatel
// Added event logging..
// 
// 2     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 1     7/28/99 6:36p Dpatel
// Initial Creation
// 
//
/*************************************************************************/
#include "RaidDefs.h"
#include "RmstrCmnds.h"
#include "RmstrEvents.h"
#include "RmstrErrors.h"

#include "DdmRaidMgmt.h"

#pragma	pack(4)



RMSTR_CAPABILITY_RAID_LEVEL Raid0Capabilities = 
{
		128,		// values are in 512k byte block i.e 64k
		{8,16,32,64,128,256,0xFFFFFFFF,0,0,0},
		0,
		{0xFFFFFFF,0,0,0,0,0,0,0,0,0}
};


RMSTR_CAPABILITY_RAID_LEVEL Raid1Capabilities = 
{
		0,
		{0xFFFFFFFF,0,0,0,0,0,0,0,0,0},
		0,
		{0xFFFFFFFF,0,0,0,0,0,0,0,0,0}
};

RMSTR_CAPABILITY_RAID_LEVEL Raid5Capabilities = 
{
		64,
		{16,32,64,128,0xFFFFFFFF,0,0,0,0,0},
		0,
		{16,32,64,128,0xFFFFFFFF,0,0,0,0,0}
};



RMSTR_CAPABILITY_DESCRIPTOR RaidCapabilities[] = {
	{
		{0,0,0}, 
		RMSTR_CAPABILITY_TABLE_VERSION, 
		sizeof(RMSTR_CAPABILITY_DESCRIPTOR),
		RMSTR_CAPABILITY_RAID0, 
		"Raid 0 Caps", 
		1,
		1
	},
	{
		{0,0,0}, 
		RMSTR_CAPABILITY_TABLE_VERSION,
		sizeof(RMSTR_CAPABILITY_DESCRIPTOR),
		RMSTR_CAPABILITY_RAID1,
		"Raid 1 Caps", 
		1, 
		1
	},
	{
		{0,0,0}, 
		RMSTR_CAPABILITY_TABLE_VERSION,
		sizeof(RMSTR_CAPABILITY_DESCRIPTOR),
		RMSTR_CAPABILITY_RAID5,
		"Raid 5 Caps", 
		0, 
		0
	},
	{
		{0,0,0},
		RMSTR_CAPABILITY_TABLE_VERSION,
		sizeof(RMSTR_CAPABILITY_DESCRIPTOR),
		RMSTR_CAPABILITY_INVALID, 
		"", 
		0,
		0
	}
};


