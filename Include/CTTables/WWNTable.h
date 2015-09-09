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
// File: WWNTable.h
// 
// Description:
// Array Descriptor Record 
// 
// $Log: /Gemini/Include/CTTables/WWNTable.h $
// 
// 1     1/05/00 6:58p Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/

#ifndef WWNTable_h
#define WWNTable_h

#include "CtTypes.h"
#include "TableMsgs.h"


#pragma	pack(4)

extern const fieldDef WWNTable_FieldDefs[];
extern const U32 sizeofWWNTable_FieldDefs;

#define WWN_TABLE "WWN_TABLE"
#define	WWN_TABLE_VERSION		1


/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdVERSION			"Version"
#define fdSIZE				"Size"
#define fdCHASSIS_WWN		"ChassisWWN"

/********************************************************************
*
* ARRAY_DESCRIPTOR
*
********************************************************************/

typedef struct
{
	rowID				rid;				// rid in descriptor table
	U32					version;			// version of WWN table
	U32					size;				// size of WWN table
	I64					chassisWWN;			// the chassis wwn
}	WWNDescriptor;

#endif