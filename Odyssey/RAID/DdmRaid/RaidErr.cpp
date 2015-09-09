/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidErr.cpp
//
// Description:	Raid Error Processing class
//
//
// Update Log: 
//	4/99	Jim Taylor:	initial creation
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
#include "RaidErr.h"
#include "RaidUtilTable.h"
#include "Raid.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "Scsi.h"
#include "OsStatus.h"

/*************************************************************************/
// Initialize
// Initialize method for the RaidErr Class
/*************************************************************************/

STATUS	RaidErr::Initialize(Raid *pRaid)
{ 

	this->pRaid = pRaid;
	Failmask = 0;
	pErrReq = NULL;

	TRACE_ENTRY(RaidErr::Initialize);

	// allocate a request queue for requests that
	// end in error while error processing
	if ((pRequestQueue = new ReqQueue(pRaid, RAIDQ_FIFO)) == NULL)
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;

	// allocate a buffer for Media Defect processing
	if ((pTestBuf = new (tUNCACHED) U8[512])== NULL)
	{
		delete pRequestQueue;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}
	// allocate an Err Reqblk for doing I/O
	if ((pTestReq = new (tZERO) ErrReqblk) == NULL)
	{
		delete pRequestQueue;
		delete pTestBuf;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}
	// allocate an SGL for the Err Reqblk
	if ((pTestReq->pSGList = new SGLIST[1]) == NULL)
	{
		delete pRequestQueue;
		delete pTestBuf;
		delete pTestReq;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}
	pTestReq->NumSGLs = 1;

	return OS_DETAIL_STATUS_SUCCESS;
}	

/*************************************************************************/
// ~RaidErr
// Destructor method for the class Raid Err
/*************************************************************************/

RaidErr::~RaidErr()
{ 
	if (pRequestQueue)
		delete pRequestQueue;
	if (pTestBuf)
		delete pTestBuf;
	if (pTestReq)
	{
		if (pTestReq->pSGList)
			delete pTestReq->pSGList;
		delete pTestReq;
	}
}

/*************************************************************************/
// ProcessError
// Called with a pReq that ended in error
/*************************************************************************/

void	RaidErr::ProcessError(Reqblk *pReq)
{
	TRACE_ENTRY(RaidErr::ProcessError);

	if (pRaid->Flags & RAID_PROCESSING_ERROR)
	{	// already in error processing state
		// error with another pReq, queue it here
		pRequestQueue->EnQueue(pReq);
	} 
	else	// first error
	{
		// set error processing flag
		pRaid->Flags |= RAID_PROCESSING_ERROR;
		pErrReq = pReq;
		Lba = pReq->ErrorLba;
		Suspect = pReq->ErrorMember;
		Failmask = 0;
		if (((pReq->Status & FCP_SCSI_DEVICE_DSC_MASK) == FCP_SCSI_DSC_CHECK_CONDITION) &&
							(pReq->SenseKey == SENSE_MEDIUM_ERROR))
 			MediaError();
		else
		{
			Failmask = pRaid->pMember[Suspect]->MemberMask;
  			SetMemberDown();
		}
	}
}

/*************************************************************************/
// ProcessError
// Called to set a member down from a Raid command
/*************************************************************************/

STATUS	RaidErr::ProcessError(U8 Member)
{ 
	if (pRaid->Flags & RAID_PROCESSING_ERROR)
		return RAIDCMD_STATUS_BUSY;

	if (!(pRaid->IsRaidUsable(Failmask)))
		return RAIDCMD_STATUS_INAPPROPRIATE_CMD;

	pRaid->Flags |= RAID_PROCESSING_ERROR;
	pErrReq = NULL;
	Suspect = Member;
	Failmask = pRaid->pMember[Member]->MemberMask;
	SetMemberDown();
	return RAIDCMD_STATUS_SUCCESS;
}

/*************************************************************************/
// MediaError
// Start Media Error Processing
/*************************************************************************/

