/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raid0.cpp
//
// Description:	Raid0 class
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
#include "RaidErr.h"
#include "RaidUtilTable.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"
#include "Raid.h"
#include "Raid0.h"
#include "OsStatus.h"


/*************************************************************************/
// ~Raid0
// Destructor method for the class Raid0
/*************************************************************************/

Raid0::~Raid0()
{ 
	if (pCmdServer)
		pCmdServer->csrvTerminate(); 
	for (int i = 0; i < NumberMembers; i++)
	{
		if (pMember[i])
			delete pMember[i];
	}
	if (pRaidErr)
		delete pRaidErr;
}

/*************************************************************************/
// Initialize
// Initialize method for the class Raid0
/*************************************************************************/

STATUS	Raid0::Initialize(DdmServices *pDdmServices,
							RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq)
{ 
	RAID_ROW_RECORD		*pRaidRow;
	STATUS				Status;

	TRACE_ENTRY(Raid0::Initialize);

	SetParentDdm(pDdmServices);
	RaidVDN = pDdmServices->GetVdn();
	RaidRowId = pArrayDesc->thisRID;
	pArrayDescriptor = pArrayDesc;
	DataBlockSize = pArrayDesc->dataBlockSize;
	Health = pArrayDesc->health;
	NumberMembers = pArrayDesc->numberMembers;
	pQuiesceIor = NULL;

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
		ReadMemberTableRow(pRaidRow, (pTSCallback_t)&Raid0::ReadMemberTableDone);
	}
	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// ReadMemberTableDone
// Callback method from reading a member table row.
// New a member class and initilize it from this data
/*************************************************************************/

void	Raid0::ReadMemberTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status)
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
		// Initialize member class. Allocate a Request Queue in member class
		if ((Status = pMember[i]->Initialize(
									&pRaidRow->RowData.MemberData,
									this,RAIDQ_ELEVATOR)) != OS_DETAIL_STATUS_SUCCESS)
			pIoreq->Status = Status;		// set error
	}

	pIoreq->Count++;						// update count of number reads done
	if (pIoreq->Count == NumberMembers)		// done with all members
	{
		if (pIoreq->Status == OS_DETAIL_STATUS_SUCCESS)
		{
			InitCmdServer();
		}
		// do callback on pIoreq
		index = --pIoreq->iCall;
		( (pIoreq->Call[index].pInst)->*(pIoreq->Call[index].pCallback) )(pIoreq);
	}
	delete pRaidRow;
}

/*************************************************************************/
// DoRead
// Read method for the class Raid0
/*************************************************************************/

