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
// File: DdmRaidMgmt.h
// 
// Description:
// DdmRaidMgmt.h
// 
// $Log: /Gemini/Include/Partition/DdmPartitionMstr.h $
// 
// 6     1/21/00 12:05p Szhang
// 
// 2     9/15/99 4:02p Dpatel
// 
// 1     9/10/99 9:42a Dpatel
// Initial creation
// 
//
/*************************************************************************/
#ifndef _DdmPartitionMstr_h
#define _DdmPartitionMstr_h


#include "CtTypes.h"

#include "CmdServer.h"
#include "CmdSender.h"

#include "DdmMaster.h"
#include "Message.h"

#include "PartitionTable.h"
#include "PmstrCmnds.h"
#include "PmstrEvents.h"
#include "PmstrErrors.h"


#include "StorageRollCallTable.h"
#include "DdmClassDescriptorTable.h"
#include "DiskDescriptor.h"
#include "VirtualClassDescriptorTable.h"

#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"

#include "stdio.h"
#include "odyssey_trace.h"
#include "Trace_Index.h"
#ifdef	TRACE_INDEX
#undef	TRACE_INDEX
#endif	// TRACE_INDEX
#define TRACE_INDEX			TRACE_RMSTR

#include "HelperServices.h"
#include "TableServices.h"
#include "DataQueue.h"


#include "UnicodeString.h"
#include "StringResourceManager.h"




#define UNUSED(p) p=p


#include "CTEvent.h"


#pragma	pack(4)

/********************************************************************
*
*	Local use for DdmPartitionMstr
*
********************************************************************/
#define PMSTR_TBL_RSRV_ENTRIES		10

/********************************************************************
*
*
*
********************************************************************/

class DdmPartitionMstr: public DdmMaster
{
public:
	DdmPartitionMstr(DID did);
	~DdmPartitionMstr();
	static Ddm *Ctor(DID MyDID);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	STATUS Initialize(Message *pMsg);
protected:

private:

	enum PMSTR_DATA_TYPE {
		PMSTR_CMND = 1,
		PMSTR_PARTITION
	};


	enum {
		SRC_FREE = false,
		SRC_USED = true
	};

	// we use 0, 1 for false and true
	enum{
		START_FAILOVER_SIMULATION = 2
	};


	#define		PMSTR_NONE_MODIFIED				0x0
	#define		PMSTR_PREV_PARTITION_MODIFIED	0x1
	#define		PMSTR_NEXT_PARTITION_MODIFIED	0x2

	struct PARTITION_CONTEXT
	{
		U32						cmnd;
		U32						state;
		U32						rowsDeleted;
		HANDLE					cmdHandle;
		U32						numProcessed;
		U32						value;		// some value
		U32						value1;		// some value
		U32						value2;		// some value
		U32						value3;			
		void					*pData;		// general purpose ptr
		void					*pData1;	// general purpose ptr
		void					*pData2;	// general purpose ptr
		void					*pData3;	// general purpose ptr
		void					*pData4;	// general purpose ptr
		void					*pData5;	// general purpose ptr
		void					*pData6;	// general purpose ptr
		void					*pData7;	// general purpose ptr
		void					*pData8;	// general purpose ptr
		rowID					newRowId;	// for inserts
		struct PARTITION_CONTEXT		*pParentContext;
		pTSCallback_t			pCallback;
		UnicodeString			ucPartitionName;
		UnicodeString			ucRemainderPartitionName;
		U32						SlotID;

		PARTITION_CONTEXT()
		{
			memset(this,0,sizeof(PARTITION_CONTEXT));
		};
		
		~PARTITION_CONTEXT(){
			if (pData){
				delete pData;
				pData = NULL;
			}
			if (pData1){
				delete pData1;
				pData1 = NULL;
			}
			if (pData2){
				delete pData2;
				pData2 = NULL;
			}
			if (pData3){
				delete pData3;
				pData3 = NULL;
			}
			if (pData4){
				delete pData4;
				pData4 = NULL;
			}
			if (pData5){
				delete pData5;
				pData5 = NULL;
			}
			if (pData6){
				delete pData6;
				pData6 = NULL;
			}
			if (pData7){
				delete pData7;
				pData7 = NULL;
			}
			if (pData8){
				delete pData8;
				pData8 = NULL;
			}

		}
	};

private:
	DID				m_PMSTRDID;
	BOOL			m_DescriptorTablesDefined;	// all Array related tables defined
	Message			*m_pInitializeMessage;
	Message			*m_pQuiesceMessage;

	CmdServer				*m_pCmdServer;			// PMSTR Cmd Server


	U32						m_SRCIsUsed;

