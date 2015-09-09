/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raid.cpp
//
// Description:	Raid base class
//
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "Ddm.h"
#include "CtTypes.h"
#include "RaidStructs.h"
#include "UtilCmd.h"
#include "RaidIorQ.h"
#include "RaidQ.h"
#include "UtilQ.h"
#include "Member.h"
#include "RaidErr.h"
#include "RaidDefs.h"
#include "RaidUtilTable.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "Scsi.h"
#include "OsStatus.h"
#include "Raid.h"

/*************************************************************************/
// Raid
// Constructor method for the class Raid
/*************************************************************************/

Raid::Raid() : DdmServices()
{
	Flags = 0;
	ResetStatus();
	ResetPerformance();
}

/*************************************************************************/
// ReadMemberTableRow
// Read specified row from Member Table
/*************************************************************************/

void	Raid::ReadMemberTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback)
{
	STATUS		Status;

	// allocate the ReadRow class
	TSReadRow	*pTSReadRow = new TSReadRow;

	// row id to read in member table
	rowID		*pRowId = &pRaidRow->RowData.MemberData.thisRID;
		
	// Initialize the ReadRow operation.
	Status = pTSReadRow->Initialize(
			this,								// DdmServices pDdmServices,
			RAID_MEMBER_DESCRIPTOR_TABLE,		// String64 rgbTableName,
			CT_PTS_RID_FIELD_NAME,				// String64 prgbKeyFieldName,
			pRowId,  			  				// void* pKeyFieldValue,
			sizeof(rowID),						// U32 cbKeyFieldValue,
			&pRaidRow->RowData.MemberData,		// void* prgbRowData,
			sizeof(RAID_ARRAY_MEMBER),			// U32 cbRowData,
			NULL,								// rowID *pRowIDRet,
			(pTSCallback_t)pCallback,			// pTSCallback_t pCallback,
			(void*)pRaidRow						// void* pContext
	);

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		// If error initializing, do callback with error
		( (this)->*(pCallback) )(pRaidRow, Status);
		// delete TSReadRow object
		delete pTSReadRow;
		return;
	}

	// Initiate the ReadRow table operation.
	pTSReadRow->Send();

	return;
}

/*************************************************************************/
// ReadUtilityTableRow
// Read specified row from Utility Table
/*************************************************************************/

void	Raid::ReadUtilityTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback)
{
	STATUS		Status;

	// allocate the ReadRow class
	TSReadRow	*pTSReadRow = new TSReadRow;

	// row id to read in utility table
	rowID		*pRowId = &pRaidRow->RowData.UtilityData.thisRID;
	
	// Initialize the ReadRow operation.
	Status = pTSReadRow->Initialize(
			this,								// DdmServices pDdmServices,
			RAID_UTIL_DESCRIPTOR_TABLE,			// String64 rgbTableName,
			CT_PTS_RID_FIELD_NAME,				// String64 prgbKeyFieldName,
			pRowId,			    				// void* pKeyFieldValue,
			sizeof(rowID),						// U32 cbKeyFieldValue,
			&pRaidRow->RowData.UtilityData,		// void* prgbRowData,
			sizeof(RAID_ARRAY_UTILITY),			// U32 cbRowData,
			NULL,								// rowID *pRowIDRet,
			(pTSCallback_t)pCallback,	 		// pTSCallback_t pCallback,
			(void*)pRaidRow						// void* pContext
	);

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		// If error initializing, do callback with error
		( (this)->*(pCallback) )(pRaidRow, Status);
		// delete TSReadRow object
		delete pTSReadRow;
		return;
	}

	// Initiate the ReadRow table operation.
	pTSReadRow->Send();

	return;
}

/*************************************************************************/
// ReadArrayTableRow
// Read specified row from Array Table
/*************************************************************************/

void	Raid::ReadArrayTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback)
{
	STATUS		Status;

	// allocate the ReadRow class
	TSReadRow	*pTSReadRow = new TSReadRow;

	// row id to read in array decriptor table
	rowID		*pRowId = &pRaidRow->RowData.ArrayData.thisRID;
		
	// Initialize the ReadRow operation.
	Status = pTSReadRow->Initialize(
			this,								// DdmServices pDdmServices,
			RAID_ARRAY_DESCRIPTOR_TABLE,		// String64 rgbTableName,
			CT_PTS_RID_FIELD_NAME,				// String64 prgbKeyFieldName,
			pRowId,  			  				// void* pKeyFieldValue,
			sizeof(rowID),						// U32 cbKeyFieldValue,
			&pRaidRow->RowData.ArrayData,		// void* prgbRowData,
			sizeof(RAID_ARRAY_DESCRIPTOR),		// U32 cbRowData,
			NULL,								// rowID *pRowIDRet,
			(pTSCallback_t)pCallback,  			// pTSCallback_t pCallback,
			(void*)pRaidRow						// void* pContext
	);

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		// If error initializing, do callback with error
		( (this)->*(pCallback) )(pRaidRow, Status);
		// delete TSReadRow object
		delete pTSReadRow;
		return;
	}

	// Initiate the ReadRow table operation.
	pTSReadRow->Send();

	return;
}

/*************************************************************************/
// UpdateUtilityStatusInPTS
// Update the status of specified utility in utility table in PTS
/*************************************************************************/

void	Raid::UpdateUtilityStatusInPTS(Utility *pUtility, pTSCallback_t pCallback)
{
	STATUS		Status;

	// allocate the ModifyField class
	TSModifyField	*pTSModifyField = new TSModifyField;

	// row id to modify
	rowID		*pRowId = &pUtility->Handle;
		
	// Initialize the ModifyField operation.
	Status = pTSModifyField->Initialize(
		this,										// DdmServices pDdmServices,
		RAID_UTIL_DESCRIPTOR_TABLE,					// String64 rgbTableName,
		CT_PTS_RID_FIELD_NAME,						// String64 prgbKeyFieldName,
		pRowId,  			  						// void* pKeyFieldValue,
		sizeof(rowID),								// U32 cbKeyFieldValue,
		fdUTIL_STATUS,								// String64 prgbFieldName,
		&pUtility->Status,    						// void* pFieldValue,
		sizeof(RAID_UTIL_STATUS),	 	 			// U32 cbFieldValue,
		(U32) 1, 									// U32 cRowsToModify,
		NULL,										// U32 *pcRowsModifiedRet,
		NULL,										// rowID *pRowIDRet,
		(U32) sizeof(rowID),  						// U32 cbMaxRowID,
		(pTSCallback_t) pCallback,					// pTSCallback_t pCallback,
		(void*)pUtility								// void* pContext
	);

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		// If error initializing, do callback with error
		( (this)->*(pCallback) )(pUtility, Status);
		// delete TSModifyField object
		delete pTSModifyField;
		return;
	}

	// Initiate the ModifyField table operation.
	pTSModifyField->Send();

	return;
}