STATUS	Raid0::DoRead(Ioreq *pIoreq)
{
	Reqblk	*pReq, *pPrev, *pNext;
	U32		Block, Row, Requested;
	U32		SectorOffset;
	U8		MemberIndex;
	STATUS	Status;

	TRACE_ENTRY(Raid0::DoRead);

	pReq = new Reqblk;
	if (pReq == NULL)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}

	pReq->Status = 0;
	pReq->Flags = 0;
	pReq->Type = RAID_READ;

	// put on Ioreq's chain of Reblks
	pReq->pIoreq = pIoreq;
	pReq->pCombForw = NULL;
	if (pIoreq->pReq)
	{
		pPrev = pIoreq->pReq;
		pNext = pPrev->pCombForw;
		while (pNext)
		{
			pPrev = pNext;
			pNext = pNext->pCombForw;
		}
		pReq->pCombBack = pPrev;
		pPrev->pCombForw = pReq;
	}
	else
	{
		pIoreq->pReq = pReq;
		pReq->pCombBack = NULL;
	}

	Block = pIoreq->Lba / DataBlockSize;
	MemberIndex = Block % NumberMembers;
	SectorOffset = pIoreq->Lba % DataBlockSize;
	Row = Block / NumberMembers;

	Requested = DataBlockSize - SectorOffset;	// next drive change
	if (Requested > pIoreq->Count)
		Requested = pIoreq->Count;				// done with requested

	pReq->Lba = Row * DataBlockSize + SectorOffset;
	pReq->Count = Requested;
	pReq->Member = MemberIndex;

	pIoreq->Count -= Requested;
	pIoreq->Lba += Requested;

	pReq->pInst = this;
	pReq->pStartRoutine = (pReqStartMethod)&Raid0::NormalIO;
	pReq->pCallback = (pReqCallbackMethod)&Raid0::ReqIODone;

	if ((Status = GetReqblkSGList(pIoreq, pReq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return (Status);
	}

	if (pIoreq->Count)
		return (Raid0::DoRead(pIoreq));		// more to do

	IoreqQueue.InsertInQueue(pIoreq);

	Raid0::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoWrite
// Write method for the class Raid0
/*************************************************************************/

STATUS	Raid0::DoWrite(Ioreq *pIoreq)
{
	Reqblk	*pReq, *pPrev, *pNext;
	U32		Block, Row, Requested;
	U32		SectorOffset;
	U8		MemberIndex;
	STATUS	Status;

	TRACE_ENTRY(Raid0::DoWrite);

	pReq = new Reqblk;
	if (pReq == NULL)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}

	pReq->Status = 0;
	pReq->Flags = 0;
	pReq->Type = RAID_WRITE;
	if (pIoreq->Flags & WRITE_VERIFY_BIT)
		pReq->Flags |= WRITE_VERIFY_BIT;

	// put on Ioreq's chain of Reblks
	pReq->pIoreq = pIoreq;
	pReq->pCombForw = NULL;
	if (pIoreq->pReq)
	{
		pPrev = pIoreq->pReq;
		pNext = pPrev->pCombForw;
		while (pNext)
		{
			pPrev = pNext;
			pNext = pNext->pCombForw;
		}
		pReq->pCombBack = pPrev;
		pPrev->pCombForw = pReq;
	}
	else
	{
		pIoreq->pReq = pReq;
		pReq->pCombBack = NULL;
	}

	Block = pIoreq->Lba / DataBlockSize;
	MemberIndex = Block % NumberMembers;
	SectorOffset = pIoreq->Lba % DataBlockSize;
	Row = Block / NumberMembers;

	Requested = DataBlockSize - SectorOffset;	// next drive change
	if (Requested > pIoreq->Count)
		Requested = pIoreq->Count;				// done with requested

	pReq->Lba = Row * DataBlockSize + SectorOffset;
	pReq->Count = Requested;
	pReq->Member = MemberIndex;

	pIoreq->Count -= Requested;
	pIoreq->Lba += Requested;

	pReq->pInst = this;
	pReq->pStartRoutine = (pReqStartMethod)&Raid0::NormalIO;
	pReq->pCallback = (pReqCallbackMethod)&Raid0::ReqIODone;

	if ((Status = GetReqblkSGList(pIoreq, pReq)) != OS_DETAIL_STATUS_SUCCESS)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return (Status);
	}

	if (pIoreq->Count)
		return (Raid0::DoWrite(pIoreq));		// more to do

	IoreqQueue.InsertInQueue(pIoreq);

	Raid0::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoReassign
// Reassign method for the class Raid0
/*************************************************************************/

STATUS	Raid0::DoReassign(Ioreq *pIoreq)
{
	Reqblk	*pReq;
	U32		Block, Row;
	U32		SectorOffset;
	U8		MemberIndex;

	TRACE_ENTRY(Raid0::DoReassign);

	pReq = new Reqblk;
	if (pReq == NULL)
	{
		FreeReqblkChain(pIoreq);
		pIoreq->pReq = NULL;
		return OS_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT;
	}

	pReq->Status = 0;
	pReq->Flags = 0;
	pReq->Type = RAID_REASSIGN;

	// put on Ioreq's chain of Reblks
	pIoreq->pReq = pReq;
	pReq->pIoreq = pIoreq;
	pReq->pCombForw = NULL;
	pReq->pCombBack = NULL;

	Block = pIoreq->Lba / DataBlockSize;
	MemberIndex = Block % NumberMembers;
	SectorOffset = pIoreq->Lba % DataBlockSize;
	Row = Block / NumberMembers;

	pReq->Lba = Row * DataBlockSize + SectorOffset;
	// Count must be 1 - only handles 1 block
	pReq->Count = pIoreq->Count;
	pReq->Member = MemberIndex;
	// don't need SGL
	pReq->NumSGLs = 0;
	pReq->pInst = this;
	pReq->pStartRoutine = (pReqStartMethod)&Raid0::ReassignBlock;
	pReq->pCallback = (pReqCallbackMethod)&Raid0::ReqIODone;

	IoreqQueue.InsertInQueue(pIoreq);

	Raid0::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// StartIoreq
// Queue each Reqblk that makes up this Ioreq, trying to combine Reqblks.
// Call StartIO.
// For Raid0, all Reqblks are Queued in the Member Queues.
/*************************************************************************/

void	Raid0::StartIoreq(Ioreq *pIoreq)
{
	Reqblk *pReq, *pNext;

	TRACE_ENTRY(Raid0::StartIoreq);

	pReq = pIoreq->pReq;
	while (pReq)
	{
		pNext = pReq->pCombForw;
		pMember[pReq->Member]->pRequestQueue->SGCombine(pReq);
//	pMember[pReq->Member]->pRequestQueue->EnQueue(pReq);
		pReq = pNext;
	}
	Raid0::StartIO();
}

/*************************************************************************/
// StartIO
// Start all I/O possible. All requests are in Request Queues in each
// Member class
/*************************************************************************/

void	Raid0::StartIO()
{
	Reqblk *pReq;

	TRACE_ENTRY(Raid0::StartIO);

	if (Flags & RAID_PROCESSING_ERROR)
		return;
	for (int i = 0; i < NumberMembers; i++)
	{
		while (TRUE)
		{
			pReq = pMember[i]->pRequestQueue->DeQueue();
			if (pReq)
			{	// Call Start routine for Reqblk
				if ( ( (pReq->pInst)->*(pReq->pStartRoutine) )(pReq) == FALSE )
				{	// Could not start - put back in Queue as Current
					pMember[i]->pRequestQueue->EnQueueAsCurrent(pReq);
					break;
				}
			}
			else
				break;		// Queue empty
		}
	}
}

/*************************************************************************/
// ReqIODone
// Call Base class ReqblkDone to free Reqblks and call completion routines
// for Ioreqs that are done. Then start more I/O
/*************************************************************************/

void	Raid0::ReqIODone(Reqblk *pReq)
{
	TRACE_ENTRY(Raid0::ReqIODone);

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

BOOL	Raid0::IsRaidUsable(U32 Failmask)
{
	if (Failmask)
		return (FALSE);
	return (TRUE);
}

/*************************************************************************/
// ConvertReqLbaToIoreqLba
// Used to report media errors
/*************************************************************************/

U32		Raid0::ConvertReqLbaToIoreqLba(Reqblk *pReq)
{
	U32		Row, SectorOffset, Lba;

	Row = pReq->ErrorLba / DataBlockSize;
	SectorOffset = pReq->ErrorLba % DataBlockSize;
	Lba = Row * NumberMembers * DataBlockSize;
	Lba += (pReq->ErrorMember * DataBlockSize);
	Lba += SectorOffset;
	return (Lba);
}

/*************************************************************************/
// RequeueRequestblk
// Called from RaidErr class. Put Reqblk back in Queue to be redone
/*************************************************************************/

void	Raid0::RequeueRequestblk(Reqblk *pReq)
{
	pMember[pReq->Member]->pRequestQueue->EnQueueAsCurrent(pReq);

	StartIO();
}

/*************************************************************************/
// ClearQueuesWithError
// Called from RaidErr class - array went offline
// Empty all Queues, end all Requests in error
/*************************************************************************/

void	Raid0::ClearQueuesWithError()
{
	Reqblk 		*pReq;

	// clear all I/O Queues
	for (U8 i = 0; i < NumberMembers; i++)
	{
		while ((pReq = pMember[i]->pRequestQueue->DeQueue()) != NULL)
		{
			pReq->Status = FCP_SCSI_HBA_DSC_DEVICE_NOT_PRESENT;
			// do callback - this pReq ends in error
			( (pReq->pInst)->*(pReq->pCallback) )(pReq);
		}
	}
}

/*************************************************************************/
// NormalIO
// Send a Read or Write Message
/*************************************************************************/

BOOL	Raid0::NormalIO(Reqblk *pReq)
{ 
	Message			*pMsg = NULL;
	SGLIST			*pSGList;
	RAID0_CONTEXT	*pR0Context;
	Member			*pMem;
	int				SglDir;

	TRACE_ENTRY(Raid0::NormalIO);

	pMem = pMember[pReq->Member];

	if (pMem->NumOutstanding >= pMem->MaxOutstanding)
		return (FALSE);

	switch(pReq->Type)
	{
		case	RAID_READ:
			pMsg = new Message(BSA_BLOCK_READ, sizeof(FCP_MSG_SIZE));
			SglDir = SGL_REPLY;
			break;
		case	RAID_WRITE:
			if (pReq->Flags & WRITE_VERIFY_BIT)
				pMsg = new Message(BSA_BLOCK_WRITE_VERIFY, sizeof(FCP_MSG_SIZE));
			else
				pMsg = new Message(BSA_BLOCK_WRITE, sizeof(FCP_MSG_SIZE));
			SglDir = SGL_SEND;
			break;
	}

	if (pMsg == NULL)
		return (FALSE);

	if ((pR0Context = new RAID0_CONTEXT) == NULL)
	{
		delete pMsg;
		return (FALSE);
	}

	pR0Context->payload.ControlFlags = 0;
	pR0Context->payload.TimeMultiplier = 0;
	pR0Context->payload.FetchAhead = 0;
	pR0Context->payload.TransferByteCount = pReq->Count * 512;
	pR0Context->payload.LogicalBlockAddress = pReq->Lba;

	pMsg->AddPayload(&pR0Context->payload, sizeof(BSA_RW_PAYLOAD));

	pSGList = pReq->pSGList;
	for (int i = 0; i < pReq->NumSGLs; i++)
	{
		pMsg->AddSgl(i, (void *)pSGList->Address, pSGList->Length, SglDir);
		pSGList++;
	}

	pR0Context->pReq = pReq;

	pMem->NumOutstanding++;

	DdmServices::Send(pMem->Vd, pMsg, pR0Context, (ReplyCallback) &Raid0::NormalIOCallback);

	return (TRUE);
}

/*************************************************************************/	 
// NormalIOCallback
// Callback routine for a Normal Read or Write Message
/*************************************************************************/

void	Raid0::NormalIOCallback(MessageReply *pMsg)
{ 
	RAID0_CONTEXT	*pR0Context;
	Member			*pMem;
	Reqblk			*pReq;

	TRACE_ENTRY(Raid0::NormalIOCallback);

	pR0Context = (RAID0_CONTEXT *)pMsg->GetContext();
	pReq = pR0Context->pReq;

	pMem = pMember[pReq->Member];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid0::NormalIOCallback) == TRUE)
			return;		// Retrying message
		else
			Raid::SetReqStatus(pReq, pMsg, pReq->Member);
	}
	pMem->NumOutstanding--;
	delete pR0Context;
	delete pMsg;

	Raid::CommandComplete(pReq);
}

