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
// $Archive: /Gemini/Include/CTTables/EnvControlTable.cpp $
// 
// Description:
//   This file contains the PTS field definitions used to create the
//   Environment Control Table.
// 
// $Log: /Gemini/Include/CTTables/EnvControlTable.cpp $
// 
// 2     9/08/99 9:06a Vnguyen
// Change Temperature Threshold from U32 to S32.
// 
// 1     9/03/99 5:39p Ewedel
// Initial revision.
//
/*************************************************************************/

#include  "EnvControlTable.h"


//  here's the raw array, which we keep private:

static const fieldDef aMyFieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName                  Size  Type         Persist yes/no

   CPTS_RECORD_BASE_FIELDS (Persistant_PT),

   CT_ENVCR_FANSPEEDSET,            8, U32_FT,      Persistant_PT,   // 2xU32
   CT_ENVCR_EXITTEMPFANUPTHRESH,    0, S32_FT,      Persistant_PT,
   CT_ENVCR_EXITTEMPFANNORMTHRESH,  0, S32_FT,      Persistant_PT,

};


//  our constructor - a simple beast
CtEnvControlRecord::CtEnvControlRecord (void) :
                        CPtsRecordBase (sizeof (CtEnvControlRecord),
                                        CT_ENV_CONTROL_TABLE_VER)
{


   FanSpeedSet[0] = FanSpeedSet[1] = 0;      // default target RPMs

   ExitTempFanUpThresh   = 0;       // degrees C
   ExitTempFanNormThresh = 0;       //    "    "

   return;

}  /* end of CtEnvControlRecord::CtEnvControlRecord */


//  here are the routines which make the table defs publicly visible:

//  the name of the PTS table whose rows we define
const char *CtEnvControlRecord::TableName (void)
{
   return ("CtEnvControlTable");
}

//  here is the standard table which defines EVC Raw Param table PTS fields
const fieldDef *CtEnvControlRecord::FieldDefs (void)
{
   return (aMyFieldDefs);
}


//  and here is the size, in bytes, of the EVC Raw Param table field defs
const U32 CtEnvControlRecord::FieldDefsSize (void)
{
   return (sizeof (aMyFieldDefs));
}




