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
// $Log: /Gemini/Include/DdmRAIDMgmt.h $
// 
// 47    12/17/99 5:25p Dpatel
// added vc modify and hc code
// 
// 46    11/23/99 6:53p Dpatel
// removed DdmVCMMsgs.h till be get it to compile..
// 
// 45    11/23/99 6:49p Dpatel
// hot copy protos, and dummyVDN for array creation for win32
// 
// 43    9/09/99 1:39p Dpatel
// added ifdef win32 for SSAPI_TEST...
// 
// 42    9/07/99 7:31p Dpatel
// 
// 41    9/07/99 1:47p Dpatel
// 
// 40    9/03/99 10:01a Dpatel
// 
// 39    9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 38    8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 37    8/30/99 6:14p Agusev
// fixed TRACE_INDEX problems
// 
//
// 06/11/99 Dipam Patel: Create file
//
/*************************************************************************/
#ifndef _DdmRaidMgmt_h
#define _DdmRaidMgmt_h


#include "CtTypes.h"

#include "CmdServer.h"
#include "CmdSender.h"

#include "DdmMaster.h"
#include "Message.h"

#include "ArrayDescriptor.h"
#include "RaidSpareDescriptor.h"
#include "RaidMemberTable.h"
#include "RaidUtilTable.h"
#include "RmstrCapabilityTable.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"
#include "RmstrInternalCommands.h"
#include "RmstrEvents.h"
#include "RmstrErrors.h"
#include "RmstrTestLevels.h"

#include "RaidCommands.h"

#include "StorageRollCallTable.h"
#include "DdmClassDescriptorTable.h"
#include "DiskDescriptor.h"
#include "VirtualClassDescriptorTable.h"
#include "VirtualDeviceTable.h"

#include "DdmVCMCommands.h"
#include "DdmVCMMsgs.h"


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
#include "RmstrCapabilities.h"

#include "UnicodeString.h"
#include "StringResourceManager.h"




extern char	*dispUtilityName[];
extern char	*dispCommandName[];
extern char	*dispEventName[];
extern char	*dispErrorName[];


extern RMSTR_CAPABILITY_DESCRIPTOR	RaidCapabilities[];
extern RMSTR_CAPABILITY_RAID_LEVEL	Raid0Capabilities;
extern RMSTR_CAPABILITY_RAID_LEVEL	Raid1Capabilities;
extern RMSTR_CAPABILITY_RAID_LEVEL	Raid5Capabilities;

#define UNUSED(p) p=p

#ifdef WIN32
#define RMSTR_SSAPI_TEST		1
#endif
//#define RMSTR_RAID_DDM_TEST	1

#include "CTEvent.h"


#pragma	pack(4)

/********************************************************************
*
*	Local use for DdmRAIDMstr
*
********************************************************************/
#define RMSTR_TBL_RSRV_ENTRIES		20

/********************************************************************
*
*
*
********************************************************************/

class DdmRAIDMstr: public DdmMaster
{
public:
	DdmRAIDMstr(DID did);
	~DdmRAIDMstr();
	static Ddm *Ctor(DID MyDID);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	STATUS Initialize(Message *pMsg);
	STATUS BuildVirtualDeviceList();
	STATUS DispatchDefault(Message *pMsg);

protected:

private:
	enum {
		RAID_ARRAY = 1,
		RAID_MEMBER,
		RAID_SPARE,
		RAID_UTILITY,
		RAID_CMND,
		RAID_ALARM
	};

	enum {
		SRC_UNUSED = 0,
		SRC_USED
	}STORAGE_ROLL_CALL_FUSED;

	struct 	RMSTR_QUEUED_CMND{
		rowID				rowId;
		RMSTR_CMND_INFO		cmdInfo;
	};

	struct RMSTR_DATA {
		U32				type;
		rowID			rowId;
		void			*pRowData;
		RMSTR_DATA		*pNext;
	};

	// we use 0, 1 for false and true
	enum{
		START_FAILOVER_SIMULATION = 2
	};

	struct ALARM_CONTEXT{
		U32			eventCode;			// event code for alarm
		rowID		alarmSourceRowId;	// src on which alarm raised
	};

