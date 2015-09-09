/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: PmstrDefineTables.h
// 
// Description:
// Defines the PTS tables required by PMSTR
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/PmstrDefineTables.cpp $
// 
// 1     9/15/99 4:01p Dpatel
// Initial creation
// 
//
/*************************************************************************/


#include "DdmPartitionMstr.h"


// Define Tables and Listener Registration states
typedef enum
{
	DEFINE_PARTITION_DESCRIPTOR_TABLE = 1
} DEFINE_DESCRIPTOR_TABLES;




//************************************************************************
//	DefineDescriptorTables
//		Define all the tables which PMSTR uses, if they are not
//		already defined.
//
//************************************************************************
STATUS DdmPartitionMstr::
DefineDescriptorTables()
{
	STATUS			status;
	PARTITION_CONTEXT			*pContext = new PARTITION_CONTEXT;	
	
	pContext->state = DEFINE_PARTITION_DESCRIPTOR_TABLE;
	status = DefinePartitionDescriptorTable(pContext);
	return status;
}	



//************************************************************************
//
//	DEFINE PARTITION DESCRIPTOR TABLE
//	
//************************************************************************
STATUS DdmPartitionMstr::
DefinePartitionDescriptorTable(PARTITION_CONTEXT *pContext)
{
	STATUS		status;

	// Allocate an Define Table object
	TSDefineTable *pPartitionDefineTable = new TSDefineTable;

	// Initialize the define Table object.
	status = pPartitionDefineTable->Initialize(
		this,									// DdmServices* pDdmServices,
		PARTITION_DESCRIPTOR_TABLE,				// String64 prgbTableName,
		(fieldDef*)PartitionDescriptorTable_FieldDefs,	// fieldDef* prgFieldDefsRet,
		sizeofPartitionDescriptorTable_FieldDefs,		// U32 cbrgFieldDefs,
		PMSTR_TBL_RSRV_ENTRIES,							// U32 cEntriesRsv,
		true,											// bool* pfPersistant,
		(pTSCallback_t)&DdmPartitionMstr::ProcessDefineDescriptorTablesReply,	// pTSCallback_t pCallback,
		pContext								// void* pContext
	);
	
	// Initiate the define table operation.
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pPartitionDefineTable->Send();

	return status;
}

//************************************************************************
//	ProcessDefineDescriptorTablesReply
//		Process the replies for the definition of the Partition table
//		When all tables are defined, initialize the PmstrData.
//
//************************************************************************
STATUS DdmPartitionMstr::
ProcessDefineDescriptorTablesReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT		*pContext = (PARTITION_CONTEXT *)_pContext;

	if (status != OS_DETAIL_STATUS_SUCCESS){
		// if table exists, then also continue to try for other tables
		if (status != ercTableExists)
			return status;
	}

	switch(pContext->state){
	case DEFINE_PARTITION_DESCRIPTOR_TABLE:
		m_DescriptorTablesDefined = 1;
		break;
	default:
		break;
	}
	if (m_DescriptorTablesDefined) {
		// Read any existing data from the tables and store it
		// in our local copy
		InitializePmstrData();
		// delete the context now.
		delete pContext;
		pContext = NULL;
	}
	return status;
}





