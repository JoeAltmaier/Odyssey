/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raid1.cpp
//
// Description:	Raid1 class
//
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "Ddm.h"
#include "RaidStructs.h"
#include "RaidQ.h"
#include "UtilQ.h"
#include "Member.h"
#include "UtilCmd.h"
#include "RaidIorQ.h"
#include "RaidLock.h"
#include "RaidErr.h"
#include "RaidUtilTable.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "Raid.h"
#include "Raid1.h"
#include "OsStatus.h"


/*************************************************************************/
// ~Raid1
// Destructor method for the class Raid1
/*************************************************************************/

Raid1::~Raid1()
{
	if (pCmdServer)
		pCmdServer->csrvTerminate(); 
	if (pRequestQueue)
		delete pRequestQueue;
	if (pUtilQueue)
		delete pUtilQueue;
	if (pRaidLock)
		delete pRaidLock;
	if (pRaidErr)
		delete pRaidErr;

	for (int i = 0; i < NumberMembers; i++)
	{
		if (pMember[i])
			delete pMember[i];
	}
}

/*************************************************************************/
// Initialize
// Initialize method for the class Raid1
/*************************************************************************/

STATUS	Raid1::Initialize(DdmServices *pDdmServices,
								RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq)
{ 
	RAID_ROW_RECORD		*pRaidRow;
	STATUS				Status;

	TRACE_ENTRY(Raid1::Initialize);

	SetParentDdm(pDdmServices);
	pArrayDescriptor = pArrayDesc;		// save for callbacks during initialization
	RaidVDN = pDdmServices->GetVdn();
	RaidRowId = pArrayDesc->thisRID;
	DataBlockSize = pArrayDesc->dataBlockSize;
	WaitMask = 0;
	Health = pArrayDesc->health;
	pQuiesceIor = NULL;
	NumberMembers = pArrayDesc->numberMembers;

	// Allocate a Request Queue in Raid class. All I/O will be Queued in same Queue
	if ((pRequestQueue = new ReqQueue(this, RAIDQ_ELEVATOR)) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Allocate a Utility Queue
	if ((pUtilQueue = new UtilQueue(RAIDQ_FIFO)) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Allocate a range locking class
	if ((pRaidLock = new RaidLock) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Allocate an error processing class
	if ((pRaidErr = new (tZERO) RaidErr) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// Initialize the error processing class
	if ((Status = pRaidErr->Initialize(this)) != OS_DETAIL_STATUS_SUCCESS)
			return (Status);

	pIoreq->Status = OS_DETAIL_STATUS_SUCCESS;
	pIoreq->Count = 0;			// use to count number of table reads done
	for (U8 i = 0; i < NumberMembers; i++)
	{
		pRaidRow = new (tZERO | tUNCACHED) RAID_ROW_RECORD;
		// set row id in member table to read
		pRaidRow->RowData.MemberData.thisRID = pArrayDesc->members[i];
		// set member index for callback routine
		pRaidRow->Index = i;
		// save init ioreq
		pRaidRow->pInitIor = pIoreq;
		// read member table row from PTS
		ReadMemberTableRow(pRaidRow,(pTSCallback_t)&Raid1::ReadMemberTableDone);
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReadMemberTableDone
// Callback method from reading a member table row.
// New a member class and initilize it from this data
/*************************************************************************/

void	Raid1::ReadMemberTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{
	Ioreq	*pIoreq;
	U8		i, index;

	pIoreq = pRaidRow->pInitIor;
 	i = pRaidRow->Index;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
		pIoreq->Status = OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;		// set error

	// Allocate a member class for this member in array
	else if ((pMember[i] = new Member) == NULL)
		pIoreq->Status = OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	else
	{
		// Initialize member class. Does not allocate a Request Queue in member class
		if ((Status = pMember[i]->Initialize(&(pRaidRow->RowData.MemberData)))
												!= OS_DETAIL_STATUS_SUCCESS)
			pIoreq->Status = Status;		// set error
	}

	pIoreq->Count++;						// update count of number reads done
	if (pIoreq->Count == NumberMembers)		// done with all members
	{
		if (pIoreq->Status != OS_DETAIL_STATUS_SUCCESS)
		{	// unable to read member configuration data,
			// do callback - DDM is not enabled and should 
			// not receive any commands
			index = --pIoreq->iCall;
			( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
		}
		else
		{
			InitCmdServer();
			// continue by reading utility table rows
			CheckUtilitiesToStart(pIoreq);
		}
	}
	delete pRaidRow;
}

/*************************************************************************/
// CheckUtilitiesToStart
// Read rows from Utility Table to resume utilities which were running or 
// suspended, and to set the RegeneratedLBA for any regenerating members
/*************************************************************************/

STATUS	Raid1::CheckUtilitiesToStart(Ioreq *pIoreq)
{
	RAID_ROW_RECORD		*pRaidRow;
	U8					i, index;

	pIoreq->Status = OS_DETAIL_STATUS_SUCCESS;
	pIoreq->Count = 0;			// use to count number of table reads done
	if (pArrayDescriptor->numberUtilities == 0)
	{
		// do callback on pIoreq
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
	else
	{
		for (i = 0; i < pArrayDescriptor->numberUtilities; i++)
		{
			pRaidRow = new (tZERO | tUNCACHED) RAID_ROW_RECORD;
			// set row id in utility table to read
			pRaidRow->RowData.UtilityData.thisRID = pArrayDescriptor->utilities[i];
			// save init ioreq
			pRaidRow->pInitIor = pIoreq;
			// read utility table row from PTS
			ReadUtilityTableRow(pRaidRow, (pTSCallback_t)&Raid1::CheckUtilitiesCallback);
		}
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// CheckUtilitiesCallback
// Callback method from reading a utility table row.
// If any are regenerates, set the RegeneratedLBA in appropriate members.
// Start any utilities which were running or suspended
/*************************************************************************/

void	Raid1::CheckUtilitiesCallback(RAID_ROW_RECORD *pRaidRow, STATUS Status)
{
	RAID_ARRAY_UTILITY	*pArrayUtility;
	Ioreq				*pIoreq;
	rowID				RowId;
	U8					i, j, index;

	pIoreq = pRaidRow->pInitIor;

	if (Status != OS_DETAIL_STATUS_SUCCESS)
		pIoreq->Status = OS_DETAIL_STATUS_DEVICE_NOT_AVAILABLE;		// set error
	else
	{
		pArrayUtility = &pRaidRow->RowData.UtilityData;
		switch (pArrayUtility->status)
		{
			case RAID_UTIL_RUNNING:
			case RAID_UTIL_SUSPENDED:
			{
				switch (pArrayUtility->utilityCode)
				{
					case RAID_UTIL_LUN_HOTCOPY:
					case RAID_UTIL_MEMBER_HOTCOPY:
					case RAID_UTIL_REGENERATE:
					{
						for (i = 0; i < MAX_ARRAY_MEMBERS; i++)
						{
							RowId = pArrayUtility->destinationRowIds[i];
							for (j = 0; j < NumberMembers; j++)
							{
								if ((pMember[j]->MemberRowId.LoPart == RowId.LoPart) &&
										(pMember[j]->MemberRowId.HiPart == RowId.HiPart))
									pMember[j]->RegeneratedLBA = pArrayUtility->currentLBA;
							}
						}
					}
				}
				Raid::StartUtility(pArrayUtility);
			}
		}
	}

	pIoreq->Count++;										// update count of number reads done
	if (pIoreq->Count == pArrayDescriptor->numberUtilities)	// done with all utilities
	{
		// do callback on pIoreq
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
	delete pRaidRow;
}

/*************************************************************************/
// DoRead
// Read method for the class Raid1
/*************************************************************************/

STATUS	Raid1::DoRead(Ioreq *pIoreq)
{
	Reqblk	*pReq;
	STATUS	Status;

	TRACE_ENTRY(Raid1::DoRead);

	pReq = new (tZERO) Reqblk;
	if (pReq == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	pReq->Status = 0;
	pReq->Flags = 0;
	pReq->Type = RAID_READ;

	// put on Ioreq's chain of Reblks
	pIoreq->pReq = pReq;
	pReq->pIoreq = pIoreq;
	pReq->pCombForw = NULL;
	pReq->pCombBack = NULL;

	pReq->Lba = pIoreq->Lba;
	pReq->Count = pIoreq->Count;
	pReq->Member = 0;

	pReq->pInst = this;
	pReq->pStartRoutine = (pReqStartMethod)&Raid1::MirroredRead;
	pReq->pCallback = (pReqCallbackMethod)&Raid1::ReqIODone;

	if ((Status = GetReqblkSGList(pIoreq, pReq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return (Status);
	}
	
	IoreqQueue.InsertInQueue(pIoreq);

	Raid1::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoWrite
// Write method for the class Raid1
/*************************************************************************/

STATUS	Raid1::DoWrite(Ioreq *pIoreq)
{
	Reqblk	*pReq;
	STATUS	Status;

	TRACE_ENTRY(Raid1::DoWrite);

	pReq = new (tZERO) Reqblk;
	if (pReq == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	pReq->Status = 0;
	pReq->Flags = 0;
	pReq->Type = RAID_WRITE;
	if (pIoreq->Flags & WRITE_VERIFY_BIT)
		pReq->Flags |= WRITE_VERIFY_BIT;

	// put on Ioreq's chain of Reblks
	pIoreq->pReq = pReq;
	pReq->pIoreq = pIoreq;
	pReq->pCombForw = NULL;
	pReq->pCombBack = NULL;

	pReq->Lba = pIoreq->Lba;
	pReq->Count = pIoreq->Count;
	pReq->Member = 0;
	pReq->pInst = this;
	pReq->pStartRoutine = (pReqStartMethod)&Raid1::MirroredWrite;
	pReq->pCallback = (pReqCallbackMethod)&Raid1::ReqIODone;

	if ((Status = GetReqblkSGList(pIoreq, pReq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return (Status);
	}

	IoreqQueue.InsertInQueue(pIoreq);

	Raid1::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoReassign
// Reassign method for the class Raid1
/*************************************************************************/

STATUS	Raid1::DoReassign(Ioreq *pIoreq)
{
	Reqblk	*pReq;

	TRACE_ENTRY(Raid1::DoReassign);

	pReq = new (tZERO) Reqblk;
	if (pReq == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	pReq->Status = 0;
	pReq->Flags = 0;
	pReq->Type = RAID_REASSIGN;

	// put on Ioreq's chain of Reblks
	pIoreq->pReq = pReq;
	pReq->pIoreq = pIoreq;
	pReq->pCombForw = NULL;
	pReq->pCombBack = NULL;

	// set Lba
	pReq->Lba = pIoreq->Lba;
	pReq->Count = pIoreq->Count;
	// send to all Up members
	pReq->Member = 0;

	// don't need SGL
	pReq->NumSGLs = 0;

	pReq->pInst = this;
	pReq->pStartRoutine = (pReqStartMethod)&Raid1::ReassignBlock;
	pReq->pCallback = (pReqCallbackMethod)&Raid1::ReqIODone;

	IoreqQueue.InsertInQueue(pIoreq);

	Raid1::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoVerify
// Verify method for the class Raid1
/*************************************************************************/

STATUS	Raid1::DoVerify(Utility *pUtility)
{

	TRACE_ENTRY(Raid1::DoVerify);

	if (UtilCmd.FindUtilityInQueue(pUtility->Handle) != NULL)
	{	// Utility is already running
		return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
	}

	return (StartUtility(pUtility, 128, 2));
}

/*************************************************************************/
// DoRegenerate
// Regenerate method for the class Raid1
/*************************************************************************/

STATUS	Raid1::DoRegenerate(Utility *pUtility)
{

	TRACE_ENTRY(Raid1::DoRegenerate);

	if (UtilCmd.FindUtilityInQueue(pUtility->Handle) != NULL)
	{	// Utility is already running
		return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
	}

	return (StartUtility(pUtility, 128, 2));
}

/*************************************************************************/
// DoBkgdInit
// Background Initialize method for the class Raid1
/*************************************************************************/

STATUS Raid1::DoBkgdInit(Utility *pUtility)
{

	TRACE_ENTRY(Raid1::DoBkgdInit);

	if (UtilCmd.FindUtilityInQueue(pUtility->Handle) != NULL)
	{	// Utility is already running
		return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
	}

	return (StartUtility(pUtility, 128, 2));
}

/*************************************************************************/
// DoReplaceMember
// Replace a failed member with a spare
/*************************************************************************/

STATUS	Raid1::DoReplaceMember(RaidRequest *pRaidRequest)
{
	STATUS		Status;
	rowID		RowId;
	Member		*pNewMem, *pOldMem;
	U8			MemberIndex;

	RowId = pRaidRequest->Data.ReplaceData.OldMemberRowID;
	pOldMem = NULL;
	// Find the member to be replaced
	for (U8 i = 0; i < NumberMembers; i++)
	{
		if ((pMember[i]->MemberRowId.LoPart == RowId.LoPart) &&
			(pMember[i]->MemberRowId.HiPart == RowId.HiPart))
		{
			pOldMem = pMember[i];		// Found
			MemberIndex = i;
			break;
		}
	}
	if (pOldMem == NULL)
		return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);		// Not found

	// check that member to be replaced is down
	if (pOldMem->Health != RAID_STATUS_DOWN)
		return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);

	// check that replacement member is down
	if (pRaidRequest->Data.ReplaceData.Member.memberHealth != RAID_STATUS_DOWN)
		return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);

	if ((pNewMem = new Member) == NULL)
		return (RAIDCMD_STATUS_INSUFFICIENT_RESOURCE);

	// Initialize member class. Does not allocate a Request Queue in member class
	if ((Status = pNewMem->Initialize(&(pRaidRequest->Data.ReplaceData.Member)))
												!= OS_DETAIL_STATUS_SUCCESS)
	{
		delete pNewMem;
		if (Status == OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT)
			return (RAIDCMD_STATUS_INSUFFICIENT_RESOURCE);
		else
			return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);
	}
	// Replace old member
	pMember[MemberIndex] = pNewMem;

	// delete old member
	delete pOldMem;
	
	return RAIDCMD_STATUS_SUCCESS;
}

/*************************************************************************/
// DoAddMember
// Add another member to be mirrored
/*************************************************************************/

STATUS	Raid1::DoAddMember(RaidRequest *pRaidRequest)
{
	STATUS		Status;
	Member		*pNewMem;
	U8			MemberIndex;

	if (NumberMembers == MAX_ARRAY_MEMBERS)
		return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);
	// check member index of member to add - only add at end
	MemberIndex = NumberMembers;
	if (pRaidRequest->Data.AddData.Member.memberIndex != MemberIndex)
		return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);

	// check that member to be added is down
	if (pRaidRequest->Data.AddData.Member.memberHealth != RAID_STATUS_DOWN)
		return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);

	if ((pNewMem = new Member) == NULL)
		return (RAIDCMD_STATUS_INSUFFICIENT_RESOURCE);

	// Initialize member class. Does not allocate a Request Queue in member class
	if ((Status = pNewMem->Initialize(&(pRaidRequest->Data.AddData.Member)))
												!= OS_DETAIL_STATUS_SUCCESS)
	{
		delete pNewMem;
		if (Status == OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT)
			return (RAIDCMD_STATUS_INSUFFICIENT_RESOURCE);
		else
			return (RAIDCMD_STATUS_INAPPROPRIATE_CMD);
	}
	// Add new member
	pMember[MemberIndex] = pNewMem;

	// Increment count of members
	NumberMembers++;
	
	return RAIDCMD_STATUS_SUCCESS;
}

/*************************************************************************/
// StartUtility
// Start a utility
/*************************************************************************/

STATUS	Raid1::StartUtility(Utility *pUtility, U32 Count, U8 NumConcurrent)
{
	UtilReqblk	*pUtilReq, *pPrev, *pNext;
	RaidEvent	Event;

	TRACE_ENTRY(Raid1::StartUtility);

	// Convert UpdateRate and PercentUpdateRate
	// from a percentage to number of blocks
	pUtility->UpdateRate = (pUtility->EndLBA / 100) * pUtility->UpdateRate;
	pUtility->PercentUpdateRate = (pUtility->EndLBA / 100) * pUtility->PercentUpdateRate;
	// Convert UpdateRate and PercentUpdateRate
	// from number of blocks to number of passes
	if (pUtility->UpdateRate != 0)
	{
		pUtility->UpdateRate /= Count;
		if (pUtility->UpdateRate == 0)
			pUtility->UpdateRate++;
	}
	if (pUtility->PercentUpdateRate != 0)
	{
		pUtility->PercentUpdateRate /= Count;
		if (pUtility->PercentUpdateRate == 0)
			pUtility->PercentUpdateRate++;
	}

	pUtility->pUtilReq = NULL;

	// Setup UtilReqs for as many as will run concurrently
	for (U8 i = 0; i < NumConcurrent; i++)
	{
		pUtilReq = new (tZERO) UtilReqblk;
		if (pUtilReq == NULL)
		{
			FreeUtilReqblkChain(pUtility);
			pUtility->pUtilReq = NULL;
			return RAIDCMD_STATUS_INSUFFICIENT_RESOURCE;
		}

		pUtilReq->Status = FCP_SCSI_DSC_SUCCESS;
		pUtilReq->Flags = RAID_UTILITY_BIT;

		// put on Utility's chain of UtilReblks
		pUtilReq->pUtility = pUtility;
		pUtilReq->pCombForw = NULL;
		if (pUtility->pUtilReq)
		{
			pPrev = pUtility->pUtilReq;
			pNext = (UtilReqblk *)pPrev->pCombForw;
			while (pNext)
			{
				pPrev = pNext;
				pNext = (UtilReqblk *)pNext->pCombForw;
			}
			pUtilReq->pCombBack = pPrev;
			pPrev->pCombForw = pUtilReq;
		}
		else
		{	// head of chain
			pUtility->pUtilReq = pUtilReq;
			pUtilReq->pCombBack = NULL;
		}

		pUtilReq->Priority = pUtility->Priority;
		pUtilReq->SourceMask = pUtility->SourceMask;
		pUtilReq->DestMask = pUtility->DestMask;

		pUtilReq->Lba = pUtility->CurrentLBA;
		pUtility->CurrentLBA += Count;
		pUtilReq->Count = Count; 
		pUtilReq->Member = 0;
		pUtilReq->pInst = this;

		switch (pUtility->Cmd)
		{
			case RAID_UTIL_LUN_HOTCOPY:
			case RAID_UTIL_MEMBER_HOTCOPY:
			case RAID_UTIL_REGENERATE:
				pUtilReq->Type = RAID_REGENERATE_CMD;
				pUtilReq->pStartRoutine = (pReqStartMethod)&Raid1::Regenerate;
				break;
			case RAID_UTIL_VERIFY:
				pUtilReq->Type = RAID_VERIFY_CMD;
				pUtilReq->pStartRoutine = (pReqStartMethod)&Raid1::Verify;
				break;
			case RAID_UTIL_BKGD_INIT:
				pUtilReq->Type = RAID_BKGDINIT_CMD;
				// for Raid1 same method as regenerate
				pUtilReq->pStartRoutine = (pReqStartMethod)&Raid1::Regenerate;
				break;
		}
		pUtilReq->pCallback = (pReqCallbackMethod)&Raid1::ContinueUtility;

//		if (!CanUtilityRun(pUtilReq))
//		{
//			FreeUtilReqblkChain(pUtility);
//			pUtility->pUtilReq = NULL;
//			return RAIDCMD_STATUS_INAPPROPRIATE_CMD;
//		}
	}

	pUtility->CurrentLBA -= Count;

	if (pUtilReq->Type == RAID_REGENERATE_CMD)
	{
		// set pMem(s)->RegeneratedLBA
		UpdateRegeneratedLba(pUtilReq);
		// set Health of pMem(s) to Regenerating
		SetMemberHealth(pUtility->DestMask, RAID_STATUS_REGENERATING);
	}

	pUtility->Status = RAID_UTIL_RUNNING;
	// Put utility on chain
	UtilCmd.InsertInQueue(pUtility);

  	// Update PTS - utility running
	UpdateUtilityStatusInPTS(pUtility, (pTSCallback_t)&Raid::UpdateUtilityDone);

	// Report Event Utility started
	Event.Event.StartUtil.UtilRowID = pUtility->Handle;
	pCmdServer->csrvReportEvent(RAID_EVT_UTIL_STARTED, &Event);

	// Put requests on utility queue
	pUtilReq = pUtility->pUtilReq;
	while (pUtilReq)
	{
		pNext = (UtilReqblk *)pUtilReq->pCombForw;
		pUtilQueue->EnQueue(pUtilReq);
		pUtilReq = pNext;
	}

	// Try to start I/O
	Raid1::StartIO();

	return RAIDCMD_STATUS_SUCCESS;
}

/*************************************************************************/
// ContinueUtility
// Continue a utility
/*************************************************************************/

void	Raid1::ContinueUtility(UtilReqblk *pUtilReq)
{
	Utility		*pUtility;
	RaidEvent	Event;

	TRACE_ENTRY(Raid1::ContinueUtility);

	pUtility = pUtilReq->pUtility;

	if (pUtility->Flags & RAID_ABORT_UTILITY)
		pUtility->Status = RAID_UTIL_ABORTED;
	else if (pUtility->Flags & RAID_SUSPEND_UTILITY)
		pUtility->Status = RAID_UTIL_SUSPENDED;
	else if (pUtilReq->Status != FCP_SCSI_DSC_SUCCESS)
		pUtility->Status = RAID_UTIL_ABORTED_IOERROR;
	else if (!CanUtilityRun(pUtilReq))
		pUtility->Status = RAID_UTIL_ABORTED_IOERROR;

	if (pUtility->Status != RAID_UTIL_RUNNING)
	{
		// Utility ends in error
		if (RemoveFromUtilityChain(pUtilReq))
		{	// all UtilReqs have finished
			if (pUtilReq->Type == RAID_REGENERATE_CMD)
				SetMemberHealth(pUtility->DestMask, RAID_STATUS_DOWN);

			EndUtility(pUtility);
		}
		// delete UtilReq
		delete pUtilReq;
		return;
	}
	pUtility->CurrentLBA += pUtilReq->Count;
	if (pUtility->CurrentLBA + pUtilReq->Count > pUtility->EndLBA)
	{
		if (pUtility->CurrentLBA < pUtility->EndLBA)
		{	// do remainder
			pUtilReq->Count = pUtility->EndLBA - pUtility->CurrentLBA;
		}
		else
		{
			// Utility ends successfully
			if (RemoveFromUtilityChain(pUtilReq))
			{	// all UtilReqs have finished
				if (pUtilReq->Type == RAID_REGENERATE_CMD)
					SetMemberHealth(pUtility->DestMask, RAID_STATUS_UP);

				pUtility->Status = RAID_UTIL_COMPLETE;
				EndUtility(pUtility);
			}

			// delete UtilReq
			delete pUtilReq;
			return;
		}
	}

	// Increment pass number and check if it's time
	// to update progress in PTS.
	// An update rate of 0 will not update progress.
	pUtility->PassNo++;
	if (pUtility->PassNo == pUtility->UpdateRate)
	{	// Time to update progress in PTS
		UpdateUtilityProgressInPTS(pUtility, (pTSCallback_t)&Raid::UpdateUtilityDone);
		pUtility->PassNo = 0;		// reset
	}
		
	// Increment percent pass number and check if it's time
	// to post a percent complete event.
	// An update rate of 0 will not post events.
	pUtility->PercentPassNo++;
	if (pUtility->PercentPassNo == pUtility->PercentUpdateRate)
	{	// Time to post percent complete event
		Event.Event.PercentUtil.UtilRowID = pUtility->Handle;
		Event.Event.PercentUtil.Percent = 0;
		if (pUtility->EndLBA != 0)
		{	// set percent complete
			Event.Event.PercentUtil.Percent = (pUtility->CurrentLBA * 100) / pUtility->EndLBA;
		}
		pCmdServer->csrvReportEvent(RAID_EVT_UTIL_PERCENT_COMPLETE, &Event);
		pUtility->PercentPassNo = 0;		// reset
	}
		
	pUtilReq->Status = FCP_SCSI_DSC_SUCCESS;
	pUtilReq->Flags = RAID_UTILITY_BIT;
	pUtilReq->Lba = pUtility->CurrentLBA;
	// Reset Priority
	pUtilReq->Priority = pUtility->Priority;
	// Reset masks - member may have failed,
	// but policy is to continue with members remaining
	pUtilReq->SourceMask = pUtility->SourceMask;
	pUtilReq->DestMask = pUtility->DestMask;

	// Put request on utility queue
	pUtilQueue->EnQueue(pUtilReq);

	// Try to start I/O
	Raid1::StartIO();
}

/*************************************************************************/
// EndUtility
// Utility is finished - Update utility status in PTS and
// continue at EndUtilityCallback
/*************************************************************************/

void	Raid1::EndUtility(Utility *pUtility)
{
	// Update PTS - utility stopped
	UpdateUtilityStatusInPTS(pUtility, (pTSCallback_t)&Raid1::EndUtilityCallback);
}

/*************************************************************************/
// EndUtilityCallback
// Utility status updated in PTS
/*************************************************************************/

void	Raid1::EndUtilityCallback(Utility *pUtility, STATUS Status)
{
#pragma unused (Status)
	RaidEvent	Event;

	// Remove utility from chain of running utilities
	UtilCmd.RemoveFromQueue(pUtility);

	// Report Event Utility stopped
	Event.Event.StopUtil.UtilRowID = pUtility->Handle;
	Event.Event.StopUtil.Reason = pUtility->Status;		// complete, aborted, error
	Event.Event.StopUtil.MiscompareCnt = pUtility->ErrorCount;
	pCmdServer->csrvReportEvent(RAID_EVT_UTIL_STOPPED, &Event);

	// delete Utility
	delete pUtility;
	if (pQuiesceIor)	// waiting for everything to finish?
		CheckForQuiesced();
	return;
}

/*************************************************************************/
// StartIoreq
// Queue each Reqblk that makes up this Ioreq, trying to combine Reqblks.
// Call StartIO.
// For Raid1, all Reqblks are Queued in the same Queue to easily combine
// Reads. When they are ready to execute, the member is chosen for Reads
/*************************************************************************/

void	Raid1::StartIoreq(Ioreq *pIoreq)
{
	Reqblk *pReq, *pNext;

	TRACE_ENTRY(Raid1::StartIoreq);

	pReq = pIoreq->pReq;
	while (pReq)
	{
		pNext = pReq->pCombForw;
		pRequestQueue->SGCombine(pReq);
//pRequestQueue->EnQueue(pReq);

		pReq = pNext;
	}
	Raid1::StartIO();
}

/*************************************************************************/
// StartIO
// Start all I/O possible. Check for any utilities to run.
// All I/O requests are in Request Queue in Raid1 class
/*************************************************************************/

void	Raid1::StartIO()
{
	Reqblk *pReq;

	TRACE_ENTRY(Raid1::StartIO);

	if (Flags & RAID_PROCESSING_ERROR)
		return;
	while (TRUE)
	{
		// check for a utility that reached priority
		// to run - and start it if possible
		if (!pUtilQueue->IsEmpty())
			pUtilQueue->CheckUtilPriorityToRun();
		pReq = pRequestQueue->DeQueue();
		if (pReq)
		{	// Call Start routine for Reqblk
			if ( ( (pReq->pInst)->*(pReq->pStartRoutine) )(pReq) == FALSE )
			{	// Could not start - put back in Queue as Current
				pRequestQueue->EnQueueAsCurrent(pReq);
				break;
			}
			else
			{	// started an I/O - update the
				// priority of all waiting utilities
				if (!pUtilQueue->IsEmpty())
					pUtilQueue->UpdateUtilPriority();
			}
		}
		else
			break;		// Queue empty
	}
	// now try to start waiting utilities
	if (!pUtilQueue->IsEmpty())
		pUtilQueue->CheckUtilToRun(this);
}

/*************************************************************************/
// ReqIODone
// Call Base class ReqblkDone to free Reqblks and call completion routines
// for Ioreqs that are done. Then start more I/O
/*************************************************************************/

void	Raid1::ReqIODone(Reqblk *pReq)
{
	TRACE_ENTRY(Raid1::ReqIODone);

	Raid::ReqblkDone(pReq);

	if (pQuiesceIor)
	{	// waiting for everything to finish
		CheckForQuiesced();
	}
	StartIO();
}

/*************************************************************************/
// IsRaidUsable
// Used by RaidErr class. If members in Failmask were down, would
// array still be usable?
/*************************************************************************/

BOOL	Raid1::IsRaidUsable(U32 Failmask)
{
	U32		MemberMask;

	MemberMask = 1;
	// if any member is Up, array is usable
	for (U8 i = 0; i < NumberMembers; i++)
	{
		if ((Failmask & MemberMask) == 0)
			if (pMember[i]->Health == RAID_STATUS_UP)
				return (TRUE);
		MemberMask <<= 1;
	}
	return (FALSE);
}

/*************************************************************************/
// ConvertReqLbaToIoreqLba
// Used to report media errors
// For Raid1 pReq->Lba and pIoreq->Lba are the same
/*************************************************************************/

U32		Raid1::ConvertReqLbaToIoreqLba(Reqblk *pReq)
{
	return (pReq->ErrorLba);
}

/*************************************************************************/
// CanUtilityRun
// 
/*************************************************************************/

BOOL	Raid1::CanUtilityRun(UtilReqblk *pUtilReq)
{
	U32		DownMask = 0;
	U32		MemberMask;
	Utility	*pUtility;

	// get mask of all down members
	MemberMask = 1;
	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (pMember[i]->Health == RAID_STATUS_DOWN)
			DownMask |= MemberMask;
		MemberMask <<= 1;
	}
	// if a member in the SourceMask failed, can't continue
	if (DownMask & pUtilReq->SourceMask)
		return (FALSE);
	if (DownMask & pUtilReq->DestMask)
	{
		if ((pUtilReq->DestMask & ~DownMask) == 0)
			return (FALSE);	// No dest members remain
		else
		{
			// check policy to see whether to continue
			pUtility = pUtilReq->pUtility;
			if (pUtility->Policy.RunThruErrors)
			{
				// remove failed members
				pUtility->DestMask &= ~DownMask;
				pUtilReq->DestMask &= ~DownMask;
				return (TRUE);
			}
			return (FALSE);
		}
	}
	return (TRUE);
}

/*************************************************************************/
// RequeueRequestblk
// Called from RaidErr class. Put Reqblk back in Queue to be redone
/*************************************************************************/

void	Raid1::RequeueRequestblk(Reqblk *pReq)
{
	if (pReq->Flags & RAID_UTILITY_BIT)
	{
		// check if can still run, else call back with error
		if (CanUtilityRun((UtilReqblk *)pReq))
		{	// clear any error and try again
			pReq->Status = FCP_SCSI_DSC_SUCCESS;
			pUtilQueue->EnQueue((UtilReqblk *)pReq);
		}
		else
		{
			pReq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
			// do callback - this pReq ends in error
			( (pReq->pInst)->*(pReq->pCallback) )(pReq);
		}
	}
	else
		pRequestQueue->EnQueueAsCurrent(pReq);

	StartIO();
}

/*************************************************************************/
// ClearQueuesWithError
// Called from RaidErr class - array went offline
// Empty all Queues, end all Requests in error
/*************************************************************************/

void	Raid1::ClearQueuesWithError()
{
	Reqblk 		*pReq;
	UtilReqblk	*pUtilReq;

	// clear I/O Queue
	while ((pReq = pRequestQueue->DeQueue()) != NULL)
	{
		pReq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
		// do callback - this pReq ends in error
		( (pReq->pInst)->*(pReq->pCallback) )(pReq);
	}
	// clear Utility Queue
	while ((pUtilReq = (UtilReqblk *)pUtilQueue->DeQueue()) != NULL)
	{
		pUtilReq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
		// do callback - this pReq ends in error
		( (pUtilReq->pInst)->*(pUtilReq->pCallback) )(pUtilReq);
	}
}

/*************************************************************************/
// MirroredWrite
// Send Write Messages to all members
/*************************************************************************/

BOOL	Raid1::MirroredWrite(Reqblk *pReq)
{ 
	RAID1_CONTEXT	*pR1Context;
	Message			*pMsg;
	SGLIST			*pSGList;
	U32				WriteMask, MemberMask;
	Member			*pMem;
	U8				Mem, NumWrites;

	TRACE_ENTRY(Raid1::MirroredWrite);

	MemberMask = 1;
	WriteMask = 0;
	NumWrites = 0;
	for (int i = 0; i < NumberMembers; i++)
	{
		pMem = pMember[i];
		if (pMem->Health != RAID_STATUS_DOWN)
		{
			WriteMask |= MemberMask;
			NumWrites++;
		}
		MemberMask <<= 1;
	}
	if (WaitMask & WriteMask)
		return (FALSE);		// can't start

	// lock this range of blocks
	if (!pRaidLock->GetReqLock(pReq))
		return (FALSE);

	pReq->State = NumWrites;
	Mem = 0;
	MemberMask = 1;
	while (WriteMask)
	{
		if (WriteMask & MemberMask)
		{
			pMem = pMember[Mem];
			WriteMask ^= MemberMask;

			pR1Context = new RAID1_CONTEXT;
			pR1Context->pReq = pReq;
			pR1Context->MemberMask = MemberMask;
			pR1Context->Member = Mem;
			pR1Context->RetryCount = 0;
			pR1Context->payload.ControlFlags = 0;
			pR1Context->payload.TimeMultiplier = 0;
			pR1Context->payload.FetchAhead = 0;
			pR1Context->payload.TransferByteCount = pReq->Count * 512;
			pR1Context->payload.LogicalBlockAddress = pReq->Lba;

			if (pReq->Flags & WRITE_VERIFY_BIT)
				pMsg = new Message(BSA_BLOCK_WRITE_VERIFY, sizeof(FCP_MSG_SIZE));
			else
				pMsg = new Message(BSA_BLOCK_WRITE, sizeof(FCP_MSG_SIZE));
			pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
			pSGList = pReq->pSGList;
			for (int i = 0; i < pReq->NumSGLs; i++)
			{
				pMsg->AddSgl(i, (void *)pSGList->Address, pSGList->Length, SGL_SEND);
				pSGList++;
			}
			pMem->NumOutstanding++;
			if (pMem->NumOutstanding == pMem->MaxOutstanding)
				WaitMask |= MemberMask;
			pMem->LastAccessedLBA = pReq->Lba;

			DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::MirroredWriteCallback);
		}
		Mem++;
		MemberMask <<= 1;
	}
	return (TRUE);
}

/*************************************************************************/	 
// MirroredWriteCallback
// Callback routine for a Mirrored Write
/*************************************************************************/

void	Raid1::MirroredWriteCallback(MessageReply *pMsg)
{ 
	RAID1_CONTEXT	*pR1Context;
	Reqblk			*pReq;
	Member			*pMem;
	U32				MemberMask;
	U8				Mem;

	TRACE_ENTRY(Raid1::MirroredWriteCallback);

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pReq = pR1Context->pReq;
	Mem = pR1Context->Member;
	MemberMask = pR1Context->MemberMask;

	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::MirroredWriteCallback) == TRUE)
			return;		// Retrying message
		else
			Raid::SetReqStatus(pReq, pMsg, Mem);
	}

	pReq->State--;
	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	delete pR1Context;
	delete pMsg;
	if (pReq->State == 0)
	{
		// release lock
		pRaidLock->ReleaseReqLock(pReq);
		Raid::CommandComplete(pReq);
	}
}

/*************************************************************************/
// MirroredRead
// Send a Read Message
/*************************************************************************/

BOOL	Raid1::MirroredRead(Reqblk *pReq)
{ 
	RAID1_CONTEXT	*pR1Context;
	SGLIST			*pSGList;
	U32				MemberMask, MemMask;
	U32				Diff, CurDiff = 0;
	Member			*pMem;
	U8				Mem = 0xff, ReadPref = 0xff;
	U16				QDepth;

	MemberMask = 1;

	TRACE_ENTRY(Raid1::MirroredRead);

	// choose member to read from
	for (int i = 0; i < NumberMembers; i++)
	{
		pMem = pMember[i];
		if (!(MemberMask & WaitMask))
		{
			if ((pMem->Health == RAID_STATUS_UP) || ((pMem->Health == RAID_STATUS_REGENERATING)
								&& (pMem->RegeneratedLBA > (pReq->Count + pReq->Lba))))
			{
				if (pMem->ReadPreference < ReadPref)
				{
					Mem = i;
					MemMask = MemberMask;
					ReadPref = pMem->ReadPreference;
					QDepth = pMem->NumOutstanding;
				}
				else if (pMem->ReadPreference == ReadPref)
				{
					if (pMem->NumOutstanding < QDepth)
					{
						Mem = i;
						MemMask = MemberMask;
						ReadPref = pMem->ReadPreference;
						QDepth = pMem->NumOutstanding;
					}
					else if (pMem->NumOutstanding == QDepth)
					{
						if (pReq->Lba > pMem->LastAccessedLBA)
							Diff = pReq->Lba - pMem->LastAccessedLBA;
						else
							Diff = pMem->LastAccessedLBA - pReq->Lba;
						if (Diff < CurDiff)
						{
							Mem = i;
							MemMask = MemberMask;
							ReadPref = pMem->ReadPreference;
							QDepth = pMem->NumOutstanding;
							CurDiff = Diff;
						}
					}
				}
			}
		}
		MemberMask <<= 1;
	}
	if (Mem == 0xff)
		return (FALSE);		// can't start

	if ((pR1Context = new RAID1_CONTEXT) == NULL)
		return (FALSE);

	pMem = pMember[Mem];

	Message *pMsg = new Message(BSA_BLOCK_READ, sizeof(FCP_MSG_SIZE));
	if (pMsg == NULL)
	{
		delete pR1Context;
		return (FALSE);
	}
	
	pR1Context->payload.ControlFlags = 0;
	pR1Context->payload.TimeMultiplier = 0;
	pR1Context->payload.FetchAhead = 0;
	pR1Context->payload.TransferByteCount = pReq->Count * 512;
	pR1Context->payload.LogicalBlockAddress = pReq->Lba;

	pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));

	pSGList = pReq->pSGList;
	for (int i = 0; i < pReq->NumSGLs; i++)
	{
		pMsg->AddSgl(i, (void *)pSGList->Address, pSGList->Length, SGL_REPLY);
		pSGList++;
	}

	pMem->NumOutstanding++;
	if (pMem->NumOutstanding == pMem->MaxOutstanding)
		WaitMask |= MemMask;
	pMem->LastAccessedLBA = pReq->Lba;

	pR1Context->pReq = pReq;
	pR1Context->MemberMask = MemMask;
	pR1Context->Member = Mem;
	pR1Context->RetryCount = 0;

	DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::MirroredReadCallback);

	return (TRUE);
}

/*************************************************************************/	 
// MirroredReadCallback
// Callback routine for a Mirrored Read
/*************************************************************************/

void	Raid1::MirroredReadCallback(MessageReply *pMsg)
{ 
	RAID1_CONTEXT	*pR1Context;
	Reqblk			*pReq;
	Member			*pMem;
	U32				MemberMask;
	U8				Mem;

	TRACE_ENTRY(Raid1::MirroredReadCallback);

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pReq = pR1Context->pReq;
	Mem = pR1Context->Member;
	MemberMask = pR1Context->MemberMask;
	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::MirroredReadCallback) == TRUE)
			return;		// Retrying message
		else
			Raid::SetReqStatus(pReq, pMsg, Mem);
	}

	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;

	delete pR1Context;
	delete pMsg;
	Raid::CommandComplete(pReq);
}

/*************************************************************************/
// ReassignBlock
// Send Reassign Block Messages to all Up members
/*************************************************************************/

BOOL	Raid1::ReassignBlock(Reqblk *pReq)
{ 
	RAID1_CONTEXT	*pR1Context;
	Message			*pMsg;
	U32				WriteMask, MemberMask;
	Member			*pMem;
	U8				Mem, NumWrites;

	MemberMask = 1;
	WriteMask = 0;
	NumWrites = 0;
	for (int i = 0; i < NumberMembers; i++)
	{
		pMem = pMember[i];
		if (pMem->Health != RAID_STATUS_DOWN)
		{
			WriteMask |= MemberMask;
			NumWrites++;
		}
		MemberMask <<= 1;
	}
	if (WaitMask & WriteMask)
		return (FALSE);		// can't start

	// lock this range of blocks
	if (!pRaidLock->GetReqLock(pReq))
		return (FALSE);

	pReq->State = NumWrites;
	Mem = 0;
	MemberMask = 1;
	while (WriteMask)
	{
		if (WriteMask & MemberMask)
		{
			pMem = pMember[Mem];
			WriteMask ^= MemberMask;
			pR1Context = new RAID1_CONTEXT;
			pR1Context->pReq = pReq;
			pR1Context->MemberMask = MemberMask;
			pR1Context->Member = Mem;
			pR1Context->RetryCount = 0;
			pR1Context->payload.ControlFlags = 0;
			pR1Context->payload.TimeMultiplier = 0;
			pR1Context->payload.FetchAhead = 0;
			pR1Context->payload.TransferByteCount = pReq->Count * 512;
			pR1Context->payload.LogicalBlockAddress = pReq->Lba;

			pMsg = new Message(BSA_BLOCK_REASSIGN, sizeof(FCP_MSG_SIZE));
			pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
			pMem->NumOutstanding++;
			if (pMem->NumOutstanding == pMem->MaxOutstanding)
				WaitMask |= MemberMask;

			DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::ReassignBlockCallback);
		}
		Mem++;
		MemberMask <<= 1;
	}
	return (TRUE);
}

/*************************************************************************/	 
// ReassignBlockCallback
// Callback routine for a ReassignBlock
/*************************************************************************/

void	Raid1::ReassignBlockCallback(MessageReply *pMsg)
{ 
	RAID1_CONTEXT	*pR1Context;
	Reqblk			*pReq;
	Member			*pMem;
	U32				MemberMask;
	U8				Mem;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pReq = pR1Context->pReq;
	Mem = pR1Context->Member;
	MemberMask = pR1Context->MemberMask;

	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::ReassignBlockCallback) == TRUE)
			return;		// Retrying message
	}

	pReq->State--;
	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	Raid::SetReqStatus(pReq, pMsg, Mem);
	delete pR1Context;
	delete pMsg;
	if (pReq->State == 0)
	{
		// release lock
		pRaidLock->ReleaseReqLock(pReq);
		// don't call CommandComplete - don't want to go to 
		// Error processing
		// do callback
		( (pReq->pInst)->*(pReq->pCallback) )(pReq);
	}
}