/*************************************************************************/
// UpdateUtilityProgressInPTS
// Update the current LBA of specified utility in utility table in PTS
/*************************************************************************/

void	Raid::UpdateUtilityProgressInPTS(Utility *pUtility, pTSCallback_t pCallback)
{
	STATUS		Status;

	// allocate the ModifyField class
	TSModifyField	*pTSModifyField = new TSModifyField;

	// row id to modify
	rowID		*pRowId = &pUtility->Handle;
	
	// Initialize the ModifyField operation.
	Status = pTSModifyField->Initialize(
		this,										// DdmServices pDdmServices,
		RAID_UTIL_DESCRIPTOR_TABLE,					// String64 rgbTableName,
		CT_PTS_RID_FIELD_NAME,						// String64 prgbKeyFieldName,
		pRowId,  			  						// void* pKeyFieldValue,
		sizeof(rowID),								// U32 cbKeyFieldValue,
		fdCURRENT_LBA,								// String64 prgbFieldName,
		&pUtility->CurrentLBA,   					// void* pFieldValue,
		sizeof(U32),				 	 			// U32 cbFieldValue,
		(U32) 1, 									// U32 cRowsToModify,
		NULL,										// U32 *pcRowsModifiedRet,
		NULL,										// rowID *pRowIDRet,
		(U32) sizeof(rowID),  						// U32 cbMaxRowID,
		(pTSCallback_t)pCallback,					// pTSCallback_t pCallback,
		(void*)pUtility								// void* pContext
	);

	if (Status != OS_DETAIL_STATUS_SUCCESS)
	{
		// If error initializing, do callback with error
		( (this)->*(pCallback) )(pUtility, Status);
		// delete TSModifyField object
		delete pTSModifyField;
		return;
	}

	// Initiate the ModifyField table operation.
	pTSModifyField->Send();

	return;
}

/*************************************************************************/
// UpdateUtilityDone
// No Op for a Modify Utility Table Callback
// Can be used by anyone who doesn't care about having a callback
/*************************************************************************/

void	Raid::UpdateUtilityDone(Utility *pUtility, STATUS Status)
{
#pragma unused (pUtility)
#pragma unused (Status)
}

/*************************************************************************/
// SetMemberHealthDownInPTS
// Set Health of all specified members Down in PTS
// Called from Error Processing class
/*************************************************************************/

STATUS	Raid::SetMemberHealthDownInPTS(ErrReqblk *pErrReq, U32 Mask)
{
	TSModifyField	*pTSModifyField;
	rowID			*pRowId;
	U32				MemberMask;

	pErrReq->Status = OS_DETAIL_STATUS_SUCCESS;
	pErrReq->State = 0;
	MemberMask = 1;
	// count number of members to update
	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (Mask & MemberMask)
			pErrReq->State++;
		MemberMask <<= 1;
	}
	MemberMask = 1;
	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (Mask & MemberMask)
		{
			// allocate the ModifyField class
			pTSModifyField = new TSModifyField;

			// row id to modify
			pRowId = &(pMember[i]->MemberRowId);

			// Initialize the ModifyField operation.
			pTSModifyField->Initialize(
				this,						   						// DdmServices pDdmServices,
				RAID_MEMBER_DESCRIPTOR_TABLE,	  					// String64 rgbTableName,
				CT_PTS_RID_FIELD_NAME,								// String64 prgbKeyFieldName,
				pRowId,  			  								// void* pKeyFieldValue,
				sizeof(rowID),										// U32 cbKeyFieldValue,
				fdMEMBER_HEALTH,									// String64 prgbFieldName,
				&pMember[i]->Health,								// void* pFieldValue,
				sizeof(RAID_MEMBER_STATUS),		 					// U32 cbFieldValue,
				(U32) 1, 											// U32 cRowsToModify,
				NULL,												// U32 *pcRowsModifiedRet,
				NULL,												// rowID *pRowIDRet,
				(U32) sizeof(rowID),  								// U32 cbMaxRowID,
				(pTSCallback_t)&Raid::SetMemberHealthDownCallback,	// pTSCallback_t pCallback,
				(void*)pErrReq										// void* pContext
			);

			// Initiate the ModifyField table operation.
			pTSModifyField->Send();
		}
		MemberMask <<= 1;
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// SetMemberHealthDownCallback
// Callback from setting member state down in PTS
// When all member updates done, callback to Error Processing class
/*************************************************************************/

void	Raid::SetMemberHealthDownCallback(ErrReqblk *pErrReq, STATUS Status)
{
	if (Status != OS_DETAIL_STATUS_SUCCESS)
		pErrReq->Status = Status;  	// set error

	pErrReq->State--;	   		  
	if (pErrReq->State == 0)		// done with all members
	{
		// do callback to Error Processing code
		( (pErrReq->pErrInst)->*(pErrReq->pErrCallback) )(pErrReq);
	}
}

/*************************************************************************/
// StartUtility
// Called during initialization after reading Utility Table and finding
// utilities to resume or after receiving a command from the CmdServer
// class
/*************************************************************************/

