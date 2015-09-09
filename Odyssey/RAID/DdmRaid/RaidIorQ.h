/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: IoreqQueue.h
//			Keeps a Queue of outstanding Ioreqs for debug and
//			to be able to tell when Quiesced
//
// Description:
//
// Update Log: 
//	5/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __IoreqQueue_h
#define __IoreqQueue_h


class IoreqQueue
{

private:

	Ioreq		*pHead;
	Ioreq		*pTail;
	U32			Count;

public:

	IoreqQueue();

	U32			GetCount();
	BOOL		IsEmpty();
	void		InsertInQueue(Ioreq *pIoreq);
	void		RemoveFromQueue(Ioreq *pIoreq);
};


/*************************************************************************/
// GetCount
// Returns number of Ioreqs in queue
/*************************************************************************/

inline U32	IoreqQueue::GetCount()
{
	return (Count);
}

/*************************************************************************/
// IsEmpty
// Returns TRUE if Queue is empty
/*************************************************************************/

inline BOOL	IoreqQueue::IsEmpty()
{
	return (pHead == NULL);
}

#endif
