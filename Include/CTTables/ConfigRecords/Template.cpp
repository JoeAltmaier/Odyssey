/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: BsaConfig.h
// 
// Description:
// This file defines the configuration data for the BsaIsm DDM. 
//
// Update Log:
//	$Log: /Gemini/Include/CTTables/ConfigRecords/Template.cpp $
// 
// 1     8/25/99 1:26p Egroff
// 
//
/*************************************************************************/

#include "CtTypes.h"
#include "PTSCommon.h"
#include "BsaConfig.h"



	fieldDef	BSAConfig_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
		"version",						32	String32_FT, Persistant_PT,
		"size",							4,	U32_FT, Persistant_PT,
	};

// defined here so other folks can get to it	
U32 cbBSAConfig_FieldDefs = sizeof(BSAConfig_FieldDefs);
				

