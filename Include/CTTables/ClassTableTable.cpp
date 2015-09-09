/* ClassTableTable.cpp -- Table of ConfigData Tables
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
//  8/19/99 Tom Nelson: Create file
// ** Log at end-of-file **

#include "ClassTableTable.h"

CHECKFIELDDEFS(ClassTableRecord);

fieldDef ClassTableRecord::rgFieldDefs[] = {
//	   FieldName								  Size 	  Type
		CPTS_RECORD_BASE_FIELDS (Persistant_PT),

		CTT_CLASS_NAME,								32,	STRING32_FT, Persistant_PT,	// Class name
		CTT_TABLE_NAME,								64,	STRING64_FT, Persistant_PT,	// Table name of config records
	};

U32	ClassTableRecord::cbFieldDefs = sizeof(ClassTableRecord::rgFieldDefs);
				

//*************************************************************************
// Update Log:
//	$Log: /Gemini/Include/CTTables/ClassTableTable.cpp $
// 
// 4     10/14/99 3:48a Iowa
// Iowa merge
// 
// 3     9/17/99 9:46p Tnelson
// 
// 2     9/16/99 4:01p Tnelson
// 
// 1     8/19/99 6:30p Tnelson