void	RaidErr::MediaError()
{
	TRACE_ENTRY(RaidErr::MediaError);

	pTestReq->Status = FCP_SCSI_DSC_SUCCESS;
	pTestReq->RetryCount = 0;
	pTestReq->Member = Suspect;
	pTestReq->Lba = Lba;
	pTestReq->Count = 1;
	pTestReq->pSGList->Address = (U32) pTestBuf;
	pTestReq->pSGList->Length = 512;
	pTestReq->pErrInst = this;
	// First get data that needs to be reassigned on media
	if (pErrReq->Flags & RAID_WRITE_BIT)
	{	// fill buffer from user data
		if (FillFromRequest() == FALSE)
		{	// should never happen
			pTestReq->pErrCallback = &RaidErr::ReadMDerrCallback;
			if (pRaid->ReadMDerrBlock (pTestReq))
				return;
		}
		else
		{
			pTestReq->pErrCallback = &RaidErr::ReassignBlockCallback;
			if (pRaid->ErrReassignBlock(pTestReq))
				return;
		}
	}
	else
	{	// read op - do appropiate read for raid level
		pTestReq->pErrCallback = &RaidErr::ReadMDerrCallback;
		if (pRaid->ReadMDerrBlock (pTestReq))
			return;
	}
	UnrecoverableMediaError(RAID_OUT_OF_RESOURCES);

}

/*************************************************************************/
// ReadMDerrCallback
// Callback from reading data to be reassigned
// regenerated from other member(s), or reread from suspect
/*************************************************************************/

void	RaidErr::ReadMDerrCallback(ErrReqblk *pTestReq)
{ 
	U8 	reason = 0;

	if (pTestReq->Status != FCP_SCSI_DSC_SUCCESS)
		reason = RAID_REASSIGN_READ_FAILURE;
	else
	{	// data sucessfully read into TestBuf
		// now reassign the block
		pTestReq->Member = Suspect;		// reset member
		pTestReq->Status = FCP_SCSI_DSC_SUCCESS;
		pTestReq->RetryCount = 0;
		pTestReq->pErrCallback = &RaidErr::ReassignBlockCallback;
		if (!pRaid->ErrReassignBlock(pTestReq))
			reason = RAID_OUT_OF_RESOURCES;
	}
	if (reason)
	{	// error - can not continue
		UnrecoverableMediaError(reason);
	}
}						


/*************************************************************************/
// ReassignBlockCallback
// Callback from Reassign Block command
/*************************************************************************/

void	RaidErr::ReassignBlockCallback(ErrReqblk *pTestReq)
{ 
	U8	reason = 0;
  
	if (pTestReq->Status != FCP_SCSI_DSC_SUCCESS)
	{
		// Reassignment failed

		// Update Stats
		pRaid->UpdateMemberReassignments(pTestReq->Status, pTestReq->Member);

		reason = RAID_BLOCK_REASSIGNMENT_FAILURE;
	}
	else
	{
		// block reassigned sucessfully - write data to member
		pTestReq->Member = Suspect;			// reset member
		pTestReq->Status = FCP_SCSI_DSC_SUCCESS;
		pTestReq->RetryCount = 0;
		pTestReq->pErrCallback = &RaidErr::WriteVerifyCallback;
		if (!pRaid->WriteVerify(pTestReq))
			reason = RAID_OUT_OF_RESOURCES;
	}
	if (reason)
		UnrecoverableMediaError(reason);
}

/*************************************************************************/
// WriteVerifyCallback
// Callback from Write Verify command
/*************************************************************************/

void	RaidErr::WriteVerifyCallback(ErrReqblk *pTestReq)
{ 

	// Update Stats
	pRaid->UpdateMemberReassignments(pTestReq->Status, pTestReq->Member);

	if (pTestReq->Status != FCP_SCSI_DSC_SUCCESS)
		UnrecoverableMediaError (RAID_REASSIGNED_WRITE_FAILURE);
	else
	{	// data sucessfully written to reassigned block on member
		ErrorDoneRestart() ;
	}
}

/*************************************************************************/
// FillFromRequest
// Fill test buffer with data for Reassigned Block
/*************************************************************************/

BOOL	RaidErr::FillFromRequest()
{ 
	U8		*pByte;
	U32		address, offset, i;

	offset = (Lba - pErrReq->Lba) << 9; /* use defined size FIXIT */

	// get address of data from SGList
	address = GetAddressFromSGL (pErrReq->pSGList, offset);
	if (address)
	{
		pByte = (U8 *)address ;
		for (i = 0; i < 512; i++)
			pTestBuf[i] = pByte[i];
	}
	else
		return (FALSE);			// should never happen
	return (TRUE);
}

