/*************************************************************************
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* File Name:
* CommandQueue.h
*
* Description:
*
* 
*
*************************************************************************/

#ifndef VAR_CMD_QUEUE
#define VAR_CMD_QUEUE
#endif

#ifndef CommandQueue_h
#define CommandQueue_h

#include "CtTypes.h"
#include "TableMsgs.h"
#pragma	pack(4)

extern fieldDef	CommandQueueTable_FieldDefs[];
extern U32		sizeofCommandQueueTable_FieldDefs;


#pragma pack(4)
/********************************************************************
*
* Field Def constants
*
********************************************************************/

#define fdOPCODE		"Opcode"			
#define fdPARAM_DATA	"ParamData"
#define fdCONTEXT_DATA	"ContextData"			


/********************************************************************
*
* COMMAND QUEUE
*
********************************************************************/

typedef struct
{
	rowID				thisRID;			// rid in descriptor table
}	CommandQueueRecord, CommandQueue[];

// support below this comment was added to allow variable cmd and 
// status data in command queues
#ifdef VAR_CMD_QUEUE

#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_CQ_TABLE_NAME         "CQTable"

#define  CT_CQ_TABLE_VER          (1)     /* current struct version */

//
// CQRecord
//
class CQRecord : public CPtsRecordBase
{
public:
	CPtsVarField<U8>	cmdData;

	CQRecord();

	CQRecord(U32 cbCmdData, U8* pCmdData);

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

};  /* end of class CQRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define  CT_CQ_TABLE_REC_VERSION        CT_PTS_VER_FIELD_NAME // Version of File record.
#define  CT_CQ_TABLE_SIZE               CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define  CT_CQ_TABLE_CMDDATA			"CmdData"           

//  here is the standard table which defines CQ table fields
extern const fieldDef aCQTable_FieldDefs[];

//  and here is the size, in bytes, of the CQ table field defs
extern const U32 cbCQTable_FieldDefs;

#endif // VAR_CMD_QUEUE

#endif