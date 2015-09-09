/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: TargetInitTables.cpp
// 
// Description:
// This module is the Target Table Handler
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpTargetInit/TargetInitTable.cpp $
// 
// 6     1/09/00 8:14p Mpanas
// Support for Read of Export table using the vdNext field
// as the key (Claim VC support)
// 
// 5     8/25/99 7:50p Mpanas
// Added ExportTable pdate code copied from Target project.  [JFL]
// 
// 4     8/23/99 2:20p Mpanas
// Changes to support Export Table States
// 
// 3     7/21/99 8:51p Mpanas
// resolve new PTS changes
// 
// 2     7/21/99 7:48p Mpanas
// Remove extern ref to Export Table
// 
// 1     7/15/99 11:48p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Target and Initiator DDM project
// 
// 
// 07/06/99 Michael G. Panas: Create file from TargetTables.cpp
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
#include "Fields.h"
#include "Listen.h"

// Tables referenced
#include "ExportTable.h"

#include "HdmFcpTargetInit.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

// Table method context
typedef struct _TI_TBL_CONTEXT {
	void							*pContext;			// other context
	HdmFcpTargetInit::pTgtCallback_t Callback;			// saved Callback address
	Message							*pMsg;				// saved Init message
	ExportTableEntry				*pExTableRow;
	VDN								 vdn;				// saved Virtual device number
	void							*buffer;
	U32								 length;
} TI_TBL_CONTEXT, *PTI_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// TI_TableInitialize
// Start of the table initialization state machine, called from the Ddm Init
// Creates these tables if they do not exist:
//	ExportTable
// Reads these tables:
//	ExportTable
/*************************************************************************/
STATUS	HdmFcpTargetInit::TI_TableInitialize(Message *pMsg)
{
	TRACE_ENTRY(TI_TableInitialize);
	
	STATUS			status = ercOK;
	TI_TBL_CONTEXT *pTC = new TI_TBL_CONTEXT;
	
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

	m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		EXPORT_TABLE,						// String64 prgbTableName,
		pciExportTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbExportTable_FieldDefs,			// U32 cbrgFieldDefs,
		25,									// U32 cEntriesRsv,
		false,								// bool* pfPersistant,
		(pTSCallback_t)&TI_TblReply2,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	m_pDefineTable->Send();

	return status;
} // TI_TableInitialize

/*************************************************************************/
// TI_TblReply2
// The Export table now exists.  Read how many entries (rows) 
// there current are.  Use this count to to build the array for local use.
/*************************************************************************/
STATUS HdmFcpTargetInit::TI_TblReply2(void *pClientContext, STATUS status)
{
	TI_TBL_CONTEXT *pTC = (TI_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(TI_TblReply2);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTI_TblReply2: status = ", status);
	}
	
	m_nTableRows = 0;

	// Allocate a Get Table Defs object for the Export Table.
	m_pTSGetTableDef = new(tUNCACHED) TSGetTableDef;

	// Initialize the GetTableDef table operation.
	m_pTSGetTableDef->Initialize( 
		this,
		EXPORT_TABLE,
		NULL,						// rgFieldDefsRet
		0,							// cbrgFieldDefsRetMax
		NULL,						// pcFieldDefsRet
		NULL,						// pcbRowRet
		&m_nTableRows,				// pcRowsRet Returned data buffer (row count).
		NULL,						// pcFieldsRet
		NULL,						// pPersistFlagsRet
		(pTSCallback_t)&TI_TblReply3,		// pTSCallback_t pCallback,
		(void*)pTC					// pContext
	  );

	// Initiate the enumerate table operation.
	m_pTSGetTableDef->Send();
	
	return status;
} // TI_TblReply2

