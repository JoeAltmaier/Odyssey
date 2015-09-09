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
// $Log: /Gemini/Odyssey/DdmPartitionMstr/PartitionMstrTest/PartitionMstrTest.h $
// 
// 1     9/15/99 4:02p Dpatel
// Initial creation
// 
//
/*************************************************************************/
#ifndef _PartitionMstrTest_h
#define _PartitionMstrTest_h


#include "CtTypes.h"


#include "CmdSender.h"

#include "DdmMaster.h"

#include "RequestCodes.h"
#include "OsTypes.h"
#include "OsStatus.h"
#include "Message.h"
#include "Ddm.h"


#include "StorageRollCallTable.h"
#include "DdmClassDescriptorTable.h"
#include "VirtualClassDescriptorTable.h"

#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"
#include "Fields.h"

#include "PmstrCmnds.h"
#include "PmstrEvents.h"
#include "PmstrErrors.h"

#include "stdio.h"

#include "odyssey_trace.h"
#include "Trace_Index.h"

#include "UnicodeString.h"
#include "StringResourceManager.h"

#ifdef	TRACE_INDEX
#undef	TRACE_INDEX
#endif	// TRACE_INDEX
#define TRACE_INDEX			TRACE_RMSTR_1

#define UNUSED(p) p=p


#pragma	pack(4)


/********************************************************************
*
*
*
********************************************************************/

class PartitionMstrTest: public DdmMaster
{
	struct CONTEXT
	{
		U32						cmnd;
		U32						state;
		U32						numProcessed;
		U32						value;		// some value
		U32						value1;		// some value
		U32						value2;		// some value
		void					*pData;		// general purpose ptr
		void					*pData1;	// general purpose ptr
		rowID					newRowId;	// for inserts
		CommandQueueRecord		*pCQRecord;
		struct CONTEXT			*pParentContext;
		pTSCallback_t			pCallback;
		CONTEXT()
		{
			cmnd = 0;
			state = 0;
			numProcessed = 0;
			value = 0;
			value1 = 0;
			value2 = 0;
			pData = NULL;
			pData1 = NULL;
			pCQRecord = NULL;
			pParentContext = NULL;
			pCallback = NULL;
		};
	};
public:
	PartitionMstrTest(DID did);
	~PartitionMstrTest();
	static Ddm *Ctor(DID MyDID);
	STATUS Quiesce(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Initialize(Message *pMsg);

protected:


private:

#define TEST_TABLES 1
#ifdef TEST_TABLES
	VDN				m_dummyVDN;
	#define			DUMMY_VDN		400

	U32				m_numberOfDummySRCRecords;
	#define			MAX_DUMMY_SRC_RECORDS		5

	TSDefineTable	*m_pSRCDefineTable;
	TSListen		*m_pSRCListen;
	
	StorageRollCallRecord*	m_pSRCRecord;		// Our own copy of SRC Record
	U32						m_sizeofSRCRecord;

	StorageRollCallRecord*	m_pModifiedSRCRecord;	// Our own copy of the StorageRollCallTable.
	U32						m_sizeofModifiedSRCRecord;

	rowID					m_ridNewSRCRecord;	// Listen returned rowID of inserted Row.
	U32						m_SRCListenerID;	// Listen returned rowID of inserted Row.
	U32						*m_pSRCListenReplyType;		// Listen returned listen type. 
	StorageRollCallRecord*	m_pNewSRCRecord;	// Our copy of a newly inserted SRC record.

	STATUS CreateSRCTable();
	STATUS SRCTableDefineReply(void *pClientContext, STATUS status);
	STATUS SRCInsertDummySRCRecord();
	STATUS SRCInsertRowReply( void* pContext, STATUS status );
	STATUS SRCReadRow(rowID* rowToRead);
	
	STATUS SRCReadRowReply( void* pContext, STATUS status );
	STATUS ListenSRCInsertRowReply( void* pContext, STATUS status );
#endif



	DID						MyDID;

	CmdSender				*m_pCmdSender;
	HANDLE					m_CmdHandle;

	StringResourceManager	*m_pStringResourceManager;

	STATUS StringResourceManagerInitializedReply(
			void			*pContext,
			STATUS			status);

	// GENERAL
	void tstObjectInitializedReply(STATUS status);
	void CommandCompletionReply(
				STATUS				statusCode,
				void				*pStatusData,
				void				*pCmdData,
				void				*pCmdContext);
	void EventHandler(
				STATUS			eventCode,
				void			*pEventData);

	// Test
	void TestCreatePartition(
			rowID				*pSrcToPartition,
			I64					size);
	void TestMergePartition(
		rowID			*pSrcToPartition1,
		rowID			*pSrcToPartition2);


	// Print
	void PrintPartitionData(PARTITION_DESCRIPTOR	*pPDTRecord);
	void PrintSRCData(StorageRollCallRecord	*pSRCRecord);

};


#endif

