/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: UtilCmd.cpp
//
// Description:
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
#include "UtilCmd.h"

/*************************************************************************/
// UtilCmd
// Constructor method for the class UtilCmd
/*************************************************************************/

UtilCmd::UtilCmd()
{
	pHead = pTail = NULL;
	Count = 0;
}

/*************************************************************************/
// InsertInQueue
// Insert Utility at Tail of Queue
/*************************************************************************/

void	UtilCmd::InsertInQueue(Utility *pUtility)
{
	pUtility->pForw = NULL;
	pUtility->pBack = pTail;
	if (pTail)
		pTail->pForw = pUtility;
	else
		pHead = pUtility;
	pTail = pUtility;
	Count++;
}

/*************************************************************************/
// RemoveFromQueue
// Remove Utility from Queue
/*************************************************************************/

void	UtilCmd::RemoveFromQueue(Utility *pUtility)
{
	Utility *pPrev, *pNext;

	pPrev = pUtility->pBack;
	pNext = pUtility->pForw;

	if (pPrev)
		pPrev->pForw = pNext;
	else
		pHead = pNext;

	if (pNext)
		pNext->pBack = pPrev;
	else
		pTail = pPrev;

	pUtility->pForw = pUtility->pBack = NULL;
	Count--;
}

/*************************************************************************/
// FindUtilityInQueue
// Return Utility with specified Handle
/*************************************************************************/

Utility	*UtilCmd::FindUtilityInQueue(rowID Handle)
{
	Utility *pUtility;

	pUtility = pHead;
	while (pUtility)
	{
		if ((pUtility->Handle.LoPart == Handle.LoPart) &&
			(pUtility->Handle.HiPart == Handle.HiPart))
		{
			return (pUtility);
		}
		pUtility = pUtility->pForw;
	}
	return (NULL);	 // not in Queue
}

/*************************************************************************/
// SuspendUtilities
// Flag all utilities to be suspended
// Used when Quiescing
/*************************************************************************/

void	UtilCmd::SuspendUtilities()
{
	Utility *pUtility;

	pUtility = pHead;
	while (pUtility)
	{
		pUtility->Flags |= RAID_SUSPEND_UTILITY;
		pUtility = pUtility->pForw;
	}
	return;
}