/*************************************************************************/
// Verify
// Start Verify command
/*************************************************************************/

BOOL	Raid1::Verify(UtilReqblk *pUtilReq)
{
	RAID1_CONTEXT			*pR1Context;
	R1_MSTR_UTIL_CONTEXT	*pMasterContext;
	Member					*pMem;
	Message					*pMsg;
	U32						NumBytes;
	U32						MemberMask, NeededMask;
	U8						Num;

	// SourceMask is primary member
	// DestMask is all other members to be compared
	NeededMask = pUtilReq->SourceMask | pUtilReq->DestMask;	
	if (WaitMask & NeededMask)
		return (FALSE);		// can't start

	// lock range
	if (!pRaidLock->GetReqLock(pUtilReq))
		return (FALSE);

	NumBytes = pUtilReq->Count * 512;

	pMasterContext = new (tZERO) R1_MSTR_UTIL_CONTEXT;
	pMasterContext->NeedsCompareMask = 0;
	pMasterContext->PrimaryDone = FALSE;
	MemberMask = 1;
	Num = 0;
	for (int i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & NeededMask)
		{
			pMasterContext->pBuffer[i] = new (tBIG | tUNCACHED | tPCI) U8[NumBytes];
			if (MemberMask & pUtilReq->SourceMask)
				pMasterContext->Primary = i;
			Num++;
		}
		MemberMask <<= 1;
	}

	// number of members involved
	pMasterContext->State = Num;

	MemberMask = 1;
	// start reads of all members being verified
	for (int i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & NeededMask)
		{
			pMem = pMember[i];

			pR1Context = new RAID1_CONTEXT;
			pR1Context->pReq = pUtilReq;
			pR1Context->MemberMask = MemberMask;
			pR1Context->Member = i;
			pR1Context->RetryCount = 0;
			pR1Context->pMstrContext = pMasterContext;
			pR1Context->payload.ControlFlags = 0;
			pR1Context->payload.TimeMultiplier = 0;
			pR1Context->payload.FetchAhead = 0;
			pR1Context->payload.TransferByteCount = NumBytes;
			pR1Context->payload.LogicalBlockAddress = pUtilReq->Lba;

			pMsg = new Message(BSA_BLOCK_READ, sizeof(FCP_MSG_SIZE));
			pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
			pMsg->AddSgl(0, (void *)pMasterContext->pBuffer[i], NumBytes, SGL_REPLY);
			pMem->NumOutstanding++;
			if (pMem->NumOutstanding >= pMem->MaxOutstanding)
				WaitMask |= MemberMask;
			pMem->LastAccessedLBA = pUtilReq->Lba;

			DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::VerifyCallback);
		}
		MemberMask <<= 1;
	}
	return (TRUE); 
}

