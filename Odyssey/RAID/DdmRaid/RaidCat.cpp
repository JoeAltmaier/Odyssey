/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidCat.cpp
//
// Description:	Raid Concatenation class
//
//
// Update Log: 
//	8/99	Jim Taylor:	initial creation
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
#include "RaidCat.h"
#include "OsStatus.h"


/*************************************************************************/
// DoRead
// Read method for the class Raid Concatenation
/*************************************************************************/

STATUS	RaidCat::DoRead(Ioreq *pIoreq)
{
	Reqblk	*pReq, *pPrev, *pNext;
	U32		Block, Requested;
	STATUS	Status;

	TRACE_ENTRY(RaidCat::DoRead);

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

	Block = pIoreq->Lba;

	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (Block < pMember[i]->EndLBA)
		{
			pReq->Member = i;
			pReq->Lba = Block - pMember[i]->StartLBA;
			Requested = pMember[i]->EndLBA - Block;		// next member change 
			break;
		}
	}
	if (Requested > pIoreq->Count)
		Requested = pIoreq->Count;						// done with requested

	pReq->Count = Requested;

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
		return (RaidCat::DoRead(pIoreq));				// more to do

	IoreqQueue.InsertInQueue(pIoreq);

	Raid0::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoWrite
// Write method for the class Raid Concatenation
/*************************************************************************/

STATUS	RaidCat::DoWrite(Ioreq *pIoreq)
{
	Reqblk	*pReq, *pPrev, *pNext;
	U32		Block, Requested;
	STATUS	Status;

	TRACE_ENTRY(RaidCat::DoWrite);

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

	Block = pIoreq->Lba;

	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (Block < pMember[i]->EndLBA)
		{
			pReq->Member = i;
			pReq->Lba = Block - pMember[i]->StartLBA;
			Requested = pMember[i]->EndLBA - Block;		// next member change 
			break;
		}
	}
	if (Requested > pIoreq->Count)
		Requested = pIoreq->Count;						// done with requested

	pReq->Count = Requested;

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
		return (RaidCat::DoWrite(pIoreq));				// more to do

	IoreqQueue.InsertInQueue(pIoreq);

	Raid0::StartIoreq(pIoreq);

	return OS_DETAIL_STATUS_SUCCESS;
}

/*************************************************************************/
// DoReassign
// Reassign method for the class Raid Concatenation
/*************************************************************************/

STATUS	RaidCat::DoReassign(Ioreq *pIoreq)
{
	Reqblk	*pReq;
	U32		Block;


	TRACE_ENTRY(RaidCat::DoReassign);

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

	Block = pIoreq->Lba;
	for (U8 i = 0; i < NumberMembers; i++)
	{
		if (Block < pMember[i]->EndLBA)
		{
			pReq->Member = i;
			pReq->Lba = Block - pMember[i]->StartLBA;
			break;
		}
	}
	// Count must be 1 - only handles 1 block
	pReq->Count = pIoreq->Count;
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
// ConvertReqLbaToIoreqLba
// Used to report media errors
/*************************************************************************/

U32		RaidCat::ConvertReqLbaToIoreqLba(Reqblk *pReq)
{
	Member	*pMem;

	pMem = pMember[pReq->Member];
	return (pReq->ErrorLba + pMem->StartLBA);
}

