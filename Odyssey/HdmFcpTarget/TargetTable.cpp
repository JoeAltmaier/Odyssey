/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: TargetTables.cpp
// 
// Description:
// This module is the Target Table Handler
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTarget/TargetTable.cpp $
// 
// 4     1/09/00 8:14p Mpanas
// Support for Read of Export table using the vdNext field
// as the key (Claim VC support)
// 
// 3     8/20/99 7:50p Mpanas
// Changes to support Export Table states
// Re-organize sources
// 
// 2     7/21/99 8:12p Mpanas
// remove return value from PTS Send() methods
// 
// 1     7/15/99 11:48p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Target DDM project
// 
// 5     6/30/99 7:57p Mpanas
// Change Trace levels
// 
// 4     5/12/99 3:50p Mpanas
// change Trace.h to Odyssey_Trace.h
// 
// 3     5/07/99 1:41p Mpanas
// minor changes to  fix compiler warnings
// 
// 2     4/01/99 1:45p Mpanas
// Changes to match with new CHAOS
// 
// 1     3/22/99 8:24p Mpanas
// Initial Checkin
// Target FCP driver PTS support code
// 
// 03/16/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "CTIdLun.h"

#include "Trace_Index.h"
#define	TRACE_INDEX		TRACE_FCP_TARGET
#include "Odyssey_Trace.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"
#include "fields.h"

// Tables referenced
#include "ExportTable.h"

#include "HdmFcpTarget.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

// Table method context
typedef struct _TAR_TBL_CONTEXT {
	void							*pContext;			// other context
	HdmFcpTarget::pTgtCallback_t	 Callback;			// saved Callback address
	Message							*pMsg;				// saved Init message
	ExportTableEntry				*pExTableRow;
	VDN								 vdn;				// saved Virtual device number
	void							*buffer;
	U32								 length;
} TAR_TBL_CONTEXT, *PTAR_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// TarTableInitialize
// Start of the table initialization state machine, called from the Ddm Init
// Creates these tables if they do not exist:
//	ExportTable
// Reads these tables:
//	ExportTable
/*************************************************************************/
STATUS	HdmFcpTarget::TarTableInitialize(Message *pMsg)
{
	TRACE_ENTRY(TarTableInitialize);
	
	STATUS			status = ercOK;
	TAR_TBL_CONTEXT *pTC = new TAR_TBL_CONTEXT;
	
	pTC->pMsg = pMsg;
	
	// This is the code to create the Export Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	fieldDef *pciExportTable_FieldDefs = 
				(fieldDef*)new(tPCI) char[cbExportTable_FieldDefs];

	memcpy( (char*)pciExportTable_FieldDefs,
			(char*)ExportTable_FieldDefs,
			cbExportTable_FieldDefs
		  ); 
		  
	m_pDefineTable = new(tUNCACHED) TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		EXPORT_TABLE,						// String64 prgbTableName,
		pciExportTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbExportTable_FieldDefs,			// U32 cbrgFieldDefs,
		25,									// U32 cEntriesRsv,
		false,								// bool* pfPersistant,
		(pTSCallback_t)&TarTblReply2,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	m_pDefineTable->Send();

	return status;
} // TarTableInitialize