/*************************************************************************/	 
// VerifyCallback
// Callback routine for Verify read command
/*************************************************************************/

void	Raid1::VerifyCallback(MessageReply *pMsg)
{ 
	RAID1_CONTEXT			*pR1Context;
	R1_MSTR_UTIL_CONTEXT	*pMasterContext;
	UtilReqblk				*pUtilReq;
	Member					*pMem;
	U8						Mem;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pMasterContext = pR1Context->pMstrContext;
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	Mem = pR1Context->Member;
	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::VerifyCallback) == TRUE)
			return;		// Retrying message

		Raid::SetReqStatus(pUtilReq, pMsg, Mem);
		VerifyDone(pMsg);	// end verify operation
		return;
	}
	// this read was successful

	if (pUtilReq->Status != FCP_SCSI_DSC_SUCCESS)
	{	// previous read failed
		VerifyDone(pMsg);	// end verify operation
		return;
	}

	if (pMasterContext->Primary == Mem)
	{	// this is the primary member
		VerifyPrimaryDone(pMsg);
	}
	else
		VerifyOtherReadDone(pMsg);
}

/*************************************************************************/
// VerifyCorrect
// Write from primary member's buffer to member that miscompared
/*************************************************************************/

void	Raid1::VerifyCorrect(R1_MSTR_UTIL_CONTEXT *pMasterContext, UtilReqblk *pUtilReq, U8 Mem)
{
	RAID1_CONTEXT			*pR1Context;
	Member					*pMem;
	Message					*pMsg;
	U32						NumBytes;

	NumBytes = pUtilReq->Count * 512;

	// write to this member from buffer for primary member
	pMem = pMember[Mem];
	pR1Context = new RAID1_CONTEXT;
	pR1Context->pReq = pUtilReq;
	pR1Context->MemberMask = pMem->MemberMask;
	pR1Context->Member = Mem;
	pR1Context->RetryCount = 0;
	pR1Context->pMstrContext = pMasterContext;
	pR1Context->payload.ControlFlags = 0;
	pR1Context->payload.TimeMultiplier = 0;
	pR1Context->payload.FetchAhead = 0;
	pR1Context->payload.TransferByteCount = NumBytes;
	pR1Context->payload.LogicalBlockAddress = pUtilReq->Lba;

	pMsg = new Message(BSA_BLOCK_WRITE, sizeof(FCP_MSG_SIZE));
	pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
	pMsg->AddSgl(0, (void *)pMasterContext->pBuffer[pMasterContext->Primary], NumBytes, SGL_SEND);

	DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::VerifyCorrectDone);
}