	HelperServices			*m_pHelperServices;
	TableServices			*m_pTableServices;
	DataQueue				*m_pDataQueue;
	StringResourceManager	*m_pStringResourceManager;

	BOOL					m_isQuiesced;
	BOOL					m_CommandInProgress;
	BOOL					m_failoverInProgress;

	// Initialization Prototypes
	void InitializePmstrData();
	STATUS InitializePmstrDataReply(
		void						*_pContext, 
		STATUS						status);
	void InitializeCommandQueues();
	STATUS PmstrStringResourceManagerInitializedReply(
		void						*pContext,
		STATUS						status);


	// PMSTR CMD QUEUE
	void PmstrCommandReceiver(
		HANDLE						handle,
		void						*pClientContext);
	STATUS PmstrEventHandler(
		void						*pModifiedSQRecord, 
		STATUS						status);
	void PmstrCmdServerInitializedReply(STATUS status);



	
	// OUTSTANDING COMMANDS
	void RunOutstandingCommand();
	void StartCommandProcessing(
		HANDLE						handle,
		PMSTR_CMND_INFO				*pCmdInfo);
	void StopCommandProcessing(
		BOOL						checkForNextCmd,
		HANDLE						handle);
	BOOL SimulateFailover(PARTITION_CONTEXT *pCmdContext);


	// DEFINE TABLES
	STATUS DefineDescriptorTables();
	STATUS DefinePartitionDescriptorTable(PARTITION_CONTEXT *_pContext);
	STATUS ProcessDefineDescriptorTablesReply(
		void						*_pContext, 
		STATUS						status);


	// CREATE PARTITION
	STATUS CreatePartitionValidation(
		HANDLE						h,
		PMSTR_CMND_INFO				*_pCmdInfo);

	STATUS ProcessCreatePartitionValidationReply(
		void						*_pContext, 
		STATUS						status);

	STATUS CreatePartition(
		HANDLE						handle,
		PMSTR_CMND_INFO				*_pCmdInfo,
		StorageRollCallRecord		*_pSRCRecord);

	STATUS ProcessCreatePartitionReply(
		void						*_pContext,
		STATUS						status);

	STATUS CreatePartitionOfPartition(
		HANDLE						handle,
		PMSTR_CMND_INFO				*_pCmdInfo,
		StorageRollCallRecord		*_pSRCRecord);

	STATUS ProcessCreatePartitionOfPartitionReply(
		void						*_pContext,
		STATUS						status);


	STATUS CheckAndInsertNewSRCEntry(
			rowID					*pRowIdToCompare,
			StorageRollCallRecord	*pNewSRCRecord,
			pTSCallback_t			cb,
			void					*pOriginalContext);
	STATUS CheckAndInsertNewSRCEntryReply(void *_pContext, STATUS status);


	// MERGE PARTITION
	STATUS MergePartitionValidation(
		HANDLE						h,
		PMSTR_CMND_INFO				*_pCmdInfo);

	STATUS ProcessMergePartitionValidationReply(
		void						*_pContext, 
		STATUS						status);
	
	
	STATUS CleanUpReply(void * _pContext,STATUS status);

	STATUS MergePartition(
		HANDLE						handle,
		PMSTR_CMND_INFO				*_pCmdInfo,
		StorageRollCallRecord		*_pSRCRecord1,
		StorageRollCallRecord		*_pSRCRecord2,
		PARTITION_DESCRIPTOR		*_pPDTRecord1,
		PARTITION_DESCRIPTOR		*_pPDTRecord2);

	STATUS ProcessMergePartitionReply(
		void						*_pContext,
		STATUS						status);

	STATUS MergePartitionOfPartition(
		HANDLE						handle,
		PMSTR_CMND_INFO				*_pCmdInfo,
		StorageRollCallRecord		*_pSRCRecord);

	STATUS ProcessMergePartitionOfPartitionReply(
		void						*_pContext,
		STATUS						status);


	void PrepareSRCEntryForPartition(
		StorageRollCallRecord		*pSRCNewPartitionRecord,
		VDN							vd,
		I64							capacity,
		rowID						*pDescriptorRowId,
		rowID						*pNameRowId);

	void PreparePartitionDescriptor(
		PARTITION_DESCRIPTOR		*pPDTRecord,
		U32							version,
		U32							size,
		VDN							vd,
		rowID						*pSrcToPartition,
		VDN							parentVDN,
		I64							startLba,
		I64							partitionSize);

	STATUS UpdatePartitionPointers(
		rowID						*pParentSrcRowId,
		PARTITION_DESCRIPTOR		*pPartitionToCheck,
		pTSCallback_t				cb,
		PARTITION_CONTEXT			*pContext);
	STATUS ProcessUpdatePartitionPointersReply(
		void						*_pContext,
		STATUS						status);

};

#endif

