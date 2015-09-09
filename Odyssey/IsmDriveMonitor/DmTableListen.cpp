/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmOther.cpp
// 
// Description:
// This module handles Table Listens used by the DriveMonitor
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmTableListen.cpp $
// 
// 2     1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 1     9/14/99 8:40p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 
// 08/23/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "Scsi.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "DriveMonitorIsm.h"
#include "DmCommon.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"


// Table Listen method context
typedef struct _LM_TBL_CONTEXT {
	U32						 LoopInstance;
} LM_TBL_CONTEXT, *PLM_TBL_CONTEXT;


#define	DM_ROLL_CALL_LISTEN_MASK	(ListenOnInsertRow|ListenOnDeleteOneRow|ListenOnModifyOneRowAnyField)

// Table Listen method context
typedef struct _DM_TBL_CONTEXT {
	Message					*pMsg;				// saved Init message
	U32						index;				// drive number index
	DM_CONTEXT				*pDmc;				// Drive Monitor internal context
	DiskDescriptor			*pDDTableRow;
	StorageRollCallRecord	*pRCTableRow;
} DM_TBL_CONTEXT, *PDM_TBL_CONTEXT;


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// DMStartListens
// We will listen on the StorageRollCall Table.
// The StorageRollCall Table will listen on Modify, Delete, and Add
/*************************************************************************/
STATUS 	DriveMonitorIsm::DMStartListens(void)
{
	DM_TBL_CONTEXT *pTC;
	STATUS status;
	
	TRACE_ENTRY(DMStartListens);
	
	//
	// Start the listen on the StorageRollCall table
	//
	
	pTC = new DM_TBL_CONTEXT;
	
	m_LSize = sizeof(StorageRollCallRecord);
	
	// Allocate a Listen object for the StorageRollCall table.
	m_Listen = new(tUNCACHED) TSListen;

	// Initialize the Listen operation.
	status = m_Listen->Initialize(
		this,								// DdmServices pDdmServices,
		DM_ROLL_CALL_LISTEN_MASK,			// U32		ListenType,
		STORAGE_ROLL_CALL_TABLE,			// String64	prgbTableName,
		fdSRC_MONITOR_VDN,					// String64	prgbRowKeyFieldName,
		(void*)&MyVd,						// void*	prgbRowKeyFieldValue,
		sizeof(VDN),						// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyWithRow|ReplyContinuous),		// U32		ReplyMode, (1=once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_LType,							// U32**	ppListenTypeRet,
		&m_Roll_Call,						// void**	ppModifiedRecordRet,
		&m_LSize,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&DmRCListenUpdate,	// pTSCallback_t pCallback,
		pTC									// void*	pContext
	);

	// Initiate the Listen operation.
	if (status == ercOK)
		m_Listen->Send();
	
	return status;
	
} // DMStartListens

/*************************************************************************/
// DmRCListenUpdate
// Called when a listen callback happens. This method will update locally
// a table row entry that has changed.
/*************************************************************************/
STATUS	DriveMonitorIsm::DmRCListenUpdate(void *pClientContext, STATUS status)
{
	DM_TBL_CONTEXT *pTC = (DM_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(DmRCListenUpdate);
	
	// no work on first reply
	if (*m_LType & ListenInitialReply)
	{
		return(0);
	}
	
	// Check what type of listen reply
	if (*m_LType & ListenOnModifyOneRowAnyField)
	{
		// lookup this RollCall entry
		
		// check for changes?
		// TODO: what do we do here?
	}

	return status;
	
} // DmRCListenUpdate


