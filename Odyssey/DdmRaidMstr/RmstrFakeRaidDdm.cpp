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
// File: RmstrAbortUtil.cpp
// 
// Description:
// Implementation for the abort utility command
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/RmstrFakeRaidDdm.cpp $
// 
// 9     9/07/99 3:40p Dpatel
// removed member hot copy...
// 
// 8     9/01/99 6:38p Dpatel
// added logging and alarm code..
// 
// 7     8/31/99 6:39p Dpatel
// events, util abort processing etc.
// 
// 6     8/24/99 5:13p Dpatel
// added failover code, create array changes for "array name"
// 
// 5     8/20/99 3:03p Dpatel
// added simulation for failover and failover code (CheckAnd...() methods)
// 
// 4     8/12/99 1:56p Dpatel
// Added Array offline event processing code.
// 
// 3     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64. added simulation raid
// ddm.
// 
// 2     8/06/99 2:06p Dpatel
// added stop util for init also..
// 
// 1     8/05/99 11:07a Dpatel
// Initial creation
// 
//
/*************************************************************************/
#include "OsTypes.h"
#include "Message.h"
#include "CTTypes.h"
#include "OsStatus.h"
#include "Ddm.h"
#include "Fields.h"
#include "Rows.h"
#include "Listen.h"
#include "ReadTable.h"
#include "Table.h"
#include "PTSCommon.h"
#include "RequestCodes.h"

#include "DdmRaidMgmt.h"

#include "StorageRollCallTable.h"
#include "RaidUtilTable.h"
#include "ArrayDescriptor.h"

#include "RMgmtPolicies.h"
#include "RaidDefs.h"
#include "RmstrCmnds.h"




#ifdef RMSTR_SSAPI_TEST

static	U32			alreadySent = false;

//**************************************************************
//
//
//
//***************************************************************
void DdmRAIDMstr
::rmstrFakeRaidDdmCmdProcessor(HANDLE handle, void	*_pRaidRequest)
{
	RaidRequest					*pRaidRequest = (RaidRequest *)_pRaidRequest;
	RAID_ARRAY_UTILITY			*pUtility;
	RaidEvent					*pRaidEvent;
	RaidEvent					*pRaidEvent1;
	rowID						*pRowId = NULL;
	RAID_ARRAY_MEMBER			*pMember = NULL;
	RaidMemberDownEvent			*pRaidMemberDownEvent = NULL;



	switch (pRaidRequest->Opcode){
	case RAID_REQUEST_START_UTIL:
		pUtility = &pRaidRequest->Data.UtilityData;
		switch (pUtility->utilityCode){
		case RAID_UTIL_BKGD_INIT:
		case RAID_UTIL_REGENERATE:
		case RAID_UTIL_LUN_HOTCOPY:
		case RAID_UTIL_VERIFY:
			pRaidEvent = new RaidEvent;
			pRaidEvent->Event.StartUtil.UtilRowID = pUtility->thisRID;
			pUtility->status = RAID_UTIL_RUNNING;
			m_pTableServices->TableServiceModifyField(
							RAID_UTIL_DESCRIPTOR_TABLE,
							&pUtility->thisRID,	// row id to modify
							fdUTIL_STATUS,
							&pUtility->status,
							sizeof(RAID_UTIL_STATUS),
							(pTSCallback_t)&DdmRAIDMstr::RaidDdmSimulationStartUtilReply,
							pRaidEvent);

			break;
		default:
			assert(0);
		}
		break;

	case RAID_REQUEST_ABORT_UTIL:
		pRowId = &pRaidRequest->Data.AbortData.UtilRowID;

// enable this when you want to simulate member down due to i/o error
#if 0
		pRaidEvent1 = new RaidEvent;
		pRaidMemberDownEvent = &pRaidEvent1->Event.MemberDown;
		GetRmstrData(
			RAID_UTILITY,
			pRowId,
			(void **)&pUtility);

		pRaidMemberDownEvent->MemberRowID = pUtility->destinationRowIds[0];
		pRaidMemberDownEvent->Reason = 5;
		GetRmstrData(
			RAID_MEMBER,
			&pRaidMemberDownEvent->MemberRowID,
			(void **)&pMember);
		assert(pMember);
		pRaidMemberDownEvent->ArrayRowID = pMember->arrayRID;
		pMember->memberHealth = RAID_STATUS_DOWN;
		m_pTableServices->TableServiceModifyField(
							RAID_MEMBER_DESCRIPTOR_TABLE,
							&pRaidMemberDownEvent->MemberRowID,
							fdMEMBER_HEALTH,
							&pMember->memberHealth,
							sizeof(RAID_MEMBER_STATUS),
							(pTSCallback_t)&DdmRAIDMstr::RaidDdmSimulationDownMemberReply,
							pRaidEvent1);
#endif


		pRaidEvent = new RaidEvent;
		pRaidEvent->Event.StopUtil.UtilRowID = *pRowId;
		pRaidEvent->Event.StopUtil.Reason = RAID_UTIL_ABORTED;
		//pRaidEvent->Event.StopUtil.Reason = RAID_UTIL_ABORTED_IOERROR;
		m_pTableServices->TableServiceModifyField(
							RAID_UTIL_DESCRIPTOR_TABLE,
							pRowId,			// row id to modify
							fdUTIL_STATUS,
							&pRaidEvent->Event.StopUtil.Reason,
							sizeof(RAID_UTIL_STATUS),
							(pTSCallback_t)&DdmRAIDMstr::RaidDdmSimulationStopUtilReply,
							pRaidEvent);
		break;

	case RAID_REQUEST_DOWN_MEMBER:
		// Generate a fake event..
		pRaidEvent = new RaidEvent;
		pRaidMemberDownEvent = &pRaidEvent->Event.MemberDown;

		pRaidMemberDownEvent->MemberRowID = pRaidRequest->Data.DownData.MemberRowID;
		pRaidMemberDownEvent->Reason = 5;
		GetRmstrData(
			RAID_MEMBER,
			&pRaidMemberDownEvent->MemberRowID,
			(void **)&pMember);
		pRaidMemberDownEvent->ArrayRowID = pMember->arrayRID;
		pMember->memberHealth = RAID_STATUS_DOWN;
		m_pTableServices->TableServiceModifyField(
							RAID_MEMBER_DESCRIPTOR_TABLE,
							&pRaidMemberDownEvent->MemberRowID,
							fdMEMBER_HEALTH,
							&pMember->memberHealth,
							sizeof(RAID_MEMBER_STATUS),
							(pTSCallback_t)&DdmRAIDMstr::RaidDdmSimulationDownMemberReply,
							pRaidEvent);
		break;

	default:
		break;
	}
	m_pFakeRaidDdmCmdServer->csrvReportCmdStatus(
				handle,					// handle
				RMSTR_SUCCESS,			// completion code
				NULL,					// result Data
				_pRaidRequest);
}


