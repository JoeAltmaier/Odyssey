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
// File: PartitionMstrTest.cpp
// 
// Description:
// Test DDM for Partition master
// 
// $Log: /Gemini/Odyssey/DdmPartitionMstr/PartitionMstrTest/PartitionMstrTest.cpp $
// 
// 1     9/15/99 4:01p Dpatel
// Initial creation
// 
//
/*************************************************************************/
#include "Buildsys.h"
#include "PartitionMstrTest.h"



#ifdef WIN32
#include <crtdbg.h>
#endif

CLASSNAME(PartitionMstrTest,SINGLE);

/********************************************************************
*
* PartitionMstrTest - Constructor
*
********************************************************************/

PartitionMstrTest::PartitionMstrTest(DID did): DdmMaster(did)
{
	m_pStringResourceManager = NULL;

#ifdef TEST_TABLES
	m_numberOfDummySRCRecords = 0;
#endif

}


/********************************************************************
*
* PartitionMstrTest - Destructor
*
********************************************************************/
PartitionMstrTest::~PartitionMstrTest()
{
	if (m_pStringResourceManager){
		delete m_pStringResourceManager;
	}
}

/********************************************************************
*
* PartitionMstrTest - Ctor
*
********************************************************************/

Ddm *PartitionMstrTest::
Ctor(DID MyDID)
{
	Ddm *pMyDDM = new PartitionMstrTest(MyDID);
	return pMyDDM;
}



//************************************************************************
//	Quiesce
//
//	pMsg	- the quiesce reply message
//
//************************************************************************
STATUS	PartitionMstrTest::
Quiesce(Message *pMsg)
{
	Reply(pMsg);
	return OS_DETAIL_STATUS_SUCCESS;
}




/********************************************************************
*
* PartitionMstrTest - Enable
*
********************************************************************/
STATUS PartitionMstrTest::
Enable(Message *pMsg)
{
	Reply(pMsg);
	return OK;
}


/********************************************************************
*
*	PartitionMstrTest - Initialize
*
********************************************************************/
STATUS PartitionMstrTest::
Initialize(Message *pMsg)
{
#ifdef WIN32
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

	// Turn On (OR) - 
	tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	// Set the new state for the flag
	_CrtSetDbgFlag( tmpFlag );
	tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
#endif

	CreateSRCTable();
	m_pCmdSender = 
			new CmdSender(
					PARTITION_MSTR_CMD_QUEUE,
					sizeof(PMSTR_CMND_INFO),
					sizeof(PMSTR_EVENT_INFO),	// union of status/event sizes
					this);
	m_pCmdSender->csndrInitialize(
			(pInitializeCallback_t)&PartitionMstrTest::tstObjectInitializedReply);
	m_pCmdSender->csndrRegisterForEvents(
		(pEventCallback_t)&PartitionMstrTest::EventHandler);

	m_pStringResourceManager = new StringResourceManager (
										this,
										(pTSCallback_t)&PartitionMstrTest::StringResourceManagerInitializedReply);

	Ddm::Initialize(pMsg);
	return OK;
}





//**************************************************************************
//
//	Cmd Sender initialized reply
//
//**************************************************************************
void PartitionMstrTest
::tstObjectInitializedReply(STATUS status)
{
	assert(status == 0);
	rowID							temp = {15,0,1};

	TestCreatePartition(
			&temp,
			0x500);
}


//**************************************************************************
//
//	String Resource Manager Initialized reply
//
//**************************************************************************
STATUS PartitionMstrTest
::StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status)
{
	assert(status == 0);
	return status;
}


//**************************************************************************
//
//	Partition Test Command Completion Reply
//
//**************************************************************************
void PartitionMstrTest
::CommandCompletionReply(
			STATUS				completionCode,
			void				*pStatusData,
			void				*pCmdData,
			void				*pCmdContext)
{
	TRACE_STRING(TRACE_RMSTR_1, "\nEnter: PartitionMstrTest::rmstrCommandCompletionReply\n");
	PMSTR_CMND_INFO *pInfo = (PMSTR_CMND_INFO *)pCmdData;
	PMSTR_CMND_PARAMETERS *pParams = &pInfo->cmdParams;
	PMSTR_CREATE_PARTITION_INFO *pCreatePartitionInfo = 
			(PMSTR_CREATE_PARTITION_INFO *)&pParams->createPartitionInfo;
	PMSTR_MERGE_PARTITION_INFO *pMergePartitionInfo = 
			(PMSTR_MERGE_PARTITION_INFO *)&pParams->mergePartitionInfo;


	CONTEXT	*pContext = (CONTEXT *)pCmdContext;
	switch(completionCode){
	case PMSTR_SUCCESS:
		switch (pInfo->opcode){
		case PMSTR_CMND_CREATE_PARTITION:
			pStatusData = pStatusData;
			break;

		case PMSTR_CMND_MERGE_PARTITIONS:
			//TRACEF_NF(TRACE_RMSTR_1, ("\t\t", pDeleteArrayInfo->arrayRowId));
			break;
		default:
			break;
		}
		break;
	default:
		//display error
		TRACEF_NF(TRACE_RMSTR_1, ("\t\tCmd Error  = 0x%x\n",completionCode)); 
		break;
	}
	if (pContext){
		delete pContext;
		pContext = NULL;
	}
}