/*************************************************************************/
// GetAddressFromSGL
// Get SGList address from offset
/*************************************************************************/

U32	RaidErr::GetAddressFromSGL(SGLIST *pSGList, U32 offset)
{ 
	U32		address;
	U8		i;

	// will need to change when we have stripe write Master Reqblks
	for (i = 0; i < pErrReq->NumSGLs; i++)
	{
		if (pSGList->Length  < offset)
		{
			address = pSGList->Address + offset;
			return (address);
		}
		else
		{
			offset -= pSGList->Length;
		 	pSGList++;
		}
	}
	return (0);		// error - not in list
}

/*************************************************************************/
// UnrecoverableMediaError
// 
/*************************************************************************/

void	RaidErr::UnrecoverableMediaError(U8 reason)
{

	// log error

	TRACE_ENTRY(RaidErr::UnrecoverableMediaError);

	if (reason == RAID_BLOCK_REASSIGNMENT_FAILURE)
	{
		if (pRaid->IsRaidUsable(pRaid->pMember[Suspect]->MemberMask))
		{
			// if array is usable with this member failed, 
			// fail it, data will be recovered on retry
			Failmask = pRaid->pMember[Suspect]->MemberMask;
			MarkMembersDown();
			return;
		}
	}
	// end this Request with media error,
	// if it is a Master find individual pReq
	// to end in error, and requeue all others
	EndErrReqblk(pErrReq);
	pErrReq = NULL;

	ErrorDoneRestart();
}

/*************************************************************************/
// ErrorDoneRestart
// Take all requests and requeue them to be redone
/*************************************************************************/

void	RaidErr::ErrorDoneRestart()
{
	Reqblk		*pReq;
	ReqQueue	*pTempReqQueue;

	TRACE_ENTRY(RaidErr::ErrorDoneRestart);

	pTempReqQueue = new ReqQueue(pRaid, RAIDQ_FIFO);

	if (pErrReq)
		pTempReqQueue->EnQueue(pErrReq);
	pErrReq = NULL;

	while ((pReq = pRequestQueue->DeQueue()) != NULL)
		pTempReqQueue->EnQueue(pReq);

	// clear error processing flag
	pRaid->Flags &= ~RAID_PROCESSING_ERROR;

	while ((pReq = pTempReqQueue->DeQueue()) != NULL)
	{
		// clear error and try again
		pReq->Status = FCP_SCSI_DSC_SUCCESS;
		pRaid->RequeueRequestblk(pReq);	// put pReq back in Queue to be redone
	}
	delete pTempReqQueue;
}

/*************************************************************************/
// ErrorDoneOffline
// Take all requests and end them with error
/*************************************************************************/

void	RaidErr::ErrorDoneOffline()
{
	Reqblk		*pReq;
	ReqQueue	*pTempReqQueue;

	TRACE_ENTRY(RaidErr::ErrorDoneOffline);

	if (pErrReq)
	{
		pErrReq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
		// do callback - this pReq ends in error
		( (pErrReq->pInst)->*(pErrReq->pCallback) )(pErrReq);
	}
	pErrReq = NULL;

	pTempReqQueue = new ReqQueue(pRaid, RAIDQ_FIFO);
	while ((pReq = pRequestQueue->DeQueue()) != NULL)
		pTempReqQueue->EnQueue(pReq);

	// clear error processing flag
	pRaid->Flags &= ~RAID_PROCESSING_ERROR;

	while ((pReq = pTempReqQueue->DeQueue()) != NULL)
	{
		pReq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
		// do callback - this pReq ends in error
		( (pReq->pInst)->*(pReq->pCallback) )(pReq);
	}
	delete pTempReqQueue;
	pRaid->ClearQueuesWithError();
}

/*************************************************************************/
// SetMemberDown
// Called as the result of Error Processing Tests or a Raid commnad
/*************************************************************************/

void	RaidErr::SetMemberDown()
{ 
	if (pRaid->IsRaidUsable(Failmask))
		MarkMembersDown();
	else
		MarkArrayOffline();
}