STATUS	Raid::StartUtility(RAID_ARRAY_UTILITY *pArrayUtility)
{
	STATUS		Status;
	rowID		RowId;
	Utility		*pUtility;
	U8			i, j;

	// Get a utility struct
	if ( (pUtility = new Utility) == NULL)
		return RAIDCMD_STATUS_INSUFFICIENT_RESOURCE;

	pUtility->pForw = pUtility->pBack = NULL;
	pUtility->Flags = 0;
	pUtility->Handle = pArrayUtility->thisRID;
	pUtility->UpdateRate = pArrayUtility->updateRate;
	pUtility->PassNo = 0;
	pUtility->PercentUpdateRate = pArrayUtility->percentCompleteUpdateRate;
	pUtility->PercentPassNo = 0;
	pUtility->Policy = pArrayUtility->policy;
	pUtility->Priority = pArrayUtility->priority;
	pUtility->ErrorCount = 0;
	// get mask of source members
	pUtility->SourceMask = 0;
	for (i = 0; i < MAX_ARRAY_MEMBERS; i++)
	{
		RowId = pArrayUtility->sourceRowIds[i];
		for (j = 0; j < NumberMembers; j++)
		{
			if ((pMember[j]->MemberRowId.LoPart == RowId.LoPart) &&
				(pMember[j]->MemberRowId.HiPart == RowId.HiPart))
				pUtility->SourceMask |= pMember[j]->MemberMask;
		}
	}
	// get mask of destination members
	pUtility->DestMask = 0;
	for (i = 0; i < MAX_ARRAY_MEMBERS; i++)
	{
		RowId = pArrayUtility->destinationRowIds[i];
		for (j = 0; j < NumberMembers; j++)
		{
			if ((pMember[j]->MemberRowId.LoPart == RowId.LoPart) &&
				(pMember[j]->MemberRowId.HiPart == RowId.HiPart))
				pUtility->DestMask |= pMember[j]->MemberMask;
		}
	}

	pUtility->CurrentLBA = pArrayUtility->currentLBA;
	pUtility->EndLBA = pArrayUtility->endLBA;
	pUtility->Cmd = pArrayUtility->utilityCode;

	switch (pUtility->Cmd)
	{
		case RAID_UTIL_LUN_HOTCOPY:
		case RAID_UTIL_MEMBER_HOTCOPY:
		case RAID_UTIL_REGENERATE:
				Status = DoRegenerate(pUtility);
				break;
		case RAID_UTIL_VERIFY:
				Status = DoVerify(pUtility);
				break;
		case RAID_UTIL_BKGD_INIT:
				Status = DoBkgdInit(pUtility);
				break;
		case RAID_UTIL_EXPAND:
				Status = DoExpand(pUtility);
				break;
		default:
			Status = RAIDCMD_STATUS_INAPPROPRIATE_CMD;
	}
	if (Status != RAIDCMD_STATUS_SUCCESS)
		delete pUtility;
	return (Status);
}

/*************************************************************************/
// InitCmdServer
// New a CmdServer and initialize. Specifies routine to be called when
// any command is inserted in Command Queue for Raid DDM, and defines
// Status Queue for Raid DDM to report Command Status or post events.
/*************************************************************************/

