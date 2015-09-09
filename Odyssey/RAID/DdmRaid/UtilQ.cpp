/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Utilq.cpp
//
// Description:
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#include "Ddm.h"
#include "OsTypes.h"
#include "Message.h"
#include "CtTypes.h"
#include "RaidStructs.h"
#include "UtilCmd.h"
#include "RaidIorQ.h"
#include "Raidq.h"
#include "Member.h"
#include "RaidUtilTable.h"
#include "Raid.h"
#include "Utilq.h"


/*************************************************************************/
// UpdateUtilPriority
// Update the priority of all queued utilities
/*************************************************************************/

void	UtilQueue::UpdateUtilPriority()
{
	UtilReqblk	*pUtilReq, *pNext;

	pUtilReq = (UtilReqblk *)pHead;
	while (pUtilReq)
	{
		pNext = (UtilReqblk *)pUtilReq->pForw;
		if (pUtilReq->Priority < MAX_UTIL_PRIORITY)
			pUtilReq->Priority++;	// update priority
		pUtilReq = pNext;
	}
}

/*************************************************************************/
// CheckUtilPriorityToRun
// Check for a utility that has reached max priority and try to start it
/*************************************************************************/

void	UtilQueue::CheckUtilPriorityToRun()
{
	UtilReqblk	*pUtilReq, *pNext;

	// try to start the first utility
	// that has reached max priority
	pUtilReq = (UtilReqblk *)pHead;
	while (pUtilReq)
	{
		pNext = (UtilReqblk *)pUtilReq->pForw;
		if (pUtilReq->Priority == MAX_UTIL_PRIORITY)
		{
			UtilQueue::StartUtility(pUtilReq);
			break;
		}
		pUtilReq = pNext;
	}
}

/*************************************************************************/
// CheckUtilToRun
// Start as many utilities as can run
/*************************************************************************/

void	UtilQueue::CheckUtilToRun(Raid *pRaid)
{
	UtilReqblk	*pUtilReq, *pPrev, *pUtilReqToRun;
	U32			NeededMask, WaitMask;
	U8			HighestPriority;

	while (TRUE)
	{
		pUtilReqToRun = NULL;
		HighestPriority = 0;   
		pUtilReq = (UtilReqblk *)pTail;
		while (pUtilReq)
		{
			pPrev = (UtilReqblk *)pUtilReq->pBack;
			NeededMask = (pUtilReq->SourceMask | pUtilReq->DestMask);
			WaitMask = pRaid->GetWaitMask();
			if ((WaitMask & NeededMask) == 0)
			{	// look for highest priority utility
				// that should be able to start
				if (pUtilReq->Priority >= HighestPriority)
				{
					HighestPriority = pUtilReq->Priority;
					pUtilReqToRun = pUtilReq;
				}
			}
			pUtilReq = pPrev;
		}
		if (pUtilReqToRun)
		{
			if (!UtilQueue::StartUtility(pUtilReqToRun))
				break;
		}
		else
			break;		// nothing that can start
	}
}



/*************************************************************************/
// StartUtility
// Try to start utility
/*************************************************************************/

BOOL	UtilQueue::StartUtility(UtilReqblk *pUtilReq)
{
	// take out of Queue
	RemoveFromQueue(pUtilReq);
	// Call Start routine for UtilReq
	if ( ( (pUtilReq->pInst)->*(pUtilReq->pStartRoutine) )(pUtilReq) == FALSE )
	{	// Could not start - put back in Queue
		EnQueue(pUtilReq);
		return (FALSE);		// didn't start
	}
	return (TRUE);			// started
}