//**************************************************************************
//
//	Partition Test Event Handler
//
//**************************************************************************
void PartitionMstrTest
::EventHandler(
			STATUS			eventCode,
			void			*pStatusData)
{
	U32					i=0;

	TRACEF_NF(TRACE_RMSTR_1,("\nEnter: PartitionMstrTest::Partition Test EventHandler\n"));
	PMSTR_EVT_PARTITION_CREATED_STATUS		*pEvtPartitionCreated = NULL;
	PMSTR_EVT_PARTITION_MODIFIED_STATUS		*pEvtPartitionModified = NULL;
	PMSTR_EVT_PARTITION_DELETED_STATUS		*pEvtPartitionDeleted = NULL;

	static		BOOL merged = false;

			

	TRACE_STRING(TRACE_RMSTR_1, "\t<<<Event Received>>>:\n");
	switch(eventCode){
	case PMSTR_EVT_PARTITION_CREATED:
		pEvtPartitionCreated = (PMSTR_EVT_PARTITION_CREATED_STATUS *)pStatusData;
		TRACE_STRING(TRACE_RMSTR_1, "\t<<<Partition Added Event Received>>>:\n");
		PrintPartitionData(&pEvtPartitionCreated->partitionData);
		PrintSRCData(&pEvtPartitionCreated->SRCData);
		if(pEvtPartitionCreated->partitionData.partitionSize == 0x500){
#if 0
			TestMergePartition(
				// merge A,B
				&pEvtPartitionCreated->partitionData.SRCTRID,
				&pEvtPartitionCreated->partitionData.nextRowId);
#endif
#if 1
			TestCreatePartition(
				&pEvtPartitionCreated->partitionData.SRCTRID,
				0x200);
#endif
		}

		if(pEvtPartitionCreated->partitionData.partitionSize == 0x200){
			TestCreatePartition(
				//&pEvtPartitionCreated->partitionData.SRCTRID,
				&pEvtPartitionCreated->partitionData.nextRowId,
				0x100);
		}

		break;

	case PMSTR_EVT_PARTITION_MODIFIED:
		pEvtPartitionModified = (PMSTR_EVT_PARTITION_MODIFIED_STATUS *)pStatusData;
		TRACE_STRING(TRACE_RMSTR_1, "\t<<<Partition Modified Event Received>>>:\n");
		pEvtPartitionModified = (PMSTR_EVT_PARTITION_MODIFIED_STATUS *)pStatusData;
		PrintPartitionData(&pEvtPartitionModified->partitionData);
		PrintSRCData(&pEvtPartitionModified->SRCData);

		// now try to merge them back
#if 0
		if (!merged){
			// we have partitions A, B, C
			TestMergePartition(
				// error test 1
				//&pEvtPartitionModified->partitionData.SRCTRID,
				//&pEvtPartitionModified->partitionData.SRCTRID);

				// error test 2
				//&pEvtPartitionModified->partitionData.previousRowId,
				//&pEvtPartitionModified->partitionData.nextRowId);

				// merge B,C
				//&pEvtPartitionModified->partitionData.SRCTRID,
				//&pEvtPartitionModified->partitionData.nextRowId);

				// merge C,B
				//&pEvtPartitionModified->partitionData.nextRowId,
				//&pEvtPartitionModified->partitionData.SRCTRID);

				// merge B,A
				//&pEvtPartitionModified->partitionData.SRCTRID,
				//&pEvtPartitionModified->partitionData.previousRowId);

				// merge A,B
				&pEvtPartitionModified->partitionData.previousRowId,
				&pEvtPartitionModified->partitionData.SRCTRID);
			merged = true;
		}
#endif

		break;

	case PMSTR_EVT_PARTITION_DELETED:
		pEvtPartitionDeleted = (PMSTR_EVT_PARTITION_DELETED_STATUS *)pStatusData;
		TRACE_STRING(TRACE_RMSTR_1, "\t<<<Partition Deleted Event Received>>>:\n");
		pEvtPartitionDeleted = (PMSTR_EVT_PARTITION_DELETED_STATUS *)pStatusData;
		PrintPartitionData(&pEvtPartitionDeleted->partitionData);
		PrintSRCData(&pEvtPartitionDeleted->SRCData);

		break;
	default:
		assert(0);		
	}
}


