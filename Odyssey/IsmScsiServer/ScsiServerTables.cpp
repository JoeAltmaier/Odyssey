/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiServerTables.cpp
// 
// Description:
// This module is the Scsi Target Server (STS) Table Handler
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiServerTables.cpp $
// 
// 6     11/17/99 1:59p Mpanas
// Fix stuck loop problem
// 
// 5     11/15/99 4:02p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// 
// 4     7/21/99 8:53p Mpanas
// resolve new PTS changes
// 
// 3     6/30/99 7:56p Mpanas
// Fail if the Export Table is not loaded for my Vdn
// 
// 2     6/06/99 4:31p Mpanas
// changed a comment
// 
// 1     4/05/99 10:13p Mpanas
// Initial checkin to
// Add Table support
// 
// 03/23/99 Michael G. Panas: Create file
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

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

/*************************************************************************/
// DmTableInitialize
// Start of the table initialization state machine, called from the Ddm Init
// Creates these tables if they do not exist:
//	StsDataTable (used to store Inq and Mode data)
// Reads these tables:
//	ExportTable
//	StsDataTable
/*************************************************************************/
STATUS	ScsiServerIsm::STSTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(STSTableInitialize);
	
	STATUS			status = OS_DETAIL_STATUS_SUCCESS;
	STS_TBL_CONTEXT *pTC = new STS_TBL_CONTEXT;
	
	pTC->pMsg = pMsg;
	pTC->pExTableRow = NULL;
	
	// start the state machine for SCSI Target Server initialization
	status = STSTblReadExport(pTC, ercOK);
	
	return (status);
} // STSTableInitialize



/*************************************************************************/
// STSTblReadExport
// last Create StsData Table CallBack, start the Export table read for our
// VirtualDevice number
/*************************************************************************/
STATUS ScsiServerIsm::STSTblReadExport(void *pClientContext, STATUS status)
{
	STS_TBL_CONTEXT *pTC = (STS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(STSTblReadExport);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rSTSTblReadExport: status = ", status);
	}
	
	// allocate a place for the row
	pStsExport = new ExportTableEntry;
	
	// Allocate a ReadRow object for the Export Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		EXPORT_TABLE,						// String64 rgbTableName,
		"vdNext",							// String64 prgbKeyFieldName,
		&MyVdn,								// void* pKeyFieldValue,
		sizeof(VDN),						// U32 cbKeyFieldValue,
		pStsExport,							// void* prgbRowData,
		sizeof(ExportTableEntry),			// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&STSTblCreateStsData,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the ReadRow table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	else
		status = STSTblCreateStsData(pTC, status);
	
	return status;
} // STSTblReadExport


/*************************************************************************/
// STSTblCreateStsData
// Create the StsData table if does not exist, fill in a default StsData
// entry for us if table did not exist or read our entry fails. If table
// does exist, try to read our entry.
/*************************************************************************/
STATUS ScsiServerIsm::STSTblCreateStsData(void *pClientContext, STATUS status)
{
	STS_TBL_CONTEXT *pTC = (STS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(STSTblCreateStsData);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rSTSTblCreateStsData: status = ", status);
		delete pStsExport;
		pStsExport = NULL;		// cause a fault if we try to use this
	}
	else
	{
		// The member data for our Export Table entry has been updated
		TRACE_HEX(TRACE_L8, "\n\rSTSTblCreateStsData: Read Export entry for = ", MyVdn);
	}
	
	// This is the code to create the StsData Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	m_pDefineTable = new TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		STS_DATA_TABLE,						// String64 prgbTableName,
		STSDataTable_FieldDefs,				// fieldDef* prgFieldDefsRet,
		cbSTSDataTable_FieldDefs,			// U32 cbrgFieldDefs,
		10,									// U32 cEntriesRsv,
		Persistant_PT,						// U32 PersistFlags,
		(pTSCallback_t)&STSTblCrStsDataReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();
	else
		status = STSTblCrStsDataReply1(pTC, status);
	
	return (status);
} // STSTblCreateStsData


