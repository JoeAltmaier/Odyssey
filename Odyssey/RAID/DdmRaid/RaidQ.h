/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raidq.h
//
// Description:	Header file for the queueing class for Reqblks
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __Raidq_h
#define __Raidq_h

// defines for QueueMethod
#define	RAIDQ_FIFO		0
#define	RAIDQ_ELEVATOR	1

// defines for Direction
#define	RAIDQ_FORWARD	0
#define	RAIDQ_BACK		1

class ReqQueue
{

protected:

	Reqblk		*pHead;
	Reqblk		*pTail;
	Reqblk		*pCurrent;
	Raid		*pRaid;
	U32			Count;
	U32			MaxCount;
	U8			Direction;
	U8			QueueMethod;

public:

	ReqQueue(Raid *pRaid, U8 QMethod);

	BOOL	IsEmpty();
	U32		GetCount();
	void	SetQueueMethod(U8 QMethod);
	U8		GetQueueMethod();
	void	EnQueue(Reqblk *pReq);
	void	EnQueueAsCurrent(Reqblk *pReq);
	Reqblk	*DeQueue();
	void	SGCombine(Reqblk *pReq);
	void	StripeCombine(Reqblk *pReq);
	BOOL	IsInQueue(Reqblk *pReq);

protected:

	void	RemoveFromQueue(Reqblk *pReq);

private:

	void	EnQueueAtHead(Reqblk *pReq);
	void	EnQueueAtTail(Reqblk *pReq);
	void	QueueBack(Reqblk *pReq);
	void	QueueForw(Reqblk *pReq);
	Reqblk	*AddToMasterSGReq(Reqblk *pMaster, Reqblk *pReq);
	Reqblk	*CreateMasterSGReq(Reqblk *pReq1, Reqblk *pReq2);
	Reqblk	*CombineMasterSGReqs(Reqblk *pReq1, Reqblk *pReq2);
	Reqblk	*SGCombineBack(Reqblk *pReq);
	Reqblk	*SGCombineForw(Reqblk *pReq);
	Reqblk	*CombineContiguous(Reqblk *pReq1, Reqblk *pReq2);
	Reqblk	*CombineOverwrites(Reqblk *pReq1, Reqblk *pReq2);
	Reqblk	*CombineMasterOverwrites(Reqblk *pReq1, Reqblk *pReq2);
	Reqblk	*AddToMasterOverwrites(Reqblk *pReq1, Reqblk *pReq2);
	Reqblk	*CreateMasterOverwrites(Reqblk *pReq1, Reqblk *pReq2);
};

/*************************************************************************/
// IsEmpty
// Returns TRUE if queue is empty
/*************************************************************************/

inline BOOL	ReqQueue::IsEmpty()
{
	return (pHead == NULL);
}

/*************************************************************************/
// GetCount
// Returns number of Reqblks in queue
/*************************************************************************/

inline U32	ReqQueue::GetCount()
{
	return (Count);
}

/*************************************************************************/
// SetQueueMethod
// Sets queueing method (FIFO, Elevator)
/*************************************************************************/

inline void	ReqQueue::SetQueueMethod(U8 QMethod)
{
	QueueMethod = QMethod;
}

/*************************************************************************/
// GetQueueMethod
// Gets queueing method (FIFO, Elevator)
/*************************************************************************/

inline U8	ReqQueue::GetQueueMethod()
{
	return (QueueMethod);
}

#endif
