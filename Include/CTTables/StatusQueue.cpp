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
* File Name:
* StatusQueue.cpp
* Description:
* This file defines the Status Queue Tale fields
* 
* Update Log: 
*
*************************************************************************/

#include "CtTypes.h"
#include "StatusQueue.h"


fieldDef	StatusQueueTable_FieldDefs[] = 
{
	fdTYPE,				4,	U32_FT,			Persistant_PT,
	fdSTATUS_CODE,		4,	U32_FT,			Persistant_PT,
	fdRESULT_DATA,		1,	BINARY_FT,		Persistant_PT,
	fdCOMMAND_DATA,		1,	BINARY_FT,		Persistant_PT
};				 


fieldDef	StatusQueueTableNoResultData_FieldDefs[] = 
{
	fdTYPE,				4,	U32_FT,			Persistant_PT,
	fdSTATUS_CODE,		4,	U32_FT,			Persistant_PT,
	fdCOMMAND_DATA,		1,	BINARY_FT,		Persistant_PT
};				 

//  size of field definition table, in bytes
U32 sizeofStatusQueueTable_FieldDefs  =  sizeof(StatusQueueTable_FieldDefs);
U32 sizeofStatusQueueTableNoResultData_FieldDefs  =  sizeof(StatusQueueTableNoResultData_FieldDefs);

// support below this comment was added to allow variable cmd and 
// status data in command queues

#ifdef VAR_CMD_QUEUE

#ifndef _DEBUG
#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#endif

//  verify that field defs agree with record def
CHECKFIELDDEFS (SQRecord);


const fieldDef aSQTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no
	CPTS_RECORD_BASE_FIELDS(Persistant_PT),
	CT_SQ_TABLE_TYPE,          0, U32_FT,    Persistant_PT,
	CT_SQ_TABLE_STATUS,          0, U32_FT,    Persistant_PT,
	CT_SQ_TABLE_CMDROWID, 0, ROWID_FT, Persistant_PT,      
	CT_SQ_TABLE_CMDDATA,          8, BINARY_FT,    VarLength_PT | Persistant_PT,
	CT_SQ_TABLE_STATUSDATA,          8, BINARY_FT,    VarLength_PT | Persistant_PT
};


//  size of field definition table, in bytes
const U32 cbSQTable_FieldDefs  =  sizeof (aSQTable_FieldDefs);

SQRecord::SQRecord() : CPtsRecordBase (sizeof (SQRecord),
											CT_SQ_TABLE_VER, 2)
{
} 
 
SQRecord::SQRecord(SQ_STATUS_TYPE type_,
		STATUS status_,
		U32 cbStatusData, U8* pStatusData) : CPtsRecordBase (sizeof (SQRecord),
											CT_SQ_TABLE_VER, 2)
{
	type = type_;
	statusCode = status_;
	cmdData.Set(0, 0);
	statusData.Set(pStatusData, cbStatusData);
}  

SQRecord::SQRecord(SQ_STATUS_TYPE type_,
		STATUS status_,
		rowID cmdRowId_, U32 cbCmdData, U8* pCmdData,
		U32 cbStatusData, U8* pStatusData) : CPtsRecordBase (sizeof (SQRecord),
											CT_SQ_TABLE_VER, 2)
{
	type = type_;
	statusCode = status_;
	cmdRowId = cmdRowId_;
	cmdData.Set(pCmdData, cbCmdData, CPtsVarField<U8>::k_eMakeCopy);
	statusData.Set(pStatusData, cbStatusData, CPtsVarField<U8>::k_eMakeCopy);
}  

//  here are the standard field defs for our row/table
/* static */
const fieldDef *SQRecord::FieldDefs (void)
{
   return (aSQTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 SQRecord::FieldDefsSize (void)
{
   return (cbSQTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *SQRecord::TableName (void)
{
   return (CT_SQ_TABLE_NAME);
}

#endif // VAR_CMD_QUEUE