/*************************************************************************/
// VerifyCorrectDone
// Write from primary member's buffer to member finished
/*************************************************************************/

void	Raid1::VerifyCorrectDone(MessageReply *pMsg)
{ 
	RAID1_CONTEXT			*pR1Context;
	R1_MSTR_UTIL_CONTEXT	*pMasterContext;
	UtilReqblk				*pUtilReq;
	U32						MemberMask;
	Member					*pMem;
	U8						Mem, i;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pMasterContext = pR1Context->pMstrContext;
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	Mem = pR1Context->Member;
	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::VerifyCorrectDone) == TRUE)
			return;		// Retrying message

		Raid::SetReqStatus(pUtilReq, pMsg, Mem);
	}
	// done with this member
	MemberMask = pR1Context->MemberMask;
	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	pMasterContext->State--;
	delete pR1Context;
	delete pMsg;
	if (pMasterContext->State == 0)
	{	// all done
		// release lock
		pRaidLock->ReleaseReqLock(pUtilReq);
		for (i = 0; i < NumberMembers; i++)
		{
			if (pMasterContext->pBuffer[i])
		   		delete pMasterContext->pBuffer[i];
		}
		delete pMasterContext;
		Raid::CommandComplete(pUtilReq);
	}
}

/*************************************************************************/	 
// VerifyPrimaryDone
// Read of primary member finished
/*************************************************************************/