/*************************************************************************/
// STSTblCrStsDataReply1
// Create StsData Table CallBack, check the status. If OK, table is new,
// create a default entry.  If error status, table may have existed, try
// to read our entry.
/*************************************************************************/
STATUS ScsiServerIsm::STSTblCrStsDataReply1(void *pClientContext, STATUS status)
{
	STS_TBL_CONTEXT *pTC = (STS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(STSTblCrStsDataReply1);
	
	if (status == ercOK)
	{
		// new table, create default entry
		status = STSTblReadDataReply1(pTC, ercKeyNotFound);
		return(status);
	}

	// old table, try to read StsData table entry for us
	TRACE_HEX(TRACE_L3, "\n\rSTSTblCrStsDataReply1: status = ", status);

	// Allocate a ReadRow object for the StsData Table.
	m_pSRCReadRow = new TSReadRow;

	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		STS_DATA_TABLE,						// String64 rgbTableName,
		"vdSTS",							// String64 prgbKeyFieldName,
		&MyVdn,								// void* pKeyFieldValue,
		sizeof(VDN),						// U32 cbKeyFieldValue,
		pData,								// void* prgbRowData,
		sizeof(StsData),					// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&STSTblReadDataReply1,	// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the ReadRow table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	else
		status = STSTblReadDataReply1(pTC, status);
	
	return status;
} // STSTblCrStsDataReply1


/*************************************************************************/
// STSTblReadDataReply1
// Read StsData Table CallBack, check the status. If OK, table is read.
// If error status, table entry did not exist, create a default entry for us.
/*************************************************************************/
STATUS ScsiServerIsm::STSTblReadDataReply1(void *pClientContext, STATUS status)
{
	STS_TBL_CONTEXT *pTC = (STS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(STSTblReadDataReply1);
	
	if (status == ercOK)
	{
		// table entry read, got to next state
		status = STSTblReplyLast(pTC, ercOK);
		return(status);
	}
	
	TRACE_HEX(TRACE_L3, "\n\rSTSTblReadDataReply1: status = ", status);
	
	// fill in default entry for us
	memset(pData, 0, sizeof(StsData));		// zero the structure
	
	pData->version = STS_DATA_VERSION;
	pData->size = sizeof(StsData);
	pData->vdSTS = MyVdn;
	
	// default values for inquiry data
	ScsiBuildInquiryData((void *)&pData->InqData);
	
	// default values for mode pages
	ScsiBuildModeSenseData((U8 *)pData->ModePages);

	// Allocate a InsertRow object for the StsData Table.
	m_pInsertRow = new TSInsertRow;

	// Initialize the insert row operation.
	status = m_pInsertRow->Initialize(
		this,								// DdmServices pDdmServices,
		STS_DATA_TABLE,						// String64 rgbTableName,
		pData,								// void* prgbRowData,
		sizeof(StsData),					// U32 cbRowData,
		&pData->rid,						// rowID *pRowIDRet,
		(pTSCallback_t)&STSTblReplyLast,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the InsertRow table operation.
	if (status == ercOK)
		m_pInsertRow->Send();
	else
		status = STSTblReplyLast(pTC, status);
	
	return status;
} // STSTblReadDataReply1



/*************************************************************************/
// STSTblReplyLast
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS ScsiServerIsm::STSTblReplyLast(void *pClientContext, STATUS status)
{
	STS_TBL_CONTEXT *pTC = (STS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(STSTblReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rSTSTblReplyLast: status = ", status);
	}
	
	// start the listens for this instance of the SCSI Target Server
	status = StsStartListens();
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rSTSTblReplyLast: StsStartListens() failed, status = ", status);
	}
	
	
	// don't return this until the last TS message
	if (pTC->pMsg)
		Reply(pTC->pMsg); 
	
	// delete our table context, we are done with it
	delete pTC;
	
	return OS_DETAIL_STATUS_SUCCESS;
	
} // STSTblReplyLast


