/* DdmPtsDefault.cpp -- SystemEntry Ddm to load PTS table defaults
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/
 
// Revision History:
//	$Log: /Gemini/Odyssey/DdmPtsDefault/DdmPtsDefault.cpp $
// 
// 5     8/10/99 1:18p Jlane
// Removed tUNCACHED and tPCI fropm message and SGL new operations.  These
// modifiers should no longer be necessary.
// 
// 4     7/21/99 10:16p Mpanas
// resolve latest PTS changes
// 
// 3     7/08/99 3:28p Mpanas
// Change to match latest Oos changes
// 
// 2     7/01/99 3:06p Mpanas
// Fix arg name in ProcessPingSlotReply()
// 
// 1     6/30/99 7:38p Mpanas
// New SystemEntry DDM to initialize PTS tables before anyone executes a
// request
//
//  6/30/99 Tom Nelson: Create file
//	6/30/99 Michael G. Panas: add code to init the Export Table
//

// 90 columns
//3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

// Public Includes

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"

#define _TRACEF
#define	TRACE_INDEX		TRACE_PTSDEFAULT
#include "Odyssey_Trace.h"


#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "Odyssey.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

// Tables referenced
#include "ExportTable.h"

// Private Includes
#include "DdmPtsDefault.h"

// BuildSys Linkage
#include "BuildSys.h"

CLASSNAME(DdmPtsDefault, SINGLE);

// Table method context
typedef struct _PTS_TBL_CONTEXT {
	Message					*pMsg;				// saved Init message
	ExportTableEntry		*pExTableRow;
} PTS_TBL_CONTEXT, *PPTS_TBL_CONTEXT;

/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/
// Default Table Data for test (defined in BuildSys.cpp)
extern	U32					numExportRows;
extern	ExportTableEntry	Export[];		 	// one for each VirtualCircuit



// .DdmPtsLoader -- Constructor --------------------------------------------DdmPtsDefault-
//
DdmPtsDefault::DdmPtsDefault(DID did) : Ddm(did) {

	TRACEF(TRACE_L8, ("EXEC  DdmPtsDefault::DdmPtsDefault\n"));

}

// .Initialize -- Process Initialize ---------------------------------------DdmPtsDefault-
//
STATUS DdmPtsDefault::Initialize(Message *pArgMsg) {

	TRACEF(TRACE_L8, ("EXEC  DdmPtsDefault::Initialize;\n"));
	
	// load the defaults into the PTS
	PtsDefInitialize(pArgMsg);
	
	//Reply(pArgMsg);	// Complete Initialize
	
	return OK;
}

// .Enable -- Process Enable -----------------------------------------------DdmPtsDefault-
//
STATUS DdmPtsDefault::Enable(Message *pArgMsg) {

	TRACEF(TRACE_L8, ("EXEC  DdmPtsDefault::Enable;\n"));
	
	Reply(pArgMsg);
	
	return OK;
}



/*************************************************************************/
// PtsDefInitialize
// Start of the table initialization state machine, called from the Ddm Init.
// Creates these tables from the data stored in the BuildSys if they do
// not exist:
//	ExportTable
/*************************************************************************/
STATUS	DdmPtsDefault::PtsDefInitialize(Message *pMsg)
{
	TRACE_ENTRY(PtsDefInitialize);
	
	STATUS			status;
	PTS_TBL_CONTEXT *pTC = new PTS_TBL_CONTEXT;
	
	pTC->pMsg = pMsg;
	
	// This is the code to create the Export Table if it does
	// not exist.  This Table structure definition is packed on 4 byte
	// boundaries.

	fieldDef *pciExportTable_FieldDefs = 
				(fieldDef*)new char[cbExportTable_FieldDefs];

	memcpy( (char*)pciExportTable_FieldDefs,
			(char*)ExportTable_FieldDefs,
			cbExportTable_FieldDefs
		  ); 
		  
	m_pDefineTable = new TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,								// Ddm* pDdm,
		EXPORT_TABLE,						// String64 prgbTableName,
		pciExportTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbExportTable_FieldDefs,			// U32 cbrgFieldDefs,
		numExportRows,						// U32 cEntriesRsv,
		false,								// bool* pfPersistant,
		(pTSCallback_t)&PtsDefReply1,		// pTSCallback_t pCallback,
		(void*)pTC							// void* pContext
	);
	
	if (status == ercOK)
		m_pDefineTable->Send();

	return status;
} // PtsDefInitialize

/*************************************************************************/
// PtsDefReply1
// Reply from creating the Export Table. If it already exist,
// we are done.  If it does not, create some dummy entries from the
// data in the BuildSys.cpp file.
/*************************************************************************/
STATUS DdmPtsDefault::PtsDefReply1(void *pClientContext, STATUS status)
{
	PTS_TBL_CONTEXT *pTC = (PTS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(PtsDefReply1);
	
	// if the table exist, do not load dummy values
	if (status != ercOK)
	{
		TRACE_STRING(TRACE_L8, "\n\rPtsDef: Export table already defined");
	
		status = PtsDefReplyLast(pTC, ercOK);
		return (status);
	}
	
	// Table did not exist, it does now, so load it with default data from
	// the BuildSys.cpp file

	TRACE_STRING(TRACE_L8, "\n\rPtsDef: loading Export table");
	
	char* pcinewExportRecord = (char*)new 
					char[sizeof(ExportTableEntry) * numExportRows];

	// generate a default Export table
	
	memcpy( (char*)pcinewExportRecord,
			(char*)&Export[0],
			sizeof(ExportTableEntry) * numExportRows
		  ); 
		  
	// Create a new InsertRow Object, Initialize it with our parameters
	// and send it off to the the table service.  This will insert
	// the new record initialized above into the DiskDescTable.
	m_pInsertRow = new TSInsertRow;
	
	status = m_pInsertRow->Initialize(
		this,							// Ddm* ClientDdm
		EXPORT_TABLE,					// prgbTableName
		pcinewExportRecord,				// prgbRowData
		sizeof(ExportTableEntry)*numExportRows,		// cbRowData
		&m_RowID1,						// *pRowIDRet
		(pTSCallback_t)&PtsDefReplyLast,	// pTSCallback_t pCallback,
		(void*)pTC						// pContext
	);
	
	if (status == ercOK)
		m_pInsertRow->Send();

	return status;
} // PtsDefReply1


/*************************************************************************/
// PtsDefReplyLast
// Last CallBack, just return the Initialize message we saved way back
// when.
/*************************************************************************/
STATUS DdmPtsDefault::PtsDefReplyLast(void *pClientContext, STATUS status)
{
	PTS_TBL_CONTEXT *pTC = (PTS_TBL_CONTEXT *)pClientContext;

	TRACE_ENTRY(PtsDefReplyLast);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L2, "\n\rPtsDefReplyLast: status = ", status);
	}
	
	// don't reply to the init message until the last TS message
	if (pTC->pMsg)
		Reply(pTC->pMsg); 
	
	// delete our context, we are done with it
	delete pTC;
	
	return status;
	
} // PtsDefReplyLast