//**************************************************************************
//
//	Test Create Partition
//
//**************************************************************************
void PartitionMstrTest
::TestCreatePartition(
		rowID			*pSrcToPartition,
		I64				size)
{

	PMSTR_CMND_INFO					*pCmdInfo;
	

	pCmdInfo = new (tZERO) PMSTR_CMND_INFO;
	pCmdInfo->opcode = PMSTR_CMND_CREATE_PARTITION;

	memcpy(
			&pCmdInfo->cmdParams.createPartitionInfo.srcToPartition,
			pSrcToPartition,
			sizeof(rowID));
	PMSTR_CREATE_PARTITION_INFO *pCreatePartitionInfo =
						&pCmdInfo->cmdParams.createPartitionInfo;
	pCreatePartitionInfo->partitionSize = size;
	m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&PartitionMstrTest::CommandCompletionReply,
		NULL);
}



//**************************************************************************
//
//	Test Create Partition
//
//**************************************************************************
void PartitionMstrTest
::TestMergePartition(
		rowID			*pSrcPartition1,
		rowID			*pSrcPartition2)
{

	PMSTR_CMND_INFO					*pCmdInfo;
	

	pCmdInfo = new (tZERO) PMSTR_CMND_INFO;
	pCmdInfo->opcode = PMSTR_CMND_MERGE_PARTITIONS;

	PMSTR_MERGE_PARTITION_INFO *pMergePartitionInfo =
						&pCmdInfo->cmdParams.mergePartitionInfo;

	pMergePartitionInfo->srcPartitionRowId1 = *pSrcPartition1;
	pMergePartitionInfo->srcPartitionRowId2 = *pSrcPartition2;
	m_pCmdSender->csndrExecute(
		pCmdInfo,
		(pCmdCompletionCallback_t)&PartitionMstrTest::CommandCompletionReply,
		NULL);
}


//**************************************************************************
//
//	Print Partition Data
//
//**************************************************************************
void PartitionMstrTest
::PrintPartitionData(PARTITION_DESCRIPTOR	*pPDTRecord)
{
	TRACE_STRING(TRACE_RMSTR_1, "\tPartition Data:\n");

	TRACEF_NF(TRACE_RMSTR_1, ("\t\tPartition RowID = %x %x %x\n", 
			pPDTRecord->rid.Table,
			pPDTRecord->rid.HiPart,
			pPDTRecord->rid.LoPart));

	TRACEF_NF(TRACE_RMSTR_1, ("\t\tPartition SRC RowID = %x %x %x\n", 
			pPDTRecord->SRCTRID.Table,
			pPDTRecord->SRCTRID.HiPart,
			pPDTRecord->SRCTRID.LoPart));

	/*
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tParent SRC RowID = %x %x %x\n", 
			pPDTRecord->parentSRCTRID.Table,
			pPDTRecord->parentSRCTRID.HiPart,
			pPDTRecord->parentSRCTRID.LoPart));

	TRACEF_NF(TRACE_RMSTR_1, ("\t\tParent VDN = %d\n", 
			pPDTRecord->parentVDN));
	*/
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tNext SRC RowID = %x %x %x\n", 
			pPDTRecord->nextRowId.Table,
			pPDTRecord->nextRowId.HiPart,
			pPDTRecord->nextRowId.LoPart));

	TRACEF_NF(TRACE_RMSTR_1, ("\t\tPrevious SRC RowID = %x %x %x\n", 
			pPDTRecord->previousRowId.Table,
			pPDTRecord->previousRowId.HiPart,
			pPDTRecord->previousRowId.LoPart));


	TRACEF_NF(TRACE_RMSTR_1, ("\t\tPartition VD = %x \n", 
			pPDTRecord->partitionVD));
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tPartition Size = %x \n", 
			pPDTRecord->partitionSize));
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tPartition Start LBA = %x \n", 
			pPDTRecord->startLBA));
}


//**************************************************************************
//
//	Print Partition Data
//
//**************************************************************************
void PartitionMstrTest
::PrintSRCData(StorageRollCallRecord	*pSRCRecord)
{
	TRACE_STRING(TRACE_RMSTR_1, "\tPartition SRC Data:\n");
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tSRC RowID = %x %x %x\n", 
			pSRCRecord->rid.Table,
			pSRCRecord->rid.HiPart,
			pSRCRecord->rid.LoPart));

	TRACEF_NF(TRACE_RMSTR_1, ("\t\tSRC Partition RowID = %x %x %x\n", 
			pSRCRecord->ridDescriptorRecord.Table,
			pSRCRecord->ridDescriptorRecord.HiPart,
			pSRCRecord->ridDescriptorRecord.LoPart));


	TRACEF_NF(TRACE_RMSTR_1, ("\t\tSRC VD = %x \n", 
			pSRCRecord->vdnBSADdm));
	TRACEF_NF(TRACE_RMSTR_1, ("\t\tSRC Capacity = %x \n", 
			pSRCRecord->Capacity));
}