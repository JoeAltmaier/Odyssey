/* VirtualRouteTable.cpp -- PTS Virtual Route Table
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
//  8/23/99 Tom Nelson: Create file
// ** Log at end-of-file **

#include "VirtualRouteTable.h"

CHECKFIELDDEFS(VirtualRouteRecord);

fieldDef VirtualRouteRecord::rgFieldDefs[] = {
//	   FieldName								  Size 	  Type
		CPTS_RECORD_BASE_FIELDS (Persistant_PT),

		VRT_VDN,									4,	U32_FT, 	 Persistant_PT,	// IOP State
		VRT_NSERVES,								4,	U32_FT, 	 Persistant_PT,	// IOP State
		VRT_MAXSERVES,								4,	U32_FT, 	 Persistant_PT,	// IOP State
		VRT_RQCODE,	  sizeof(REQUESTCODE) * MAXSERVES,  U32_FT,		 Persistant_PT,	// KEY FIELD
	};

U32	VirtualRouteRecord::cbFieldDefs = sizeof(VirtualRouteRecord::rgFieldDefs);
				

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/VirtualRouteTable.cpp $
// 
// 5     10/14/99 3:49a Iowa
// Iowa merge
// 
// 4     9/16/99 4:00p Tnelson
// 
// 3     9/03/99 3:12p Tnelson
// 
// 2     8/26/99 3:46p Tnelson
// Latest and Greatest!
// 
// 1     8/25/99 5:34p Tnelson
// Created


