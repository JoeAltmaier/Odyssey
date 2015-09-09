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
// Description:
//   This file contains the PTS field definitions used to create the
//   EVC Policy table.
// 
// $Log: /Gemini/Include/CTTables/EVCPolicyRecord.cpp $
// 
// 1     12/13/99 1:30p Vnguyen
// Initial check-in for Environment Ddm.
// 
/*************************************************************************/

#include "EVCPolicyRecord.h"

//  verify that field defs agree with record def
CHECKFIELDDEFS (EVCStatusRecord);


//  should be private (static), but left public for legacy support:
const fieldDef aEvcPolicyTable_FieldDefs[] = { 
	// Field Definitions follow one per row.
	// FieldName                  Size  Type         Persist yes/no

	CPTS_RECORD_BASE_FIELDS (Persistant_PT),

	CT_EVCST_SENSORNAME,		8, S32_FT,	NotPersistant_PT,

	CT_EVCST_NTW,				8, S32_FT,	NotPersistant_PT,
	CT_EVCST_WTN,				8, S32_FT,	NotPersistant_PT,
	CT_EVCST_WTA,				8, S32_FT,	NotPersistant_PT,
	CT_EVCST_ATW,				8, S32_FT,	NotPersistant_PT,
	CT_EVCST_NTA,				8, S32_FT,	NotPersistant_PT,
	CT_EVCST_ATN,				8, S32_FT,	NotPersistant_PT
};

//  size of field definition table, in bytes
const U32 cbEvcPolicyTable_FieldDefs  =  sizeof (aEvcPolicyTable_FieldDefs);


EVCPolicyRecord::EVCPolicyRecord () : CPtsRecordBase(sizeof (EVCPolicyRecord),
                                                     EVC_POLICY_TABLE_VER)
{
	NormalToWarning = 0;
	WarningToNormal = 0;
	WarningToAlarm = 0;
	AlarmToWarning = 0;
	NormalToAlarm = 0;
	AlarmToNormal = 0;
}  /* end of EVCPolicyRecord::EVCPolicyRecord */

//  here is the standard table which defines EVC Status table PTS fields
const fieldDef *EVCPolicyRecord::FieldDefs()
{
	return (aEvcPolicyTable_FieldDefs);
}


//  and here is the size, in bytes, of the EVC Status table field defs
const U32 EVCPolicyRecord::FieldDefsSize()
{
	return (sizeof (aEvcPolicyTable_FieldDefs));
}


//  here is the name of the PTS table whose rows we define
const char *EVCPolicyRecord::TableName()
{
	return ("EVC_Policy_Table");
}