void	Raid1::VerifyPrimaryDone(MessageReply *pMsg)
{
	RAID1_CONTEXT			*pR1Context;
	R1_MSTR_UTIL_CONTEXT	*pMasterContext;
	UtilReqblk				*pUtilReq;
	Utility					*pUtility;
	Member					*pMem;
	U32						MemberMask;
	U8						Mem, i;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pMasterContext = pR1Context->pMstrContext;
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	Mem = pR1Context->Member;
	pMem = pMember[Mem];

	// set primary read done
	pMasterContext->PrimaryDone = TRUE;

	pUtility = pUtilReq->pUtility;
	// check if any other reads finished first
	// and need to be compared now
	MemberMask = 1;
	for (i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & pMasterContext->NeedsCompareMask)
		{
			// turn off needs compare for this member
			pMasterContext->NeedsCompareMask ^= MemberMask;
			// compare buffers
			if (!(CompareBuffers( (I64 *)pMasterContext->pBuffer[pMasterContext->Primary],
						(I64 *)pMasterContext->pBuffer[i],
						pUtilReq->Count * 512 / sizeof(I64)) ) )

			{
				// they don't compare - check policy to see whether to correct
				if (pUtility->Policy.DontFixErrors)
				{
					// don't fix - done with this member			
					// msg and context for this member were already deleted				
					WaitMask &= ~MemberMask;
					pMember[i]->NumOutstanding--;
					pMasterContext->State--;
				}
				else
					// not done with this member yet
					VerifyCorrect(pMasterContext, pUtilReq, i);
				pUtility->ErrorCount++;		// count miscompares
			}
			else
			{	// they compare - done with this member
			  	// msg and context for this member were already deleted				
				WaitMask &= ~MemberMask;
				pMember[i]->NumOutstanding--;
				pMasterContext->State--;
			}
		}
		MemberMask <<= 1;
	}
	// set member mask to primary
	MemberMask = pR1Context->MemberMask;
	// done with primary member
	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	pMasterContext->State--;
	delete pR1Context;
	delete pMsg;
	if (pMasterContext->State == 0)
	{	// all done
		// release lock
		pRaidLock->ReleaseReqLock(pUtilReq);
		// delete buffers allocated
		for (i = 0; i < NumberMembers; i++)
		{
			if (pMasterContext->pBuffer[i])
		   		delete pMasterContext->pBuffer[i];
		}
		delete pMasterContext;
		Raid::CommandComplete(pUtilReq);
	}
}