/*************************************************************************/
// MarkMembersDown
// Set Member Health to Down and call to set in PTS
/*************************************************************************/

void	RaidErr::MarkMembersDown()
{
	U32			MemberMask;

	MemberMask = 1;
	for (U8 i = 0; i < pRaid->NumberMembers; i++)
	{
		if (Failmask & MemberMask)
			pRaid->pMember[i]->Health = RAID_STATUS_DOWN;
		MemberMask <<= 1;
	}
	// mark down in PTS, when done continue at MarkMembersDownCallback
	pTestReq->pErrInst = this;
	pTestReq->pErrCallback = &RaidErr::MarkMembersDownCallback;
	pRaid->SetMemberHealthDownInPTS(pTestReq, Failmask);
}

/*************************************************************************/
// MarkMembersDownCallback
// Callback from marking member health in PTS, now generate event
/*************************************************************************/

void	RaidErr::MarkMembersDownCallback(ErrReqblk *pTestReq)
{
	RaidEvent	Event;
	U32			MemberMask;

	if (pTestReq->Status != OS_DETAIL_STATUS_SUCCESS)
		;		// ??
		
	MemberMask = 1;
	for (U8 i = 0; i < pRaid->NumberMembers; i++)
	{
		if (Failmask & MemberMask)
		{
			// Report Event Member Down
			Event.Event.MemberDown.ArrayRowID = pRaid->RaidRowId;
			Event.Event.MemberDown.MemberRowID = pRaid->pMember[i]->MemberRowId;
			pRaid->pCmdServer->csrvReportEvent(RAID_EVT_MEMBER_DOWN, &Event);
		}
		MemberMask <<= 1;
	}
	ErrorDoneRestart();
}

/*************************************************************************/
// MarkArrayOffline
// 
/*************************************************************************/

void	RaidErr::MarkArrayOffline()
{
	RaidEvent	Event;

	pRaid->Health = RAID_OFFLINE;

	// Report Event ArrayOffline
	Event.Event.RaidOffline.ArrayRowID = pRaid->RaidRowId;
	Event.Event.RaidOffline.MemberRowID = pRaid->pMember[Suspect]->MemberRowId;
	pRaid->pCmdServer->csrvReportEvent(RAID_EVT_ARRAY_OFFLINE, &Event);

	ErrorDoneOffline();
}

/*************************************************************************/
// IsReqblkInError
// Return TRUE if this is the Reqblk in the range of the Media Defect
/*************************************************************************/

BOOL	RaidErr::IsReqblkInError(Reqblk *pReq)
{
	if (pReq->Lba <= Lba)
		if ((pReq->Lba + pReq->Count) >= Lba)
		// don't compare member and suspect - not OK for Raid1
		// for Raid5 need to check member or parity with suspect
		// raid5 method to get ioreq lba from error lba needs to
		// consider error member may be parity member
			return (TRUE);
	return(FALSE);
}

/*************************************************************************/
// EndErrReqblk
// End this pReq with a media defect error status
// If this pReq is a Master, find the right pReq to end in error and
// requeue all others to be redone
/*************************************************************************/

void	RaidErr::EndErrReqblk(Reqblk *pReq)
{
	Reqblk	*pNext;

	while (pReq)
	{
		pNext = pReq->pForw;					// Next element of combined chain
		if (pReq->Flags & MASTER_BIT)
		{
			EndErrReqblk(pReq->pCombBack);	 	// Head of combined chain
												// Call recursively, might be another Master
			pRaid->FreeReqblk(pReq); 			// free Master reqblk
		}
		else
		{
			if (IsReqblkInError(pReq))
			{
				// set error in Reqblk
				pReq->ErrorMember = Suspect;
				pReq->ErrorLba = Lba;
				pReq->Status = FCP_SCSI_DSC_CHECK_CONDITION;
				pReq->SenseKey = SENSE_MEDIUM_ERROR;
				// do callback - this pReq ends in error
				( (pReq->pInst)->*(pReq->pCallback) )(pReq);
			}
			else
				pRaid->RequeueRequestblk(pReq);	// put pReq back in Queue to be redone
		}
		pReq = pNext;							// continue with next
	}
}
