/* VirtualDeviceTable.h -- PTS VirtualDeviceTable Definitions
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/
 
// Revision History: 
//  8/06/99 Jerry Lane: Create file
//  ** Log at end-of-file **

#include "VirtualDeviceTable.h"

CHECKFIELDDEFS(VirtualDeviceRecord);

const fieldDef VirtualDeviceRecord::rgFieldDefs[] = {
	// FieldName								  Size 	  Type
		CPTS_RECORD_BASE_FIELDS (Persistant_PT),
	
		// Persistant Fields
		VDT_RID_VDOWNERUSE_FIELD,					8,	ROWID_FT, Persistant_PT,	// Configuration data for the VD.
		VDT_CLASSNAME_FIELD,						32,	STRING32_FT, Persistant_PT,	// Class type name of the VD.
		VDT_SLOT_PRIMARY_FIELD,						4,	U32_FT,	Persistant_PT,		// DID of Primary DDM.
		VDT_SLOT_SECONDARY_FIELD,					4,	U32_FT,	Persistant_PT,		// DID of secondary DDM.
		VDT_RID_DDM_CFG_REC,						8,	ROWID_FT, Persistant_PT,	// Configuration data for the VD.
//		VDT_STATE_DESIRED_FIELD,					4,	U32_FT,	Persistant_PT,		//** OBSOLETE **
//		VDT_STATE_ACTUAL_FIELD,						4,	U32_FT,	Persistant_PT,		//** OBSOLETE **
		VDT_ATTRS_FIELD,							4,  U32_FT, Persistant_PT,		// Attributes  (Persistant)
		VDT_NSERVES_FIELD,							4,  U32_FT, Persistant_PT,		// Number of VirtualServes

		// Non-Persistant Fields
		VDT_FLAGS_FIELD,							4,	U32_FT,	NotPersistant_PT,	// State Flags (Not Persistant)
//		VDT_FIOPHASVDR_FIELD,						4,  U32_FT, NotPersistant_PT,	//** OBSOLETE **
		VDT_FIOPHASDID_FIELD,						4,	U32_FT,	NotPersistant_PT,
		VDT_DID_PRIMARY_FIELD,						4,	DID_FT,	NotPersistant_PT,	// DID of Primary DDM.
		VDT_DID_SECONDARY_FIELD,					4,	DID_FT,	NotPersistant_PT,	// DID of Secondary DDM.
		VDT_DID_ACTIVE_FIELD,						4,	DID_FT,	NotPersistant_PT,	// DID of Active DDM.

		// Persistant non-duplicate key
		VDT_KEY	,						  sizeof(Key),  BINARY_FT, Persistant_PT //| NonDuplicate_PT		
	};

const U32 VirtualDeviceRecord::cbFieldDefs = sizeof(VirtualDeviceRecord::rgFieldDefs);


//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/VirtualDeviceTable.cpp $
// 
// 15    12/09/99 1:28a Iowa
// 
// 14    11/04/99 1:23p Jlane
// Roll in tom's changes
// 
// 13    10/14/99 3:48a Iowa
// Iowa merge
// 
// 11    9/16/99 4:00p Tnelson
// 
// 10    9/03/99 3:12p Tnelson
// 
// 9     8/26/99 3:46p Tnelson
// Latest and Greatest!
// 
// 8     8/26/99 2:09p Tnelson
// Couple of fixes
// 
// 7     8/26/99 2:08p Tnelson
// Couple of fixes
// 
// 6     8/26/99 2:04p Tnelson
// Moved PTS Request Classes into VirtualDeviceRecord as per Eric's
// request
// 
// 5     8/20/99 2:58p Tnelson
// Move rgFieldDefs[] and cbFieldDefs into VirtualDeviceRecord as statics
// 
// 4     8/14/99 7:39p Tnelson
// Make VirtualDeviceTable_FieldDefs[] use the #define field names that
// are in VirtualDeviceTable.h.
// VirtualDeviceTable_FieldDefs[] was missing fields that were defined
// in the VirtualDeviceRecord struct
// 
// 3     8/14/99 2:09p Tnelson
// Added static access methods the VirtualDeviceRecord
// 
// 2     8/08/99 11:22a Jlane
// Compile fixes - case errors etc.
// 
// 1     8/06/99 10:48a Jlane
// Initial Checkin.
// 