/*************************************************************************/	 
// VerifyOtherReadDone
// Read of member other than primary finished
/*************************************************************************/

void	Raid1::VerifyOtherReadDone(MessageReply *pMsg)
{
	RAID1_CONTEXT			*pR1Context;
	R1_MSTR_UTIL_CONTEXT	*pMasterContext;
	Utility					*pUtility;
	UtilReqblk				*pUtilReq;
	Member					*pMem;
	U32						MemberMask;
	U8						Mem, i;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pMasterContext = pR1Context->pMstrContext;
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	pUtility = pUtilReq->pUtility;
	MemberMask = pR1Context->MemberMask;
	Mem = pR1Context->Member;
	pMem = pMember[Mem];

	if (pMasterContext->PrimaryDone)	// read of primary is done
	{
		// compare buffers
		if (!(CompareBuffers( (I64 *)pMasterContext->pBuffer[pMasterContext->Primary],
						(I64 *)pMasterContext->pBuffer[Mem],
						pUtilReq->Count * 512 / sizeof(I64)) ) )

		{
			// they don't compare - check policy to see whether to correct
			if (pUtility->Policy.DontFixErrors)
			{
				// don't fix
				// done with this member			
				WaitMask &= ~MemberMask;
				pMember[Mem]->NumOutstanding--;
				pMasterContext->State--;
			}
			else
				VerifyCorrect(pMasterContext, pUtilReq, Mem);
			pUtility->ErrorCount++;		// count miscompares
		}
		else
		{	// compares - done with this member
			WaitMask &= ~MemberMask;
			pMem->NumOutstanding--;
			pMasterContext->State--;
		}
		delete pR1Context;
		delete pMsg;
		if (pMasterContext->State == 0)
		{	// all done
   			for (i = 0; i < NumberMembers; i++)
   			{
   				if (pMasterContext->pBuffer[i])
   			   		delete pMasterContext->pBuffer[i];
   			}
   			// release lock
   			pRaidLock->ReleaseReqLock(pUtilReq);
   			delete pMasterContext;
   			Raid::CommandComplete(pUtilReq);
		}
	}
	else
	{	// mark to be compared when read of primary finishes
		pMasterContext->NeedsCompareMask |= MemberMask;
		delete pR1Context;
		delete pMsg;
	}
}

