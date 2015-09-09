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
* CommandQueue.cpp
* Description:
* This file defines the Command Queue Tale fields
* 
* Update Log: 
*
*************************************************************************/

#include "CtTypes.h"
#include "CommandQueue.h"

fieldDef	CommandQueueTable_FieldDefs[] = 
{
	fdPARAM_DATA,		64,	BINARY_FT,		Persistant_PT
};				 

//  size of field definition table, in bytes
U32 sizeofCommandQueueTable_FieldDefs  =  sizeof(CommandQueueTable_FieldDefs);

// support below this comment was added to allow variable cmd and 
// status data in command queues
#ifdef VAR_CMD_QUEUE

#ifndef _DEBUG
#define _DEBUG    /* needed by CHECKFIELDDEFS(), otherwise harmless */
#endif

//  verify that field defs agree with record def
CHECKFIELDDEFS (CQRecord);


const fieldDef aCQTable_FieldDefs[] = { 
   // Field Definitions follow one per row.
   // FieldName               Size  Type         Persist yes/no
	CPTS_RECORD_BASE_FIELDS(Persistant_PT),
	CT_CQ_TABLE_CMDDATA,          8, BINARY_FT,    VarLength_PT | Persistant_PT
 };


//  size of field definition table, in bytes
const U32 cbCQTable_FieldDefs  =  sizeof (aCQTable_FieldDefs);

CQRecord::CQRecord() : CPtsRecordBase (sizeof (CQRecord),
											CT_CQ_TABLE_VER, 1)
{
}  

CQRecord::CQRecord(U32 cbCmdData, U8* pCmdData) : CPtsRecordBase (sizeof (CQRecord),
											CT_CQ_TABLE_VER, 1)
{
	cmdData.Set(pCmdData, cbCmdData);
}  

//  here are the standard field defs for our row/table
/* static */
const fieldDef *CQRecord::FieldDefs (void)
{
   return (aCQTable_FieldDefs);
}

//  and here is the size, in bytes, of our field defs
/* static */
const U32 CQRecord::FieldDefsSize (void)
{
   return (cbCQTable_FieldDefs);
}

//  here is the name of the PTS table whose rows we define
/* static */
const char *CQRecord::TableName (void)
{
   return (CT_CQ_TABLE_NAME);
}

#endif // VAR_CMD_QUEUE
