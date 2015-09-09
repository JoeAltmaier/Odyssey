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
// Log: $
// 
// Description:
// This is the definition of the System Status table.
// 
// $Log: /Gemini/Include/CTTables/SystemStatusTable.cpp $
// 
// 3     10/28/99 9:25a Sgavarre
// Added IOPsOnPCIMask field for split boot support.
// 
// 2     8/23/99 1:47p Jlane
// Got to compile.
// 
// 1     8/20/99 2:48p Jlane
// Initial Checkin.
// 
/*************************************************************************/

#include  "SystemStatusTable.h"


const fieldDef aSystemStatusTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName                  Size  Type         Persist yes/no

   // rowID - automatically supplied
   CT_SYSST_REC_VERSION,			0, U32_FT,      Persistant_PT,
   CT_SYSST_SIZE,					0, U32_FT,      Persistant_PT,
   CT_SYSST_PRESENTIOPSMASK,        0, U32_FT,      Persistant_PT,
   CT_SYSST_IOPSONPCIMASK,			0, U32_FT,      Persistant_PT,
   CT_SYSST_ACTIVEIOPSMASK,         0, U32_FT,      Persistant_PT,
 };


//  size of field definition table, in bytes
const U32 cbSystemStatusTable_FieldDefs  =  sizeof (aSystemStatusTable_FieldDefs);