STATUS	Raid::InitCmdServer()
{

	// new CmdServer class for Command and Status Queues
	pCmdServer = new CmdServer(RAID_COMMAND_QUEUE, sizeof(RaidRequest),
								sizeof(RaidEvent),
								this, (pCmdCallback_t) &RaidCommand);

	// initialize the CmdServer
	pCmdServer->csrvInitialize((pInitializeCallback_t) &CmdServerInited);
	
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// CmdServerInited
// Called when CmdServer initialization is complete
/*************************************************************************/

void	Raid::CmdServerInited(STATUS Status)
{
#pragma unused (Status)
//temp
//RaidEvent	Event;

//pCmdServer->csrvReportEvent(RAID_EVT_ENABLED, &Event);


}

/*************************************************************************/
// RaidCommand
// Called from CmdServer when a command is put in the Command Queue
/*************************************************************************/

void	Raid::RaidCommand(HANDLE Handle, RaidRequest *pRaidRequest)
{
	STATUS		Status;

	if (pRaidRequest->RaidVDN != RaidVDN)
		return;		// not for us
	switch (pRaidRequest->Opcode)
	{
		case RAID_REQUEST_START_UTIL:
			Status = StartUtility(&pRaidRequest->Data.UtilityData);
			break;
		case RAID_REQUEST_ABORT_UTIL:
			Status = AbortUtility(pRaidRequest->Data.AbortData.UtilRowID);
			break;
		case RAID_REQUEST_DOWN_MEMBER:
			Status = RaidDownMember(pRaidRequest);
			break;
		case RAID_REQUEST_CHG_PRIORITY:
			Status = SetUtilityPriority(pRaidRequest->Data.PriorityData.UtilRowID,
										pRaidRequest->Data.PriorityData.Priority);
			break;
		case RAID_REQUEST_REPLACE_MEMBER:
			Status = DoReplaceMember(pRaidRequest);
			break;
		case RAID_REQUEST_ADD_MEMBER:
			Status = DoAddMember(pRaidRequest);
			break;
		default:
			Status = RAIDCMD_STATUS_INAPPROPRIATE_CMD;
			break;
	}
	// report status
	pCmdServer->csrvReportCmdStatus(Handle, Status, NULL, pRaidRequest);
}

/*************************************************************************/
// DoVerify
// Return error
// Needs to be implemented in any derived class that supports Verify
/*************************************************************************/

STATUS	Raid::DoVerify(Utility *pUtility)
{
#pragma unused (pUtility)

	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
}

/*************************************************************************/
// DoRegenerate
// Return error
// Needs to be implemented in any derived class that supports Regenerate
/*************************************************************************/

STATUS	Raid::DoRegenerate(Utility *pUtility)
{
#pragma unused (pUtility)

	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
}

/*************************************************************************/
// DoBkgdInit
// Return error
// Needs to be implemented in any derived class that supports BkgdInit
/*************************************************************************/

STATUS	Raid::DoBkgdInit(Utility *pUtility)
{
#pragma unused (pUtility)

	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
}

/*************************************************************************/
// DoExpand
// Return error
// Only implemented by the Raid Expand class
/*************************************************************************/

STATUS	Raid::DoExpand(Utility *pUtility)
{
#pragma unused (pUtility)

	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
}

/*************************************************************************/
// DoReplaceMember
// Return error
// For replacing a Down member with a spare
// Needs to be implemented in any derived class that supports it
/*************************************************************************/

STATUS	Raid::DoReplaceMember(RaidRequest *pRaidRequest)
{
#pragma unused (pRaidRequest)

	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
}

/*************************************************************************/
// DoAddMember
// Return error
// Only implemented in RAID1 class
/*************************************************************************/

STATUS	Raid::DoAddMember(RaidRequest *pRaidRequest)
{
#pragma unused (pRaidRequest)

	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
}

/*************************************************************************/
// RaidDownMember
// Call Error Processing class to Down a member
// Called from CmdServer request
/*************************************************************************/

STATUS	Raid::RaidDownMember(RaidRequest *pRaidRequest)
{
	STATUS		Status;
	rowID		RowId;

	if (pRaidErr)
	{
		RowId = pRaidRequest->Data.DownData.MemberRowID;
		Status = RAIDCMD_STATUS_INAPPROPRIATE_CMD;
		for (U8 i = 0; i < NumberMembers; i++)
		{
			if ((pMember[i]->MemberRowId.LoPart == RowId.LoPart) &&
				(pMember[i]->MemberRowId.HiPart == RowId.HiPart))
			{
				Status = pRaidErr->ProcessError(i);
				break;
			}
		}
	}
	else
		Status = RAIDCMD_STATUS_INAPPROPRIATE_CMD;
	return (Status);
}

/*************************************************************************/
// AbortUtility
// Flag a running utility to be aborted
/*************************************************************************/

STATUS	Raid::AbortUtility(rowID Handle)
{
	Utility *pUtility;

	pUtility = UtilCmd.FindUtilityInQueue(Handle);
	if (pUtility)
	{
		pUtility->Flags |= RAID_ABORT_UTILITY;		// flag utility to abort
		return RAIDCMD_STATUS_SUCCESS;
	}
	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;		// utility not running
}

/*************************************************************************/
// SetUtilityPriority
// Set a new priority for a running utility
/*************************************************************************/

STATUS	Raid::SetUtilityPriority(rowID Handle, RAID_UTIL_PRIORITY Priority)
{
	Utility *pUtility;

	pUtility = UtilCmd.FindUtilityInQueue(Handle);
	if (pUtility)
	{
		pUtility->Priority = Priority;				// set new priority
		return RAIDCMD_STATUS_SUCCESS;
	}
	return RAIDCMD_STATUS_INAPPROPRIATE_CMD;		// utility not running
}

/*************************************************************************/
// SuspendAllUtilities
// Flag all running utilities to be suspended
// Used when Quiescing
/*************************************************************************/

void	Raid::SuspendAllUtilities()
{
	UtilReqblk	*pUtilReq;

	// Flag all outstanding utilities to be suspended
	UtilCmd.SuspendUtilities();

	if (pUtilQueue)
	{
		// Do callback on all queued UtilReqs
		// Any outstanding will end on next pass
		while ((pUtilReq = (UtilReqblk *)pUtilQueue->DeQueue()) != NULL)
		{
			pUtilReq->Status = FCP_SCSI_HBA_DSC_REQUEST_ABORTED;
			// do callback
			( (pUtilReq->pInst)->*(pUtilReq->pCallback) )(pUtilReq);
		}
	}
	return;
}

/*************************************************************************/
// DoStatusCheck
// Send Status Check Message to every member
/*************************************************************************/

STATUS	Raid::DoStatusCheck(Ioreq *pIoreq)
{
	RAID_MSTR_STATUS		*pMasterContext;
	RAID_STATUS_CONTEXT		*pStatusContext;
	Message					*pMsg;
	Member					*pMem;

	TRACE_ENTRY(Raid::DoStatusCheck);

	pMasterContext = new RAID_MSTR_STATUS;
	pMasterContext->pIoreq = pIoreq;
	pMasterContext->State = NumberMembers;
	pMasterContext->Failmask = 0;
	IoreqQueue.InsertInQueue(pIoreq);
	for (int i = 0; i < NumberMembers; i++)
	{
		pMem = pMember[i];
		pMsg = new Message(BSA_STATUS_CHECK, sizeof(FCP_MSG_SIZE));

		pStatusContext = new RAID_STATUS_CONTEXT;
		pStatusContext->pMaster = pMasterContext;
		pStatusContext->MemberMask = pMem->MemberMask;
		DdmServices::Send(pMem->Vd, pMsg, pStatusContext, (ReplyCallback) &Raid::StatusCallback);
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoPowerManagement
// Send Power Management Message to every member
/*************************************************************************/

STATUS	Raid::DoPowerManagement(Ioreq *pIoreq)
{
	RAID_MSTR_STATUS		*pMasterContext;
	RAID_STATUS_CONTEXT		*pStatusContext;
	Message					*pMsg;
	Member					*pMem;

	TRACE_ENTRY(Raid::DoPowerManagement);

	pMasterContext = new RAID_MSTR_STATUS;
	pMasterContext->pIoreq = pIoreq;
	pMasterContext->State = NumberMembers;
	pMasterContext->Failmask = 0;
	IoreqQueue.InsertInQueue(pIoreq);
	for (int i = 0; i < NumberMembers; i++)
	{
		pMem = pMember[i];
		pMsg = new Message(BSA_POWER_MANAGEMENT, sizeof(FCP_MSG_SIZE));

		pStatusContext = new RAID_STATUS_CONTEXT;
		pStatusContext->pMaster = pMasterContext;
		pStatusContext->MemberMask = pMem->MemberMask;
		DdmServices::Send(pMem->Vd, pMsg, pStatusContext, (ReplyCallback) &Raid::StatusCallback);
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/	 
// StatusCallback
// Callback for Status Check or Power Management
/*************************************************************************/

void	Raid::StatusCallback(MessageReply *pMsg)
{ 
	RAID_MSTR_STATUS		*pMasterContext;
	RAID_STATUS_CONTEXT		*pStatusContext;
	Ioreq					*pIoreq;
	U8						index;

	TRACE_ENTRY(Raid::StatusCallback);

	pStatusContext = (RAID_STATUS_CONTEXT *)pMsg->GetContext();
	pMasterContext = pStatusContext->pMaster;

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		pMasterContext->Failmask |= pStatusContext->MemberMask;
	}

	pMasterContext->State--;
	delete pStatusContext;
	delete pMsg;
	if (pMasterContext->State == 0)
	{	// done
		// if array would be usable without these members,
		// then set sucessful status
		pIoreq = pMasterContext->pIoreq;
		if (IsRaidUsable(pMasterContext->Failmask))
			pIoreq->Status = FCP_SCSI_DSC_SUCCESS;
		else
			pIoreq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
		delete pMasterContext;

		IoreqQueue.RemoveFromQueue(pIoreq);
		// do callback on pIoreq
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
}

/*************************************************************************/
// UpdateSGCombined
// Update Number of S/G Combinations in Performance record
/*************************************************************************/

void 	Raid::UpdateSGCombined(U8 Type, U8 Num)
{
	if (Type == RAID_READ)
		RaidPerformance.NumSGCombinedReads += Num;
	else
		RaidPerformance.NumSGCombinedWrites += Num;
}

/*************************************************************************/
// UpdatePerfReads
// Increment Number of reads in Performance record indexed roughly by 
// number of blocks, and increment blocks read by number of blocks
/*************************************************************************/

void 	Raid::UpdatePerfReads(U32 NumBlocks)
{
	if (NumBlocks > 1024)						// > 512k
		RaidPerformance.NumReads[11]++;
	else if (NumBlocks > 512)					// > 256k <= 512k
		RaidPerformance.NumReads[10]++;
	else if (NumBlocks > 256)					// > 128k <= 256k
		RaidPerformance.NumReads[9]++;
	else if (NumBlocks > 128)					// > 64k <= 128k
		RaidPerformance.NumReads[8]++;
	else if (NumBlocks > 64)					// > 32k <= 64k
		RaidPerformance.NumReads[7]++;
	else if (NumBlocks > 32)					// > 16k <= 32k
		RaidPerformance.NumReads[6]++;
	else if (NumBlocks > 16)				   	// > 8k <= 16k
		RaidPerformance.NumReads[5]++;
	else if (NumBlocks > 8)						// > 4k <= 8k
		RaidPerformance.NumReads[4]++;
	else if (NumBlocks > 4)						// > 2k <= 4k
		RaidPerformance.NumReads[3]++;
	else if (NumBlocks > 2)						// > 1k <= 2k
		RaidPerformance.NumReads[2]++;
	else if (NumBlocks > 1)						// = 1k
		RaidPerformance.NumReads[1]++;
	else
		RaidPerformance.NumReads[0]++;			// = 512

	RaidPerformance.NumBlocksRead += NumBlocks;
}

/*************************************************************************/
// UpdatePerfWrites
// Increment Number of writes in Performance record indexed roughly by 
// number of blocks, and update blocks written by number of blocks
/*************************************************************************/

void 	Raid::UpdatePerfWrites(U32 NumBlocks)
{
	if (NumBlocks > 1024)						// > 512k
		RaidPerformance.NumWrites[11]++;
	else if (NumBlocks > 512)					// > 256k <= 512k
		RaidPerformance.NumWrites[10]++;
	else if (NumBlocks > 256)					// > 128k <= 256k
		RaidPerformance.NumWrites[9]++;
	else if (NumBlocks > 128)					// > 64k <= 128k
		RaidPerformance.NumWrites[8]++;
	else if (NumBlocks > 64)					// > 32k <= 64k
		RaidPerformance.NumWrites[7]++;
	else if (NumBlocks > 32)					// > 16k <= 32k
		RaidPerformance.NumWrites[6]++;
	else if (NumBlocks > 16)				   	// > 8k <= 16k
		RaidPerformance.NumWrites[5]++;
	else if (NumBlocks > 8)						// > 4k <= 8k
		RaidPerformance.NumWrites[4]++;
	else if (NumBlocks > 4)						// > 2k <= 4k
		RaidPerformance.NumWrites[3]++;
	else if (NumBlocks > 2)						// > 1k <= 2k
		RaidPerformance.NumWrites[2]++;
	else if (NumBlocks > 1)						// = 1k
		RaidPerformance.NumWrites[1]++;
	else
		RaidPerformance.NumWrites[0]++;			// = 512

	RaidPerformance.NumBlocksWritten += NumBlocks;
}

/*************************************************************************/
// UpdateRaidReassignments
// Increment Number of Reassign Block commands that were sent to Raid DDM
/*************************************************************************/

void 	Raid::UpdateRaidReassignments(STATUS Status)
{
	if (Status == FCP_SCSI_DSC_SUCCESS)
		RaidStatus.NumRaidReassignedSuccess++;
	else
		RaidStatus.NumRaidReassignedFailed++;
}

/*************************************************************************/
// UpdateMemberReassignments
// Increment Number of Reassign Block commands that were sent to a member
/*************************************************************************/

void 	Raid::UpdateMemberReassignments(STATUS Status, U8 Mem)
{
	if (Status == FCP_SCSI_DSC_SUCCESS)
		RaidStatus.NumReassignedSuccess[Mem]++;
	else
		RaidStatus.NumReassignedFailed[Mem]++;
}

/*************************************************************************/
// UpdateNumRecoveredErrors
// Increment Number of Recovered Errors reported by member
/*************************************************************************/

void 	Raid::UpdateNumRecoveredErrors(U8 Mem)
{
	RaidStatus.NumRecoveredErrors[Mem]++;
}

/*************************************************************************/
// Quiesce
// 
/*************************************************************************/

void	Raid::Quiesce(Ioreq *pIoreq)
{

	SuspendAllUtilities();
	 
	// signal that we're waiting for Quiesce
	pQuiesceIor = pIoreq;
	// check if already quiesced
	CheckForQuiesced();
}

/*************************************************************************/
// CheckForQuiesced
// 
/*************************************************************************/

void	Raid::CheckForQuiesced()
{
	U8		index;

	if (IsRaidIdle())
	{	// if already Idle - do callback
		index = --pQuiesceIor->iCall;
		( (pQuiesceIor->Call[index].pInst)->*(pQuiesceIor->Call[index].pCallback) )(pQuiesceIor);
	}
}

/*************************************************************************/
// GetWaitMask
// Return mask of all members with max outstanding requests
/*************************************************************************/

U32	Raid::GetWaitMask()
{
	U32		WaitMask = 0, MemberMask = 1;
	Member	*pMem;

	for (int i = 0; i < NumberMembers; i++)
	{
		pMem = pMember[i];
		if (pMem->NumOutstanding >= pMem->MaxOutstanding)
			WaitMask |= MemberMask;
		MemberMask <<= 1;
	}
	return (WaitMask);
}

/*************************************************************************/
// IsRaidIdle
// Return TRUE if no Ioreqs are queued and utility queue is empty
/*************************************************************************/

BOOL	Raid::IsRaidIdle()
{
	if (UtilCmd.IsEmpty())
	{	// no utilities running
		if (IoreqQueue.IsEmpty())
		{	// and no I/O Requests outstanding
			return (TRUE);
		}
	}
	return (FALSE);
}

/*************************************************************************/
// SetMemberHealth
// Set Health of all members in mask to health passed
/*************************************************************************/

void	Raid::SetMemberHealth(U32 Mask, RAID_MEMBER_STATUS Health)
{
	U32 MemberMask = 1;

	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (Mask & MemberMask)
			pMember[i]->Health = Health;
		MemberMask <<= 1;
	}
}
												   
/*************************************************************************/
// GetReqblkSGList
// Get a SGList for pReq to handle the requested number of sectors 
// and copy from pIoreq. Adjust pIoreq for next pass
/*************************************************************************/

STATUS	Raid::GetReqblkSGList(Ioreq *pIoreq, Reqblk *pReq)
{
	SGLIST	*pSGListSrc, *pSGListDest;
	U32		TotalBytes, SGBytes, RemBytes;
	U8		SGCount;

	SGBytes = 0;
	SGCount = 0;
	TotalBytes = pReq->Count * 512;
	pSGListSrc = pIoreq->pCurSGList;

	// count SGList elements needed for Reqblk
	while (SGBytes < TotalBytes)
	{
		SGCount++;
		SGBytes += pSGListSrc->Length;
		pSGListSrc++;
	}

	// allocate a SGList for Reqblk
	pReq->pSGList = new SGLIST[SGCount];
	if (pReq->pSGList == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Set number of SGList elements in Reqblk
	pReq->NumSGLs = SGCount;

	// copy Ioreq SGList to Reqblk SGList
	pSGListSrc = pIoreq->pCurSGList;
	pSGListDest = pReq->pSGList;
	RemBytes = TotalBytes;
	for (int i = 0; i < (SGCount-1); i++)
	{
		pSGListDest->Address = pSGListSrc->Address;
		pSGListDest->Length = pSGListSrc->Length;
		RemBytes -= pSGListSrc->Length;
		pSGListSrc++;
		pSGListDest++;
	}

	// fill in last element
	pSGListDest->Address = pSGListSrc->Address;
	pSGListDest->Length = RemBytes;

	if (RemBytes == pSGListSrc->Length)		// used all bytes from this element
		pSGListSrc++;						// point to next SGList element
	else
	{
		// adjust last SGList element in Ioreq by remaining bytes
		pSGListSrc->Address += RemBytes;
		pSGListSrc->Length -= RemBytes;
	}
	pIoreq->pCurSGList = pSGListSrc;		// update position for next pass

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// FreeUtilReqblkChain
// Free all UtilReqblks chained together for pUtility
/*************************************************************************/

void	Raid::FreeUtilReqblkChain(Utility *pUtility)
{
	UtilReqblk *pReq, *pNext;

	pReq = pUtility->pUtilReq;	 	// head of chain of UtilReqblks for this Utility
	while (pReq)
	{
		pNext = (UtilReqblk *)pReq->pCombForw;
									// next Reqblk chained for this Ioreq
		delete pReq;				// delete this UtilReqblk
		pReq = pNext;				// continue with next
	}
}

/*************************************************************************/
// RemoveFromUtilityChain
// Take UtilReqblk off its Utility's chain of UtilReqblks.
// If it is the last UtilReqblk on its Utility's chain, then the Utility is done
/*************************************************************************/

BOOL	Raid::RemoveFromUtilityChain(UtilReqblk *pUtilReq)
{
	UtilReqblk	*pPrev, *pNext;
	Utility		*pUtility;

	pUtility = pUtilReq->pUtility;

	pPrev = (UtilReqblk *)pUtilReq->pCombBack;
	pNext = (UtilReqblk *)pUtilReq->pCombForw;

	if (pPrev)
		pPrev->pCombForw = pNext;
	else								// was head of chain
		pUtility->pUtilReq = pNext;		// set new head

	if (pNext)
		pNext->pCombBack = pPrev;

	return (pUtility->pUtilReq == NULL); // if TRUE, Utility is complete
}

/*************************************************************************/
// FreeReqblkChain
// Free all Reqblks chained together for pIoreq
/*************************************************************************/

void	Raid::FreeReqblkChain(Ioreq *pIoreq)
{
	Reqblk *pReq, *pNext;

	pReq = pIoreq->pReq;			// head of chain of Reqblks for this Ioreq
	while (pReq)
	{
		pNext = pReq->pCombForw;	// next Reqblk chained for this Ioreq
		FreeReqblk(pReq);			// deletes this Reqblk and its SGList
		pReq = pNext;				// continue with next
	}
}

/*************************************************************************/
// FreeReqblk
// Delete Reqblk and SGList if one exists
/*************************************************************************/

void	Raid::FreeReqblk(Reqblk *pReq)
{
	if (pReq->NumSGLs)
		delete []pReq->pSGList;		// delete the SGList
	delete pReq;					// delete the Reqblk
}

/*************************************************************************/
// RemoveFromIorChain
// Take Reqblk off its Ioreq's chain of Reqblks.
// If it is the last Reqblk on its Ioreq's chain, then the Ioreq is done
/*************************************************************************/

BOOL	Raid::RemoveFromIorChain(Reqblk *pReq)
{
	Reqblk	*pPrev, *pNext;
	Ioreq	*pIoreq;

	pIoreq = pReq->pIoreq;

	pPrev = pReq->pCombBack;
	pNext = pReq->pCombForw;

	if (pPrev)
		pPrev->pCombForw = pNext;
	else								// was head of chain
		pIoreq->pReq = pNext;			// set new head

	if (pNext)
		pNext->pCombBack = pPrev;

	return (pIoreq->pReq == NULL);		// if TRUE, Ioreq is complete
}

/*************************************************************************/
// ReqblkDone
// Reqblk is done. If it is a Master Reqblk, then all its chained Reqblks
// are done. Take each individual Reqblk off its Ioreq's chain of Reqblks.
// When the last Reqblk is done on an Ioreq's chain, then the Ioreq is
// done. This routine is recursive because a Master Reqblk may contain
// other Master Reqblks.
/*************************************************************************/

void	Raid::ReqblkDone(Reqblk *pReq)
{
	Reqblk	*pNext;
	Ioreq	*pIoreq;
	STATUS	Status;
	U8		index;

	Status = pReq->Status;
	while (pReq)
	{
		pNext = pReq->pForw;				// Next element of combined chain
		if (pReq->Flags & MASTER_BIT)
		{
			// Set Status from Master pReq
			pReq->pCombBack->Status = Status;
			ReqblkDone(pReq->pCombBack); 	// Head of combined chain
											// Call recursively, might be another Master
		}
		else
		{
			// Set Status - from itself or copied
			// through chain from Master Reqblk
			pReq->Status = Status;
			SetIoreqStatus(pReq);			// Set Ioreq status from pReq
			if (RemoveFromIorChain(pReq))	// Not a Master Req, so has a pIoreq
			{								// Ioreq is done. Do callback on Ioreq
				pIoreq = pReq->pIoreq;
				// Remove Ioreq from Queue of outstanding
				IoreqQueue.RemoveFromQueue(pIoreq);
				// Do callback
				index = --pIoreq->iCall;
				( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
			}
		}
		FreeReqblk(pReq);					// deletes this Reqblk and its SGList
		pReq = pNext;						// continue with next
	}
}

/*************************************************************************/	 
// CommandComplete
// 
/*************************************************************************/

void	Raid::CommandComplete(Reqblk *pReq)
{
	if (pReq->Status != OS_DETAIL_STATUS_SUCCESS)
		pRaidErr->ProcessError(pReq);	// error with this request
	else
	{
		// Call Callback routine for Reqblk
		( (pReq->pInst)->*(pReq->pCallback) )(pReq);
	}
}

/*************************************************************************/	 
// SetIoreqStatus
// Set Ioreq status from pReq
/*************************************************************************/

void	Raid::SetIoreqStatus(Reqblk *pReq)
{
	Ioreq	*pIoreq;
	 
	pIoreq = pReq->pIoreq;
	if (pIoreq)
	{
		if (pIoreq->Status == FCP_SCSI_DSC_SUCCESS)
		{
			pIoreq->Status = pReq->Status;
			if ((pIoreq->Status & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION)
			{
				pIoreq->SenseKey = pReq->SenseKey;
				pIoreq->ErrorLba = ConvertReqLbaToIoreqLba(pReq);
			}
		}
	}
}

/*************************************************************************/	 
// SetReqStatus
// Set Reqblk status from pMsg
/*************************************************************************/

void	Raid::SetReqStatus(Reqblk *pReq, MessageReply *pMsg, U8 Mem)
{
	CNVTR				cnvtr;
	BSA_REPLY_PAYLOAD	*pPayload;
	 
	if (pReq->Status == FCP_SCSI_DSC_SUCCESS)
	{
		pReq->Status = pMsg->DetailedStatusCode;
		if (pReq->Status != FCP_SCSI_DSC_SUCCESS)
		{
			pReq->ErrorMember = Mem;
			if ((pReq->Status & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION)
			{
				pPayload = (BSA_REPLY_PAYLOAD *) pMsg->GetPPayload();
				pReq->SenseKey = pPayload->SenseData[2] & 0x0f;
				pReq->Flags &= ~RAID_WRITE_BIT;
				if (((pMsg->reqCode == BSA_BLOCK_WRITE) ||
					 (pMsg->reqCode == BSA_BLOCK_WRITE_VERIFY)) &&
					((pPayload->SenseData[0] & 0x01) == 0))
				{
					// error on a write and not a deferred error
					// so we have the data to redo the write command
					pReq->Flags |= RAID_WRITE_BIT;
				}
				switch (pReq->SenseKey)
				{
					case SENSE_NO_SENSE:
						pReq->Status = FCP_SCSI_DSC_SUCCESS;
						break;
					case SENSE_RECOVERED_ERROR:
						UpdateNumRecoveredErrors(Mem);
						pReq->Status = FCP_SCSI_DSC_SUCCESS;
						break;
					case SENSE_MEDIUM_ERROR:
						cnvtr.charval[0] = pPayload->SenseData[3];
						cnvtr.charval[1] = pPayload->SenseData[4];
						cnvtr.charval[2] = pPayload->SenseData[5];
						cnvtr.charval[3] = pPayload->SenseData[6];
						pReq->ErrorLba = cnvtr.ulngval;
						break;
				}
			}
		}
	}
}

/*************************************************************************/
// ReadMDerrBlock
// Reread block command for MediaError processing
/*************************************************************************/

BOOL	Raid::ReadMDerrBlock(ErrReqblk *pErrReq)
{
	Message			*pMsg;
	Member			*pMem;
	SGLIST			*pSGList;
	RAID_CONTEXT	*pRContext;

	pMsg = new Message(BSA_BLOCK_READ, sizeof(FCP_MSG_SIZE));
	if (pMsg == NULL)
		return (FALSE);

	pMem = pMember[pErrReq->Member];

	pRContext = new RAID_CONTEXT;
	pRContext->pErrReq = pErrReq;

	pRContext->payload.ControlFlags = 0;
	pRContext->payload.TimeMultiplier = 0;
	pRContext->payload.FetchAhead = 0;
	pRContext->payload.TransferByteCount = pErrReq->Count << 9;
	pRContext->payload.LogicalBlockAddress = pErrReq->Lba;

	pMsg->AddPayload(&pRContext->payload, sizeof(BSA_RW_PAYLOAD));

	pSGList = pErrReq->pSGList;
	for (int i = 0; i < pErrReq->NumSGLs; i++)
	{
		pMsg->AddSgl(i, (void *)pSGList->Address, pSGList->Length, SGL_REPLY);
		pSGList++;
	}
	
	pMem->NumOutstanding++;


	DdmServices::Send(pMem->Vd, pMsg, pRContext, (ReplyCallback) &Raid::ReadMDerrCallback);

	return (TRUE);
}

/*************************************************************************/
// ReadMDerrCallback
// Callback for ReadMDerrBlock
/*************************************************************************/

void	Raid::ReadMDerrCallback(MessageReply *pMsg)
{ 
	RAID_CONTEXT	*pRContext;
	ErrReqblk		*pErrReq;
	Member			*pMem;
	U8				Mem;

	pRContext = (RAID_CONTEXT *)pMsg->GetContext();
	pErrReq = pRContext->pErrReq;
	Mem = pErrReq->Member;
	pMem = pMember[Mem];
	pMem->NumOutstanding--;

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid::ReadMDerrCallback) == TRUE)
			return;		// Retrying message
	}
	SetReqStatus(pErrReq, pMsg, pErrReq->Member);
	// delete Message and Context
	delete pMsg;
	delete pRContext;
	// do callback to Error Processing code
	( (pErrReq->pErrInst)->*(pErrReq->pErrCallback) )(pErrReq);

}					
	
/*************************************************************************/
// ErrReassignBlock
// Used for Error Recovery of Media Defect Error
/*************************************************************************/

BOOL	Raid::ErrReassignBlock(ErrReqblk *pErrReq)
{
	Message			*pMsg;
	Member			*pMem;
	RAID_CONTEXT	*pRContext;

	pMsg = new Message(BSA_BLOCK_REASSIGN, sizeof(FCP_MSG_SIZE));
	if (pMsg == NULL)
		return (FALSE);

	pMem = pMember[pErrReq->Member];

	pRContext = new RAID_CONTEXT;
	pRContext->pErrReq = pErrReq;

	pRContext->payload.ControlFlags = 0;
	pRContext->payload.TimeMultiplier = 0;
	pRContext->payload.FetchAhead = 0;
	pRContext->payload.TransferByteCount = 8;
	pRContext->payload.LogicalBlockAddress = pErrReq->Lba;

	pMsg->AddPayload(&pRContext->payload, sizeof(BSA_RW_PAYLOAD));

	pMem->NumOutstanding++;

	DdmServices::Send(pMem->Vd, pMsg, pRContext, (ReplyCallback) &Raid::ErrReassignBlockCallback);

	return (TRUE);
}

/*************************************************************************/
// ErrReassignBlockCallback
// Callback from Reassign Block command
/*************************************************************************/

void	Raid::ErrReassignBlockCallback(MessageReply *pMsg)
{ 
	RAID_CONTEXT	*pRContext;
	ErrReqblk		*pErrReq;
	Member			*pMem;

	pRContext = (RAID_CONTEXT *)pMsg->GetContext();
	pErrReq = pRContext->pErrReq;

	pMem = pMember[pErrReq->Member];
  
	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		// Reassignment failed - retry
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid::ErrReassignBlockCallback) == TRUE)
			return;		// Retrying message
	}
	pMem->NumOutstanding--;
	SetReqStatus(pErrReq, pMsg, pErrReq->Member);
	UpdateMemberReassignments(pMsg->DetailedStatusCode, pErrReq->Member);
	// delete Message and Context
	delete pMsg;
	delete pRContext;
	// do callback to Error Processing code
	( (pErrReq->pErrInst)->*(pErrReq->pErrCallback) )(pErrReq);
}

/*************************************************************************/
// WriteVerify
// Used for Error Recovery of Media Defect Error
/*************************************************************************/

BOOL	Raid::WriteVerify(ErrReqblk *pErrReq)
{
	Message			*pMsg;
	Member			*pMem;
	SGLIST			*pSGList;
	RAID_CONTEXT	*pRContext;

	pMsg = new Message(BSA_BLOCK_WRITE_VERIFY, sizeof(FCP_MSG_SIZE));
	if (pMsg == NULL)
		return (FALSE);

	pMem = pMember[pErrReq->Member];

	pRContext = new RAID_CONTEXT;
	pRContext->pErrReq = pErrReq;

	pRContext->payload.ControlFlags = 0;
	pRContext->payload.TimeMultiplier = 0;
	pRContext->payload.FetchAhead = 0;
	pRContext->payload.TransferByteCount = pErrReq->Count << 9;
	pRContext->payload.LogicalBlockAddress = pErrReq->Lba;

	pMsg->AddPayload(&pRContext->payload, sizeof(BSA_RW_PAYLOAD));

	pSGList = pErrReq->pSGList;
	for (int i = 0; i < pErrReq->NumSGLs; i++)
	{
		pMsg->AddSgl(i, (void *)pSGList->Address, pSGList->Length, SGL_SEND);
		pSGList++;
	}
	
	pMem->NumOutstanding++;

	DdmServices::Send(pMem->Vd, pMsg, pRContext, (ReplyCallback) &Raid::WriteVerifyCallback);

	return (TRUE);
}

/*************************************************************************/
// WriteVerifyCallback
// Callback from Write Verify command
/*************************************************************************/

void	Raid::WriteVerifyCallback(MessageReply *pMsg)
{ 
	RAID_CONTEXT	*pRContext;
	ErrReqblk		*pErrReq;
	Member			*pMem;

	pRContext = (RAID_CONTEXT *)pMsg->GetContext();
	pErrReq = pRContext->pErrReq;

	pMem = pMember[pErrReq->Member];
  
	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		// write verify failed - retry
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid::WriteVerifyCallback) == TRUE)
			return;		// Retrying message
	}
	pMem->NumOutstanding--;
	SetReqStatus(pErrReq, pMsg, pErrReq->Member);
	// delete Message and Context
	delete pMsg;
	delete pRContext;
	// do callback to Error Processing code
	( (pErrReq->pErrInst)->*(pErrReq->pErrCallback) )(pErrReq);
}

/*************************************************************************/	 
// RetryCommand
// Fill in Message payload again and send it.
/*************************************************************************/

BOOL	Raid::RetryCommand(Member *pMem, MessageReply *pMsg, ReplyCallback rc)
{ 
	RAID_CONTEXT	*pRContext;
	Reqblk  		*pErrReq;

	pRContext = (RAID_CONTEXT *)pMsg->GetContext();
	pErrReq = pRContext->pErrReq;

	if (pErrReq->RetryCount < pMem->MaxRetryCount)
	{	
		pErrReq->RetryCount++;
		// fill payload again
		pMsg->AddPayload(&pRContext->payload, sizeof(BSA_RW_PAYLOAD));
		// send the message
		DdmServices::Send(pMem->Vd, pMsg, pRContext, rc);
		return (TRUE);	// message sent
	}
	return (FALSE);		// exhausted retries
}

