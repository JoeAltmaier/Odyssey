/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiTableListen.cpp
// 
// Description:
// This module is the SCSI Target Server Table Listen Handler
//	Handles:
//		Export Table Reads
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiTableListen.cpp $
// 
// 1     11/15/99 4:02p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// 
// 
// 11/08/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"

#include "ScsiServ.h"

// Tables referenced
#include "ExportTable.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

// Table method context
typedef struct _STS_TBL_CONTEXT {
	Message					*pMsg;				// saved Init message
	ExportTableEntry		*pExTableRow;
} STS_TBL_CONTEXT, *PSTS_TBL_CONTEXT;


#define	STS_LISTEN_MASK	(ListenOnDeleteOneRow|ListenOnModifyOneRowAnyField)

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// StsStartListens
// We will listen on the StsData Table and the Export Table.
// The both Tables will listen on Modify, Delete
/*************************************************************************/
STATUS 	ScsiServerIsm::StsStartListens(void)
{
	STATUS status;
	
	TRACE_ENTRY(StsStartListens);
	
	//
	// Start the listen on the StsData table
	//
	
	m_LSize = sizeof(StsData);
	
	// Allocate a Listen object for the LoopDescriptor table.
	m_Listen = new(tUNCACHED) TSListen;

	// Initialize the modify row operation.
	status = m_Listen->Initialize(
		this,								// DdmServices pDdmServices,
		STS_LISTEN_MASK,					// U32		ListenType,
		STS_DATA_TABLE,						// String64	prgbTableName,
		"vdSTS",							// String64	prgbRowKeyFieldName,
		(void*)&MyVdn,						// void*	prgbRowKeyFieldValue,
		sizeof(VDN),						// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyWithRow|ReplyContinuous),		// U32		ReplyMode, (1=once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_LType,							// U32**	ppListenTypeRet,
		&m_StsData,							// void**	ppModifiedRecordRet,
		&m_LSize,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&StsDataListenUpdate,	// pTSCallback_t pCallback,
		NULL								// void*	pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_Listen->Send();
	
	//
	//	Start the listen on the Export Table
	//
	
	m_LSize1 = sizeof(ExportTableEntry);
	
	// Allocate a Listen object for the LoopDescriptor table.
	m_Listen1 = new(tUNCACHED) TSListen;

	// Initialize the modify row operation.
	status = m_Listen1->Initialize(
		this,								// DdmServices pDdmServices,
		STS_LISTEN_MASK,					// U32		ListenType,
		EXPORT_TABLE,						// String64	prgbTableName,
		"vdNext",							// String64	prgbRowKeyFieldName,
		(void*)&MyVdn,						// void*	prgbRowKeyFieldValue,
		sizeof(U32),						// U32		cbRowKeyFieldValue,
		NULL,								// String64	prgbFieldName,
		NULL,								// void*	prgbFieldValue,
		0,									// U32		cbFieldValue,
		(ReplyWithRow|ReplyContinuous),		// U32		ReplyMode, (1=once)
		NULL,								// void**	ppTableDataRet,
		0,									// U32*		pcbTableDataRet,
		NULL,								// U32*		pListenerIDRet,
		&m_LType1,							// U32**	ppListenTypeRet,
		&m_Export,							// void**	ppModifiedRecordRet,
		&m_LSize1,							// U32*		pcbModifiedRecordRet,
		(pTSCallback_t)&StsExportListenUpdate,		// pTSCallback_t pCallback,
		NULL								// void*	pContext
	);

	// Initiate the Modify Row operation.
	if (status == ercOK)
		m_Listen1->Send();
	
	return status;
	
} // StsStartListens


/*************************************************************************/
// StsDataListenUpdate
// Called when a listen callback for the LoopDescriptor Table happens.
// Update the LoopDescriptors with the new row data.  Check if the
// DriveMonitor VDN has been set, send a scan message if it is.
/*************************************************************************/
STATUS	ScsiServerIsm::StsDataListenUpdate(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(StsDataListenUpdate);
	
	// no work on first reply
	if (*m_LType & ListenInitialReply)
	{
		return(0);
	}
	
	// Check what type of listen reply
	if (*m_LType & ListenOnModifyOneRowAnyField)
	{
		// store the row data that just came in
		memcpy(pData, m_StsData, sizeof(StsData));
	}
	else if (*m_LType & ListenOnDeleteOneRow)
	{
		// Row was deleted, mark circuit as OFFLINE
		m_VcStatus = VC_OFFLINE;
		
		delete pData;
		pData = NULL;
	}

	return status;
	
} // StsDataListenUpdate


/*************************************************************************/
// StsExportListenUpdate
// Called when a listen callback for the Export Table happens.
/*************************************************************************/
STATUS	ScsiServerIsm::StsExportListenUpdate(void *pClientContext, STATUS status)
{
	TRACE_ENTRY(StsExportListenUpdate);
	
	// no work on first reply
	if (*m_LType1 & ListenInitialReply)
	{
		return(ercOK);
	}
	
	// Check what type of listen reply
	if (*m_LType1 & ListenOnModifyOneRowAnyField)
	{		
		// store the row data that just came in
		memcpy(pStsExport, m_Export, sizeof(ExportTableEntry));

#if 0
		// Row was modified - check the (new) state
		switch(m_Export->ReadyState)
		{
			case StateConfiguredAndExporting:
				break;
			
			case StateConfiguredAndExported:
				break;

			case StateConfiguredAndActive:
				break;

			case StateOffLine:
				break;

			case StateQuiesced:
				break;

			default:
				// all other states are NOPs
				break;
		}
#endif
	}
	else if (*m_LType1 & ListenOnDeleteOneRow)
	{
		// Row was deleted, mark circuit as OFFLINE
		m_VcStatus = VC_OFFLINE;

		delete pStsExport;
		pStsExport = NULL;
	}

	status = ercOK;
	return status;
	
} // StsExportListenUpdate


