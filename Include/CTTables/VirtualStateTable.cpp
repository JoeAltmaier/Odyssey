/* VirtualStateTable.cpp -- IOP Status for VirtualManager
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
//  8/20/99 Tom Nelson: Create file
// ** Log at end-of-file **

#include "Odyssey_Trace.h"
#include "VirtualStateTable.h"

CHECKFIELDDEFS(VirtualStateRecord);

fieldDef VirtualStateRecord::rgFieldDefs[] = {
//	   FieldName								  Size 	  Type
//		"rowID",									8,	ROWID_FT,    Persistant_PT,
		"size",										4,	U32_FT, 	 Persistant_PT,	
		"version",									4,  U32_FT,		 Persistant_PT,	
		VST_SLOT,									4,  U32_FT,		 Persistant_PT,	// KEY FIELD
		VST_STATE,									4,	U32_FT, 	 Persistant_PT	// IOP State
	};

U32	VirtualStateRecord::cbFieldDefs = sizeof(VirtualStateRecord::rgFieldDefs);
				

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/VirtualStateTable.cpp $
// 
// 4     9/16/99 4:00p Tnelson
// 
// 3     9/03/99 3:12p Tnelson
// 
// 2     8/26/99 3:46p Tnelson
// Latest and Greatest!
// 
// 1     8/25/99 5:19p Tnelson


