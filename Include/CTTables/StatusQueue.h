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
* StatusQueue.h
*
* Description:
*
* 
*
*************************************************************************/

#ifndef StatusQueue_h
#define StatusQueue_h

#include "CtTypes.h"
#include "TableMsgs.h"

#include "CommandQueue.h"

#pragma	pack(4)

extern fieldDef	StatusQueueTable_FieldDefs[];
extern U32		sizeofStatusQueueTable_FieldDefs;


extern fieldDef	StatusQueueTableNoResultData_FieldDefs[];
extern U32		sizeofStatusQueueTableNoResultData_FieldDefs;


#pragma pack(4)

typedef enum {
	SQ_COMMAND_STATUS = 1,
	SQ_EVENT_STATUS
} SQ_STATUS_TYPE;

/********************************************************************
*
* Field Def constants
*
********************************************************************/

#ifndef fdTHIS_ROWID
#define fdTHIS_ROWID		"rid"
#endif
#define fdTYPE				"Type"
#define fdSTATUS_CODE		"StatusCode"
#define fdRESULT_DATA		"ResultData"
#define fdCOMMAND_DATA		"CommandData"



/********************************************************************
*
* STATUS QUEUE
*
********************************************************************/

typedef struct
{
	rowID				thisRID;		// rid in descriptor table
	SQ_STATUS_TYPE		type;			// if Event or Command status
	STATUS				statusCode;		// status for cmd or Event code
}	StatusQueueRecord, StatusQueueTable[];

// Note:
//	StatusQueueRecord will contain resultData after statusCode
//	The data after resultData will be the commandQueueRecord data
//
//

// support below this comment was added to allow variable cmd and 
// status data in command queues
#ifdef VAR_CMD_QUEUE

#ifndef _PtsRecordBase_h
# include  "PtsRecordBase.h"
#endif

#pragma pack(4)      // standard packing for PTS things

//  table name
#define  CT_SQ_TABLE_NAME         "SQTable"

#define  CT_SQ_TABLE_VER          (1)     /* current struct version */

//
// SQRecord
//
class SQRecord : public CPtsRecordBase
{
public:
	SQ_STATUS_TYPE		type;			// if Event or Command status
	STATUS				statusCode;		// status for cmd or Event code
	rowID				cmdRowId;
	CPtsVarField<U8>	cmdData;
	CPtsVarField<U8>	statusData;

	SQRecord();

	SQRecord(SQ_STATUS_TYPE type_,
		STATUS status_,
		U32 cbStatusData, U8* pStatusData);

	SQRecord(SQ_STATUS_TYPE type_,
		STATUS status_,
		rowID cmdRowId_, U32 cbCmdData, U8* pCmdData,
		U32 cbStatusData, U8* pStatusData);

	//  here are the standard field defs for our row/table
    static const fieldDef *FieldDefs (void);

    //  and here is the size, in bytes, of our field defs
    static const U32 FieldDefsSize (void);

    //  here is the name of the PTS table whose rows we define
    static const char *TableName (void);

};  /* end of class SQRecord */

//  compiler-checkable aliases for table / field names

//  field defs
#define  CT_SQ_TABLE_REC_VERSION        CT_PTS_VER_FIELD_NAME // Version of File record.
#define  CT_SQ_TABLE_SIZE               CT_PTS_SIZE_FIELD_NAME// # of bytes in record.
#define  CT_SQ_TABLE_TYPE				"Type"           
#define  CT_SQ_TABLE_STATUS				"Status"           
#define  CT_SQ_TABLE_STATUSDATA			"StatusData"           
#define  CT_SQ_TABLE_CMDROWID			"CmdRowId"           
#define  CT_SQ_TABLE_CMDDATA			"CmdData"           

//  here is the standard table which defines SQ table fields
extern const fieldDef aSQTable_FieldDefs[];

//  and here is the size, in bytes, of the SQ table field defs
extern const U32 cbSQTable_FieldDefs;

#endif // VAR_CMD_QUEUE

#endif