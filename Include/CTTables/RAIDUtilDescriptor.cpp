/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
*
* 
* Update Log: 
* 3/99 Ron Parks: Create file
*
*************************************************************************/

#include "RaidUtilTable.h"

const fieldDef UtilDescriptorTable_FieldDefs[] =
{
	fdUDT_VERSION,				4,	U32_FT,			Persistant_PT,
	fdUDT_SIZE,					4,	U32_FT,			Persistant_PT,
	fdTARGET_ARRAY,				8,	ROWID_FT,		Persistant_PT,
	fdSTART_TIME,				4,	U32_FT,			Persistant_PT,
	fdUPDATE_RATE,				4,	U32_FT,			Persistant_PT,
	fdUTIL_CODE,				4,	U32_FT,			Persistant_PT,
	fdPRIORITY,					4,	U32_FT,			Persistant_PT,
	fdUTIL_STATUS,				4,	U32_FT,			Persistant_PT,
	fdPERCENT_UPDATE_RATE,		4,	U32_FT,			Persistant_PT,
	fdPERCENT_DONE,				4,	U32_FT,			Persistant_PT,
	fdSOURCE_ROWIDS	,			8*MAX_ARRAY_MEMBERS,	ROWID_FT,		Persistant_PT,
	fdDESTINATION_ROWIDS,		8*MAX_ARRAY_MEMBERS,	ROWID_FT,		Persistant_PT,
	fdCURRENT_LBA,				8,	U64_FT,			Persistant_PT,
	fdENDUTIL_LBA,				8,	U64_FT,			Persistant_PT,
	fdPOLICY,					4,	U32_FT,			Persistant_PT,
	// STATE IDENTIFIER, for RMSTR use only
	fdCMD_ROWID,				8,	ROWID_FT,		Persistant_PT,
	fdCMD_OPCODE,				4,	U32_FT,			Persistant_PT,
	fdCMD_STATE,				4,	U32_FT,			Persistant_PT,
	fdCMD_INDEX,				4,	U32_FT,			Persistant_PT		
	// STATE IDENTIFIER end
};


const U32 sizeofUtilDescriptorTable_FieldDefs =  sizeof (UtilDescriptorTable_FieldDefs);