//**************************************************************
//
//
//
//***************************************************************
STATUS DdmRAIDMstr
::RaidDdmSimulationStartUtilReply(void *pContext, STATUS status)
{
	RaidEvent			*pRaidEvent = (RaidEvent *)pContext;
	m_pFakeRaidDdmCmdServer->csrvReportEvent(
					RAID_EVT_UTIL_STARTED,
					pRaidEvent);
#if 1
	// if we want to abort a util as soon as it starts
	pRaidEvent->Event.StopUtil.UtilRowID = pRaidEvent->Event.StartUtil.UtilRowID;
	if (alreadySent){
		pRaidEvent->Event.StopUtil.Reason = RAID_UTIL_COMPLETE;
	} else {
		pRaidEvent->Event.StopUtil.Reason = RAID_UTIL_ABORTED_IOERROR;
		pRaidEvent->Event.StopUtil.Reason = RAID_UTIL_COMPLETE;
	}

	m_pTableServices->TableServiceModifyField(
								RAID_UTIL_DESCRIPTOR_TABLE,
								&pRaidEvent->Event.StopUtil.UtilRowID,
								fdUTIL_STATUS,
								&pRaidEvent->Event.StopUtil.Reason,
								sizeof(RAID_UTIL_STATUS),
								(pTSCallback_t)&DdmRAIDMstr::RaidDdmSimulationStopUtilReply,
								pRaidEvent);
#else 
	delete pRaidEvent;
#endif
	return status;
}


//**************************************************************
//
//
//
//***************************************************************
STATUS DdmRAIDMstr
::RaidDdmSimulationStopUtilReply(void *pContext, STATUS status)
{
	RaidEvent			*pRaidEvent = (RaidEvent *)pContext;


	m_pFakeRaidDdmCmdServer->csrvReportEvent(
					RAID_EVT_UTIL_STOPPED,
					pRaidEvent);

#if 0
	if (alreadySent == false){
	// if we choose to fake a down member while any util is running...
		RaidMemberDownEvent			*pRaidMemberDownEvent = NULL;
		RAID_ARRAY_MEMBER			*pMember = NULL;


		pRaidMemberDownEvent = &pRaidEvent->Event.MemberDown;

		pRaidMemberDownEvent->MemberRowID.Table = 9;
		pRaidMemberDownEvent->MemberRowID.HiPart = 0;
		pRaidMemberDownEvent->MemberRowID.LoPart = 3;
		pRaidMemberDownEvent->Reason = 5;
		GetRmstrData(
			RAID_MEMBER,
			&pRaidMemberDownEvent->MemberRowID,
			(void **)&pMember);
		pRaidMemberDownEvent->ArrayRowID = pMember->arrayRID;
		pMember->memberHealth = RAID_STATUS_DOWN;
		m_pTableServices->TableServiceModifyField(
							RAID_MEMBER_DESCRIPTOR_TABLE,
							&pRaidMemberDownEvent->MemberRowID,
							fdMEMBER_HEALTH,
							&pMember->memberHealth,
							sizeof(RAID_MEMBER_STATUS),
							(pTSCallback_t)&DdmRAIDMstr::RaidDdmSimulationDownMemberReply,
							pRaidEvent);
		alreadySent = true;
	}
#else
	delete pRaidEvent;
#endif
	return status;
}



//**************************************************************
//
//
//
//***************************************************************
STATUS DdmRAIDMstr
::RaidDdmSimulationDownMemberReply(void *pContext, STATUS status)
{
	RaidEvent			*pRaidEvent = (RaidEvent *)pContext;

//#define SIMULATE_ARRAY_OFFLINE
#ifdef SIMULATE_ARRAY_OFFLINE
	RaidOfflineEvent	*pRaidOfflineEvent;
	
	pRaidOfflineEvent = &pRaidEvent->Event.RaidOffline;

	pRaidOfflineEvent->ArrayRowID = pRaidEvent->Event.MemberDown.ArrayRowID;
	pRaidOfflineEvent->MemberRowID = pRaidEvent->Event.MemberDown.MemberRowID;

	m_pFakeRaidDdmCmdServer->csrvReportEvent(
					RAID_EVT_ARRAY_OFFLINE,
					pRaidEvent);
	delete pRaidEvent;
#else
	m_pFakeRaidDdmCmdServer->csrvReportEvent(
					RAID_EVT_MEMBER_DOWN,
					pRaidEvent);
	delete pRaidEvent;
#endif
	return status;
}


void DdmRAIDMstr
::rmstrFakeRaidDdmCmdServerInitializedReply(STATUS status)
{
	status = status;
}

#endif