	struct CONTEXT
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
		rowID					newRowId;	// for inserts
		struct CONTEXT			*pParentContext;
		pTSCallback_t			pCallback;
		UnicodeString			ucArrayName;
		UnicodeString			ucMemberName;
		U32						SlotID;
		CONTEXT()
		{
			memset(this,0,sizeof(CONTEXT));
		};
		
		~CONTEXT(){
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
		}
	};

private:
	DID				m_RmstrDID;
	BOOL			m_DescriptorTablesDefined;	// all Array related tables defined
	Message			*m_pInitializeMessage;
	Message			*m_pQuiesceMessage;

	CmdServer		*m_pCmdServer;			// RMSTR Cmd Server
	CmdSender		*m_pInternalCmdSender;	// RMSTR Internal Cmd Sender

#ifdef RMSTR_SSAPI_TEST
	CmdServer		*m_pFakeRaidDdmCmdServer;
#endif

	CmdSender		*m_pRaidCmdSender;	// Raid DDm Cmd Sender
	CmdSender		*m_pVCMCmdSender;	// VCM DDm Cmd Sender
	BOOL			m_RaidCmdSenderInitialized;	

	U32				m_SRCIsUsed;

	RmstrCapabilities		*m_pRmstrCapability;
	TableServices			*m_pTableServices;
	HelperServices			*m_pHelperServices;

	BOOL					m_CommandInProgress;
	BOOL					m_failoverInProgress;
	BOOL					m_isQuiesced;

	StringResourceManager	*m_pStringResourceManager;
	UnicodeString			m_ucArrayName;

	// keep PTS data locally
	RMSTR_DATA		*m_pArrayData;
	RMSTR_DATA		*m_pMemberData;
	RMSTR_DATA		*m_pSpareData;
	RMSTR_DATA		*m_pUtilityData;
	RMSTR_DATA		*m_pCmdData;
	RMSTR_DATA		*m_pAlarmData;
	


	RMSTR_DATA		*m_pArrayDataTail;
	RMSTR_DATA		*m_pMemberDataTail;
	RMSTR_DATA		*m_pSpareDataTail;
	RMSTR_DATA		*m_pUtilityDataTail;
	RMSTR_DATA		*m_pCmdDataTail;
	RMSTR_DATA		*m_pAlarmDataTail;

#ifdef WIN32
	VDN				m_dummyVDN;
#endif
#ifdef TEST_TABLES
	VDN				m_dummyVDN;
	#define			DUMMY_VDN		500
	// tst SRC Table
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

	STATUS CreateNewVirtualDevice( );
	STATUS CreateVirtualDeviceReply( void *pContext, STATUS status);

	// tst SRC Table end
#endif


	// Initialization Prototypes
	void InitializeRmstrData();
	STATUS InitializeRmstrDataReply(void *_pContext, STATUS status);
	void InitializeCommandQueues();
	STATUS rmstrStringResourceManagerInitializedReply(
					void			*pContext,
					STATUS			status);

#ifdef RMSTR_SSAPI_TEST	
	void rmstrFakeRaidDdmCmdServerInitializedReply(STATUS status);
	void rmstrFakeRaidDdmCmdProcessor(HANDLE handle, void	*_pRaidRequest);
	STATUS RaidDdmSimulationStartUtilReply(void *pContext, STATUS status);
	STATUS RaidDdmSimulationStopUtilReply(void *pContext, STATUS status);
	STATUS RaidDdmSimulationDownMemberReply(void *pContext, STATUS status);