/*************************************************************************/	 
// VerifyDone
// 
/*************************************************************************/

void	Raid1::VerifyDone(MessageReply *pMsg)
{
	RAID1_CONTEXT			*pR1Context;
	R1_MSTR_UTIL_CONTEXT	*pMasterContext;
	UtilReqblk				*pUtilReq;
	Member					*pMem;
	U32						MemberMask;
	U8						Mem, i;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pMasterContext = pR1Context->pMstrContext;
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	Mem = pR1Context->Member;
	MemberMask = pR1Context->MemberMask;
	pMem = pMember[Mem];

	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	pMasterContext->State--;
	delete pR1Context;
	delete pMsg;
	if (pMasterContext->Primary == Mem)
	{
		MemberMask = 1;
		for (i = 0; i < NumberMembers; i++)
		{
			if (MemberMask & pMasterContext->NeedsCompareMask)
			{
				// decrement state for all waiting for master
				pMasterContext->NeedsCompareMask ^= MemberMask;
				pMasterContext->State--;
				WaitMask &= ~MemberMask;
				pMember[i]->NumOutstanding--;
			}
			MemberMask <<= 1;
		}
	}
	if (pMasterContext->State == 0)
	{	// all done
		for (i = 0; i < NumberMembers; i++)
		{
			if (pMasterContext->pBuffer[i])
		   		delete pMasterContext->pBuffer[i];
		}
		// release lock
		pRaidLock->ReleaseReqLock(pUtilReq);
		delete pMasterContext;
		Raid::CommandComplete(pUtilReq);
	}
	return;
}
 
/*************************************************************************/
// Regenerate
// Start regenerate command
/*************************************************************************/

BOOL	Raid1::Regenerate(UtilReqblk *pUtilReq)
{
	RAID1_CONTEXT			*pR1Context;
	Member					*pMem;
	Message					*pMsg;
	U32						NumBytes;
	U32						MemberMask, NeededMask;
	U8						Mem;
	U8						*pBuffer;

	// SourceMask is member to read from
	// DestMask is member(s) to write to
	NeededMask = pUtilReq->DestMask | pUtilReq->SourceMask;	
	if (WaitMask & NeededMask)
		return (FALSE);		// can't start

	// lock range
	if (!pRaidLock->GetReqLock(pUtilReq))
		return (FALSE);

	NumBytes = pUtilReq->Count * 512;

	MemberMask = 1;
	Mem = 0;
	// find member to read from
	for (int i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & pUtilReq->SourceMask)
		{
			Mem = i;
			pMem = pMember[i];
			break;
		}
		MemberMask <<= 1;
	}

	// allocate buffer for regenerate
	pBuffer = new (tBIG | tUNCACHED | tPCI) U8[NumBytes];

	pMsg = new Message(BSA_BLOCK_READ, sizeof(FCP_MSG_SIZE));
	pR1Context = new RAID1_CONTEXT;
	pR1Context->pReq = pUtilReq;
	pR1Context->MemberMask = pUtilReq->SourceMask;
	pR1Context->Member = Mem;
	pR1Context->RetryCount = 0;
	pR1Context->payload.ControlFlags = 0;
	pR1Context->payload.TimeMultiplier = 0;
	pR1Context->payload.FetchAhead = 0;
	pR1Context->payload.TransferByteCount = NumBytes;
	pR1Context->payload.LogicalBlockAddress = pUtilReq->Lba;

	pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
	pMsg->AddSgl(0, (void *)pBuffer, NumBytes, SGL_REPLY);
	pMem->NumOutstanding++;
	if (pMem->NumOutstanding == pMem->MaxOutstanding)
		WaitMask |= MemberMask;
	pMem->LastAccessedLBA = pUtilReq->Lba;

	DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::RegenerateReadDone);
	return (TRUE); 
}