/*************************************************************************/
// ReassignBlock
// Send a Reassign Block Message
/*************************************************************************/

BOOL	Raid0::ReassignBlock(Reqblk *pReq)
{ 
	Message			*pMsg = NULL;
	RAID0_CONTEXT	*pR0Context;
	Member			*pMem;

	TRACE_ENTRY(Raid0::ReassignBlock);

	pMem = pMember[pReq->Member];

	if (pMem->NumOutstanding >= pMem->MaxOutstanding)
		return (FALSE);

	pMsg = new Message(BSA_BLOCK_REASSIGN, sizeof(FCP_MSG_SIZE));

	if (pMsg == NULL)
		return (FALSE);

	if ((pR0Context = new RAID0_CONTEXT) == NULL)
	{
		delete pMsg;
		return (FALSE);
	}

	pR0Context->payload.ControlFlags = 0;
	pR0Context->payload.TimeMultiplier = 0;
	pR0Context->payload.FetchAhead = 0;
	pR0Context->payload.TransferByteCount = pReq->Count * 512;
	pR0Context->payload.LogicalBlockAddress = pReq->Lba;

	pMsg->AddPayload(&pR0Context->payload, sizeof(BSA_RW_PAYLOAD));

	pR0Context->pReq = pReq;

	pMem->NumOutstanding++;

	DdmServices::Send(pMem->Vd, pMsg, pR0Context, (ReplyCallback) &Raid0::ReassignBlockCallback);

	return (TRUE);
}