/*************************************************************************/
// TarTblReply2
// The Export table now exists.  Read how many entries (rows) 
// there current are.  Use this count to to build the array for local use.
/*************************************************************************/
STATUS HdmFcpTarget::TarTblReply2(void *pClientContext, STATUS status)
{
	TAR_TBL_CONTEXT *pTC = (TAR_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(TarTblReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTarTblReply2: status = ", status);
	}
	
	m_nTableRows = 0;

	// Allocate an Get Table Defs object for the Export Table.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the GetTableDef table operation.
	status = m_pTSGetTableDef->Initialize( 
		this,
		EXPORT_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nTableRows,				// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&TarTblReply3,		// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the GetTableDef table operation.
	m_pTSGetTableDef->Send();
	
	return status;
} // TarTblReply2

/*************************************************************************/
// TarTblReply3
// Number of rows is now read into m_nTableRows.  Use this count to
// create the local Export table. And start the read of all the entries.
/*************************************************************************/
STATUS HdmFcpTarget::TarTblReply3(void *pClientContext, STATUS status)
{
	TAR_TBL_CONTEXT *pTC = (TAR_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(TarTblReply3);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTarTblReply3: status = ", status);
	}
	
	// check for a prior table
	if (Tar_TS_Export)
	{
		delete Tar_TS_Export;
	}
	
	TRACE_HEX(TRACE_L8, "\n\rTar: num of Export Table rows = ", m_nTableRows);
	
	// Allocate the Local Export Table
	Tar_TS_Export = new ExportTableEntry[m_nTableRows];
		
	// Allocate an Enumerate Table object for the Export Table.
	m_EnumTable = new(tUNCACHED) TSEnumTable;

	// Initialize the enumerate table operation.
	status = m_EnumTable->Initialize(
		this,
		EXPORT_TABLE,						// Name of table to read.
		0,									// Starting row number.
		Tar_TS_Export,						// Returned data buffer.
		sizeof(ExportTableEntry) * m_nTableRows,	// max size of returned data.
		&m_numBytesEnumed,					// pointer to # of returned bytes.
		(pTSCallback_t)&TarTblReplyLast,	// pTSCallback_t pCallback,
		(void*)pTC							// pContext
						  );

	// Initiate the enumerate table operation.
	m_EnumTable->Send();
	
	return status;
} // TarTblReply3

/*************************************************************************/
// TarTblReplyLast
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS HdmFcpTarget::TarTblReplyLast(void *pClientContext, STATUS status)
{
	TAR_TBL_CONTEXT *pTC = (TAR_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(TarTblReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTarTblReplyLast: status = ", status);
	}
	
	// Start the Listen on Export table changes
	TarStartListen(config.loop_instance);
		
	// Build local LUN translation tables based on 
	// the table we just loaded
	Target_Build_Tables();
	
	// don't reply to the init message until the last TS message
	// and the Hash Tables are built
	if (pTC->pMsg)
		Reply(pTC->pMsg); 
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // TarTblReplyLast



/*************************************************************************/
// TarTblModifyExportState
// Modify the ExportTableEntry ReadyState field with the new state passed.
// no callback or context are used. pState and pRow must point to static data!
/*************************************************************************/
STATUS 	HdmFcpTarget::TarTblModifyExportState(rowID *pRow, CTReadyState *pState)
{
	STATUS			 status;
	TSModifyField 	*pModifyField;
	
	TRACE_ENTRY(TarTblModifyExportState);
	
	// Allocate an Modify Field object for the Export table
	pModifyField = new TSModifyField;

	// Initialize the modify field operation.
	status = pModifyField->Initialize(
		this,							// DdmServices pDdmServices,
		EXPORT_TABLE,					// String64 rgbTableName,
		"rid",							// String64 prgbKeyFieldName,
		(void*)pRow,					// void* pKeyFieldValue,
		sizeof(rowID),					// U32 cbKeyFieldValue,
		"ReadyState",					// String64 prgbFieldName,
		(void*)pState,					// void* pbFieldValue,
		sizeof(CTReadyState),			// U32 cbFieldValue,
		0,								// U32 cRowsToModify,
		NULL,							// U32 *pcRowsModifiedRet,
		NULL,							// rowID *pRowIDRet,
		0,								// U32 cbMaxRowID,
		(pTSCallback_t)&TarTblModifyExportStateCallback, // pTSCallback_t pCallback,
		(void*)NULL						// void* pContext
	);

	// Initiate the Modify Field operation.
	if (status == ercOK)
		pModifyField->Send();
	
	return status;
	
} // TarTblModifyExportState


/*************************************************************************/
// TarTblModifyExportStateCallback
// Export table State change callback, do nothing here
/*************************************************************************/
STATUS HdmFcpTarget::TarTblModifyExportStateCallback(void *pClientContext, STATUS status)
{

	return(status);
} // TarTblModifyExportStateCallback


/*************************************************************************/
// TarTblMatchVdn
// Read all the Export table entries that match the Vdn passed in Vdn. The
// rows are returned in whatever buffer points to.  Max buffer length is 
// in length.  pContext is returned when the Callback method is called.
/*************************************************************************/
STATUS HdmFcpTarget::TarTblMatchVdn(void *pContext, 
										pTgtCallback_t Callback,
										void *buffer,
										U32	length,
										VDN vdn)
{
	STATUS			 status;
	TAR_TBL_CONTEXT *pTC = new TAR_TBL_CONTEXT;
	
	TRACE_ENTRY(TarTblMatchVdn);
	
	// build a local context (need static values)
	pTC->pContext = pContext;
	pTC->Callback = Callback;
	pTC->vdn = vdn;
	pTC->buffer = buffer;
	pTC->length = length;
	
	// Allocate a ReadRow object for the Export Table.
	m_pSRCReadRow = new TSReadRow;

	// Use the vdNext field as the key to read in the row desired
	// Initialize the read row operation.
	status = m_pSRCReadRow->Initialize(
		this,								// DdmServices pDdmServices,
		EXPORT_TABLE,						// String64 rgbTableName,
		"vdNext",							// String64 prgbKeyFieldName,
		(void*)&pTC->vdn,					// void* pKeyFieldValue,
		sizeof(VDN),						// U32 cbKeyFieldValue,
		buffer,								// void* prgbRowData,
		length,								// U32 cbRowData,
		NULL,								// rowID *pRowIDRet,
		(pTSCallback_t)&TarTblMatchVdnReplyLast, // pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
} // TarTblMatchVdn


/*************************************************************************/
// TarTblMatchVdnReplyLast
// Call the callback (if any) and cleanup
/*************************************************************************/
STATUS HdmFcpTarget::TarTblMatchVdnReplyLast(void *pClientContext, STATUS status)
{
	TAR_TBL_CONTEXT *pTC = (TAR_TBL_CONTEXT *)pClientContext;
	pTgtCallback_t 	 Callback = pTC->Callback;

	TRACE_ENTRY(TarTblMatchVdnReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTarTblMatchVdnReplyLast: status = ", status);
	}
	
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pTC->pContext, status);
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // TarTblReplyLast




