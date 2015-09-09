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
// File: PmstrData.cpp
// 
// Description:
// Partition Master local data
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/PmstrData.cpp $
// 
// 5     1/21/00 12:02p Szhang
// Changed Initialization to handle failover.
// 
// 1     9/15/99 4:01p Dpatel
// Initial creation
// 
//
/*************************************************************************/

#include "DdmPartitionMstr.h"

#pragma	pack(4)



char *dispPartitionCommandName[] =
{
	"INVALID CMD",
	"PMSTR_CMND_CREATE_PARTITION",
	"PMSTR_CMND_DELETE_PARTITION"
};


char *dispPartitionEventName[] =
{
	"INVALID_UTIL",
	"PMSTR_EVT_PARTITION_CREATED",
	"PMSTR_EVT_PARTITION_DELETED"
};



char *dispPartitionErrorName[] =
{
	"INVALID_ERROR",
	"PMSTR_ERR_PMSTR_NOT_INITIALIZED",
	"PMSTR_ERR_STORAGE_ELEMENT_IN_USE",
	"PMSTR_ERR_PARTITION_ALREADY_EXISTS",
	"PMSTR_ERR_INVALID_COMMAND"
};





enum {
	INITIALIZE_PMSTR_DATA_PDT_ENUMERATED = 100
};


//************************************************************************
//	InitializePmstrData
//		Enum the PDT Table and store all entries in our local data
//
//************************************************************************
void DdmPartitionMstr::
InitializePmstrData()
{
	PARTITION_CONTEXT		*pContext= new PARTITION_CONTEXT;

	pContext->state		= INITIALIZE_PMSTR_DATA_PDT_ENUMERATED;

	m_pTableServices->TableServiceEnumTable(
				PARTITION_DESCRIPTOR_TABLE,		// tableName
				&pContext->pData,				// table data returned
				&pContext->value1,				// data returned size
				&pContext->value,				// number of rows returned here
				pContext,						// context
				(pTSCallback_t)&DdmPartitionMstr::InitializePmstrDataReply);
}



//************************************************************************
//	InitializePmstrDataReply
//		Process the enum replies for different tables.
//
//************************************************************************
STATUS DdmPartitionMstr::
InitializePmstrDataReply(void *_pContext, STATUS status)
{
	PARTITION_CONTEXT				*pContext = NULL;	
	
	STATUS							rc;
	BOOL							initializeComplete = false;
	U32								numberOfRows = 0;
	void							*pPDTTableData = NULL;
	U32								i = 0;
	PARTITION_DESCRIPTOR			*pPDTRecord = NULL;
	rc								= PMSTR_SUCCESS;
	pContext						= (PARTITION_CONTEXT *)_pContext;
	
	numberOfRows = pContext->value;
	if (status != OS_DETAIL_STATUS_SUCCESS){
		initializeComplete = true;
	} else {
		switch(pContext->state){
		case INITIALIZE_PMSTR_DATA_PDT_ENUMERATED:
			// pValidationContext->pData = enum data		
			// pData2 = NULL
			// pData3 = NULL
			// read the data that was enumerated
			pPDTTableData = pContext->pData;
			for (i= pContext->value2; i < numberOfRows; i++){
					pPDTRecord = (PARTITION_DESCRIPTOR *)
						((char *)pPDTTableData+(sizeof(PARTITION_DESCRIPTOR)*i));
					if (pPDTRecord){
						pContext->value2++;
						if(RowId(pPDTRecord->SRCTRID)){
							m_pDataQueue->Add(
								PMSTR_PARTITION, 
								&pPDTRecord->rid, 
								&pPDTRecord->stateIdentifier,
								pPDTRecord, 
								sizeof(PARTITION_DESCRIPTOR));
						}
						// Handle failover happened before deleting PDT.
						// When Initialization start again, if find any PDT which does 
						// not have SRC in DataQueue, delete PDT from Table. 
						else{
							// Now delete the PDT record
							status = m_pTableServices->TableServiceDeleteRow(
							PARTITION_DESCRIPTOR_TABLE,
							&pPDTRecord->rid,		// pdt row id
							TSCALLBACK(DdmPartitionMstr,InitializePmstrDataReply),
							pContext);
						}
					} 
			}
			if (numberOfRows){
				if (pContext->pData){
					delete pContext->pData;
					pContext->pData = NULL;
				}
			}
			pContext->value = 0;
			pContext->value1 = 0;
			initializeComplete = true;
			break;

		default:
			assert(0);
		}
	}
	if (initializeComplete){
		delete pContext;
		pContext = NULL;
		// Now initialize the command queues
		InitializeCommandQueues();
	}
	return status;
}