/*************************************************************************/	 
// ReassignBlockCallback
// Callback routine for a Reassign Block Message
/*************************************************************************/

void	Raid0::ReassignBlockCallback(MessageReply *pMsg)
{ 
	RAID0_CONTEXT	*pR0Context;
	Member			*pMem;
	Reqblk			*pReq;

	TRACE_ENTRY(Raid0::ReassignBlockCallback);

	pR0Context = (RAID0_CONTEXT *)pMsg->GetContext();
	pReq = pR0Context->pReq;

	pMem = pMember[pReq->Member];

	if (pMsg->DetailedStatusCode != FCP_SCSI_DSC_SUCCESS)
	{
		if (RetryCommand(pMem,pMsg,(ReplyCallback) &Raid0::NormalIOCallback) == TRUE)
			return;		// Retrying message
		else
			Raid::SetReqStatus(pReq, pMsg, pReq->Member);
	}
	pMem->NumOutstanding--;
	delete pR0Context;
	delete pMsg;

	// don't call CommandComplete - don't want to go to 
	// Error processing
	// do callback
	( (pReq->pInst)->*(pReq->pCallback) )(pReq);
}

/*************************************************************************/	 
// RetryCommand
// Get a new message to retry command with, fill it in, and send it.
/*************************************************************************/

BOOL	Raid0::RetryCommand(Member *pMem, MessageReply *pMsg, ReplyCallback rc)
{ 
	RAID0_CONTEXT	*pR0Context;

	TRACE_ENTRY(Raid0::RetryCommand);

	pR0Context = (RAID0_CONTEXT *)pMsg->GetContext();
	if (pR0Context->RetryCount < pMem->MaxRetryCount)
	{	
		pR0Context->RetryCount++;
		// fill payload again
		pMsg->AddPayload(&pR0Context->payload, sizeof(BSA_RW_PAYLOAD));
		// send the message
		DdmServices::Send(pMem->Vd, pMsg, pR0Context, rc);
		return (TRUE);	// message sent
	}
	return (FALSE);		// exhausted retries
}

