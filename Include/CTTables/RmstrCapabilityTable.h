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
// File: RmstrCapabilityTable.h
// 
// Description:
//	Rmstr Capability record
// 
// $Log: /Gemini/Include/CTTables/RmstrCapabilityTable.h $
// 
// 2     8/11/99 3:01p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 1     8/02/99 2:48p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#ifndef RmstrCapabilityTable_h
#define RmstrCapabilityTable_h

#include "CtTypes.h"
#include "TableMsgs.h"

#pragma	pack(4)

extern const fieldDef RmstrCapabilityTable_FieldDefs[];
extern const U32 sizeofRmstrCapabilityTable_FieldDefs;

#define RMSTR_CAPABILITY_TABLE "RMSTR CAPABILITY TBL"
#define RMSTR_CAPABILITY_TABLE_VERSION		1

/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdCAP_VERSION				"CAPVersion"
#define fdCAP_SIZE					"CAPSize"

#define	fdCODE				"CapabilityCode"
#define	fdDESCRIPTION		"Description"
#define	fdPURCHASED			"IsPurchased"
#define	fdENABLED			"IsEnabled"
#define	fdCAPABILITIES		"Capabilities"


typedef	enum
{
	RMSTR_CAPABILITY_RAID0 = 1,
	RMSTR_CAPABILITY_RAID1,
	RMSTR_CAPABILITY_RAID5,
	RMSTR_CAPABILITY_INVALID
}	RMSTR_CAPABILITY_CODE;



typedef struct
{
	U32					defaultDataBlockSize;
	U32					validDataBlockSizes[10];
	U32					defaultParityBlockSize;
	U32					validParityBlockSizes[10];
}	RMSTR_CAPABILITY_RAID_LEVEL;


/********************************************************************
*
* RmstrCapability Descriptor
*
********************************************************************/

typedef struct
{
	rowID					thisRID;		// member descriptor id
	U32						version;
	U32						size;
	RMSTR_CAPABILITY_CODE	capabilityCode;	// 
	UnicodeString32			description;
	U32						isPurchased;
	U32						isEnabled;
	char					capabilities[64];					
}	RMSTR_CAPABILITY_DESCRIPTOR;

#endif // RmstrCapabilityTable_h