/*************************************************************************/	 
// RegenerateReadDone
// Callback routine for Read complete for Regenerate command
/*************************************************************************/

void	Raid1::RegenerateReadDone(MessageReply *pMsg)
{
	RAID1_CONTEXT			*pR1Context;
	UtilReqblk				*pUtilReq;
	Member					*pMem;
	U32						NumBytes;
	U32						MemberMask;
	U8						*pBuffer;
	U8						Num, Mem;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	MemberMask = pR1Context->MemberMask;
	Mem = pR1Context->Member;
	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::RegenerateReadDone) == TRUE)
			return;		// Retrying message

		Raid::SetReqStatus(pUtilReq, pMsg, Mem);
		WaitMask &= ~MemberMask;
		pMem->NumOutstanding--;
		// get buffer address
		pMsg->GetSgl(0, (void **)&pBuffer, &NumBytes);
		// free buffer
		delete pBuffer;
		// release lock
		pRaidLock->ReleaseReqLock(pUtilReq);
		Raid::CommandComplete(pUtilReq);
		return;
	}
	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	MemberMask = 1;
	Num = 0;
	// count number of writes
	for (int i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & pUtilReq->DestMask)
			Num++;
		MemberMask <<= 1;
	}

	pMsg->GetSgl(0, (void **)&pBuffer, &NumBytes);

	NumBytes = pUtilReq->Count * 512;

	// set state to number of writes
	pUtilReq->State = Num;

	delete pMsg;
	delete pR1Context;

	MemberMask = 1;
	for (int i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & pUtilReq->DestMask)
		{
			pMem = pMember[i];
			pR1Context = new RAID1_CONTEXT;
			pR1Context->pReq = pUtilReq;
			pR1Context->MemberMask = MemberMask;
			pR1Context->Member = i;
			pR1Context->RetryCount = 0;
			pR1Context->payload.ControlFlags = 0;
			pR1Context->payload.TimeMultiplier = 0;
			pR1Context->payload.FetchAhead = 0;
			pR1Context->payload.TransferByteCount = NumBytes;
			pR1Context->payload.LogicalBlockAddress = pUtilReq->Lba;

			pMsg = new Message(BSA_BLOCK_WRITE, sizeof(FCP_MSG_SIZE));
			pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
			pMsg->AddSgl(0, (void *)pBuffer, NumBytes, SGL_SEND);
			pMem->NumOutstanding++;
			if (pMem->NumOutstanding == pMem->MaxOutstanding)
				WaitMask |= MemberMask;
			pMem->LastAccessedLBA = pUtilReq->Lba;

			DdmServices::Send(pMem->Vd, pMsg, pR1Context, (ReplyCallback) &Raid1::RegenerateWriteDone);
		}
		MemberMask <<= 1;
	}
}
 
/*************************************************************************/	 
// RegenerateWriteDone
// Callback routine for Write complete for Regenerate command
/*************************************************************************/

void	Raid1::RegenerateWriteDone(MessageReply *pMsg)
{
	RAID1_CONTEXT		*pR1Context;
	UtilReqblk	   		*pUtilReq;
	Member				*pMem;
	U8	 				*pBuffer;
	U32					MemberMask;
	U32	 				NumBytes;
	U8					Mem;

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	pUtilReq = (UtilReqblk *)pR1Context->pReq;
	MemberMask = pR1Context->MemberMask;
	Mem = pR1Context->Member;

	pMem = pMember[Mem];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid1::RegenerateWriteDone) == TRUE)
			return;		// Retrying message
		else
			Raid::SetReqStatus(pUtilReq, pMsg, Mem);
	}

	pUtilReq->State--;
	WaitMask &= ~MemberMask;
	pMem->NumOutstanding--;
	// get buffer address
	pMsg->GetSgl(0, (void **)&pBuffer, &NumBytes);
	delete pR1Context;
	delete pMsg;
	if (pUtilReq->State == 0)
	{
		// free buffer
		delete pBuffer;
		// release lock
		pRaidLock->ReleaseReqLock(pUtilReq);
		if (pUtilReq->Type == RAID_REGENERATE_CMD)
		{	// check type - could be background init
			if (pUtilReq->Status == FCP_SCSI_DSC_SUCCESS)
				UpdateRegeneratedLba(pUtilReq);
		}
		Raid::CommandComplete(pUtilReq);
	}
}
 
/*************************************************************************/
// UpdateRegeneratedLba
// Set pMem->RegeneratedLBA to lowest LBA of all UtilReqs for this utility
/*************************************************************************/

void	Raid1::UpdateRegeneratedLba(UtilReqblk *pUtilReq)
{
	UtilReqblk	*pNext;
	Utility		*pUtility;
	U32			MemberMask, RegeneratedLba;

	// set RegeneratedLba to lowest of all UtilReqs
	
	pUtility = pUtilReq->pUtility;
	// head of UtilReq chain for pUtility
	pNext = pUtility->pUtilReq;
	RegeneratedLba = pNext->Lba;
	pNext = (UtilReqblk *)pNext->pCombForw;
	while (pNext)
	{
		if (pNext->Lba < RegeneratedLba)
			RegeneratedLba = pNext->Lba;
		pNext = (UtilReqblk *)pNext->pCombForw;
	}

	// set RegeneratedLba of all members being regenerated
	MemberMask = 1;
	for (int i = 0; i < NumberMembers; i++)
	{
		if (MemberMask & pUtilReq->DestMask)
			pMember[i]->RegeneratedLBA = RegeneratedLba;
		MemberMask <<= 1;
	}
}

/*************************************************************************/	 
// CompareBuffers
// 
/*************************************************************************/

BOOL	Raid1::CompareBuffers(I64 *pBuf1, I64 *pBuf2, U32 Count)
{
	for (U32 i = 0; i < Count; i++)
		if (*pBuf1++ != *pBuf2++)
			return (FALSE);
	return (TRUE);
}

/*************************************************************************/	 
// RetryCommand
// Get a new message to retry command with, fill it in, and send it.
/*************************************************************************/

BOOL	Raid1::RetryCommand(Member *pMem, MessageReply *pMsg, ReplyCallback rc)
{ 
	RAID1_CONTEXT	*pR1Context;

	TRACE_ENTRY(Raid1::RetryCommand);

	pR1Context = (RAID1_CONTEXT *)pMsg->GetContext();
	if (pR1Context->RetryCount < pMem->MaxRetryCount)
	{	
		pR1Context->RetryCount++;
		// fill payload again
		pMsg->AddPayload(&pR1Context->payload, sizeof(BSA_RW_PAYLOAD));
		// send the message
		DdmServices::Send(pMem->Vd, pMsg, pR1Context, rc);
		return (TRUE);	// message sent
	}
	return (FALSE);		// exhausted retries
}

/*************************************************************************/
// ReadMDerrBlock
// Reread block command for MediaError processing
/*************************************************************************/

BOOL	Raid1::ReadMDerrBlock(ErrReqblk *pErrReq)
{
	Member			*pMem;
	U8				i, Mem;

	Mem = pErrReq->Member;

	// try to find another Up member to read to 
	// recover data for bad block. If no other Up
	// members, just retry member
	for (i = 0; i < NumberMembers; i++)
	{
		if (i != pErrReq->Member)
		{
			pMem = pMember[i];
			if (pMem->Health == RAID_STATUS_UP)
			{
				Mem = i;
				break;
			}
			else if (pMem->Health == RAID_STATUS_REGENERATING)
			{ 
				if (!(pMem->RegeneratedLBA < pErrReq->Lba))
				{
					Mem = i;
					break ;
				}
			}
		}
	}
	// set member to read
	pErrReq->Member = Mem;
	// Call base raid class method
	return (Raid::ReadMDerrBlock(pErrReq));
}