#endif

	// VCM CMD QUEUE
	void VCMCmdSenderInitializedReply(STATUS status);


	// RMSTR CMD QUEUE
	void rmstrCommandReceiver(HANDLE handle, void* pClientContext);
	STATUS rmstrEventHandler(void* pModifiedSQRecord, STATUS status);
	void rmstrCmdServerInitializedReply(STATUS status);
	void rmstrInternalCmdSenderInitializedReply(STATUS status);


	// RMSTR DATA
	void PrepareRmstrData(
			U32				type, 
			RMSTR_DATA		*pRmstrData, 
			U32				*pSizeofData,
			RMSTR_DATA		**ppHead, 
			RMSTR_DATA		**ppTail);

	void AddRmstrData(U32 type, rowID *pRowId, void *pData);
	void AddRmstrDataToHead(U32 type, rowID *pRowId, void *pData);

	void GetRmstrData(U32 type, rowID *pRowId, void **ppRmstrDataRet);
	void RemoveRmstrData(U32 type, rowID *pRowId);
	void ModifyRmstrData(
			U32		type,
			rowID	*pRowId,
			void	*pData);
	void TraverseRmstrData(
			U32		type,
			rowID	*pRowId, 
			void	**ppRmstrDataRet);
	void CleanRmstrData();
	void CleanData(U32 type);
	void CheckRmstrDataConsistency();
	void CheckArrayData();
	void CheckUtilityData();




	BOOL CheckIfStateAlreadyProcessed(
				U32					type,
				STATE_IDENTIFIER	*pStateIdentifier,
				void				*pBuffer);

	BOOL CheckIfDataAlreadyInserted(
				U32					type,
				rowID				*pRowId);
	
	STATUS CheckAndModifyRow(
			U32					type,
			STATE_IDENTIFIER	*pStateIdentifier,
			String64			tableName,
			rowID				*pRowToModify,
			void				*pBuffer,
			U32					sizeofData,
			rowID				*pNewRowIdRet,
			pTSCallback_t		cb,
			void				*pOriginalContext);
	STATUS CheckAndInsertRow(
			U32					type,
			STATE_IDENTIFIER	*pStateIdentifier,
			String64			tableName,
			void				*pBuffer,
			U32					sizeofData,
			rowID				*pNewRowIdRet,
			pTSCallback_t		cb,
			void				*pOriginalContext);
	void SetStateIdentifier(
			STATE_IDENTIFIER	*pStateIdentifier,
			U32					opcode,
			rowID				*pCmdRowId,
			U32					state,
			U32					index);
	BOOL SimulateFailover(CONTEXT *pCmdContext);

	// RAID DDM QUEUE
	void RaidCmdSenderInitializedReply(STATUS status);
	void RaidDdmCommandCompletionReply(
				STATUS				completionCode,
				void				*pStatusData,
				void				*pCmdData,
				void				*pCmdContext);
	void RaidDdmEventHandler(
			STATUS			eventCode,
			void			*pStatusData);
	void ProcessUtilityEvents(
			STATUS			eventCode,
			void			*pStatusData);
	STATUS ProcessUtilityEventsReply(void *_pContext, STATUS status);


	void CheckForEventsToBeGeneratedOnCmdCompletion(
				CONTEXT		*pRmstrCmdContext);

	// PROCESS MEMBER DOWN EVENT
	STATUS ProcessDownMemberEvent(HANDLE h, RMSTR_CMND_INFO *_pCmndInfo);
	STATUS ProcessMemberDownEventReply(
			void			*_pContext,
			STATUS			status);

	// PROCESS ARRAY OFFLINE EVENT
	STATUS ProcessArrayOfflineEvent(HANDLE h, RMSTR_CMND_INFO *_pCmndInfo);
	STATUS ProcessArrayOfflineEventReply(
			void			*_pContext,
			STATUS			status);

	// PROCESS STOP UTILITY EVENT
	STATUS ProcessStopUtilEvent(HANDLE h, RMSTR_CMND_INFO *_pCmndInfo);	
	STATUS ProcessStopUtilEventReply(void *_pContext, STATUS status);
	void UpdateArrayStateAndGenerateEvents(
			RAID_ARRAY_DESCRIPTOR		*pADTRecord, 
			RAID_ARRAY_UTILITY			*pUtility);
	STATUS ProcessUpdateArrayStateAndGenerateEventsReply(
			void						*_pContext, 
			STATUS						status);



	// OUTSTANDING COMMANDS
	void AddCommandToQueue(
				HANDLE			handle,
				RMSTR_CMND_INFO *pCmdInfo,
				BOOL			addToHead);
	void RunOutstandingCommand();
	void StartCommandProcessing(
				HANDLE			handle,
				RMSTR_CMND_INFO	*pCmdInfo);
	void StopCommandProcessing(
				BOOL	checkForNextCmd,
				HANDLE	handle);

	// RMSTR INTERNAL COMMANDS
	STATUS StartInternalDownMember(RAID_ARRAY_DESCRIPTOR	*pADTRecord);

	STATUS StartInternalHotCopy(
			RAID_ARRAY_DESCRIPTOR			*pADTRecord,
			RAID_UTIL_PRIORITY				priority);
	STATUS StartInternalBkgdInit(RAID_ARRAY_DESCRIPTOR *pADTRecord);
	STATUS StartInternalRegenerate(RAID_ARRAY_DESCRIPTOR *pADTRecord);
	STATUS StartInternalChangeSourceMember(RAID_ARRAY_DESCRIPTOR *pADTRecord);
	STATUS StartInternalDeleteArray(
			RAID_ARRAY_DESCRIPTOR			*pADTRecord,
			BOOL							breakHotCopy,
			BOOL							useSourceAsExport);

	void StartInternalCommitSpare(
			RAID_SPARE_DESCRIPTOR		*pSpare,
			RAID_ARRAY_DESCRIPTOR		*pADTRecord,
			RAID_ARRAY_MEMBER			*pMember);
	void StartInternalProcessMemberDownEvent(
			rowID						*pArrayRowId,
			rowID						*pMemberRowId,
			U32							reason);
	void StartInternalProcessArrayOfflineEvent(
			rowID			*pArrayRowId,
			rowID			*pMemberRowId,
			U32				reason);

	void StartInternalProcessStopUtilEvent(
			rowID						*pUtilRowId,
			U32							miscompareCount,
			RAID_UTIL_STATUS			reason);


	void rmstrInternalCmdCompletionReply (
			STATUS			completionCode,
			void			*pResultData,
			void			*pCmdData,
			void			*pCmdContext);


	// DEFINE TABLES
	STATUS rmstrDefineDescriptorTables();
	STATUS rmstrDefineArrayDescriptorTable(CONTEXT *_pContext);
	STATUS rmstrDefineMemberDescriptorTable(CONTEXT *_pContext);
	STATUS rmstrDefineSpareDescriptorTable(CONTEXT *_pContext);
	STATUS rmstrDefineUtilDescriptorTable(CONTEXT *_pContext);
	STATUS rmstrDefineCapabiliyTable(CONTEXT *pContext);
	STATUS rmstrPopulateCapabilitiesTableReply(
				pTSCallback_t	cb,
				CONTEXT			*pContext);
	STATUS rmstrPopulateCapabilitiesTableReply(
				void			*_pContext,
				STATUS			status);

	STATUS rmstrPopulateCapabilitiesTable(pTSCallback_t cb, CONTEXT *pContext);
	STATUS rmstrProcessDefineDescriptorTablesReply(void *pClientContext, STATUS status);

	// CREATE ARRAY
	STATUS caCreateArrayValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo);
	STATUS caProcessCreateArrayReply(void *pClientContext, STATUS status);
	STATUS caInsertArrayDescriptorRecord(
				HANDLE				handle,
				RMSTR_CMND_INFO		*pCmdInfo);
	STATUS caInsertMemberDescriptorRecords(
				RAID_ARRAY_DESCRIPTOR			*pADTRecord,
				StorageRollCallRecord			*pSRCRecord,
				RAID_ARRAY_MEMBER				*pMember,
				U32								memberIndex,
				CONTEXT							*pCmdContext);
	STATUS caInsertSpareDescriptorRecords(CONTEXT *pContext);

	STATUS caCheckAndInsertSRCEntryForArray(
				StorageRollCallRecord			*pSRCRecord,
				CONTEXT							*pParentContext,
				pTSCallback_t					cb);
	STATUS caCheckAndInsertSRCEntryForArrayReply(
				void							*_pContext,
				STATUS							status);
	void caPrepareSRCEntryForArray(
				StorageRollCallRecord			*pSRCRecord,
				RMSTR_CREATE_ARRAY_DEFINITION	*pCreateArrayDefinition,
				RAID_ARRAY_DESCRIPTOR			*pADTRecord);


		// Create Array Valiation
		STATUS caProcessCreateArrayValidationReply(
				void				*_pContext,
				STATUS				status);		
		STATUS caReadSRCTRecord(
				rowID				*pRowToRead,
				pTSCallback_t		cb,
				CONTEXT				*pValidationContext);
		STATUS CheckRaidLevelCapability(
				RAID_LEVEL						raidLevel,
				RMSTR_CAPABILITY_RAID_LEVEL		*pRaidLevelCapability,
				U32								*pDataBlockSize,
				U32								*pParityBlockSize);


	// DELETE ARRAY
	STATUS DeleteTheArray(
				HANDLE					h,
				RMSTR_CMND_INFO			*pCmdInfo,
				RAID_ARRAY_DESCRIPTOR	*pADTRecord);
	STATUS ProcessDeleteArrayReply(void *_pContext, STATUS status);
	STATUS DeleteTableRowsForArrayElements(
				U32						type,
				RAID_ARRAY_DESCRIPTOR	*pADTRecord,
				pTSCallback_t			cb,
				CONTEXT					*pCmdContext);
	STATUS ProcessDeleteTableRowsReply(void *_pContext, STATUS status);
	void BreakHotCopy(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMember,
		pTSCallback_t				cb,
		void						*_pContext);
	STATUS ProcessBreakHotCopyReply(
				void					*_pContex,
				STATUS					status);
	STATUS DeleteArray_VCModifyMessageReply(Message *pMsg);

		// Delete Array Validation
		STATUS DeleteArrayValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo);
		STATUS ProcessDeleteArrayValidationReply(void *_pContext, STATUS status);





	// CHANGE ARRAY NAME
	STATUS ChangeArrayName(HANDLE h, RMSTR_CMND_INFO *pCmndInfo);
	STATUS ChangeTheArrayName(
				HANDLE						handle,
				RMSTR_CMND_INFO				*pCmdInfo,
				RAID_ARRAY_DESCRIPTOR		*pADTRecord);
	STATUS ProcessChangeArrayNameReply(
					void			*_pContext,
					STATUS			status);
		// Change Array Name Valiation
		STATUS ChangeArrayNameValidation(
					HANDLE			h,
					RMSTR_CMND_INFO *pCmdInfo);
		STATUS ProcessChangeArrayNameValidationReply(
						void		*_pContext,
						STATUS		status);

	// CREATE SPARES
	STATUS rmstrCreateTheSpare(
			HANDLE							handle,
			RMSTR_CMND_INFO					*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR			*_pADTRecord,
			StorageRollCallRecord			*_pSRCRecord);
	STATUS rmstrProcessCreateSpareReply(void *pClientContext, STATUS status);
	STATUS rmstrInsertSpareDescriptor(
			STATE_IDENTIFIER	*pStateIdentifier,
			RAID_SPARE_TYPE		s,
			rowID				*pRid,
			rowID				*pSRC,
			rowID				*pHost,
			I64					capacity,
			VDN					bsaVdn,
			pTSCallback_t		cb,
			CONTEXT				*pCmdContext);
		// Create Spare Valiation
		STATUS rmstrCreateSpareValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo);
		STATUS rmstrProcessCreateSpareValidationReply(void *_pContext, STATUS status);

	// DELETE SPARE
	STATUS DeleteTheSpare(
		HANDLE						handle,
		RMSTR_CMND_INFO				*_pCmdInfo,
		RAID_SPARE_DESCRIPTOR		*_pSpare);
	STATUS ProcessDeleteSpareReply(
		void						*pClientContext, 
		STATUS						status);
	BOOL TraverseArrayThatFitsForThisSpare(
		RAID_SPARE_DESCRIPTOR		*pSpare,
		RAID_ARRAY_DESCRIPTOR		**ppArrayThatFits);



		// Delete Spare Valiation
		STATUS DeleteSpareValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo);
		STATUS ProcessDeleteSpareValidationReply(void *_pContext, STATUS status);

	// START UTILITY
	STATUS utilStartValidatedUtility(
				HANDLE					handle,
				RMSTR_CMND_INFO			*pCmdInfo,
				RAID_ARRAY_DESCRIPTOR	*pADTRecord);

	void PrepareVerifyMembers(
			RAID_ARRAY_DESCRIPTOR		*pADTRecord,
			rowID						**ppSrcIds,
			rowID						**ppDestIds,
			U32							*pNumberSourceMembers,
			U32							*pNumberDestinationMembers);
	void PrepareRegenerateMembers(
			RAID_ARRAY_DESCRIPTOR		*pADTRecord,
			rowID						**ppSrcIds,
			rowID						**ppDestIds,
			U32							*pNumberSourceMembers,
			U32							*pNumberDestinationMembers);

	STATUS rmstrProcessStartUtilityReply(void *pCmdContext, STATUS status);
	STATUS rmstrInsertUtilDescriptor(
			STATE_IDENTIFIER	*pStateIdentifier,
			RAID_UTIL_NAME		pUtilityCode,
			rowID				*pArrayRID,
			RAID_UTIL_PRIORITY	priority,
			U32					percentCompleteUpdateRate,
			RAID_UTIL_POLICIES	policy,
			rowID				*pSourceRID,
			U32					numSourceMembers,
			rowID				*pDestinationRIDs,
			U32					numDestinationMembers,
			I64					endLBA,
			pTSCallback_t		cb,
			CONTEXT				*pContext);
		// start utility validation
		STATUS rmstrStartUtilReadTarget(
			HANDLE						handle, 
			RMSTR_CMND_INFO				*pCmdInfo);
		STATUS ProcessStartUtilReadTargetReply(
			void						*_pContext, 
			STATUS						status);

		STATUS rmstrStartUtilValidation(
			HANDLE						h,
			rowID						*pADTRid,
			RMSTR_CMND_INFO				*pCmdInfo);
		STATUS rmstrProcessStartUtilValidationReply(void *_pContext, STATUS status);
		STATUS CheckIfUtilAllowed(
			RAID_UTIL_NAME			utilityName,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord);
		STATUS CheckIfUtilAlreadyRunning(
			RAID_UTIL_NAME			utilityName,
			RAID_UTIL_POLICIES		policy,
			U32						numberDestinationMembers,
			rowID					*pDestinationRowIds,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord);

	// HOT COPY
	STATUS ProcessModifyVCReply(Message* pMsg);
	STATUS StartHotCopy(
			RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
			pTSCallback_t			pCallback,
			CONTEXT					*_pParentContext);
	STATUS ProcessStartHotCopyReply(
			void					*_pContext, 
			STATUS					status);

	// ABORT UTILITY
	STATUS AbortTheValidatedUtility(
			HANDLE					handle,
			RMSTR_CMND_INFO			*pCmdInfo,
			RAID_ARRAY_UTILITY		*pUtility);
	STATUS AbortUtilValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo);

	// CHANGE PRIORITY
	STATUS ChangeThePriority(
			HANDLE					handle,
			RMSTR_CMND_INFO			*pCmdInfo,
			RAID_ARRAY_UTILITY		*pUtility);
	STATUS ProcessChangePriorityReply(void *_pContext, STATUS status);
	STATUS ChangePriorityValidation(HANDLE h, RMSTR_CMND_INFO *pCmdInfo);

	// DOWN A MEMBER
	STATUS DownAMember(
		HANDLE						handle,
		RMSTR_CMND_INFO				*pCmndInfo,
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMDTRecord);
	STATUS DownAMemberValidation(
		HANDLE						h,
		RMSTR_CMND_INFO				*pCmdInfo);
	BOOL CheckIfArrayCritical(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		BOOL						generateEvent);
	BOOL CheckIfArrayOffline(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord);
	BOOL CheckForAllCriticalArrays(
		RAID_SPARE_DESCRIPTOR		*pSpare);
	void FindFirstDownMember(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		void						**ppMember);


	// ADD MEMBER
	STATUS AddTheMember(
			HANDLE							handle,
			RMSTR_CMND_INFO					*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR			*_pADTRecord,
			StorageRollCallRecord			*_pSRCRecord);

	STATUS ProcessAddMemberReply(void *_pContext, STATUS status);
	STATUS AddMemberValidation(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo);
	STATUS ProcessAddMemberValidationReply(void *_pContext, STATUS status);
	void rmstrServicePrepareMemberInformation(
			RAID_ARRAY_MEMBER			*pMember,
			rowID						*pArrayRowId,
			rowID						*pSRCRowId,
			RAID_MEMBER_STATUS			memberHealth,
			U32							memberIndex,
			I64							endLBA,
			I64							startLBA,
			VDN							memberVD,
			U32							maxRetryCnt,
			RAID_QUEUE_METHOD			queueMethod,
			U32							maxOutstanding,
			RAID_MEMBER_POLICIES		policy);


	// REMOVE MEMBER
	STATUS RemoveMember(HANDLE h, RMSTR_CMND_INFO *pCmndInfo);
	STATUS RemoveTheMember(
			HANDLE							handle,
			RMSTR_CMND_INFO					*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR			*_pADTRecord,
			RAID_ARRAY_MEMBER				*_pMember);
	STATUS RemoveMemberValidation(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo);

	// CHANGE PREFERRED MEMBER
	STATUS ChangeThePreferredMember(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
			RAID_ARRAY_MEMBER		*_pMember);
	STATUS ProcessChangePreferredMemberReply(
			void					*_pContext,
			STATUS					status);
	STATUS ChangePreferredMemberValidation(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo);


	// CHANGE SOURCE MEMBER
	STATUS ChangeTheSourceMember(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo,
			RAID_ARRAY_DESCRIPTOR	*_pADTRecord,
			RAID_ARRAY_MEMBER		*_pMember);
	STATUS ProcessChangeSourceMemberReply(
			void					*_pContext,
			STATUS					status);
	STATUS ChangeSourceMemberValidation(
			HANDLE					handle,
			RMSTR_CMND_INFO			*_pCmdInfo);


	// COMMIT SPARE (INTERNAL CMD)
	STATUS CommitFirstValidSpare(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMember);
	void CommitSpare(
		HANDLE						handle,
		RMSTR_CMND_INFO				*pCmdInfo);

	STATUS CommitTheSpare(
		HANDLE						handle,
		RMSTR_CMND_INFO				*pCmdInfo,
		RAID_SPARE_DESCRIPTOR		*pSpare,
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMember);
	STATUS ProcessCommitSpareReply(
		void						*_pContext,
		STATUS						status);
	STATUS GetValidSpare(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_SPARE_DESCRIPTOR		**ppSpare);

	// LOG EVENTS
	void LogSpareAddedEvent(
		CONTEXT						*pCmdContext, 
		RAID_SPARE_DESCRIPTOR		*pSpare);

	void LogSpareDeletedEvent(
		CONTEXT						*pCmdContext, 
		RAID_SPARE_DESCRIPTOR		*pSpare);

	void LogNoMoreSparesEvent(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_SPARE_DESCRIPTOR		*pSpare);
	void LogSpareActivatedEvent(
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_SPARE_DESCRIPTOR		*pSpare);
	STATUS ProcessLogSpareActivatedEventReply(
		void						*_pContext, 
		STATUS						status);
	void LogEventWithArrayName(
		U32							eventCode,
		rowID						*pNameRowId);
	STATUS ProcessLogEventWithArrayNameReply(
		void						*_pContext, 
		STATUS						status);
	void LogMemberEvent(
		U32							eventCode,
		RAID_ARRAY_DESCRIPTOR		*pADTRecord,
		RAID_ARRAY_MEMBER			*pMember);
	STATUS ProcessLogMemberEventReply(
		void						*_pContext, 
		STATUS						status);

	// ALARMS
	void RmstrSubmitAlarm(
		U32							eventCode,
		U32							type,
		rowID						*pRowId);
	STATUS ProcessRmstrSubmitAlarmReply(
		void						*_pContext, 
		STATUS						status);
	void RmstrRemitAlarm(
		U32							eventCode,
		rowID						*pRowId);
	void RmstrGetOutstandingAlarm(
		U32							eventCode,
		rowID						*pRowId,
		ALARM_CONTEXT				**_pAlarmContext);
	void cbRecoverAlarm( 
		void						*_pAlarmContext,
		STATUS						status);



	// SERVICES
	STATUS rmstrServiceCheckSize(
			BOOL					isMember,
			StorageRollCallRecord	*pSRCRecord,
			I64						smallestMemberCapacity);
	BOOL rmstrServiceDeleteMemberSpareUtilityFromADT(
			U32						type,
			rowID					*pRowId,
			RAID_ARRAY_DESCRIPTOR	*pADTRecord);



};

#endif