/*************************************************************************/
// TI_TblReply3
// Number of rows is now read into m_nTableRows.  Use this count to
// create the local Export table. And start the read of all the entries.
/*************************************************************************/
STATUS HdmFcpTargetInit::TI_TblReply3(void *pClientContext, STATUS status)
{
	TI_TBL_CONTEXT *pTC = (TI_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(TI_TblReply3);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTI_TblReply3: status = ", status);
	}
	
	// check for a prior table
	if (TI_TS_Export)
	{
		delete TI_TS_Export;
	}
	
	TRACE_HEX(TRACE_L8, "\n\rTI_: num of Export Table rows = ", m_nTableRows);
	
	// Allocate the Local Export Table
	TI_TS_Export = new(tPCI) ExportTableEntry[m_nTableRows];
		
	// Allocate an Enumerate Table object for the Export Table.
	m_EnumTable = new(tUNCACHED) TSEnumTable;

	// Initialize the enumerate table operation.
	m_EnumTable->Initialize(
		this,
		EXPORT_TABLE,						// Name of table to read.
		0,									// Starting row number.
		TI_TS_Export,						// Returned data buffer.
		sizeof(ExportTableEntry) * m_nTableRows,	// max size of returned data.
		&m_numBytesEnumed,					// pointer to # of returned bytes.
		(pTSCallback_t)&TI_TblReplyLast,	// pTSCallback_t pCallback,
		(void*)pTC							// pContext
						  );

	// Initiate the enumerate table operation.
	m_EnumTable->Send();
	
	return status;
} // TI_TblReply3

/*************************************************************************/
// TI_TblReplyLast
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS HdmFcpTargetInit::TI_TblReplyLast(void *pClientContext, STATUS status)
{
	TI_TBL_CONTEXT *pTC = (TI_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(TI_TblReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rTI_TblReplyLast: status = ", status);
	}
	
	// Start the Listen on Export table changes
	TI_StartListen(config.loop_instance);
		
	// Build local LUN translation tables based on 
	// the table we just loaded
	TI_Build_Tables();
		
	// don't reply to the init message until the last TS message
	// and the Hash Tables are built
	if (pTC->pMsg)
		Reply(pTC->pMsg); 
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // TI_TblReplyLast

/*************************************************************************/
// TI_TblModifyExportState
// Modify the ExportTableEntry ReadyState field with the new state passed.
// no callback or context are used. pState and pRow must point to static data!
/*************************************************************************/
STATUS 	HdmFcpTargetInit::TI_TblModifyExportState(rowID *pRow, CTReadyState *pState)
{
	STATUS			 status;
	TSModifyField 	*pModifyField;
	
	TRACE_ENTRY(TarTblModifyExportState);
	
	// Allocate an Modify Field object for the Export table
	pModifyField = new TSModifyField;

	// Initialize the modify row operation.
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
		(pTSCallback_t)&TI_TblModifyExportStateCallback, // pTSCallback_t pCallback,
		(void*)NULL						// void* pContext
	);

	// Initiate the Modify Field operation.
	if (status == ercOK)
		pModifyField->Send();
	
	return status;
	
} // TarTblModifyExportState


/*************************************************************************/
// TI_TblModifyExportStateCallback
// Export table State change callback, do nothing here
/*************************************************************************/
STATUS HdmFcpTargetInit::TI_TblModifyExportStateCallback(void *pClientContext, STATUS status)
{

	return(status);
} // TarTblModifyExportStateCallback


/*************************************************************************/
// TI_TblMatchVdn
// Read all the Export table entries that match the Vdn passed in Vdn. The
// rows are returned in whatever buffer points to.  Max buffer length is 
// in length.  pContext is returned when the Callback method is called.
/*************************************************************************/
STATUS HdmFcpTargetInit::TI_TblMatchVdn(void *pContext, 
										pTgtCallback_t Callback,
										void *buffer,
										U32	length,
										VDN vdn)
{
	STATUS			 status;
	TI_TBL_CONTEXT *pTC = new TI_TBL_CONTEXT;
	
	TRACE_ENTRY(TI_TblMatchVdn);
	
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
		(pTSCallback_t)&TI_TblMatchVdnReplyLast, // pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);

	// Initiate the enumerate table operation.
	if (status == ercOK)
		m_pSRCReadRow->Send();
	
	return status;
} // TI_TblMatchVdn


/*************************************************************************/
// TI_TblMatchVdnReplyLast
// Call the callback (if any) and cleanup
/*************************************************************************/
STATUS HdmFcpTargetInit::TI_TblMatchVdnReplyLast(void *pClientContext, STATUS status)
{
	TI_TBL_CONTEXT *pTC = (TI_TBL_CONTEXT *)pClientContext;
	pTgtCallback_t 	 Callback = pTC->Callback;

	TRACE_ENTRY(TI_TblMatchVdnReplyLast);
	
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
	
} // TI_TblMatchVdnReplyLast




